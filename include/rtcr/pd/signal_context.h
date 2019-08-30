/*
 * \brief  Monitoring PD::alloc_context and PD::free_context
 * \author Denis Huber
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
	struct Signal_context;
}

/**
 * List element to store Signal_context_capabilities created by the pd session
 */
struct Rtcr::Signal_context : Genode::List<Signal_context>::Element,
			Genode::Fifo<Signal_context>::Element
{
	/******************
	 ** COLD STORAGE **
	 ******************/
	
  	Genode::uint16_t ck_signal_source_badge;
	unsigned long ck_imprint;

	bool ck_bootstrapped;
	Genode::addr_t ck_kcap;
	Genode::uint16_t ck_badge;

	/*****************
	 ** HOT STORAGE **
	 *****************/
	
	// Creation arguments and result
	Genode::Signal_context_capability         const cap;
	Genode::Capability<Genode::Signal_source> const ss_cap;
	unsigned long                             const imprint;
	bool bootstrapped;

	/**
	 * List and Fifo provide a next() method. In general, you want to use the
	 * list implementation.
	 */	
	using Genode::List<Signal_context>::Element::next;

	
	Signal_context(Genode::Signal_context_capability sc_cap,
						Genode::Capability<Genode::Signal_source> ss_cap,
						unsigned long imprint,
						bool bootstrapped)
		:
		bootstrapped(bootstrapped),
		cap     (sc_cap),
		ss_cap  (ss_cap),
		imprint (imprint)
		{ }

	void checkpoint() {
		ck_badge = cap.local_name();
		ck_bootstrapped = bootstrapped;
		ck_imprint = imprint;
		ck_signal_source_badge = cap.local_name();
	}
  
	Signal_context *find_by_badge(Genode::uint16_t badge) {
		if(badge == cap.local_name())
			return this;
		Signal_context *info = next();
		return info ? info->find_by_badge(badge) : 0;
	}

	void print(Genode::Output &output) const {
		using Genode::Hex;

		Genode::print(output, "sc ", cap, ", ss ", ss_cap, ", imprint=",
					  Hex(imprint), ", ");
	}
};


#endif /* _RTCR_SIGNAL_CONTEXT_INFO_H_ */
