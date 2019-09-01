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
#include <base/native_capability.h>

/* Rtcr includes */
#include <rtcr/info_structs.h>

namespace Rtcr {
	struct Native_capability;
	struct Native_capability_info;	
}


struct Rtcr::Native_capability_info : Normal_info {
	Genode::uint16_t ep_badge;

	void print(Genode::Output &output) const {
		using Genode::Hex;
		Normal_info::print(output);
		Genode::print(output, ", ep_badge=", ep_badge);

	}	
};


/**
 * List element to store a capability which is created by the pd session
 * They are usually created by client's entrypoint and therefore require
 * a Cpu_thread_capability
 */
struct Rtcr::Native_capability : private Simple_counter<Native_capability>,
			Genode::List<Native_capability>::Element,
			Genode::Fifo<Native_capability>::Element			
{
	/******************
	 ** COLD STORAGE **
	 ******************/
	Native_capability_info info;


	/*****************
	 ** HOT STORAGE **
	 *****************/
	
	
	// Creation arguments and result
	Genode::Native_capability const cap;
	Genode::Native_capability const ep_cap;
	bool bootstrapped;

	/**
	 * List and Fifo provide a next() method. In general, you want to use the
	 * list implementation.
	 */	
	using Genode::List<Native_capability>::Element::next;

	
	Native_capability(Genode::Native_capability native_cap,
						   Genode::Native_capability ep_cap,
						   bool bootstrapped)
		:
		bootstrapped (bootstrapped),
		cap    (native_cap),
		ep_cap (ep_cap)
		{ }

	void checkpoint() {
		info.badge = cap.local_name();
		info.bootstrapped = bootstrapped;
		info.ep_badge = ep_cap.local_name();
	}
  
	Native_capability *find_by_native_badge(Genode::uint16_t badge) {
		if(badge == cap.local_name())
			return this;
		Native_capability *info = next();
		return info ? info->find_by_native_badge(badge) : 0;
	}

	Genode::size_t timestamp() const {
		return Simple_counter<Native_capability>::id();
	}

	void print(Genode::Output &output) const {
		using Genode::Hex;

		Genode::print(output, "native ", cap, ", ep ", ep_cap, ", timestamp=",
					  timestamp(), ", ");
	}
};


#endif /* _RTCR_NATIVE_CAPABILITY_INFO_H_ */
