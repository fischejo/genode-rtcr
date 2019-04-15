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
#include <rtcr/module_factory.h>
#include <rtcr/target_state.h>
#include <rtcr_core/core_module.h>

/* Timer Module includes */
#include <rtcr_timer/timer_module.h>


namespace Rtcr {
    class Timer_module;
    class Timer_module_factory;
}

using namespace Rtcr;


class Rtcr::Timer_module : public virtual Module
{
private:  
    Genode::Env        &_env;
    Genode::Allocator  &_md_alloc;
    Genode::Entrypoint &_ep;

    Timer_root *_timer_root;
    Genode::Local_service *_timer_service;
    bool &_bootstrap;
    Core_module *_core_module;
  

  void _destroy_stored_timer_session(Target_state &state,
				     Stored_timer_session_info &stored_info);
  
public:
  
  Timer_module(Genode::Env &env,
	      Genode::Allocator &md_alloc,
	      Genode::Entrypoint &ep,
	      bool &bootstrap);

  ~Timer_module();
    void initialize(Genode::List<Module> &modules);
    void checkpoint(Target_state &state);
    void restore(Target_state &state);
    Genode::Service *resolve_session_request(const char *service_name, const char *args);
};


// create a factory class for Cpu_session_handler
class Rtcr::Timer_module_factory : public Module_factory
{
public:
  Module* create(Genode::Env &env,
		 Genode::Allocator &md_alloc,
		 Genode::Entrypoint &ep,
		 const char* label,
		 bool &bootstrap,
		 Genode::Xml_node *config)
  {    
    return new (md_alloc) Timer_module(env, md_alloc, ep,  bootstrap);
  }
    
  Module_name name()
  {
    return "timer";
  }
  
};

#endif /* _RTCR_TIMER_MODULE_H_ */
