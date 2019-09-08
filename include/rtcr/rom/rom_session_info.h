
/*
 * \brief  Intercepting ROM session
 * \author Denis Huber
 * \author Johannes Fischer
 * \date   2019-08-29
 */

#ifndef _RTCR_ROM_SESSION_INFO_H_
#define _RTCR_ROM_SESSION_INFO_H_

/* Genode includes */

/* Rtcr includes */
#include <rtcr/info_structs.h>


namespace Rtcr {
	class Rom_session_info;
}


struct Rtcr::Rom_session_info : Session_info {

	Genode::uint16_t i_dataspace_badge;
	Genode::uint16_t i_sigh_badge;

 Rom_session_info(const char* creation_args, Genode::uint16_t badge)
	 : Session_info(creation_args, badge) {}
	
	void print(Genode::Output &output) const {
		Genode::print(output, " Rom session:\n  ");
		Session_info::print(output);
		Genode::print(output,
					  " dataspace_badge=", i_dataspace_badge,
					  ", sigh_badge=", i_sigh_badge, "'\n");
	}
};

#endif /* _RTCR_ROM_SESSION_INFO_H_ */
