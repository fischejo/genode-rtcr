/*
 * \brief  Intercepting Log session
 * \author Denis Huber
 * \author Johannes Fischer
 * \date   2019-08-29
 */

#include <rtcr/log/log_session.h>

using namespace Rtcr;

#ifdef PROFILE
#include <util/profiler.h>
#define PROFILE_THIS_CALL PROFILE_FUNCTION("green");
#else
#define PROFILE_THIS_CALL
#endif

#if DEBUG 
#define DEBUG_THIS_CALL Genode::log("\e[38;5;118m", __PRETTY_FUNCTION__, "\033[0m");
#else
#define DEBUG_THIS_CALL
#endif


Log_session::Log_session(Genode::Env &env,
                         Genode::Allocator &md_alloc,
                         Genode::Entrypoint &ep,
                         const char *creation_args,
                         Child_info *child_info)
	:
	Checkpointable(env, "log_session"),
	Log_session_info(creation_args, cap().local_name()),
	_env (env),
	_md_alloc     (md_alloc),
	_ep           (ep),
	_parent_log   (env, child_info->name.string()),
	_child_info (child_info)
{
	DEBUG_THIS_CALL;
	_ep.rpc_ep().manage(this);
	child_info->log_session = this;		
}


Log_session::~Log_session() {
	_ep.rpc_ep().dissolve(this);
	_child_info->log_session = nullptr;	
}


Genode::size_t Log_session::write(String const &string)
{
	return _parent_log.write(string);
}


void Log_session::checkpoint()
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL
		i_upgrade_args = _upgrade_args;
}


void Log_session::upgrade(const char *upgrade_args)
{
	/* instead of upgrading the intercepting session, the
	   intercepted session is upgraded */
	_env.parent().upgrade(Genode::Parent::Env::pd(), upgrade_args);
	_upgrade_args = upgrade_args;
}
