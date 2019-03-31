/*
 * \brief  RAM Session Handler
 * \author Johannes Fischer
 * \date   2019-03-21
 */

#ifndef _RTCR_RAM_SESSION_HANDLER_H_
#define _RTCR_RAM_SESSION_HANDLER_H_

/* Genode includes */
#include <base/component.h>
#include <base/log.h>
#include <base/service.h>
#include <base/env.h>
#include <base/child.h>
#include <base/service.h>
#include <base/snprintf.h>


/* Global includes */
#include <rtcr/session_handler.h>
#include <rtcr/session_handler_factory.h>

/* Local includes */
#include <rtcr_ram/ram_session.h>

namespace Rtcr {
  class Ram_session_handler;
  class Ram_session_handler_factory;  
}

using namespace Rtcr;

class Rtcr::Ram_session_handler: public Session_handler
{
public:
	Ram_session_handler(Genode::Env &env,
			    Genode::Allocator &md_alloc,
			    Genode::Entrypoint &ep,
			    const char* label,
			    bool &bootstrap);
	
	~Ram_session_handler();

        void session_init();
	void session_checkpoint();
	void session_restore();

	Ram_root *ram_root;
	Genode::Local_service *ram_service;
	Ram_session_component  *ram_session;	

	Genode::Env        &_env;
	Genode::Allocator  &_md_alloc;
	Genode::Entrypoint &_ep;	
	
};

// create a factory class for Cpu_session_handler
class Rtcr::Ram_session_handler_factory : public Session_handler_factory
{
 public:
  Session_handler* create(Genode::Env &env,
			  Genode::Allocator &md_alloc,
			  Genode::Entrypoint &ep,
			  Genode::List<Session_handler> *_session_handlers,			  
			  const char* label,
			  bool &bootstrap)
  {
    return new (md_alloc) Ram_session_handler(env, md_alloc, ep, label, bootstrap);
  }
    
  char const* name()
  {
    return "RAM";
  }
  
};


#endif /* _RTCR_Ram_SESSION_HANDLER_H_ */
