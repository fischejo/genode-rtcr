/*
 * \brief  Intercepting Log session
 * \author Denis Huber
 * \author Johannes Fischer
 * \date   2019-08-29
 */

#ifndef _RTCR_LOG_SESSION_INFO_H_
#define _RTCR_LOG_SESSION_INFO_H_

/* Rtcr includes */
#include <rtcr/info_structs.h>


namespace Rtcr {
	class Log_session_info;
}


struct Rtcr::Log_session_info : Session_info {
	Log_session_info(const char* creation_args, Genode::uint16_t badge)
		: Session_info(creation_args, badge) {}

	Log_session_info() {};
	
	void print(Genode::Output &output) const {
		Genode::print(output, " Log session:\n  ");
		Session_info::print(output);
		Genode::print(output, "\n");
		
	}
};

#endif /* _RTCR_LOG_SESSION_INFO_H_ */
