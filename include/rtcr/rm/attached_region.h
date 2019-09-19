/*
 * \brief  Monitoring RM::attach and RM::detach
 * \author Denis Huber
 * \author Johannes Fischer
 * \date   2019-08-29
 */

#ifndef _RTCR_ATTACHED_REGION_H_
#define _RTCR_ATTACHED_REGION_H_

/* Genode includes */
#include <dataspace/capability.h>

/* Rtcr includes */
#include <rtcr/rm/attached_region_info.h>

namespace Rtcr {
	class Attached_region;
}

/**
 * Record of an attached dataspace
 */
class Rtcr::Attached_region : public Rtcr::Attached_region_info
{
public:
	const Genode::Dataspace_capability attached_ds_cap;
	bool bootstrapped;
 
	Attached_region(Genode::Dataspace_capability attached_ds_cap,
	                Genode::size_t size,
	                Genode::off_t offset,
	                Genode::addr_t local_addr,
	                bool executable,
	                bool bootstrapped)
		:
		Attached_region_info(size,
		                     offset,
		                     local_addr,
		                     executable,
		                     attached_ds_cap.local_name()),
		attached_ds_cap(attached_ds_cap),
		bootstrapped(bootstrapped)
	{}
};


#endif /* _RTCR_ATTACHED_REGION_H_ */
