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
	_env (env),
	_ep               (ep),
	_parent_rm        (env),
	_child_info (child_info)
{
	DEBUG_THIS_CALL;
	_ep.rpc_ep().manage(this);
	child_info->rm_session = this;	
}


Rm_session::~Rm_session()
{
	_ep.rpc_ep().dissolve(this);
	_child_info->rm_session = nullptr;	
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
	                                                        _child_info->bootstrapped,
	                                                        _ep);

	/* Insert custom Region map into list */
	Genode::Lock::Guard lock(_region_maps_lock);
	_region_maps.insert(new_region_map);

	return *new_region_map;
}


void Rm_session::_destroy(Region_map *region_map)
{
	/* Reverse order as in _create */
	auto parent_cap = region_map->parent_cap();

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


void Rm_session::upgrade(const char *upgrade_args)
{
	/* instead of upgrading the intercepting session, the
	   intercepted session is upgraded */
	_env.parent().upgrade(Genode::Parent::Env::pd(), upgrade_args);
	_upgrade_args = upgrade_args;
}

