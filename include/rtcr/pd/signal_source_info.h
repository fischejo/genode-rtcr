/*
 * \brief  Monitoring PD::alloc_signal_source and PD::free_signal_source
 * \author Denis Huber
 * \date   2016-10-06
 */

#ifndef _RTCR_SIGNAL_SOURCE_INFO_H_
#define _RTCR_SIGNAL_SOURCE_INFO_H_

/* Genode includes */
#include <util/list.h>
#include <util/fifo.h>
#include <base/capability.h>

/* Rtcr includes */

namespace Rtcr {
	class Signal_source_info;
}

class Rtcr::Signal_source_info : public Normal_info,
                                 public Genode::List<Signal_source_info>::Element,
                                 public Genode::Fifo<Signal_source_info>::Element
{
public:
	using Genode::List<Signal_source_info>::Element::next;

	Signal_source_info(Genode::uint16_t badge) : Normal_info(badge) {};

	Signal_source_info() {};

	Signal_source_info *find_by_badge(Genode::uint16_t badge) {
		if(badge == i_badge)
			return this;
		Signal_source_info *info = next();
		return info ? info->find_by_badge(badge) : 0;
	}
};

#endif /* _RTCR_SIGNAL_SOURCE_INFO_H_ */
