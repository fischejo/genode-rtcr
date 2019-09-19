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
	DEBUG_THIS_CALL;

	/* Create real CPU thread from parent */
	auto cpu_thread_cap = _parent_cpu.create_thread(parent_pd_cap,
	                                                name,
	                                                affinity,
	                                                weight,
	                                                utcb);

	/* Create custom CPU thread */
	Cpu_thread *new_cpu_thread = new (_md_alloc) Cpu_thread(_md_alloc,
	                                                        cpu_thread_cap,
	                                                        child_pd_cap,
	                                                        name.string(),
	                                                        weight,
	                                                        utcb,
	                                                        affinity,
	                                                        _child_info->bootstrapped,
	                                                        _ep);

	/* Insert custom CPU thread into list */
	Genode::Lock::Guard _lock_guard(_cpu_threads_lock);
	_cpu_threads.insert(new_cpu_thread);

	return *new_cpu_thread;
}


void Cpu_session::_kill_thread(Cpu_thread_info &cpu_thread_info)
{
	Cpu_thread &cpu_thread = static_cast<Cpu_thread&>(cpu_thread_info);
	auto parent_cap = cpu_thread.parent_cap();

	/* Remove custom CPU thread form list */
	Genode::Lock::Guard lock(_destroyed_cpu_threads_lock);
	_destroyed_cpu_threads.enqueue(&cpu_thread_info);

	/* Destroy real CPU thread from parent */
	_parent_cpu.kill_thread(parent_cap);
}


Cpu_session::Cpu_session(Genode::Env &env,
                         Genode::Allocator &md_alloc,
                         Genode::Entrypoint &ep,
                         const char *creation_args,
                         Child_info *child_info)
	:
	Checkpointable(env, "cpu_session"),
	Cpu_session_info(creation_args, cap().local_name()),
	_env             (env),
	_md_alloc        (md_alloc),
	_config (env, "config"),
	_ep              (ep),
	_parent_cpu      (env, child_info->name.string()),
	_child_affinity (_read_child_affinity(child_info->name.string())),
	_child_info (child_info)
{
	DEBUG_THIS_CALL;

	_ep.rpc_ep().manage(this);
}


Cpu_session::~Cpu_session()
{
	_ep.rpc_ep().dissolve(this);
	
	while(Cpu_thread_info *cpu_thread_info = _cpu_threads.first()) {
		_cpu_threads.remove(cpu_thread_info);
		Genode::destroy(_md_alloc, cpu_thread_info);
	}
}


Genode::Affinity::Location Cpu_session::_read_child_affinity(const char* child_name)
{
	try {
		Genode::Xml_node config_node = _config.xml();
		Genode::Xml_node ck_node = config_node.sub_node("child");
		Genode::String<30> node_name;
		while(Genode::strcmp(child_name, ck_node.attribute_value("name", node_name).string()))
			ck_node = ck_node.next("child");

		long const xpos = ck_node.attribute_value<long>("xpos", 0);
		long const ypos = ck_node.attribute_value<long>("ypos", 0);
		return Genode::Affinity::Location(xpos, ypos, 1 ,1);
	}
	catch (...) { return Genode::Affinity::Location(0, 0, 1, 1);}
	return Genode::Affinity::Location(0, 0, 1, 1);
}



void Cpu_session::checkpoint()
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL
		i_upgrade_args = _upgrade_args;
	i_sigh_badge = _sigh.local_name();

	Cpu_thread_info *cpu_thread;
	while(cpu_thread = _destroyed_cpu_threads.dequeue()) {
		_cpu_threads.remove(cpu_thread);
		Genode::destroy(_md_alloc, &cpu_thread);
	}

	cpu_thread = _cpu_threads.first();
	while(cpu_thread) {
		static_cast<Cpu_thread*>(cpu_thread)->checkpoint();
		cpu_thread = cpu_thread->next();
	}

	/* checkpoint current state of Cpu_thread list. */
	i_cpu_thread_info = _cpu_threads.first();
}

void Cpu_session::pause()
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL;

	Cpu_thread_info *cpu_thread = _cpu_threads.first();
	while(cpu_thread) {
		/* if the object is in the destroyed queue, it means that it is already
		 * destroyed */
		if(!cpu_thread->enqueued())
			static_cast<Cpu_thread*>(cpu_thread)->silent_pause();
		cpu_thread = cpu_thread->next();
	}
	_cpu_threads_lock.unlock();
}

void Cpu_session::resume()
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL

		Cpu_thread_info *cpu_thread = _cpu_threads.first();
	while(cpu_thread) {
		/* if the object is in the destroyed queue, it means that it is already
		 * destroyed */
		if(!cpu_thread->enqueued())
			static_cast<Cpu_thread*>(cpu_thread)->silent_resume();
		cpu_thread = cpu_thread->next();
	}
}


Genode::Thread_capability Cpu_session::create_thread(Genode::Pd_session_capability child_pd_cap,
                                                     Genode::Cpu_session::Name const &name,
                                                     Genode::Affinity::Location affinity,
                                                     Genode::Cpu_session::Weight weight,
                                                     Genode::addr_t utcb)
{
	DEBUG_THIS_CALL;
	/* Find corresponding parent PD session cap for the given custom PD session
	 * cap */
	Genode::log("child_info name=", _child_info->name);
	if(!_child_info->pd_session) {
		Genode::error("Thread creation failed: PD session ",
		              child_pd_cap, " is unknown.");
		throw Genode::Exception();
	}

	/* Create custom CPU thread */
	Pd_session *pd_session = static_cast<Pd_session *>(_child_info->pd_session);
	Cpu_thread &new_cpu_thread = _create_thread(pd_session->cap(),
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
	Genode::Lock::Guard lock (_cpu_threads_lock);
	Cpu_thread_info *cpu_thread = _cpu_threads.first();
	if(cpu_thread) cpu_thread = cpu_thread->find_by_badge(thread_cap.local_name());
	if(cpu_thread) {
		Genode::error("Issuing Rm_session::destroy, which is bugged and hangs up.");
		_kill_thread(*cpu_thread);
	} else {
		Genode::error("No thread with ", thread_cap, " found!");
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


Cpu_factory::Cpu_factory(Genode::Env &env,
                         Genode::Allocator &md_alloc,
                         Genode::Entrypoint &ep,
                         Genode::Lock &childs_lock,
                         Genode::List<Child_info> &childs)
	:
	_env              (env),
	_md_alloc         (md_alloc),
	_ep               (ep),
	_childs_lock(childs_lock),
	_childs(childs),
	_service(*this)
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL;
}

Cpu_session *Cpu_factory::_create(Child_info *info, const char *args)
{
	return new (_md_alloc) Cpu_session(_env, _md_alloc, _ep, args, info);
}

Cpu_session &Cpu_factory::create(Genode::Session_state::Args const &args, Genode::Affinity)
{
	DEBUG_THIS_CALL;

	char label_buf[160];
	Genode::Arg label_arg = Genode::Arg_string::find_arg(args.string(), "label");
	label_arg.string(label_buf, sizeof(label_buf), "");

	_childs_lock.lock();
	Child_info *info = _childs.first();
	if(info) info = info->find_by_name(label_buf);
	if(!info) {
		info = new(_md_alloc) Child_info(label_buf);
		_childs.insert(info);
	}
	_childs_lock.unlock();

	/* Create custom Pd_session */
	Cpu_session *new_session = _create(info, args.string());

	info->cpu_session = new_session;
	return *new_session;
}


void Cpu_factory::upgrade(Cpu_session&, Genode::Session_state::Args const &)
{
	// char ram_quota_buf[32];
	// char new_upgrade_args[160];

	// Genode::strncpy(new_upgrade_args, session->upgrade_args(), sizeof(new_upgrade_args));

	// Genode::size_t ram_quota = Genode::Arg_string::find_arg(new_upgrade_args, "ram_quota").ulong_value(0);
	// Genode::size_t extra_ram_quota = Genode::Arg_string::find_arg(upgrade_args, "ram_quota").ulong_value(0);
	// ram_quota += extra_ram_quota;

	// Genode::snprintf(ram_quota_buf, sizeof(ram_quota_buf), "%zu", ram_quota);
	// Genode::Arg_string::set_arg(new_upgrade_args, sizeof(new_upgrade_args), "ram_quota", ram_quota_buf);

	// _env.parent().upgrade(Genode::Parent::Env::pd(), upgrade_args);
	// session->upgrade(upgrade_args);
}


void Cpu_factory::destroy(Cpu_session&)
{
	// Genode::Lock::Guard lock(_childs_lock);
	// Child_info *info = _childs.first();
	// while(info) {
	// 	Genode::destroy(_md_alloc, info->pd_session);		
	// 	info->pd_session = nullptr;
	// 	if(info->child_destroyed()) _childs.remove(info);
	// 	info = info->next();
	// }
}

