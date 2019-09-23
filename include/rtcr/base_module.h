/*
 * \brief  Base Module creates all intecepting sessions
 * \author Johannes Fischer
 * \date   2019-08-29
 */

#ifndef _RTCR_BASE_MODULE_H_
#define _RTCR_BASE_MODULE_H_

/* Genode includes */
#include <base/heap.h>
#include <base/allocator.h>
#include <base/service.h>


/* Local includes */
#include <rtcr/init_module.h>
#include <rtcr/module_factory.h>

#include <rtcr/cpu/cpu_session.h>
#include <rtcr/pd/pd_session.h>
#include <rtcr/rm/rm_session.h>
#include <rtcr/log/log_session.h>
#include <rtcr/timer/timer_session.h>
#include <rtcr/rom/rom_session.h>
#include <rtcr/cap/capability_mapping.h>

#include <rtcr/child_info.h>

namespace Rtcr {
	class Base_module;
	class Base_module_factory;
}

using namespace Rtcr;


/**
 * The class Rtcr::Base_module provides the intercepting sessions and handles
 * the communication between a target child and the corresponding sessions.
 */
class Rtcr::Base_module : public Init_module
{
private:
	Genode::Entrypoint _ep;    
	Pd_root _pd_factory;
	Cpu_root _cpu_factory;
	Log_root _log_factory;
	Timer_root _timer_factory;
	Rom_root _rom_factory;
	Rm_root _rm_factory;      
  
public:	
	Base_module(Genode::Env &env, Genode::Allocator &alloc);


	Genode::Service *resolve_session_request(const char *service_name,
	                                         const char *args);


  
	Module_name name() override { return "base"; }
};

/**
 * Factory class for creating the Rtcr::Base_module
 */
class Rtcr::Base_module_factory : public Module_factory
{
public:
	Init_module* create(Genode::Env &env, Genode::Allocator &alloc) override {
		return new (alloc) Base_module(env, alloc);
	}
    
	Module_name name() override { return "base"; }
};

#endif /* _RTCR_BASE_MODULE_H_ */
