/*
 * \brief  Core functionality for pausing/checkpoint/resuming 
 * \author Johannes Fischer
 * \date   2019-08-29
 */

#ifndef _RTCR_INIT_MODULE_H_
#define _RTCR_INIT_MODULE_H_

/* Genode includes */
#include <base/allocator.h>
#include <base/service.h>
#include <util/list.h>
#include <base/attached_rom_dataspace.h>
#include <base/registry.h>


/* Rtcr includes */
#include <rtcr/cpu/cpu_session.h>
#include <rtcr/pd/pd_session.h>
#include <rtcr/rm/rm_session.h>
#include <rtcr/log/log_session.h>
#include <rtcr/timer/timer_session.h>
#include <rtcr/rom/rom_session.h>
#include <rtcr/cap/capability_mapping.h>
#include <rtcr/child_info.h>

namespace Rtcr {
	class Init_module;
	typedef Genode::String<16> Module_name;
}


using namespace Rtcr;



/**
 * The class Rtcr::Init_module provides the intercepting sessions and handles
 * the communication between a target child and the corresponding sessions.
 */
class Rtcr::Init_module
{
protected:
	/**
	 * Entrypoint for managing child's resource-sessions (PD, CPU, RAM)
	 */
	Genode::Allocator &_alloc;
	Genode::Env &_env;

	Genode::Lock _childs_lock;
	Genode::List<Child_info> _childs;

	Genode::Registry<Genode::Service> _services;
	
	/**
	 * Rom dataspace holding configuration
	 */
	Genode::Attached_rom_dataspace _config;
	bool _parallel;
	inline bool read_parallel();

	void checkpoint(Child_info *child);

public:

	Init_module(Genode::Env &env, Genode::Allocator &alloc);
	~Init_module();


	Child_info *child_info(const char* name);
	Genode::List<Child_info> *child_info() { return &_childs; }

	Genode::Registry<Genode::Service> &services() {
		return _services;
	}
	
	void pause();
	void resume();
	void checkpoint();

	/**
	 * Name of your module.
	 *
	 * \return Name of your module
	 */
	virtual Module_name name() = 0;
};

#endif /* _RTCR_BASE_MODULE_H_ */
