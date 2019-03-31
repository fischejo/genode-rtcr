/*
 * \brief  Session Handler Factory
 * \author Johannes Fischer
 * \date   2019-03-21
 */

#include <rtcr/session_handler_factory.h>

using namespace Rtcr;

// initialize List
Session_handler_factory* Session_handler_factory::_head = NULL;

Session_handler_factory::~Session_handler_factory() {}

Session_handler_factory::Session_handler_factory() {
  _next = _head;
  _head = this;  
}

void Session_handler_factory::print()
{
  Session_handler_factory* r = Session_handler_factory::first();
  while(r)
    {
      Genode::log("Register Session_handler: ", r->name());
      r = r->next();
    }
}

Session_handler_factory* Session_handler_factory::get(const char* name)
{
  Session_handler_factory* r = Session_handler_factory::first();
  while(r)
    {
      if(!Genode::strcmp(r->name(), name))
	return r;
      r = r->next();
    }
  return NULL;
}



Session_handler_factory* Session_handler_factory::first()
{
  return _head;
}
Session_handler_factory* Session_handler_factory::next()
{
  return _next;
}
