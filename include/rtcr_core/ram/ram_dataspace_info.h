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

	Ram_dataspace_info(Genode::Ram_dataspace_capability ds_cap, Genode::size_t size, Genode::Cache_attribute cached,
			   bool bootstrapped)
		:
		Normal_obj_info (bootstrapped),
		cap      (ds_cap),
		size     (size),
		cached   (cached)
	{ }

	/* one method should be virtual, otherwise this class is not polymorphic
	 * and therefore dynamic_cast can't be applied. */
	virtual ~Ram_dataspace_info() {};
		
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



#endif /* _RTCR_RAM_DATASPACE_INFO_H_ */
