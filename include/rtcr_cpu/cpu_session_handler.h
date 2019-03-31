/*
 * \brief  CPU Session Handler
 * \author Johannes Fischer
 * \date   2019-03-21
 */

#ifndef _RTCR_CPU_SESSION_HANDLER_H_
#define _RTCR_CPU_SESSION_HANDLER_H_

/* Genode includes */
#include <base/component.h>
#include <base/log.h>
#include <cpu_session/cpu_session.h>
#include <base/service.h>
#include <base/env.h>
#include <base/child.h>
#include <base/service.h>
#include <base/snprintf.h>
#include <util/list.h>

/* Local includes */
#include <rtcr/session_handler.h>
#include <rtcr/session_handler_factory.h>

#include <rtcr_cpu/cpu_session.h>
#include <rtcr_pd/pd_session.h>
#include <rtcr_pd/pd_session_handler.h>



namespace Rtcr {
  class Cpu_session_handler;
  class Cpu_session_handler_factory;
}

using namespace Rtcr;

class Rtcr::Cpu_session_handler: public Session_handler
{
public:
	Cpu_session_handler(Genode::Env &env,
			    Genode::Allocator &md_alloc,
			    Genode::Entrypoint &ep,
			    Pd_root &pd_root,
			    const char* label,
			    bool &bootstrap);

	~Cpu_session_handler();

        void session_init();
	void session_checkpoint();
	void session_restore();

	Cpu_root *cpu_root;
	Genode::Local_service *cpu_service;
	Cpu_session_component *cpu_session;


	Genode::Env        &_env;
	Genode::Allocator  &_md_alloc;
	Genode::Entrypoint &_ep;	
};


// create a factory class for Cpu_session_handler
class Rtcr::Cpu_session_handler_factory : public Session_handler_factory
{
 public:
  Session_handler* create(Genode::Env &env,
			  Genode::Allocator &md_alloc,
			  Genode::Entrypoint &ep,
			  Genode::List<Session_handler> *_session_handlers,			  
			  const char* label,
			  bool &bootstrap)
  {
    
    Session_handler *session_handler = _session_handlers->first();
    Pd_session_handler *pd_session_handler = NULL;
    while (!pd_session_handler && session_handler)
      {
	// try to cast to Pd_session_handler
	pd_session_handler = dynamic_cast<Pd_session_handler*>(session_handler);
	session_handler = session_handler->next();
      }

    if(!pd_session_handler)
      Genode::error("No PD session handler found! ");
    
    return new (md_alloc) Cpu_session_handler(env,
					      md_alloc,
					      ep,
					      *(pd_session_handler->pd_root),
					      label,
					      bootstrap);
  }
    
  char const* name()
  {
    return "CPU";
  }
  
};

#endif /* _RTCR_CPU_SESSION_HANDLER_H_ */
