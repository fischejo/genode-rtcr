/*
 * \brief  Intercepting Region map
 * \author Denis Huber
 * \date   2016-08-09
 */

#ifndef _RTCR_REGION_MAP_COMPONENT_H_
#define _RTCR_REGION_MAP_COMPONENT_H_

/* Genode includes */
#include <region_map/client.h>
#include <dataspace/client.h>
#include <base/allocator.h>
#include <base/rpc_server.h>

/* Rtcr includes */
#include <rtcr/rm/attached_region_info.h>

namespace Rtcr {
	class Region_map_component;

	constexpr bool region_map_verbose_debug = false;
}

/**
 * Custom Region map intercepting RPC methods
 */
class Rtcr::Region_map_component : public Genode::Rpc_object<Genode::Region_map>,
                                   public Genode::List<Region_map_component>::Element
{
public:
  bool ck_bootstrapped;
  Genode::uint16_t ck_badge;
  Genode::addr_t ck_kcap;
  
  Genode::size_t ck_size;
  Genode::uint16_t ck_ds_badge;
  Genode::uint16_t ck_sigh_badge;
  Genode::List<Attached_region_info> ck_attached_regions;

  void checkpoint();

  void print(Genode::Output &output) const
  {
    using Genode::Hex;

    Genode::print(output,
		  ", ck_size=", ck_size,
		  ", ck_ds_badge=", ck_ds_badge,
		  ", ck_sigh_badge=", ck_sigh_badge);
  }

  Genode::Lock _new_attached_regions_lock;
  Genode::List<Attached_region_info> _new_attached_regions;
  
protected:
  const char* _upgrade_args;

  /**
	 * Size of the region map
	 */
	Genode::size_t const _size;
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
	Genode::List<Attached_region_info> _destroyed_attached_regions;


  
private:
	static constexpr bool verbose_debug = region_map_verbose_debug;

	/**
	 * Allocator for Region map's attachments
	 */
	Genode::Allocator         &_md_alloc;
	/**
	 * Reference to Target_child's bootstrap phase
	 */
	bool                      &_bootstrap_phase;
	/**
	 * Name of the Region map for debugging
	 */
	const char*                _label;
	/**
	 * Wrapped region map from parent, usually core
	 */
	Genode::Region_map_client  _parent_region_map;

public:

  Region_map_component(Genode::Allocator &md_alloc,
		       Genode::Capability<Genode::Region_map>
		       region_map_cap,
		       Genode::size_t size,
		       const char *label,
		       bool &bootstrap_phase);

  ~Region_map_component();

  Genode::Capability<Genode::Region_map> parent_cap() { return _parent_region_map; }

  
	Region_map_component *find_by_badge(Genode::uint16_t badge);

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
