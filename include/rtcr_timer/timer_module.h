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


class Rtcr::Timer_module : public virtual Module
{
private:  
    Genode::Env        &_env;
    Genode::Allocator  &_alloc;
    Genode::Entrypoint &_ep;

    Timer_root *_timer_root;
    Genode::Local_service *_timer_service;
    bool &_bootstrap;
    Core_module_abstract *_core_module;
    Timer_state &_timer_state;

    void _destroy_stored_timer_session(Stored_timer_session_info &stored_info);
    Timer_state &_initialize_state(Genode::Allocator &_alloc);
    
public:
  
    Timer_module(Genode::Env &env,
		 Genode::Allocator &alloc,
		 Genode::Entrypoint &ep,
		 bool &bootstrap);

    ~Timer_module();
    void initialize(Genode::List<Module> &modules);
    Module_state *checkpoint();
    void restore(Module_state *state);
    Genode::Service *resolve_session_request(const char *service_name, const char *args);

    Module_name name(){
	return "timer";
    }
};


// create a factory class for Cpu_session_handler
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
    
    Module_name name()
	{
	    return "timer";
	}
  
};

#endif /* _RTCR_TIMER_MODULE_H_ */
