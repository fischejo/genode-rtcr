/*
 * \brief  Monitoring PD::alloc_context and PD::free_context
 * \author Denis Huber
 * \author Johannes Fischer
 * \date   2016-10-06
 */

#ifndef _RTCR_SIGNAL_CONTEXT_INFO_H_
#define _RTCR_SIGNAL_CONTEXT_INFO_H_

/* Genode includes */
#include <util/list.h>
#include <util/fifo.h>
#include <base/signal.h>

/* Rtcr includes */
#include <rtcr/info_structs.h>

namespace Rtcr {
	class Signal_context_info;
}


class Rtcr::Signal_context_info : public Normal_info,
								  public Genode::List<Signal_context_info>::Element,
								  public Genode::Fifo<Signal_context_info>::Element
{
public:	
	using Genode::List<Signal_context_info>::Element::next;
	
  	Genode::uint16_t i_signal_source_badge;
	unsigned long i_imprint;

	Signal_context_info(Genode::uint16_t badge) : Normal_info(badge) {};

	Signal_context_info() {};
	
	void print(Genode::Output &output) const {
		using Genode::Hex;
		Normal_info::print(output);
		Genode::print(output,
					  ", signal_source_badge=", i_signal_source_badge,
					  ", imprint=",Hex(i_imprint));
	}

	Signal_context_info *find_by_badge(Genode::uint16_t badge) {
		if(badge == i_badge)
			return this;
		Signal_context_info *info = next();
		return info ? info->find_by_badge(badge) : 0;
	}
	
};


#endif /* _RTCR_SIGNAL_CONTEXT_INFO_H_ */
