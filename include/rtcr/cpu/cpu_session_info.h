/*
 * \brief  Intercepting Cpu session
 * \author Denis Huber
 * \author Johannes Fischer
 * \date   2019-08-29
 */

#ifndef _RTCR_CPU_SESSION_INFO_H_
#define _RTCR_CPU_SESSION_INFO_H_

/* Genode includes */

/* Rtcr includes */
#include <rtcr/cpu/cpu_thread_info.h>
#include <rtcr/info_structs.h>


namespace Rtcr {
	class Cpu_session_info;
}

class Rtcr::Cpu_session_info : public Rtcr::Session_info
{
 public:
	Cpu_thread_info *i_cpu_thread_info;
	Genode::uint16_t i_sigh_badge;

 Cpu_session_info(const char* creation_args, Genode::uint16_t badge)
	 : Session_info(creation_args, badge) {}

	Cpu_session_info() {}
	
	void print(Genode::Output &output) const {
		Genode::print(output, " CPU session:\n  ");
		Session_info::print(output);
		Genode::print(output, "\n");
		
		Cpu_thread_info *cpu_thread = i_cpu_thread_info;
		if(!cpu_thread) Genode::print(output, "  <empty>\n");
		while(cpu_thread) {
			Genode::print(output, "  ", cpu_thread, "\n");
			cpu_thread = cpu_thread->next();
		}
	}
};


#endif /* _RTCR_CPU_SESSION_INFO_H_ */
