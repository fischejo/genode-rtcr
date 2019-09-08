/*
 * \brief  Intercepting timer session
 * \author Denis Huber
 * \author Johannes Fischer
 * \date   2019-08-29
 */

#ifndef _RTCR_TIMER_SESSION_INFO_H_
#define _RTCR_TIMER_SESSION_INFO_H_

/* Rtcr includes */
#include <rtcr/info_structs.h>

namespace Rtcr {
	class Timer_session_info;
}

struct Rtcr::Timer_session_info : Session_info {
	unsigned i_timeout;
	bool i_periodic;
	Genode::uint16_t i_sigh_badge;

 Timer_session_info(const char* creation_args, Genode::uint16_t badge)
	 : Session_info(creation_args, badge) {}
	
	void print(Genode::Output &output) const {
		Genode::print(output, " Timer session:\n  ");
		Session_info::print(output);
		Genode::print(output,
					  ", sigh_badge ", i_sigh_badge,
					  ", timeout=", i_timeout,
					  ", periodic=", i_periodic, "\n");
	}
};


#endif /* _RTCR_TIMER_SESSION_INFO_H_ */
