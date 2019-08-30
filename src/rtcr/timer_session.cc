/*
 * \brief  Intercepting timer session
 * \author Denis Huber
 * \author Johannes Fischer
 * \date   2019-08-29
 */

#include <rtcr/timer/timer_session.h>

#ifdef PROFILE
#include <util/profiler.h>
#define PROFILE_THIS_CALL PROFILE_FUNCTION("coral");
#else
#define PROFILE_THIS_CALL
#endif

#if DEBUG 
#define DEBUG_THIS_CALL Genode::log("\e[38;5;209m", __PRETTY_FUNCTION__, "\033[0m");
#else
#define DEBUG_THIS_CALL
#endif

using namespace Rtcr;


Timer_session::Timer_session(Genode::Env &env,
							 Genode::Allocator &md_alloc,
							 Genode::Entrypoint &ep,
							 const char *creation_args,
							 bool bootstrapped,
							 Genode::Xml_node *config)
	:
	Checkpointable(env, config, "timer_session"),
	_md_alloc     (md_alloc),
	_ep           (ep),
	_parent_timer (env),
	_bootstrapped (bootstrapped),
	ck_creation_args (creation_args)
{
	DEBUG_THIS_CALL
}


void Timer_session::checkpoint()
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL
	ck_badge = cap().local_name();
	ck_bootstrapped = _bootstrapped;
	ck_upgrade_args = _upgrade_args;
	ck_sigh_badge = _sigh.local_name();
	ck_timeout = _timeout;
	ck_periodic = _periodic;

	// TODO
	//  ck_kcap = _core_module->find_kcap_by_badge(ck_badge);
}


void Timer_session::trigger_once(unsigned us)
{
	_timeout = us;
	_periodic = false;
	_parent_timer.trigger_once(us);
}


void Timer_session::trigger_periodic(unsigned us)
{
	_timeout = us;
	_periodic = true;
	_parent_timer.trigger_periodic(us);
}


void Timer_session::sigh(Genode::Signal_context_capability sigh)
{
	_sigh = sigh;
	_parent_timer.sigh(sigh);
}


unsigned long Timer_session::elapsed_ms() const
{
	return _parent_timer.elapsed_ms();
}


unsigned long Timer_session::now_us() const
{
	return _parent_timer.now_us();
}


void Timer_session::msleep(unsigned ms)
{
	_timeout = 1000*ms;
	_periodic = false;
	_parent_timer.msleep(ms);
}


void Timer_session::usleep(unsigned us)
{
	_timeout = us;
	_periodic = false;
	_parent_timer.usleep(us);
}

Timer_session *Timer_root::_create_session(const char *args)
{
	/* Revert ram_quota calculation, because the monitor needs the original
	 * session creation argument */
	char ram_quota_buf[32];
	char readjusted_args[160];
	Genode::strncpy(readjusted_args, args, sizeof(readjusted_args));

	Genode::size_t readjusted_ram_quota = Genode::Arg_string::find_arg(
		readjusted_args, "ram_quota").ulong_value(0);

	readjusted_ram_quota = readjusted_ram_quota
		+ sizeof(Timer_session)
		+ md_alloc()->overhead(sizeof(Timer_session));

	Genode::snprintf(ram_quota_buf,
					 sizeof(ram_quota_buf),
					 "%zu",
					 readjusted_ram_quota);

	Genode::Arg_string::set_arg(readjusted_args,
								sizeof(readjusted_args),
								"ram_quota",
								ram_quota_buf);

	/* Create virtual session object */
	Timer_session *new_session =
		new (md_alloc()) Timer_session(_env,
									   _md_alloc,
									   _ep,
									   readjusted_args,
									   _bootstrap_phase,
									   _config);

	Genode::Lock::Guard guard(_objs_lock);
	_session_rpc_objs.insert(new_session);

	return new_session;
}


void Timer_root::_upgrade_session(Timer_session *session, const char *upgrade_args)
{
	char ram_quota_buf[32];
	char new_upgrade_args[160];

	Genode::strncpy(new_upgrade_args,session->upgrade_args(),sizeof(new_upgrade_args));
	Genode::size_t ram_quota = Genode::Arg_string::find_arg(
		new_upgrade_args, "ram_quota").ulong_value(0);

	Genode::size_t extra_ram_quota = Genode::Arg_string::find_arg(
		upgrade_args, "ram_quota").ulong_value(0);
	ram_quota += extra_ram_quota;

	Genode::snprintf(ram_quota_buf, sizeof(ram_quota_buf), "%zu", ram_quota);
	Genode::Arg_string::set_arg(new_upgrade_args,
								sizeof(new_upgrade_args),
								"ram_quota",
								ram_quota_buf);

	_env.parent().upgrade(session->parent_cap(), upgrade_args);
	session->upgrade(upgrade_args);
}


void Timer_root::_destroy_session(Timer_session *session)
{
	_session_rpc_objs.remove(session);
	destroy(_md_alloc, session);

}


Timer_root::Timer_root(Genode::Env &env,
					   Genode::Allocator &md_alloc,
					   Genode::Entrypoint &session_ep,
					   bool &bootstrap_phase,
					   Genode::Xml_node *config)
	:
	Root_component<Timer_session>(session_ep, md_alloc),
	_env              (env),
	_md_alloc         (md_alloc),
	_ep               (session_ep),
	_bootstrap_phase  (bootstrap_phase),
	_objs_lock        (),
	_session_rpc_objs (),
	_config (config)
{
}


Timer_root::~Timer_root()
{
	while(Timer_session *obj = _session_rpc_objs.first()) {
		_session_rpc_objs.remove(obj);
		Genode::destroy(_md_alloc, obj);
	}
}


Timer_session *Timer_root::find_by_badge(Genode::uint16_t badge)
{
	Timer_session *obj = _session_rpc_objs.first();
	while(obj) { 
		if(badge == obj->cap().local_name())
			return obj;
		obj = obj->next();
	}
	return 0;
}

