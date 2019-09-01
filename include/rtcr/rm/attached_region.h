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
	struct Attached_region;
	struct Attached_region_info;
}


struct Rtcr::Attached_region_info : Normal_info {
	Genode::Ram_dataspace_capability memory_content;
    Genode::uint16_t attached_ds_badge;
	/**
	 * Size of occupied region
	 */
	const Genode::size_t size;
	/**
	 * Offset in occupied region
	 */
	const Genode::off_t  offset;
	/**
	 * Address of occupied region
	 */
	const Genode::addr_t rel_addr;
	/**
	 * Indicates whether occupied region is executable
	 */
	const bool executable;

	Attached_region_info(Genode::size_t size,
						 Genode::off_t offset,
						 Genode::addr_t local_addr,
						 bool executable)
		:
		size       (size),
		offset     (offset),
		rel_addr   (local_addr),
		executable (executable) {}

	
	void print(Genode::Output &output) const {
		using Genode::Hex;		
		Genode::print(output, attached_ds_badge);
		Genode::print(output, " [", Hex(rel_addr, Hex::PREFIX, Hex::PAD));
		Genode::print(output, ", ", Hex(rel_addr + size - offset, Hex::PREFIX, Hex::PAD));
		Genode::print(output, ") exec=", executable, "\n");
	}

};


/**
 * Record of an attached dataspace
 */
struct Rtcr::Attached_region : Genode::List<Attached_region>::Element,
			Genode::Fifo<Attached_region>::Element
{
	/******************
	 ** COLD STORAGE **
	 ******************/
	Attached_region_info info;
	
	/*****************
	 ** HOT STORAGE **
	 *****************/
	const Genode::Dataspace_capability attached_ds_cap;
	/**
	 * List and Fifo provide a next() method. In general, you want to use the
	 * list implementation.
	 */
	using Genode::List<Attached_region>::Element::next;


	bool bootstrapped;

	void checkpoint() {
		info.bootstrapped = bootstrapped;
		info.attached_ds_badge = attached_ds_cap.local_name();
	}
  
  
	Attached_region(Genode::Dataspace_capability attached_ds_cap,
					     Genode::size_t size,
						 Genode::off_t offset,
						 Genode::addr_t local_addr,
						 bool executable,
						 bool bootstrapped)
		:
		bootstrapped (bootstrapped),
		info(size, offset, local_addr, executable),
		attached_ds_cap(attached_ds_cap)
		{}

	Attached_region *find_by_addr(Genode::addr_t addr) {
		Genode::log("find_by_addr attached_ds_cap=", attached_ds_cap,
					" rel_addr=", info.rel_addr,
					" size=", info.size);
		if((addr >= info.rel_addr) && (addr <= info.rel_addr + info.size))
			return this;
		Attached_region *info = next();
		return info ? info->find_by_addr(addr) : 0;
	}
  
	Attached_region *find_by_badge(Genode::uint16_t badge) {
		if(badge == attached_ds_cap.local_name())
			return this;
		Attached_region *info = next();
		return info ? info->find_by_badge(badge) : 0;
	}

};


#endif /* _RTCR_ATTACHED_REGION_INFO_H_ */
