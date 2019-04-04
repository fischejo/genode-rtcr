/*
 * \brief  Monitoring ram dataspace creation/destruction
 * \author Denis Huber
 * \date   2016-10-06
 */

#ifndef _RTCR_RAM_DATASPACE_INFO_H_
#define _RTCR_RAM_DATASPACE_INFO_H_

/* Genode includes */
#include <util/list.h>
#include <ram_session/ram_session.h>
#include <region_map/client.h>

/* Rtcr includes */
#include <rtcr/info_structs.h>

namespace Rtcr {
	struct Ram_dataspace_info;
	struct Managed_region_map_info;
	struct Designated_dataspace_info;

	constexpr bool dd_verbose_debug = false;
}

/**
 * Monitors allocated Ram dataspaces
 */
struct Rtcr::Ram_dataspace_info : Normal_obj_info, private Simple_counter<Ram_dataspace_info>,
                                  Genode::List<Ram_dataspace_info>::Element
{
	/**
	 * Allocated Ram dataspace
	 */
	Genode::Ram_dataspace_capability const cap;
	Genode::size_t                   const size;
	Genode::Cache_attribute          const cached;
	/**
	 * If the pointer is null, then this is an ordinary Ram dataspace.
	 * If the pointer is not null, then this Ram dataspace is managed.
	 * A managed Ram dataspace is a Region map consisting of designated dataspaces.
	 */
	Managed_region_map_info *mrm_info;

	Ram_dataspace_info(Genode::Ram_dataspace_capability ds_cap, Genode::size_t size, Genode::Cache_attribute cached,
			bool bootstrapped, Managed_region_map_info *mrm_info = nullptr)
	:
		Normal_obj_info (bootstrapped),
		cap      (ds_cap),
		size     (size),
		cached   (cached),
		mrm_info (mrm_info)
	{ }

	Ram_dataspace_info *find_by_badge(Genode::uint16_t badge)
	{
		if(badge == cap.local_name())
			return this;
		Ram_dataspace_info *info = next();
		return info ? info->find_by_badge(badge) : 0;
	}

	Ram_dataspace_info *find_by_timestamp(Genode::size_t timestamp)
	{
		if(timestamp == this->timestamp())
			return this;
		Ram_dataspace_info *info = next();
		return info ? info->find_by_timestamp(timestamp) : 0;
	}

	Genode::size_t timestamp() const
	{
		return Simple_counter<Ram_dataspace_info>::id();
	}

	void print(Genode::Output &output) const
	{
		using Genode::Hex;

		Genode::print(output, cap, ", size=", Hex(size), ", cached=", static_cast<unsigned>(cached),
				", timestamp=", timestamp(), ", ");
		Normal_obj_info::print(output);
	}
};


/**
 * This struct holds information about a Region map, its designated Ram dataspaces
 */
struct Rtcr::Managed_region_map_info
{
	/**
	 * Region_map which is monitored
	 */
	Genode::Capability<Genode::Region_map> const region_map_cap;
	/**
	 * List of designated Ram dataspaces
	 */
	Genode::List<Designated_dataspace_info> dd_infos;
	/**
	 * Signal context for receiving page faults
	 */
	Genode::Signal_context context;

	Managed_region_map_info(Genode::Capability<Genode::Region_map> region_map_cap)
	:
		region_map_cap(region_map_cap), dd_infos(), context()
	{ }

};


/**
 * A Designated_dataspace_info belongs to a Managed_region_map_info
 * It holds information about the address in the region map, the size,
 * and whether it is attached in the region map
 */
struct Rtcr::Designated_dataspace_info : public Genode::List<Designated_dataspace_info>::Element
{
	/**
	 * Enable log output for debugging
	 */
	static constexpr bool verbose_debug = dd_verbose_debug;

	/**
	 * Reference to the Managed_region_map_info to which this dataspace belongs to
	 */
	Managed_region_map_info      const &mrm_info;
	/**
	 * Dataspace which will be attached to / detached from the Managed_region_map_info's Region_map
	 */
	Genode::Dataspace_capability const  cap;
	/**
	 * Starting address of the dataspace; it is a relative address, because it is local
	 * to the Region_map to which it will be attached
	 */
	Genode::addr_t               const rel_addr;
	/**
	 * Size of the dataspace
	 */
	Genode::size_t               const size;
	/**
	 * Indicates whether this dataspace is attached to its Region_map
	 */
	bool attached;

	/**
	 * Constructor
	 */
	Designated_dataspace_info(Managed_region_map_info &mrm_info, Genode::Dataspace_capability ds_cap,
			Genode::addr_t addr, Genode::size_t size)
	:
		mrm_info(mrm_info), cap(ds_cap), rel_addr(addr), size(size), attached(false)
	{
		// Every new dataspace shall be attached and marked
		attach();
	}

	/**
	 * Find Attachable_dataspace_info which contains the address addr
	 *
	 * \param addr Local address of the corresponding Region_map
	 *
	 * \return Attachable_dataspace_info which contains the local address addr
	 */
	Designated_dataspace_info *find_by_addr(Genode::addr_t addr)
	{
		if((addr >= rel_addr) && (addr <= rel_addr + size))
			return this;
		Designated_dataspace_info *info = next();
		return info ? info->find_by_addr(addr) : 0;
	}

	void print(Genode::Output &output) const
	{
		using Genode::Hex;

		Genode::print(output, cap, ", rel_addr=", Hex(rel_addr), " size=", Hex(size));
	}
	/**
	 * Attach dataspace and mark it as attached
	 */
	void attach()
	{
		if(verbose_debug)
		{
			Genode::log("Attaching dataspace ", cap,
					" to region map ", mrm_info.region_map_cap,
					" on location ", Genode::Hex(rel_addr));
		}

		if(!attached)
		{
			// Attaching Dataspace to designated location
			Genode::addr_t addr =
					Genode::Region_map_client{mrm_info.region_map_cap}.attach_at(cap, rel_addr);

			// Dataspace was not attached on the right location
			if(addr != rel_addr)
			{
				Genode::warning("Designated_dataspace_info::attach Dataspace was not attached on its designated location!");
				Genode::warning("  designated", Genode::Hex(rel_addr), " != attached=", Genode::Hex(addr));
			}

			// Mark as attached
			attached = true;
		}
		else
		{
			Genode::warning("Designated_dataspace_info::attach Trying to attach an already attached Dataspace:",
					" DS ", cap,
					" RM ", mrm_info.region_map_cap,
					" Loc ", Genode::Hex(rel_addr));
		}
	}
	/**
	 * Detach dataspace and mark it as not attached
	 */
	void detach()
	{
		if(verbose_debug)
		{
			Genode::log("Detaching dataspace ", cap,
					" from region map ", mrm_info.region_map_cap,
					" on location ", Genode::Hex(rel_addr), " (local to Region_map)");
		}

		if(attached)
		{
			// Detaching Dataspace
			Genode::Region_map_client{mrm_info.region_map_cap}.detach(rel_addr);

			// Mark as detached
			attached = false;
		}
		else
		{
			Genode::warning("Trying to detach an already detached Dataspace:",
					" DS ", cap,
					" RM ", mrm_info.region_map_cap,
					" Loc ", Genode::Hex(rel_addr));
		}
	}
};

#endif /* _RTCR_RAM_DATASPACE_INFO_H_ */
