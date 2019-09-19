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
	Genode::Ram_dataspace_capability i_src_cap;
	Genode::size_t                   i_size;
	Genode::Cache_attribute          i_cached;

	Ram_dataspace_info(Genode::Ram_dataspace_capability const src_cap,
					   Genode::size_t const size,
					   Genode::Cache_attribute const cached)
		: Normal_info(src_cap.local_name()),
		  i_src_cap(src_cap), i_size(size), i_cached(cached) {}

	Ram_dataspace_info() {};
	
	void print(Genode::Output &output) const {
		using Genode::Hex;
		Normal_info::print(output);
		Genode::print(output,
					  ", src_cap=", i_src_cap,
					  ", dst_cap=", i_dst_cap,					  
					  ", size=", Hex(i_size),
					  ", cached=", static_cast<unsigned>(i_cached),
				  ", timestamp=", i_timestamp, "\n");
	}

	Ram_dataspace_info *find_by_badge(Genode::uint16_t badge) {
		if(badge == i_src_cap.local_name())
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
