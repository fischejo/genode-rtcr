/*
 * \brief  Intercepting Region map
 * \author Denis Huber
 * \author Johannes Fischer
 * \date   2019-08-29
 */

#ifndef _RTCR_REGION_MAP_COMPONENT_H_
#define _RTCR_REGION_MAP_COMPONENT_H_

/* Genode includes */
#include <util/list.h>
#include <util/fifo.h>
#include <region_map/client.h>
#include <dataspace/client.h>
#include <base/allocator.h>
#include <base/rpc_server.h>

/* Rtcr includes */
#include <rtcr/rm/attached_region_info.h>

namespace Rtcr {
	class Region_map;
}

/**
 * Custom Region map intercepting RPC methods
 */
class Rtcr::Region_map : public Genode::Rpc_object<Genode::Region_map>,
						 public Genode::List<Region_map>::Element,
						 public Genode::Fifo<Region_map>::Element
{
public:
	/******************
	 ** COLD STORAGE **
	 ******************/
	
	bool ck_bootstrapped;
	Genode::uint16_t ck_badge;
	Genode::addr_t ck_kcap;
  
	Genode::size_t ck_size;
	Genode::uint16_t ck_ds_badge;
	Genode::uint16_t ck_sigh_badge;
	Genode::List<Attached_region_info>::Element *ck_attached_regions;
  
protected:
	/*****************
	 ** HOT STORAGE **
	 *****************/
	
	const char* _upgrade_args;

	/**
	 * Size of the region map
	 */
	Genode::size_t const _size;

	/**
	 * Wrapped region map from parent, usually core
	 */
	Genode::Region_map_client _parent_region_map;
	
	/**
	 * Dataspace representation
	 */
	Genode::Dataspace_capability const _ds_cap;

	/**
	 * Signal context of the fault handler

	 */
	Genode::Signal_context_capability _sigh;

	/**
	 * List of attached regions
	 */
	Genode::Lock _destroyed_attached_regions_lock;
	Genode::Fifo<Attached_region_info> _destroyed_attached_regions;

	Genode::Lock _attached_regions_lock;
	Genode::List<Attached_region_info> _attached_regions;
	
	/**
	 * Allocator for Region map's attachments
	 */
	Genode::Allocator &_md_alloc;

	/**
	 * Reference to Target_child's bootstrap phase
	 */
	bool &_bootstrap_phase;

	/**
	 * Name of the Region map for debugging
	 */
	const char* _label;

public:
	/**
	 * List and Fifo provide a next() method. In general, you want to use the
	 * list implementation.
	 */
	using Genode::List<Region_map>::Element::next;

	Region_map(Genode::Allocator &md_alloc,
		       Genode::Capability<Genode::Region_map>
		       region_map_cap,
		       Genode::size_t size,
		       const char *label,
		       bool &bootstrap_phase);

	~Region_map();

	Genode::Capability<Genode::Region_map> parent_cap() { return _parent_region_map; }


	/* TODO FJO: this is not thread safe... */
	Genode::List<Attached_region_info> attached_regions() { return _attached_regions; }

	void checkpoint();

	void print(Genode::Output &output) const {
		using Genode::Hex;

		Genode::print(output,
					  ", ck_size=", ck_size,
					  ", ck_ds_badge=", ck_ds_badge,
					  ", ck_sigh_badge=", ck_sigh_badge);
	}
	
  
	Region_map *find_by_badge(Genode::uint16_t badge);

	/******************************
	 ** Region map Rpc interface **
	 ******************************/

	/**
	 * Attaches a dataspace to parent's Region map and stores information
	 * about the attachment
	 */
	Local_addr attach(Genode::Dataspace_capability ds_cap,
					  Genode::size_t size,
					  Genode::off_t offset,
					  bool use_local_addr,
					  Region_map::Local_addr local_addr,
					  bool executable) override;
	/**
	 * Detaches the dataspace from parent's region map and destroys the
	 * information about the attachment
	 */
	void detach(Region_map::Local_addr local_addr) override;
	
	void fault_handler(Genode::Signal_context_capability handler) override;
	
	State state() override;

	Genode::Dataspace_capability dataspace() override;
};

#endif /* _RTCR_REGION_MAP_COMPONENT_H_ */
