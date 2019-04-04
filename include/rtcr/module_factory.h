/*
 * \brief  Session Handler Factory
 * \author Johannes Fischer
 * \date   2019-03-21
 */

#ifndef _RTCR_MODULE_FACTORY_H_
#define _RTCR_MODULE_FACTORY_H_

/* Genode includes */
#include <util/list.h>
#include <base/heap.h>
#include <base/env.h>

/* Localn includes */
#include <rtcr/module.h>

using namespace Rtcr;

namespace Rtcr {
	class Module_factory;
}

class Rtcr::Module_factory
{
 private:
  static Module_factory* _head;
  Module_factory* _next;
  
 public:

  virtual Module* create(Genode::Env &env,
			 Genode::Allocator &md_alloc,
			 Genode::Entrypoint &ep,
			 Genode::List<Session_handler> *_session_handlers,
			 const char* label,
			 bool &bootstrap) = 0;
  
  virtual char const* name() = 0;
  
  Module_factory();
  ~Module_factory();

  static void print();
  static Module_factory* get(char const* name);  
  static Module_factory* first();
  Module_factory* next();
};


#endif /* _RTCR_MODULE_FACTORY_H_ */
