/*
 * \brief  Monitoring RM::attach and RM::detach
 * \author Denis Huber
 * \author Johannes Fischer
 * \date   2019-08-29
 */

#ifndef _RTCR_ATTACHED_REGION_INFO_H_
#define _RTCR_ATTACHED_REGION_INFO_H_

/* Genode includes */
#include <util/list.h>
#include <util/fifo.h>
#include <dataspace/capability.h>

/* Rtcr includes */
#include <rtcr/info_structs.h>

namespace Rtcr {
	class Attached_region_info;
}


class Rtcr::Attached_region_info : public Rtcr::Normal_info,
								   public Genode::List<Attached_region_info>::Element,
								   public Genode::Fifo<Attached_region_info>::Element
{
public:
	using Genode::List<Attached_region_info>::Element::next;
													  
	Genode::Ram_dataspace_capability i_memory_content;
	/**
	 * Size of occupied region
	 */
	const Genode::size_t i_size;
	/**
	 * Offset in occupied region
	 */
	const Genode::off_t  i_offset;
	/**
	 * Address of occupied region
	 */
	const Genode::addr_t i_rel_addr;
	/**
	 * Indicates whether occupied region is executable
	 */
	const bool i_executable;

	Attached_region_info(Genode::size_t size,
						 Genode::off_t offset,
						 Genode::addr_t local_addr,
						 bool executable,
						 Genode::uint16_t badge)
		:
		Normal_info(badge),
		i_size       (size),
		i_offset     (offset),
		i_rel_addr   (local_addr),
		i_executable (executable) {}

	
	void print(Genode::Output &output) const {
		using Genode::Hex;		
		Normal_info::print(output),
		Genode::print(output, ", [", Hex(i_rel_addr, Hex::PREFIX, Hex::PAD));
		Genode::print(output, ", ", Hex(i_rel_addr + i_size - i_offset, Hex::PREFIX, Hex::PAD));
		Genode::print(output, ") exec=", i_executable, "\n");
	}


	Attached_region_info *find_by_addr(Genode::addr_t addr) {
		if((addr >= i_rel_addr) && (addr <= i_rel_addr + i_size))
			return this;
		Attached_region_info *info = next();
		return info ? info->find_by_addr(addr) : 0;
	}
  
	Attached_region_info *find_by_badge(Genode::uint16_t badge) {
		if(badge == i_badge)
			return this;
		Attached_region_info *info = next();
		return info ? info->find_by_badge(badge) : 0;
	}

	
};


#endif /* _RTCR_ATTACHED_REGION_INFO_H_ */
