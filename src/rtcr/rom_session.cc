/*
 * \brief  Intercepting ROM session
 * \author Denis Huber
 * \author Johannes Fischer
 * \date   2019-08-29
 */

#include <rtcr/rom/rom_session.h>

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
							   const char *label,
							   const char *creation_args,
							   Child_info *child_info)
	:
	Checkpointable(env, "rom_session"),
	_env          (env),
	_md_alloc     (md_alloc),
	_ep           (ep),
	_parent_rom   (env, label),
	_child_info (child_info),
	info (creation_args)
{
	DEBUG_THIS_CALL
}


void Rom_session::checkpoint()
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL
	info.badge = cap().local_name();
	info.bootstrapped = _child_info->bootstrapped;
	info.upgrade_args = _upgrade_args;
	info.dataspace_badge = _dataspace.local_name();
	info.sigh_badge = _sigh.local_name();
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


Rom_session *Rom_root::_create_session(const char *args)
{
	DEBUG_THIS_CALL;	
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
	readjusted_ram_quota = readjusted_ram_quota + sizeof(Rom_session) + md_alloc()->overhead(sizeof(Rom_session));

	Genode::snprintf(ram_quota_buf, sizeof(ram_quota_buf), "%zu", readjusted_ram_quota);
	Genode::Arg_string::set_arg(readjusted_args, sizeof(readjusted_args), "ram_quota", ram_quota_buf);

	_childs_lock.lock();
	Child_info *info = _childs.first();
	if(info) info = info->find_by_name(label_buf);
	if(!info) {
		info = new(_md_alloc) Child_info(label_buf);
		_childs.insert(info);		
	}
	_childs_lock.unlock();
	
	/* Create custom Rom_session */
	Rom_session *new_session = new (md_alloc()) Rom_session(_env,
															_md_alloc,
															_ep,
															label_buf,
															readjusted_args,
															info);
	info->rom_session = new_session;
	Genode::log("rom leaving");
	return new_session;
}


void Rom_root::_upgrade_session(Rom_session *session, const char *upgrade_args)
{
	char ram_quota_buf[32];
	char new_upgrade_args[160];

//	Genode::strncpy(new_upgrade_args, session->parent_state().upgrade_args.string(), sizeof(new_upgrade_args));

	Genode::size_t ram_quota = Genode::Arg_string::find_arg(new_upgrade_args, "ram_quota").ulong_value(0);
	Genode::size_t extra_ram_quota = Genode::Arg_string::find_arg(upgrade_args, "ram_quota").ulong_value(0);
	ram_quota += extra_ram_quota;

	Genode::snprintf(ram_quota_buf, sizeof(ram_quota_buf), "%zu", ram_quota);
	Genode::Arg_string::set_arg(new_upgrade_args, sizeof(new_upgrade_args), "ram_quota", ram_quota_buf);

	// TODO
	// session->parent_state().upgrade_args = new_upgrade_args;

	_env.parent().upgrade(session->parent_cap(), upgrade_args);
}


void Rom_root::_destroy_session(Rom_session *session)
{
	// TODO FJO
}


Rom_root::Rom_root(Genode::Env &env,
				   Genode::Allocator &md_alloc,
				   Genode::Entrypoint &session_ep,
				   Genode::Lock &childs_lock,
				   Genode::List<Child_info> &childs)
	:
	Root_component<Rom_session>(session_ep, md_alloc),
	_env              (env),
	_md_alloc         (md_alloc),
	_ep               (session_ep),
	_childs_lock(childs_lock),
	_childs(childs)
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL;	
}


Rom_root::~Rom_root()
{
	Genode::Lock::Guard lock(_childs_lock);
	Child_info *info = _childs.first();
	while(info) {
		Genode::destroy(_md_alloc, info->rom_session);		
		info->rom_session = nullptr;
		if(info->child_destroyed()) _childs.remove(info);
		info = info->next();
	}
}
