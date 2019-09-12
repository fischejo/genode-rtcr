/*
 * \brief  Monitoring PD::alloc_rpc_cap and PD::free_rpc_cap
 * \author Denis Huber
 * \date   2016-10-06
 */

#ifndef _RTCR_NATIVE_CAPABILITY_INFO_H_
#define _RTCR_NATIVE_CAPABILITY_INFO_H_

/* Genode includes */
#include <util/list.h>
#include <util/fifo.h>

/* Rtcr includes */
#include <rtcr/info_structs.h>

namespace Rtcr {
	class Native_capability_info;	
}


class Rtcr::Native_capability_info : public Normal_info,
									 public Genode::List<Native_capability_info>::Element,
									 public Genode::Fifo<Native_capability_info>::Element
{
public:
	using Genode::List<Native_capability_info>::Element::next;
	
	Genode::uint16_t i_ep_badge;

	Native_capability_info(Genode::uint16_t badge,Genode::uint16_t ep_badge)
		:
		Normal_info(badge),
		i_ep_badge(ep_badge) {};

	Native_capability_info() {};
	
	void print(Genode::Output &output) const {
		using Genode::Hex;
		Normal_info::print(output);
		Genode::print(output, ", ep_badge=", i_ep_badge);
	}

	Native_capability_info *find_by_native_badge(Genode::uint16_t badge) {
		if(badge == i_badge)
			return this;
		Native_capability_info *info = next();
		return info ? info->find_by_native_badge(badge) : 0;
	}
};


#endif /* _RTCR_NATIVE_CAPABILITY_INFO_H_ */
