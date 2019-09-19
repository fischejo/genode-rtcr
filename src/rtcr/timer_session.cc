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
	_md_alloc     (md_alloc),
	_ep           (ep),
	_parent_timer (env),
	_child_info (child_info)
{
  DEBUG_THIS_CALL;
  _ep.rpc_ep().manage(this);
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



Timer_factory::Timer_factory(Genode::Env &env,
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

Timer_session *Timer_factory::_create(Child_info *info, const char *args)
{
    return new (_md_alloc) Timer_session(_env, _md_alloc, _ep, args, info);
}

Timer_session &Timer_factory::create(Genode::Session_state::Args const &args, Genode::Affinity)
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
	Timer_session *new_session = _create(info, args.string());

	info->timer_session = new_session;
	return *new_session;
}


void Timer_factory::upgrade(Timer_session&, Genode::Session_state::Args const &)
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


void Timer_factory::destroy(Timer_session&)
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

