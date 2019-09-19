/*
 * \brief  Intercepting ROM session
 * \author Denis Huber
 * \author Johannes Fischer
 * \date   2019-08-29
 */

#include <rtcr/rom/rom_session.h>
#include <base/session_label.h>

#ifdef PROFILE
#include <util/profiler.h>
#define PROFILE_THIS_CALL PROFILE_FUNCTION("lime");
#else
#define PROFILE_THIS_CALL
#endif

#if DEBUG 
#define DEBUG_THIS_CALL Genode::log("\e[38;5;48m", __PRETTY_FUNCTION__, "\033[0m");
#else
#define DEBUG_THIS_CALL
#endif


using namespace Rtcr;


Rtcr::Rom_session::Rom_session(Genode::Env& env,
							   Genode::Allocator& md_alloc,
							   Genode::Entrypoint& ep,
							   const char *creation_args,
							   const char *label,
							   Child_info *child_info)
	:
	Checkpointable(env, "rom_session"),
	Rom_session_info(creation_args, cap().local_name()),
	_env          (env),
	_md_alloc     (md_alloc),
	_ep           (ep),
	_parent_rom   (env, label),
	_child_info (child_info)
{
  DEBUG_THIS_CALL;
  _ep.rpc_ep().manage(this);
}


void Rom_session::checkpoint()
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL
	i_upgrade_args = _upgrade_args;
	i_dataspace_badge = _dataspace.local_name();
	i_sigh_badge = _sigh.local_name();
}


Genode::Rom_dataspace_capability Rtcr::Rom_session::dataspace()
{
	auto result = _parent_rom.dataspace();
	_dataspace = result;
	_size = Genode::Dataspace_client(Genode::static_cap_cast<Genode::Dataspace>(result)).size();
	return result;
}


bool Rtcr::Rom_session::update()
{
	return _parent_rom.update();
}


void Rtcr::Rom_session::sigh(Genode::Signal_context_capability sigh)
{
	_sigh = sigh;
	_parent_rom.sigh(sigh);
}



Rom_factory::Rom_factory(Genode::Env &env,
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

Rom_session *Rom_factory::_create(Child_info *info, const char *args)
{
  char label_buf[128];
  Genode::Arg label_arg = Genode::Arg_string::find_arg(args, "label");
  label_arg.string(label_buf, sizeof(label_buf), "");

  return new (_md_alloc) Rom_session(_env, _md_alloc, _ep, args, label_buf, info);
}

Rom_session &Rom_factory::create(Genode::Session_state::Args const &args, Genode::Affinity)
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
	Rom_session *new_session = _create(info, args.string());

	info->rom_session = new_session;
	return *new_session;
}


void Rom_factory::upgrade(Rom_session&, Genode::Session_state::Args const &)
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


void Rom_factory::destroy(Rom_session&)
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

