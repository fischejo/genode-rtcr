/*
 * \brief  Monitoring ram dataspace creation/destruction
 * \author Denis Huber
 * \author Johannes Fischer
 * \date   2019-08-29
 */

#ifndef _RTCR_RAM_DATASPACE_INFO_H_
#define _RTCR_RAM_DATASPACE_INFO_H_

/* Genode includes */
#include <util/list.h>
#include <util/fifo.h>
#include <ram_session/ram_session.h>

/* Rtcr includes */
#include <rtcr/info_structs.h>


namespace Rtcr {
	class Ram_dataspace_info;
}


class Rtcr::Ram_dataspace_info : public Rtcr::Normal_info,
								 public Genode::List<Ram_dataspace_info>::Element,
								 public Genode::Fifo<Ram_dataspace_info>::Element
{
public:
	using Genode::List<Ram_dataspace_info>::Element::next;
	
	Genode::Ram_dataspace_capability i_dst_cap;
	Genode::size_t i_timestamp;
	Genode::Ram_dataspace_capability const i_cap;
	Genode::size_t                   const i_size;
	Genode::Cache_attribute          const i_cached;

	Ram_dataspace_info(Genode::Ram_dataspace_capability const cap,
					   Genode::size_t const size,
					   Genode::Cache_attribute const cached)
		: Normal_info(cap.local_name()),
		  i_cap(cap), i_size(size), i_cached(cached) {}
	
	void print(Genode::Output &output) const {
		using Genode::Hex;
		Normal_info::print(output);
		Genode::print(output,
					  ", cap=", i_cap,
					  ", size=", Hex(i_size),
					  ", cached=", static_cast<unsigned>(i_cached),
				  ", timestamp=", i_timestamp, "\n");
	}

	Ram_dataspace_info *find_by_badge(Genode::uint16_t badge) {
		if(badge == i_cap.local_name())
			return this;
		Ram_dataspace_info *info = next();
		return info ? info->find_by_badge(badge) : 0;
	}

	Ram_dataspace_info *find_by_timestamp(Genode::size_t timestamp) {
		if(timestamp == i_timestamp)
			return this;
		Ram_dataspace_info *info = next();
		return info ? info->find_by_timestamp(timestamp) : 0;
	}
	
};

#endif /* _RTCR_RAM_DATASPACE_INFO_H_ */
