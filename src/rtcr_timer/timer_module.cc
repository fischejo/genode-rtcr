/*
 * \brief  Timer module
 * \description This module provides a timer session
 * \author Johannes Fischer
 * \date   2019-04-12
 */

#include <rtcr_timer/timer_module.h>

using namespace Rtcr;

/* Create a static instance of the Core_module_factory. This registers the module */

Rtcr::Timer_module_factory _timer_module_factory_instance;


Timer_module::Timer_module(Genode::Env &env,
			   Genode::Allocator &md_alloc,
			   Genode::Entrypoint &ep,
			   const char* label,
			   bool &bootstrap)
  :
    _env(env),
    _md_alloc(md_alloc),
    _ep(ep),
    _bootstrap(bootstrap)
{

}



void Timer_module::checkpoint(Target_state &state)
{  

}


void Timer_module::restore(Target_state &state)
{

}


Genode::Service *Timer_module::resolve_session_request(const char *service_name,
						       const char *args)
{
  if(!_timer_root) {
    _timer_root = new (_md_alloc) Timer_root(_env,
					     _md_alloc,
					     _ep,
					     _bootstrap);

    _timer_service = new (_md_alloc) Genode::Local_service("Timer",
							   _timer_root);
  }

  return _timer_service;
}


