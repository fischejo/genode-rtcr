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




Rm_session::Rm_session(Genode::Env &env,
                       Genode::Allocator &md_alloc,
                       Genode::Entrypoint &ep,
                       const char *creation_args,
                       Child_info *child_info)
	:
	Checkpointable(env, "rm_session"),
	Rm_session_info(creation_args, cap().local_name()),
	_md_alloc         (md_alloc),
	_ep               (ep),
	_parent_rm        (env),
	_child_info (child_info)
{
	DEBUG_THIS_CALL;
	_ep.rpc_ep().manage(this);
}


Rm_session::~Rm_session()
{
	while(Region_map_info *rm = _region_maps.first()) {
		_region_maps.remove(static_cast<Region_map*>(rm));
		Genode::destroy(_md_alloc, rm);
	}
}


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
	_ep.rpc_ep().manage(new_region_map);

	/* Insert custom Region map into list */
	Genode::Lock::Guard lock(_region_maps_lock);
	_region_maps.insert(new_region_map);
	Genode::log("new_region_map ds cap=", new_region_map->dataspace());
	//	_ram_session.mark_region_map_dataspace(new_region_map->dataspace()); TODO FJO
	return *new_region_map;
}


void Rm_session::_destroy(Region_map *region_map)
{
	/* Reverse order as in _create */
	auto parent_cap = region_map->parent_cap();

	/* Dissolve custom RPC object */
	_ep.dissolve(*region_map);

	/* Destroy real Region map from parent */
	_parent_rm.destroy(parent_cap);
}



void Rm_session::checkpoint()
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL
		i_upgrade_args = _upgrade_args;

	Region_map_info *region_map = nullptr;
	while(region_map = _destroyed_region_maps.dequeue()) {
		_region_maps.remove(region_map);
		Genode::destroy(_md_alloc, &region_map);
	}

	region_map = _region_maps.first();
	while(region_map) {
		static_cast<Region_map*>(region_map)->checkpoint();
		region_map = region_map->next();
	}

	i_region_maps = _region_maps.first();
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
	Region_map_info *region_map = _region_maps.first();
	if(region_map) region_map = region_map->find_by_badge(region_map_cap.local_name());
	if(region_map) {
		Genode::error("Issuing Rm_session::destroy, which is bugged and hangs up.");

		Genode::Lock::Guard lock(_destroyed_region_maps_lock);
		_destroyed_region_maps.enqueue(region_map);

		_destroy(static_cast<Region_map*>(region_map));
	} else {
		Genode::error("No Region map with ", region_map_cap, " found!");
	}
}


Rm_factory::Rm_factory(Genode::Env &env,
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

Rm_session *Rm_factory::_create(Child_info *info, const char *args)
{
	return new (_md_alloc) Rm_session(_env, _md_alloc, _ep, args, info);
}

Rm_session &Rm_factory::create(Genode::Session_state::Args const &args, Genode::Affinity)
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
	Rm_session *new_session = _create(info, args.string());

	info->rm_session = new_session;
	return *new_session;
}


void Rm_factory::upgrade(Rm_session&, Genode::Session_state::Args const &)
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


void Rm_factory::destroy(Rm_session&)
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

