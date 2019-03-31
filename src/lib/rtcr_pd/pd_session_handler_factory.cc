/*
 * \brief  CPU Session Handler
 * \author Johannes Fischer
 * \date   2019-03-21
 */

#include "pd_session_handler_factory.h"

using namespace Rtcr;



Pd_session_handler_factory pd_factory;

Session_handler* Pd_session_handler_factory::create(Genode::Allocator &md_alloc)
  {
    return new (md_alloc) Pd_session_handler();
  }

char const* Pd_session_handler_factory::name()
{
  return "pd";
}
