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
#include <os/config.h>
/* Localn includes */
#include <rtcr/module.h>
#include <util/string.h>

using namespace Rtcr;

namespace Rtcr {
  class Module_factory;
  typedef Genode::String<16> Module_name;
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
			 const char* label,
			 bool &bootstrap,
			 Genode::Xml_node *config,
			 Genode::List<Module> &modules) = 0;
  
  virtual Module_name name() = 0;
  
  Module_factory();
  ~Module_factory();

  static void print();
  static Module_factory* get(const Module_name name);  
  static Module_factory* first();
  Module_factory* next();
};


#endif /* _RTCR_MODULE_FACTORY_H_ */
