/*
 * \brief  Base Module creates all intecepting sessions
 * \author Johannes Fischer
 * \date   2019-08-29
 */

#ifndef _RTCR_BASE_MODULE_H_
#define _RTCR_BASE_MODULE_H_

/* Genode includes */
#include <base/heap.h>

/* Local includes */
#include <rtcr/module.h>
#include <rtcr/module_factory.h>

#include <rtcr/cpu/cpu_session.h>
#include <rtcr/pd/pd_session.h>
#include <rtcr/ram/ram_session.h>
#include <rtcr/rm/rm_session.h>
#include <rtcr/log/log_session.h>
#include <rtcr/timer/timer_session.h>
#include <rtcr/rom/rom_session.h>
#include <rtcr/cap/capability_mapping.h>

namespace Rtcr {
	class Base_module;
	class Base_module_factory;
}

using namespace Rtcr;

/**
 * The class Rtcr::Base_module provides the intercepting sessions and handles
 * the communication between a target child and the corresponding sessions.
 */
class Rtcr::Base_module : public virtual Module
{
protected:
	/**
	 * Entrypoint for managing child's resource-sessions (PD, CPU, RAM)
	 */
	Genode::Entrypoint _ep;

	
	/* intercepting RAM session. The order of following declarations defines the
	 * order of initialization. _pd_root depends on _ram_session! */
	Ram_root _ram_root;
	
	/* intercepting PD session */
	Pd_root _pd_root;

	/* intercepting CPU session. _cpu_root depends on _pd_root.*/
	Cpu_root _cpu_root;

	/* intercepting RM session. _rm_root depends on _ram_session.*/
	Rm_root _rm_root;

	/* intercepting ROM session */
	Rom_root _rom_root;

	/* intercepting Log session */
	Log_root _log_root;

	/* intercepting Timer session */
	Timer_root _timer_root;
   
	bool _bootstrap;

	
public:  

	Base_module(Genode::Env &env, Genode::Allocator &alloc);

	~Base_module() {};

	 Pd_root &pd_root() override { return _pd_root; }
	 Cpu_root &cpu_root() override { return _cpu_root;}
	 Ram_root &ram_root() override {return _ram_root;}
	 Rm_root &rm_root() override {return _rm_root;}
	 Rom_root &rom_root() override {return _rom_root;}
	 Log_root &log_root() override {return _log_root;}
	 Timer_root &timer_root() override {return _timer_root;}
	
	Module_name name() override { return "base"; }

};


/**
 * Factory class for creating the Rtcr::Base_module
 */
class Rtcr::Base_module_factory : public Module_factory
{
public:
	Module* create(Genode::Env &env,
				   Genode::Allocator &alloc) override
		{
			return new (alloc) Base_module(env, alloc);
		}
    
	Module_name name() override { return "base"; }
};

#endif /* _RTCR_BASE_MODULE_H_ */
