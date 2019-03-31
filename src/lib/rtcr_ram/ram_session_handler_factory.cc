/*
 * \brief  Ram Session Handler
 * \author Johannes Fischer
 * \date   2019-03-21
 */

#include "ram_session_handler_factory.h"

using namespace Rtcr;



Ram_session_handler_factory ram_factory;

Session_handler* Ram_session_handler_factory::create(Genode::Allocator &md_alloc)
  {
    return new (md_alloc) Ram_session_handler();
  }

char const* Ram_session_handler_factory::name()
{
  return "ram";
}
