/*
 * \brief  Intercepting Cpu session
 * \author Denis Huber
 * \author Johannes Fischer
 * \date   2019-08-29
 */

#include <rtcr/cpu/cpu_session.h>

#ifdef PROFILE
#include <util/profiler.h>
#define PROFILE_THIS_CALL PROFILE_FUNCTION("yellow");
#else
#define PROFILE_THIS_CALL
#endif

#if DEBUG 
#define DEBUG_THIS_CALL Genode::log("\e[38;5;226m", __PRETTY_FUNCTION__, "\033[0m");
#else
#define DEBUG_THIS_CALL
#endif

using namespace Rtcr;

Cpu_thread &Cpu_session::_create_thread(Genode::Pd_session_capability child_pd_cap,
										Genode::Pd_session_capability parent_pd_cap,
										Genode::Cpu_session::Name const &name,
										Genode::Affinity::Location affinity,
										Genode::Cpu_session::Weight weight,
										Genode::addr_t utcb)
{
	/* Create real CPU thread from parent */
	auto cpu_thread_cap = _parent_cpu.create_thread(parent_pd_cap,
													name,
													affinity, // TODO FJO: _child_affinity ?
													weight,
													utcb);

	/* Create custom CPU thread */
	Cpu_thread *new_cpu_thread =
		new (_md_alloc) Cpu_thread(_md_alloc,
								   cpu_thread_cap,
								   child_pd_cap,
								   name.string(),
								   weight,
								   utcb,
								   affinity,
								   _bootstrap_phase);

	/* Manage custom CPU thread */
	_ep.manage(*new_cpu_thread);

	/* Insert custom CPU thread into list */
	Genode::Lock::Guard _lock_guard(_new_cpu_threads_lock);
	_new_cpu_threads.insert(new_cpu_thread);

	return *new_cpu_thread;
}


void Cpu_session::_kill_thread(Cpu_thread &cpu_thread)
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


Cpu_session::Cpu_session(Genode::Env &env,
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
	DEBUG_THIS_CALL
}


Cpu_session::~Cpu_session()
{
	while(Cpu_thread *cpu_thread = ck_cpu_threads.first()) {
		_kill_thread(*cpu_thread);
	}
}


Genode::Affinity::Location Cpu_session::_read_child_affinity(Genode::Xml_node *config, const char* child_name)
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



void Cpu_session::checkpoint()
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL
	ck_badge = cap().local_name();
	ck_bootstrapped = _bootstrapped;
	ck_upgrade_args = _upgrade_args;

	// TODO
	//  ck_kcap = _core_module->find_kcap_by_badge(ck_badge);

	ck_sigh_badge = _sigh.local_name();
  
	Cpu_thread *cpu_thread = nullptr;
	while(cpu_thread = _new_cpu_threads.first()) {
		_new_cpu_threads.remove(cpu_thread);		
		ck_cpu_threads.insert(cpu_thread);
	}

	while(cpu_thread = _destroyed_cpu_threads.first()) {
		_destroyed_cpu_threads.remove(cpu_thread);
		ck_cpu_threads.remove(cpu_thread);
		Genode::destroy(_md_alloc, &cpu_thread);
	}

	cpu_thread = ck_cpu_threads.first();
	while(cpu_thread) {
		cpu_thread->checkpoint();
		cpu_thread = cpu_thread->next();
	}  
}

void Cpu_session::pause()
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL
	Cpu_thread *cpu_thread = _new_cpu_threads.first();
	while(cpu_thread) {
		cpu_thread->silent_pause();
		cpu_thread = cpu_thread->next();
	}

	cpu_thread = ck_cpu_threads.first();
	while(cpu_thread) {
		cpu_thread->silent_pause();    
		cpu_thread = cpu_thread->next();
	}    
}

void Cpu_session::resume()
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL

	Cpu_thread *cpu_thread = ck_cpu_threads.first();
	while(cpu_thread) {
		cpu_thread->silent_resume();
		cpu_thread = cpu_thread->next();
	}

	cpu_thread = _new_cpu_threads.first();
	while(cpu_thread) {
		cpu_thread->silent_resume();
		cpu_thread = cpu_thread->next();
	}
}


Cpu_session *Cpu_session::find_by_badge(Genode::uint16_t badge)
{
	if(badge == cap().local_name())
		return this;
	
	Cpu_session *obj = next();
	return obj ? obj->find_by_badge(badge) : 0;
}


Genode::Thread_capability Cpu_session::create_thread(Genode::Pd_session_capability child_pd_cap,
													 Genode::Cpu_session::Name const &name,
													 Genode::Affinity::Location affinity,
													 Genode::Cpu_session::Weight weight,
													 Genode::addr_t utcb)
{
	/* Find corresponding parent PD session cap for the given custom PD session
	 * cap */
	Pd_session *pd_session = _pd_root.session_infos().first();
	if(pd_session) pd_session = pd_session->find_by_badge(child_pd_cap.local_name());
	if(!pd_session) {
		Genode::error("Thread creation failed: PD session ",
					  child_pd_cap, " is unknown.");
		throw Genode::Exception();
	}
	
	/* Create custom CPU thread */
	Cpu_thread &new_cpu_thread = _create_thread(child_pd_cap,
												pd_session->parent_cap(),
												name,
												affinity,
												weight,
												utcb);

	return new_cpu_thread.cap();
}


void Cpu_session::kill_thread(Genode::Thread_capability thread_cap)
{
	/*  Find CPU thread for the given capability */
	Genode::Lock::Guard lock (_new_cpu_threads_lock);
	Cpu_thread *cpu_thread = _new_cpu_threads.first();
	if(cpu_thread) cpu_thread = cpu_thread->find_by_badge(thread_cap.local_name());
	if(!cpu_thread) {
		cpu_thread = ck_cpu_threads.first();
		if(cpu_thread) cpu_thread = cpu_thread->find_by_badge(thread_cap.local_name());	  	}
	/* If found, delete everything concerning this RPC object */
	if(cpu_thread) {
		Genode::error("Issuing Rm_session::destroy, which is bugged and hangs up.");

		_kill_thread(*cpu_thread);
	} else {
		Genode::error("No Region map with ", thread_cap, " found!");
	}
}


void Cpu_session::exception_sigh(Genode::Signal_context_capability handler)
{
	_sigh = handler;
	_parent_cpu.exception_sigh(handler);
}


Genode::Affinity::Space Cpu_session::affinity_space() const
{
	return _parent_cpu.affinity_space();
}


Genode::Dataspace_capability Cpu_session::trace_control()
{
	return _parent_cpu.trace_control();
}


Genode::Cpu_session::Quota Cpu_session::quota()
{
	return _parent_cpu.quota();
}


int Cpu_session::ref_account(Genode::Cpu_session_capability c)
{
	return _parent_cpu.ref_account(c);
}


int Cpu_session::transfer_quota(Genode::Cpu_session_capability c, Genode::size_t q)
{
	return _parent_cpu.transfer_quota(c, q);
}


Genode::Capability<Genode::Cpu_session::Native_cpu> Cpu_session::native_cpu()
{
	return _parent_cpu.native_cpu();
}


int Cpu_session::set_sched_type(unsigned core, unsigned sched_type)
{
	return _parent_cpu.set_sched_type(core, sched_type);
}


int Cpu_session::get_sched_type(unsigned core)
{
	return _parent_cpu.get_sched_type(core);
}


void Cpu_session::set(Genode::Ram_session_capability ram_cap)
{
	_parent_cpu.set(ram_cap);
}


void Cpu_session::deploy_queue(Genode::Dataspace_capability ds)
{
	_parent_cpu.deploy_queue(ds);
}


void Cpu_session::rq(Genode::Dataspace_capability ds)
{
	_parent_cpu.rq(ds);
}


void Cpu_session::dead(Genode::Dataspace_capability ds)
{
	_parent_cpu.dead(ds);
}


void Cpu_session::killed()
{
	_parent_cpu.killed();
}


Cpu_session *Cpu_root::_create_session(const char *args)
{
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
	readjusted_ram_quota = readjusted_ram_quota + sizeof(Cpu_session) + md_alloc()->overhead(sizeof(Cpu_session));

	Genode::snprintf(ram_quota_buf, sizeof(ram_quota_buf), "%zu", readjusted_ram_quota);
	Genode::Arg_string::set_arg(readjusted_args, sizeof(readjusted_args), "ram_quota", ram_quota_buf);
	
	/* Create custom Rm_session */
	Cpu_session *new_session =
		new (md_alloc()) Cpu_session(_env,
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


void Cpu_root::_upgrade_session(Cpu_session *session, const char *upgrade_args)
{
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


void Cpu_root::_destroy_session(Cpu_session *session)
{
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
	Root_component<Cpu_session>(session_ep, md_alloc),
	_env              (env),
	_md_alloc         (md_alloc),
	_ep               (session_ep),
	_bootstrap_phase  (bootstrap_phase),
	_pd_root          (pd_root),
	_objs_lock        (),
	_session_rpc_objs (),
	_config(config)
{
}


Cpu_root::~Cpu_root()
{
	while(Cpu_session *obj = _session_rpc_objs.first()) {
		_session_rpc_objs.remove(obj);
		Genode::destroy(_md_alloc, obj);
	}
}
