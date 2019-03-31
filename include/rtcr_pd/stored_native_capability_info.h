/*
 * \brief  Structure for storing signal context information
 * \author Denis Huber
 * \date   2016-10-26
 */

#ifndef _RTCR_STORED_NATIVE_CAPABILITY_INFO_H_
#define _RTCR_STORED_NATIVE_CAPABILITY_INFO_H_

/* Genode includes */
#include <util/list.h>

/* Rtcr includes */
#include <rtcr_pd/native_capability_info.h>
#include <rtcr/stored_info_structs.h>

namespace Rtcr {
	struct Stored_native_capability_info;
}


struct Rtcr::Stored_native_capability_info : Stored_normal_info, Genode::List<Stored_native_capability_info>::Element
{
	Genode::uint16_t const ep_badge;

	Stored_native_capability_info(Native_capability_info &info, Genode::addr_t targets_kcap)
	:
		Stored_normal_info(targets_kcap, info.cap.local_name(), info.bootstrapped),
		ep_badge(info.ep_cap.local_name())
	{ }

	Stored_native_capability_info(Genode::addr_t kcap,
                                        Genode::uint16_t local_name,
                                        bool bootstrapped,
					Genode::uint16_t _ep_badge)
	:
		Stored_normal_info(kcap,local_name,bootstrapped),
		ep_badge(_ep_badge)
	{ }

	Stored_native_capability_info *find_by_badge(Genode::uint16_t badge)
	{
		if(badge == this->badge)
			return this;
		Stored_native_capability_info *info = next();
		return info ? info->find_by_badge(badge) : 0;
	}

	void print(Genode::Output &output) const
	{
		using Genode::Hex;

		Stored_normal_info::print(output);
		Genode::print(output, ", ep_badge=", ep_badge);
	}

};

#endif /* _RTCR_STORED_NATIVE_CAPABILITY_INFO_H_ */
