/*
 * \brief  Intercepting Rm session
 * \author Denis Huber
 * \author Johannes Fischer
 * \date   2019-08-29
 */

#ifndef _RTCR_RM_SESSION_H_
#define _RTCR_RM_SESSION_H_

/* Genode includes */
#include <rm_session/connection.h>
#include <base/allocator.h>
#include <root/component.h>
#include <util/list.h>
#include <base/session_object.h>

/* Rtcr includes */
#include <rtcr/checkpointable.h>
#include <rtcr/rm/region_map.h>
#include <rtcr/rm/rm_session_info.h>
#include <rtcr/child_info.h>

namespace Rtcr {
	class Rm_session;
}

/**
 * Custom RPC session object to intercept its creation, modification, and
 * destruction through its interface
 */
class Rtcr::Rm_session : public Rtcr::Checkpointable,
                         public Genode::Rpc_object<Genode::Rm_session>,
                         public Rtcr::Rm_session_info
{
protected:
	const char* _upgrade_args;
	Genode::Lock _region_maps_lock;
	Genode::Lock _destroyed_region_maps_lock;
	Genode::List<Region_map_info> _region_maps;
	Genode::Fifo<Region_map_info> _destroyed_region_maps;

	/**
	 * Allocator for Rpc objects created by this session and also for monitoring structures
	 */
	Genode::Allocator &_md_alloc;

	Genode::Env &_env;
	/**
	 * Entrypoint for managing created Rpc objects
	 */
	Genode::Entrypoint &_ep;

	/**
	 * Parent's session connection which is used by the intercepted methods
	 */
	Genode::Rm_connection  _parent_rm;

	Region_map &_create(Genode::size_t size);
	void _destroy(Region_map *region_map);

	Child_info *_child_info;

public:
	using Genode::Rpc_object<Genode::Rm_session>::cap;

	Rm_session(Genode::Env &env,
	           Genode::Allocator &md_alloc,
	           Genode::Entrypoint &ep,
	           const char *creation_args,
	           Child_info *child_info);

	~Rm_session();

	Genode::Rm_session_capability parent_cap() { return _parent_rm.cap(); }

	void checkpoint() override;

	void upgrade(const char *upgrade_args);

	const char* upgrade_args() { return _upgrade_args; }

	/******************************
	 ** Rm session Rpc interface **
	 ******************************/

	/**
	 * Create a virtual Region map, its real counter part and a list element to
	 * manage them
	 */
	Genode::Capability<Genode::Region_map> create(Genode::size_t size) override;
	/**
	 * Destroying the virtual Region map, its real counter part, and the list
	 * element it was managed in
	 *
	 * XXX Untested! During the implementation time, the destroy method was not working.
	 */
	void destroy(Genode::Capability<Genode::Region_map> region_map_cap) override;
};

#endif /* _RTCR_RM_SESSION_H_ */
