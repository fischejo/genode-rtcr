/*
 * \brief  Monitoring PD::alloc_rpc_cap and PD::free_rpc_cap
 * \author Denis Huber
 * \date   2016-10-06
 */

#ifndef _RTCR_NATIVE_CAPABILITY_H_
#define _RTCR_NATIVE_CAPABILITY_H_

/* Genode includes */
#include <base/native_capability.h>

/* Rtcr includes */
#include <rtcr/pd/native_capability_info.h>

namespace Rtcr {
	class Native_capability;
}


/**
 * List element to store a capability which is created by the pd session
 * They are usually created by client's entrypoint and therefore require
 * a Cpu_thread_capability
 */
class Rtcr::Native_capability : private Simple_counter<Native_capability>,
                                public Rtcr::Native_capability_info

{
public:	
	// Creation arguments and result
	Genode::Native_capability const cap;
	Genode::Native_capability const ep_cap;
	bool bootstrapped;

	Native_capability(Genode::Native_capability native_cap,
	                  Genode::Native_capability ep_cap,
	                  bool bootstrapped)
		:
		Native_capability_info(native_cap.local_name(),
		                       ep_cap.local_name()),
		cap    (native_cap),
		ep_cap (ep_cap),
		bootstrapped (bootstrapped)
	{ }
  
	Genode::size_t timestamp() const {
		return Simple_counter<Native_capability>::id();
	}

};


#endif /* _RTCR_NATIVE_CAPABILITY_H_ */
