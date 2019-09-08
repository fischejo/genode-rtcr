/*
 * \brief  Intercepting Region map
 * \author Denis Huber
 * \author Johannes Fischer
 * \date   2019-08-29
 */

#ifndef _RTCR_REGION_MAP_INFO_H_
#define _RTCR_REGION_MAP_INFO_H_

/* Genode includes */
#include <util/list.h>
#include <util/fifo.h>

/* Rtcr includes */
#include <rtcr/rm/attached_region_info.h>
#include <rtcr/info_structs.h>

namespace Rtcr {
	class Region_map_info;
}
	
class Rtcr::Region_map_info : public Normal_info,
							  public Genode::List<Region_map_info>::Element,
							  public Genode::Fifo<Region_map_info>::Element
{
public:
	using Genode::List<Region_map_info>::Element::next;
	
	Genode::size_t i_size;
	Genode::uint16_t i_sigh_badge;
	Genode::uint16_t i_ds_badge; 	
	Attached_region_info *i_attached_regions;

	Region_map_info(Genode::uint16_t badge) : Normal_info(badge) {};
											  
	void print(Genode::Output &output) const {
		Genode::print(output, "  Region Map:\n");
		Normal_info::print(output);
		Genode::print(output,
					  ", size=", i_size,
					  ", ds_badge=", i_ds_badge,
					  ", sigh_badge=", i_sigh_badge);

		Attached_region_info *rm = i_attached_regions;
		if(!rm) Genode::print(output, "   <empty>\n");
		while(rm) {
			Genode::print(output, "   ", rm);
			rm = rm->next();
		}
	}

	Region_map_info *find_by_badge(Genode::uint16_t badge) {
		if(badge == i_badge)
			return this;
		Region_map_info *obj = next();
			return obj ? obj->find_by_badge(badge) : 0;
	}
	
};


#endif /* _RTCR_REGION_MAP_INFO_H_ */
