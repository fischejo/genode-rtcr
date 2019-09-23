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
                             Child_info *child_info)
	:
	Checkpointable(env, "timer_session"),
	Timer_session_info(creation_args, cap().local_name()),
	_env(env),
	_md_alloc     (md_alloc),
	_ep           (ep),
	_parent_timer (env),
	_child_info (child_info)
{
	DEBUG_THIS_CALL;
	_ep.rpc_ep().manage(this);
	child_info->timer_session = this;	
}


Timer_session::~Timer_session() {
	_ep.rpc_ep().dissolve(this);
	_child_info->timer_session = nullptr;	
}


void Timer_session::checkpoint()
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL
		i_upgrade_args = _upgrade_args;
	i_sigh_badge = _sigh.local_name();
	i_timeout = _timeout;
	i_periodic = _periodic;
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


unsigned long Timer_session::elapsed_us() const
{
	return _parent_timer.elapsed_us();
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


void Timer_session::upgrade(const char *upgrade_args)
{
	/* instead of upgrading the intercepting session, the
	   intercepted session is upgraded */
	_env.parent().upgrade(Genode::Parent::Env::pd(), upgrade_args);
	_upgrade_args = upgrade_args;
}
