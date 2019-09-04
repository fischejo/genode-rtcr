/*
 * \brief  Container for child objects
 * \author Johannes Fischer
 * \date   2019-09-02
 */

#ifndef _RTCR_CHILD_INFO_H_
#define _RTCR_CHILD_INFO_H_

#include <rtcr/cpu/cpu_session.h>
#include <rtcr/pd/pd_session.h>
#include <rtcr/ram/ram_session.h>
#include <rtcr/rm/rm_session.h>
#include <rtcr/log/log_session.h>
#include <rtcr/timer/timer_session.h>
#include <rtcr/rom/rom_session.h>
#include <rtcr/cap/capability_mapping.h>
#include <util/list.h>

namespace Rtcr {
	class Ram_session;
	class Cpu_session;
	class Pd_session;
	class Log_session;
	class Timer_session;
	class Rm_session;
	class Rom_session;
	class Capability_mapping;	
	struct Child_info;
}

using namespace Rtcr;

struct Rtcr::Child_info : Genode::List<Child_info>::Element
{
	Genode::String<100> name;
	bool bootstrapped;
	
	Ram_session *ram_session;
	Cpu_session *cpu_session;
	Pd_session *pd_session;
	Log_session *log_session;
	Timer_session *timer_session;
	Rm_session *rm_session;
	Rom_session *rom_session;
	Capability_mapping *capability_mapping;

	Child_info(const char* _name) : name(_name) {};
	~Child_info() {};	
	
	Child_info *find_by_name(const char *_name);
	bool child_destroyed();

	void print(Genode::Output &output) const;
};


#endif /* _RTCR_CHILD_INFO_H_ */	
