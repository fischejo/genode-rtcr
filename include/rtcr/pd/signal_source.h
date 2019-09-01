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
	struct Signal_source;
	struct Signal_source_info;
}

struct Rtcr::Signal_source_info : Normal_info {};



/**
 * List element to store Signal_source_capabilities created by the pd session
 */
struct Rtcr::Signal_source : Genode::List<Signal_source>::Element,
			Genode::Fifo<Signal_source>::Element
{
	/******************
	 ** COLD STORAGE **
	 ******************/
	Signal_source_info info;

	/*****************
	 ** HOT STORAGE **
	 *****************/
	

	bool bootstrapped;
	// Creation result
	Genode::Capability<Genode::Signal_source> const cap;

	/**
	 * List and Fifo provide a next() method. In general, you want to use the
	 * list implementation.
	 */	
	using Genode::List<Signal_source>::Element::next;

	
	Signal_source(Genode::Capability<Genode::Signal_source> cap, bool bootstrapped)
		:
		bootstrapped(bootstrapped),
		cap(cap)
		{ }

  
	void checkpoint() {
		info.badge = cap.local_name();
		info.bootstrapped = bootstrapped;
	}

  
	Signal_source *find_by_badge(Genode::uint16_t badge) {
		if(badge == cap.local_name())
			return this;
		Signal_source *info = next();
		return info ? info->find_by_badge(badge) : 0;
	}

	void print(Genode::Output &output) const {
		using Genode::Hex;

		Genode::print(output, cap, ", ");
	}
};

#endif /* _RTCR_SIGNAL_SOURCE_INFO_H_ */
