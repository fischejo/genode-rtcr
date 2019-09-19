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
	_md_alloc     (md_alloc),
	_ep           (ep),
	_parent_log   (env, child_info->name.string()),
	_child_info (child_info)
{
  DEBUG_THIS_CALL;
  _ep.rpc_ep().manage(this);
}


Log_session::~Log_session() {}


Genode::size_t Log_session::write(String const &string)
{
	return _parent_log.write(string);
}


void Log_session::checkpoint()
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL
	i_upgrade_args = _upgrade_args;
}


Log_factory::Log_factory(Genode::Env &env,
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

Log_session *Log_factory::_create(Child_info *info, const char *args)
{
    return new (_md_alloc) Log_session(_env, _md_alloc, _ep, args, info);
}

Log_session &Log_factory::create(Genode::Session_state::Args const &args, Genode::Affinity)
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
	Log_session *new_session = _create(info, args.string());

	info->log_session = new_session;
	return *new_session;
}


void Log_factory::upgrade(Log_session&, Genode::Session_state::Args const &)
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


void Log_factory::destroy(Log_session&)
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

