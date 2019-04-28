/*
 * \brief  Timer Module
 * \author Johannes Fischer
 * \date   2019-04-12
 */

#ifndef _RTCR_TIMER_MODULE_H_
#define _RTCR_TIMER_MODULE_H_

/* Genode includes */
#include <util/list.h>
#include <base/heap.h>
#include <util/list.h>
#include <os/config.h>
#include <base/service.h>

/* Rtcr includes */
#include <rtcr/module.h>
#include <rtcr/module_state.h>
#include <rtcr/module_factory.h>
#include <rtcr/core_module_abstract.h>

/* Timer Module includes */
#include <rtcr_timer/timer_module.h>
#include <rtcr_timer/timer_state.h>


namespace Rtcr {
	class Timer_module;
	class Timer_module_factory;
}

using namespace Rtcr;

/**
 * The Rtcr::Timer_module provides a Timer session and is able to
 * checkpoint/restore this session.
 */
class Rtcr::Timer_module : public virtual Module
{
private:  
	Genode::Env        &_env;
	Genode::Allocator  &_alloc;
	Genode::Entrypoint &_ep;

protected:    
	Timer_root *_timer_root;
	Genode::Local_service *_timer_service;
	bool &_bootstrap;
	Core_module_abstract *_core_module;
	Timer_state &_timer_state;

	void _destroy_stored_timer_session(Stored_timer_session_info &stored_info);

	/**
	 * Create a Module_state object with the given allocator
	 */
	Timer_state &_initialize_state(Genode::Allocator &_alloc);
    
public:
	Timer_module(Genode::Env &env,
		     Genode::Allocator &alloc,
		     Genode::Entrypoint &ep,
		     bool &bootstrap);

	~Timer_module();

	/**
	 * Called by the Target_child when all modules are initialized.
	 *
	 * \param modules which were loaded. 
	 */
	void initialize(Genode::List<Module> &modules) override;

	/**
	 * Checkpoint Timer session
	 *
	 * \return the internal Timer_state object which contains the checkpointed
	 *         state of the Timer session.
	 */
	Module_state *checkpoint() override;

	/**
	 * Restore this module to a state.
	 *
	 * Not yet implemented.
	 */    
	void restore(Module_state *state) override;

	/**
	 * If the child requests a Timer service, this module provides it.
	 */    
	Genode::Service *resolve_session_request(const char *service_name,
						 const char *args) override;

	Module_name name() override{ return "timer"; }
};


/**
 * Factory class for creating the Rtcr::Timer_module
 */
class Rtcr::Timer_module_factory : public Module_factory
{
public:
	Module* create(Genode::Env &env,
		       Genode::Allocator &alloc,
		       Genode::Entrypoint &ep,
		       const char* label,
		       bool &bootstrap,
		       Genode::Xml_node *config)
	{    
		return new (alloc) Timer_module(env, alloc, ep,  bootstrap);
	}
    
	Module_name name() override { return "timer"; }
};

#endif /* _RTCR_TIMER_MODULE_H_ */
