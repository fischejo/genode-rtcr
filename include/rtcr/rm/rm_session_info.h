/*
 * \brief  Intercepting Rm session
 * \author Denis Huber
 * \author Johannes Fischer
 * \date   2019-08-29
 */

#ifndef _RTCR_RM_SESSION_INFO_H_
#define _RTCR_RM_SESSION_INFO_H_

/* Rtcr includes */
#include <rtcr/rm/region_map_info.h>
#include <rtcr/info_structs.h>


namespace Rtcr {
	class Rm_session_info;
}

class Rtcr::Rm_session_info : public Session_info {
 public:	
	Region_map_info *i_region_maps;

 Rm_session_info(const char* creation_args, Genode::uint16_t badge)
	 : Session_info(creation_args, badge) {}
	
	void print(Genode::Output &output) const {
		Genode::print(output, " RM session:\n  ");
		Session_info::print(output);
		Genode::print(output, "\n");		
		const Region_map_info *rm = i_region_maps;
		if(!rm) Genode::print(output, "   <empty>\n");
		while(rm) {
			Genode::print(output, "   ", rm);
			rm = rm->next();
		}
	}
};

#endif /* _RTCR_RM_SESSION_INFO_H_ */
