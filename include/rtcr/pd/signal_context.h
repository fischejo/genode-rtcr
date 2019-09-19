/*
 * \brief  Monitoring PD::alloc_context and PD::free_context
 * \author Denis Huber
 * \date   2016-10-06
 */

#ifndef _RTCR_SIGNAL_CONTEXT_H_
#define _RTCR_SIGNAL_CONTEXT_H_

/* Genode includes */
#include <base/signal.h>

/* Rtcr includes */
#include <rtcr/pd/signal_context_info.h>

namespace Rtcr {
	class Signal_context;
}

/**
 * List element to store Signal_context_capabilities created by the pd session
 */
class Rtcr::Signal_context : public Signal_context_info
{
public:
	// Creation arguments and result
	Genode::Signal_context_capability         const cap;
	Genode::Capability<Genode::Signal_source> const ss_cap;
	unsigned long                             const imprint;
	bool bootstrapped;

	Signal_context(Genode::Signal_context_capability sc_cap,
	               Genode::Capability<Genode::Signal_source> ss_cap,
	               unsigned long imprint,
	               bool bootstrapped)
		:
		Signal_context_info(sc_cap.local_name(), ss_cap.local_name()),
		cap     (sc_cap),
		ss_cap  (ss_cap),
		imprint (imprint),
		bootstrapped(bootstrapped)
	{ }

	void checkpoint() {
		i_imprint = imprint;
	}
};


#endif /* _RTCR_SIGNAL_CONTEXT_H_ */
