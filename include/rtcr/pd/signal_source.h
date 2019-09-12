/*
 * \brief  Monitoring PD::alloc_signal_source and PD::free_signal_source
 * \author Denis Huber
 * \date   2016-10-06
 */

#ifndef _RTCR_SIGNAL_SOURCE_H_
#define _RTCR_SIGNAL_SOURCE_H_

/* Genode includes */
#include <base/capability.h>

/* Rtcr includes */
#include <rtcr/pd/signal_source_info.h>

namespace Rtcr {
	class Signal_source;
}

/**
 * List element to store Signal_source_capabilities created by the pd session
 */
class Rtcr::Signal_source : public Signal_source_info
{
public:	
	Genode::Capability<Genode::Signal_source> const cap;
	bool bootstrapped;
	
	Signal_source(Genode::Capability<Genode::Signal_source> cap,
			bool bootstrapped)
		:
		Signal_source_info(cap.local_name()),
		cap(cap),
		bootstrapped(bootstrapped)
		{ }
};

#endif /* _RTCR_SIGNAL_SOURCE_H_ */
