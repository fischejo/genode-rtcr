/*
 * \brief  Container for child objects
 * \author Johannes Fischer
 * \date   2019-09-02
 */

#ifndef _RTCR_CHILD_INFO_H_
#define _RTCR_CHILD_INFO_H_

#include <rtcr/cpu/cpu_session_info.h>
#include <rtcr/pd/pd_session_info.h>
#include <rtcr/rm/rm_session_info.h>
#include <rtcr/log/log_session_info.h>
#include <rtcr/timer/timer_session_info.h>
#include <rtcr/rom/rom_session_info.h>
#include <rtcr/cap/capability_mapping.h>
#include <util/list.h>

namespace Rtcr {
	class Cpu_session_info;
	class Pd_session_info;
	class Log_session_info;
	class Timer_session_info;
	class Rm_session_info;
	class Rom_session_info;
	class Capability_mapping;	
	struct Child_info;
}

using namespace Rtcr;




struct Rtcr::Child_info : Genode::List<Child_info>::Element
{
	Genode::String<100> name;
	bool bootstrapped;
	
	Cpu_session_info *cpu_session;
	Pd_session_info *pd_session;
	Log_session_info *log_session;
	Timer_session_info *timer_session;
	Rm_session_info *rm_session;
	Rom_session_info *rom_session;
	Capability_mapping *capability_mapping;

	Child_info(const char* _name) : name(_name) {};
	~Child_info() {};	
	
	Child_info *find_by_name(const char *_name);
	bool child_destroyed();

	void print(Genode::Output &output) const;
};


#endif /* _RTCR_CHILD_INFO_H_ */	
