/*
 * \brief Factory class for registring and creating a module at runtime.
 * \author Johannes Fischer
 * \date   2019-03-21
 */

#include <rtcr/module_factory.h>

using namespace Rtcr;

// initialize List
Session_handler_factory* Module_factory::_head = NULL;

Module_factory::~Module_factory() {}

Module_factory::Module_factory() {
  _next = _head;
  _head = this;  
}

void Module_factory::print()
{
  Module_factory* r = Module_factory::first();
  while(r)
    {
      Genode::log("Register Module: ", r->name());
      r = r->next();
    }
}

Module_factory* Module_factory::get(const char* name)
{
  Module_factory* r = Module_factory::first();
  while(r)
    {
      if(!Genode::strcmp(r->name(), name))
	return r;
      r = r->next();
    }
  return NULL;
}



Module_factory* Module_factory::first()
{
  return _head;
}
Module_factory* Module_factory::next()
{
  return _next;
}
