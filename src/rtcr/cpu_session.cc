/*
 * \brief  Intercepting Cpu session
 * \author Denis Huber
 * \date   2016-08-10
 */

#include <rtcr/cpu/cpu_session.h>

using namespace Rtcr;


Cpu_session_component* Cpu_session_component::current_session = nullptr;


Cpu_thread_component &Cpu_session_component::_create_thread(Genode::Pd_session_capability child_pd_cap,
							    Genode::Pd_session_capability parent_pd_cap,
							    Genode::Cpu_session::Name const &name,
							    Genode::Affinity::Location affinity,
							    Genode::Cpu_session::Weight weight,
							    Genode::addr_t utcb)
{
	/* Create real CPU thread from parent */
	auto cpu_thread_cap = _parent_cpu.create_thread(parent_pd_cap, name, _child_affinity, weight, utcb);

	/* Create custom CPU thread */
	Cpu_thread_component *new_cpu_thread =
		new (_md_alloc) Cpu_thread_component(_md_alloc,
						     cpu_thread_cap,
						     child_pd_cap,
						     name.string(),
						     weight,
						     utcb,
						     _child_affinity,
						     _bootstrap_phase);

	/* Manage custom CPU thread */
	_ep.manage(*new_cpu_thread);

	/* Insert custom CPU thread into list */
	Genode::Lock::Guard _lock_guard(_new_cpu_threads_lock);
	_new_cpu_threads.insert(new_cpu_thread);

	return *new_cpu_thread;
}


void Cpu_session_component::_kill_thread(Cpu_thread_component &cpu_thread)
{
	auto parent_cap = cpu_thread.parent_cap();

	/* Remove custom CPU thread form list */
	Genode::Lock::Guard lock(_destroyed_cpu_threads_lock);
	_destroyed_cpu_threads.insert(&cpu_thread);

	/* Dissolve custom CPU thread */
	_ep.dissolve(cpu_thread);

	/* Destroy real CPU thread from parent */
	_parent_cpu.kill_thread(parent_cap);
}


Cpu_session_component::Cpu_session_component(Genode::Env &env,
					     Genode::Allocator &md_alloc,
					     Genode::Entrypoint &ep,
					     Pd_root &pd_root,
					     const char *label,
					     const char *creation_args,
					     bool &bootstrap_phase,
					     Genode::Xml_node *config)

	:
  Checkpointable(env, config, "cpu_session"),
	_env             (env),
	_md_alloc        (md_alloc),
	_ep              (ep),
	_bootstrap_phase (bootstrap_phase),
	_pd_root         (pd_root),
	_parent_cpu      (env, label),
	_child_affinity (_read_child_affinity(config, label))
{
	if(verbose_debug) Genode::log("\033[33m", "Cpu", "\033[0m(parent ", _parent_cpu,")");
	
	current_session = this;
}


Cpu_session_component::~Cpu_session_component()
{
	while(Cpu_thread_component *cpu_thread = ck_cpu_threads.first()) {
		_kill_thread(*cpu_thread);
	}

	// TODO FJO: delete _destroyed* & _new*
	if(verbose_debug) Genode::log("\033[33m", "~Cpu", "\033[0m ", _parent_cpu);
}


Genode::Affinity::Location Cpu_session_component::_read_child_affinity(Genode::Xml_node *config, const char* child_name)
{
	try {	
	  Genode::Xml_node affinity_node = config->sub_node("child");
	  // TODO FJO: iterate through all child nodes and find node with name="child_name"
		long const xpos = affinity_node.attribute_value<long>("xpos", 0);
		long const ypos = affinity_node.attribute_value<long>("ypos", 0);
		return Genode::Affinity::Location(xpos, ypos, 1 ,1);
	}
	catch (...) { return Genode::Affinity::Location(0, 0, 1, 1);}
}



void Cpu_session_component::checkpoint()
{
	ck_badge = cap().local_name();
  ck_bootstrapped = _bootstrapped;
  ck_upgrade_args = _upgrade_args;

  // TODO
  //  ck_kcap = _core_module->find_kcap_by_badge(ck_badge);

  ck_sigh_badge = _sigh.local_name();
  
  Cpu_thread_component *cpu_thread = nullptr;
  while(cpu_thread = _new_cpu_threads.first()) {
    ck_cpu_threads.insert(cpu_thread);
    _new_cpu_threads.remove(cpu_thread);
  }

  while(cpu_thread = _destroyed_cpu_threads.first()) {
    ck_cpu_threads.remove(cpu_thread);
    _destroyed_cpu_threads.remove(cpu_thread);
    Genode::destroy(_md_alloc, &cpu_thread);
  }

  cpu_thread = ck_cpu_threads.first();
  while(cpu_thread) {
    cpu_thread->checkpoint();
    cpu_thread = cpu_thread->next();
  }  
}

void Cpu_session_component::pause()
{
  Cpu_thread_component *cpu_thread = nullptr;
  while(cpu_thread = _new_cpu_threads.first()) {
    cpu_thread->silent_pause();
    cpu_thread = cpu_thread->next();    
  }

  cpu_thread = ck_cpu_threads.first();
  while(cpu_thread) {
    cpu_thread->silent_pause();    
    cpu_thread = cpu_thread->next();
  }    
}

void Cpu_session_component::resume()
{
  Cpu_thread_component *cpu_thread = nullptr;
  while(cpu_thread = _new_cpu_threads.first()) {
    cpu_thread->silent_resume();
    cpu_thread = cpu_thread->next();    
  }

  cpu_thread = ck_cpu_threads.first();
  while(cpu_thread) {
    cpu_thread->silent_resume();    
    cpu_thread = cpu_thread->next();
  }    
}


Cpu_session_component *Cpu_session_component::find_by_badge(Genode::uint16_t badge)
{
	if(badge == cap().local_name())
		return this;
	
	Cpu_session_component *obj = next();
	return obj ? obj->find_by_badge(badge) : 0;
}


Genode::Thread_capability Cpu_session_component::create_thread(Genode::Pd_session_capability child_pd_cap,
							       Genode::Cpu_session::Name const &name,
							       Genode::Affinity::Location affinity,
							       Genode::Cpu_session::Weight weight,
							       Genode::addr_t utcb)
{
	if(verbose_debug) Genode::log("Cpu::\033[33m", __func__, "\033[0m(name=", name.string(), ")");

	// Find corresponding parent PD session cap for the given custom PD session cap
	Pd_session_component *pd_session = _pd_root.session_infos().first();
	if(pd_session) pd_session = pd_session->find_by_badge(child_pd_cap.local_name());
	if(!pd_session) {
		Genode::error("Thread creation failed: PD session ", child_pd_cap, " is unknown.");
		throw Genode::Exception();
	}
	
	// Create custom CPU thread
	Cpu_thread_component &new_cpu_thread = _create_thread(child_pd_cap,
							      pd_session->parent_cap(),
							      name,
							      _child_affinity,
							      weight,
							      utcb);

	if(verbose_debug)
		Genode::log("  Created custom CPU thread ", new_cpu_thread.cap());

	return new_cpu_thread.cap();
}


void Cpu_session_component::kill_thread(Genode::Thread_capability thread_cap)
{
	if(verbose_debug) Genode::log("Cpu::\033[33m", __func__, "\033[0m(", thread_cap,")");

	// Find CPU thread for the given capability
	Genode::Lock::Guard lock (_new_cpu_threads_lock);
	Cpu_thread_component *cpu_thread = _new_cpu_threads.first();
	if(cpu_thread) cpu_thread = cpu_thread->find_by_badge(thread_cap.local_name());
	if(!cpu_thread) {
	  cpu_thread = ck_cpu_threads.first();
	  if(cpu_thread) cpu_thread = cpu_thread->find_by_badge(thread_cap.local_name());	  	}
	// If found, delete everything concerning this RPC object
	if(cpu_thread) {
		if(verbose_debug) Genode::log("  deleting ", cpu_thread->cap());

		Genode::error("Issuing Rm_session::destroy, which is bugged and hangs up.");

		_kill_thread(*cpu_thread);
	} else {
		Genode::error("No Region map with ", thread_cap, " found!");
	}
}


void Cpu_session_component::exception_sigh(Genode::Signal_context_capability handler)
{
	if(verbose_debug) Genode::log("Cpu::\033[33m", __func__, "\033[0m(", handler, ")");

	_sigh = handler;
	_parent_cpu.exception_sigh(handler);
}


Genode::Affinity::Space Cpu_session_component::affinity_space() const
{
	if(verbose_debug) Genode::log("Cpu::\033[33m", __func__, "\033[0m()");

	auto result = _parent_cpu.affinity_space();
	return result;
}


Genode::Dataspace_capability Cpu_session_component::trace_control()
{
	if(verbose_debug) Genode::log("Cpu::\033[33m", __func__, "\033[0m()");

	auto result = _parent_cpu.trace_control();

	if(verbose_debug) Genode::log("  result: ", result);

	return result;
}


Genode::Cpu_session::Quota Cpu_session_component::quota()
{
	if(verbose_debug) Genode::log("Cpu::\033[33m", __func__, "\033[0m()");

	auto result = _parent_cpu.quota();

	if(verbose_debug) Genode::log("  result: super_period_us=", result.super_period_us, ", us=", result.us);

	return result;
}


int Cpu_session_component::ref_account(Genode::Cpu_session_capability c)
{
	if(verbose_debug) Genode::log("Cpu::\033[33m", __func__, "\033[0m(", c, ")");

	auto result = _parent_cpu.ref_account(c);

	if(verbose_debug) Genode::log("  result: ", result);

	return result;
}


int Cpu_session_component::transfer_quota(Genode::Cpu_session_capability c, Genode::size_t q)
{
	if(verbose_debug) Genode::log("Cpu::\033[33m", __func__, "\033[0m(to ", c, "quota=", q, ")");

	auto result = _parent_cpu.transfer_quota(c, q);

	if(verbose_debug) Genode::log("  result: ", result);

	return result;
}


Genode::Capability<Genode::Cpu_session::Native_cpu> Cpu_session_component::native_cpu()
{
	if(verbose_debug) Genode::log("Cpu::\033[33m", __func__, "\033[0m()");

	auto result = _parent_cpu.native_cpu();

	if(verbose_debug) Genode::log("  result: ", result);

	return result;
}


int Cpu_session_component::set_sched_type(unsigned core, unsigned sched_type)
{
	// TODO verbose_debug

	auto result = _parent_cpu.set_sched_type(core, sched_type);

	if(verbose_debug) Genode::log("  result: ", result);

	return result;
}


int Cpu_session_component::get_sched_type(unsigned core)
{
	// TODO verbose_debug

	auto result = _parent_cpu.get_sched_type(core);

	if(verbose_debug) Genode::log("  result: ", result);

	return result;
}


void Cpu_session_component::set(Genode::Ram_session_capability ram_cap)
{
	// TODO verbose_debug
	_parent_cpu.set(ram_cap);
}


void Cpu_session_component::deploy_queue(Genode::Dataspace_capability ds)
{
	// TODO verbose_debug
	_parent_cpu.deploy_queue(ds);
}


void Cpu_session_component::rq(Genode::Dataspace_capability ds)
{
	// TODO verbose_debug
	_parent_cpu.rq(ds);
}


void Cpu_session_component::dead(Genode::Dataspace_capability ds)
{
	// TODO verbose_debug
	_parent_cpu.dead(ds);
}


void Cpu_session_component::killed()
{
	// TODO verbose_debug
	_parent_cpu.killed();
}


Cpu_session_component *Cpu_root::_create_session(const char *args)
{
	if(verbose_debug) Genode::log("Rm_root::\033[33m", __func__, "\033[0m(", args,")");

	/* Extracting label from args */
	char label_buf[128];
	Genode::Arg label_arg = Genode::Arg_string::find_arg(args, "label");
	label_arg.string(label_buf, sizeof(label_buf), "");

	/* Revert ram_quota calculation, because the monitor needs the original
	 * session creation argument */
	char ram_quota_buf[32];
	char readjusted_args[160];
	Genode::strncpy(readjusted_args, args, sizeof(readjusted_args));

	Genode::size_t readjusted_ram_quota = Genode::Arg_string::find_arg(readjusted_args, "ram_quota").ulong_value(0);
	readjusted_ram_quota = readjusted_ram_quota + sizeof(Cpu_session_component) + md_alloc()->overhead(sizeof(Cpu_session_component));

	Genode::snprintf(ram_quota_buf, sizeof(ram_quota_buf), "%zu", readjusted_ram_quota);
	Genode::Arg_string::set_arg(readjusted_args, sizeof(readjusted_args), "ram_quota", ram_quota_buf);
	
	/* Create custom Rm_session */
	Cpu_session_component *new_session =
	  new (md_alloc()) Cpu_session_component(_env,
						 _md_alloc,
						 _ep,
						 _pd_root,
						 label_buf,
						 readjusted_args,
						 _bootstrap_phase,
						 _config);

	Genode::Lock::Guard lock(_objs_lock);
	_session_rpc_objs.insert(new_session);

	return new_session;
}


void Cpu_root::_upgrade_session(Cpu_session_component *session, const char *upgrade_args)
{
	if(verbose_debug) Genode::log("Cpu_root::\033[33m", __func__, "\033[0m(session ", session->cap(),", args=", upgrade_args,")");

	char ram_quota_buf[32];
	char new_upgrade_args[160];

//	Genode::strncpy(new_upgrade_args, session->parent_state().upgrade_args.string(), sizeof(new_upgrade_args));

	Genode::size_t ram_quota = Genode::Arg_string::find_arg(new_upgrade_args, "ram_quota").ulong_value(0);
	Genode::size_t extra_ram_quota = Genode::Arg_string::find_arg(upgrade_args, "ram_quota").ulong_value(0);
	ram_quota += extra_ram_quota;

	Genode::snprintf(ram_quota_buf, sizeof(ram_quota_buf), "%zu", ram_quota);
	Genode::Arg_string::set_arg(new_upgrade_args, sizeof(new_upgrade_args), "ram_quota", ram_quota_buf);

	// TODO FJO
	//	session->parent_state().upgrade_args = new_upgrade_args;

	_env.parent().upgrade(session->parent_cap(), upgrade_args);
}


void Cpu_root::_destroy_session(Cpu_session_component *session)
{
	if(verbose_debug) Genode::log("Cpu_root::\033[33m", __func__, "\033[0m(session ", session->cap(),")");

	_session_rpc_objs.remove(session);
	Genode::destroy(_md_alloc, session);
}


Cpu_root::Cpu_root(Genode::Env &env,
		   Genode::Allocator &md_alloc,
		   Genode::Entrypoint &session_ep,
		   Pd_root &pd_root,
		   bool &bootstrap_phase,
		   Genode::Xml_node *config)
	:
	Root_component<Cpu_session_component>(session_ep, md_alloc),
	_env              (env),
	_md_alloc         (md_alloc),
	_ep               (session_ep),
	_bootstrap_phase  (bootstrap_phase),
	_pd_root          (pd_root),
	_objs_lock        (),
	_session_rpc_objs (),
	_config(config)
{
	if(verbose_debug) Genode::log("\033[33m", __func__, "\033[0m");
}


Cpu_root::~Cpu_root()
{
	while(Cpu_session_component *obj = _session_rpc_objs.first()) {
		_session_rpc_objs.remove(obj);
		Genode::destroy(_md_alloc, obj);
	}

	if(verbose_debug) Genode::log("\033[33m", __func__, "\033[0m");
}
