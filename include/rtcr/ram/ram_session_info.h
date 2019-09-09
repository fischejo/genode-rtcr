/*
 * \brief  Intercepting Ram session
 * \author Denis Huber
 * \date   2016-08-12
 */

#ifndef _RTCR_RAM_SESSION_INFO_H_
#define _RTCR_RAM_SESSION_INFO_H_

/* Rtcr includes */
#include <rtcr/ram/ram_dataspace_info.h>
#include <rtcr/info_structs.h>


namespace Rtcr {
	class Ram_session_info;
}


class Rtcr::Ram_session_info : public Session_info
{
public:
	Ram_dataspace_info *i_ram_dataspaces;
	Genode::Ram_session_capability   i_ref_account_cap;

	Ram_session_info(const char* creation_args, Genode::uint16_t badge)
		: Session_info(creation_args, badge) {}

	Ram_session_info() {}
	
	void print(Genode::Output &output) const {
		Genode::print(output, " Ram session:\n ");
		Session_info::print(output);
		Genode::print(output, "\n");
		Ram_dataspace_info *ds = i_ram_dataspaces;
		if(!ds) Genode::print(output, "  <empty>\n");
		while(ds) {
			Genode::print(output, "  ", ds);
			ds = ds->next();
		}
	}
};


#endif /* _RTCR_RAM_SESSION_INFO_H_ */
