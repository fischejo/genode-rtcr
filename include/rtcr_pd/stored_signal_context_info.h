/*
 * \brief  Structure for storing signal context information
 * \author Denis Huber
 * \date   2016-10-26
 */

#ifndef _RTCR_STORED_SIGNAL_CONTEXT_INFO_H_
#define _RTCR_STORED_SIGNAL_CONTEXT_INFO_H_

/* Genode includes */
#include <util/list.h>

/* Rtcr includes */
#include <rtcr_pd/signal_context_info.h>
#include <rtcr/stored_info_structs.h>

namespace Rtcr {
	struct Stored_signal_context_info;
}

struct Rtcr::Stored_signal_context_info : Stored_normal_info, Genode::List<Stored_signal_context_info>::Element
{
	Genode::uint16_t const signal_source_badge;
	unsigned long    const imprint;

	Stored_signal_context_info(Signal_context_info &info, Genode::addr_t targets_kcap)
	:
		Stored_normal_info(targets_kcap,
				info.cap.local_name(),
				info.bootstrapped),
		signal_source_badge(info.ss_cap.local_name()),
		imprint(info.imprint)
	{ }

	Stored_signal_context_info(Genode::addr_t kcap,
                                        Genode::uint16_t local_name,
                                        bool bootstrapped,
					Genode::uint16_t _signal_source_badge, unsigned long _imprint)
	:
		Stored_normal_info(kcap,local_name,bootstrapped),
		signal_source_badge(_signal_source_badge),
                imprint(_imprint)
	{ }

	Stored_signal_context_info *find_by_badge(Genode::uint16_t badge)
	{
		if(badge == this->badge)
			return this;
		Stored_signal_context_info *info = next();
		return info ? info->find_by_badge(badge) : 0;
	}

	void print(Genode::Output &output) const
	{
		using Genode::Hex;

		Stored_normal_info::print(output);
		Genode::print(output, ", signal_source_badge=", signal_source_badge, ", imprint=", Hex(imprint));
	}

};

#endif /* _RTCR_STORED_SIGNAL_CONTEXT_INFO_H_ */
