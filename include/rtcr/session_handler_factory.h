/*
 * \brief  Session Handler Factory
 * \author Johannes Fischer
 * \date   2019-03-21
 */

#ifndef _RTCR_SESSION_HANDLER_FACTORY_H_
#define _RTCR_SESSION_HANDLER_FACTORY_H_

/* Genode includes */
#include <util/list.h>
#include <base/heap.h>
#include <rtcr/session_handler.h>

#include <base/env.h>
#include <rtcr_pd/pd_session.h>




using namespace Rtcr;

namespace Rtcr {
	class Session_handler_factory;
}

class Rtcr::Session_handler_factory
{
 private:
  static Session_handler_factory* _head;
  Session_handler_factory* _next;
  
 public:

  virtual Session_handler* create(Genode::Env &env,
				  Genode::Allocator &md_alloc,
				  Genode::Entrypoint &ep,
				  Genode::List<Session_handler> *_session_handlers,
				  const char* label,
				  bool &bootstrap) = 0;
  
  virtual char const* name() = 0;
  
  Session_handler_factory();
  ~Session_handler_factory();

  static void print();
  static Session_handler_factory* get(char const* name);  
  static Session_handler_factory* first();
  Session_handler_factory* next();

};


#endif /* _RTCR_SESSION_HANDLER_FACTORY_H_ */
