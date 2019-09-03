/*
 * \brief  Intercepting Rm session
 * \author Denis Huber
 * \author Johannes Fischer
 * \date   2019-08-29
 */

#include <rtcr/rm/rm_session.h>
#include <rtcr/child_info.h>

#ifdef PROFILE
#include <util/profiler.h>
#define PROFILE_THIS_CALL PROFILE_FUNCTION("magenta");
#else
#define PROFILE_THIS_CALL
#endif

#if DEBUG 
#define DEBUG_THIS_CALL Genode::log("\e[38;5;207m", __PRETTY_FUNCTION__, "\033[0m");
#else
#define DEBUG_THIS_CALL
#endif

using namespace Rtcr;

Region_map &Rm_session::_create(Genode::size_t size)
{
	/* Create real Region map from parent */
	auto parent_cap = _parent_rm.create(size);

	/* Create custom Region map */
	Region_map *new_region_map = new (_md_alloc) Region_map(_md_alloc,
															parent_cap,
															size,
															"custom",
															_child_info->bootstrapped);
	/* Manage custom Region map */
	_ep.manage(*new_region_map);

	/* Insert custom Region map into list */
	Genode::Lock::Guard lock(_region_maps_lock);
	_region_maps.insert(new_region_map);
//	_ram_session.mark_region_map_dataspace(new_region_map->dataspace()); TODO FJO
	return *new_region_map;
}


void Rm_session::_destroy(Region_map &region_map)
{
	/* Reverse order as in _create */
	auto parent_cap = region_map.parent_cap();

	/* Dissolve custom RPC object */
	_ep.dissolve(region_map);

	/* Destroy real Region map from parent */
	_parent_rm.destroy(parent_cap);
}


Rm_session::Rm_session(Genode::Env &env,
					   Genode::Allocator &md_alloc,
					   Genode::Entrypoint &ep,
					   const char *creation_args,
					   Child_info *child_info)
	:
	Checkpointable(env, "rm_session"),
	_md_alloc         (md_alloc),
	_ep               (ep),
	_parent_rm        (env),
	_child_info (child_info),
	info (creation_args)
{
	DEBUG_THIS_CALL
}


Rm_session::~Rm_session()
{
	while(Region_map *rm = _region_maps.first()) {
		_region_maps.remove(rm);
		Genode::destroy(_md_alloc, rm);
	}
}


void Rm_session::checkpoint()
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL

	info.badge = cap().local_name();
	info.bootstrapped = _child_info->bootstrapped;
	info.upgrade_args = _upgrade_args;

	// TODO
	//  ck_kcap = _core_module->find_kcap_by_badge(ck_badge);

	Region_map *region_map = nullptr;
	while(region_map = _destroyed_region_maps.dequeue()) {
		_region_maps.remove(region_map);
		Genode::destroy(_md_alloc, &region_map);
	}

	region_map = _region_maps.first();
	while(region_map) {
		region_map->checkpoint();
		region_map = region_map->next();
	}

	info.region_maps = _region_maps.first();
}


Genode::Capability<Genode::Region_map> Rm_session::create(Genode::size_t size)
{
	DEBUG_THIS_CALL
	/* Create custom Region map */
	Region_map &new_region_map = _create(size);
	return new_region_map.cap();
}


void Rm_session::destroy(Genode::Capability<Genode::Region_map> region_map_cap)
{
	/* Find RPC object for the given Capability */
	Region_map *region_map = _region_maps.first();
	if(region_map) region_map = region_map->find_by_badge(region_map_cap.local_name());
	if(region_map) {
		Genode::error("Issuing Rm_session::destroy, which is bugged and hangs up.");

		Genode::Lock::Guard lock(_destroyed_region_maps_lock);
		_destroyed_region_maps.enqueue(region_map);
		
		_destroy(*region_map);
	} else {
		Genode::error("No Region map with ", region_map_cap, " found!");
	}
}



Rm_session *Rm_root::_create_rm_session(Child_info *info, const char *args)
{
	return new (md_alloc()) Rm_session(_env, _md_alloc, _ep, args, info);
}


Rm_session *Rm_root::_create_session(const char *args)
{
	DEBUG_THIS_CALL;
	/* Revert ram_quota calculation, because the monitor needs the original
	 * session creation argument */
	char ram_quota_buf[32];
	char readjusted_args[160];
	Genode::strncpy(readjusted_args, args, sizeof(readjusted_args));

	Genode::size_t readjusted_ram_quota = Genode::Arg_string::find_arg(readjusted_args, "ram_quota").ulong_value(0);
	readjusted_ram_quota = readjusted_ram_quota + sizeof(Rm_session) + md_alloc()->overhead(sizeof(Rm_session));

	Genode::snprintf(ram_quota_buf, sizeof(ram_quota_buf), "%zu", readjusted_ram_quota);
	Genode::Arg_string::set_arg(readjusted_args, sizeof(readjusted_args), "ram_quota", ram_quota_buf);


	/* Extracting label from args */
	char label_buf[128];
	Genode::Arg label_arg = Genode::Arg_string::find_arg(args, "label");
	label_arg.string(label_buf, sizeof(label_buf), "");

	_childs_lock.lock();	
	Child_info *info = _childs.first();
	if(info) info = info->find_by_name(label_buf);	
	if(!info) {
		info = new(_md_alloc) Child_info(label_buf);
		_childs.insert(info);			
	}
	_childs_lock.unlock();
	
	/* Create custom Rm_session */
	Rm_session *new_session =  _create_rm_session(info, readjusted_args);
	info->rm_session = new_session;	
	return new_session;
}


void Rm_root::_upgrade_session(Rm_session *session, const char *upgrade_args)
{
	char ram_quota_buf[32];
	char new_upgrade_args[160];

	Genode::strncpy(new_upgrade_args, session->upgrade_args(), sizeof(new_upgrade_args));
	Genode::size_t ram_quota = Genode::Arg_string::find_arg(new_upgrade_args, "ram_quota").ulong_value(0);
	Genode::size_t extra_ram_quota = Genode::Arg_string::find_arg(upgrade_args, "ram_quota").ulong_value(0);
	ram_quota += extra_ram_quota;

	Genode::snprintf(ram_quota_buf, sizeof(ram_quota_buf), "%zu", ram_quota);
	Genode::Arg_string::set_arg(new_upgrade_args, sizeof(new_upgrade_args), "ram_quota", ram_quota_buf);

	_env.parent().upgrade(session->parent_cap(), upgrade_args);
	session->upgrade(upgrade_args);
}


void Rm_root::_destroy_session(Rm_session *session)
{

}


Rm_root::Rm_root(Genode::Env &env,
				 Genode::Allocator &md_alloc,
				 Genode::Entrypoint &session_ep,
				   Genode::Lock &childs_lock,
				   Genode::List<Child_info> &childs)
	:
	Root_component<Rm_session>(session_ep, md_alloc),
	_env              (env),
	_md_alloc         (md_alloc),
	_ep               (session_ep),
	_childs_lock (childs_lock),
	_childs (childs)
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL;	
}

Rm_root::~Rm_root()
{
	Genode::Lock::Guard lock(_childs_lock);
	Child_info *info = _childs.first();
	while(info) {
		Genode::destroy(_md_alloc, info->rm_session);		
		info->rm_session = nullptr;
		if(info->child_destroyed()) _childs.remove(info);
		info = info->next();
	}
}


