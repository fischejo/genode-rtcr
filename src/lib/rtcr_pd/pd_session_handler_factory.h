/*
 * \brief  CPU Session Handler
 * \author Johannes Fischer
 * \date   2019-03-21
 */

#ifndef _RTCR_PD_SESSION_HANDLER_FACTORY_H_
#define _RTCR_PD_SESSION_HANDLER_FACTORY_H_

/* Genode includes */
#include <util/list.h>
#include <base/heap.h>
#include <base/component.h>
#include <base/log.h>


/* Global includes */
#include <rtcr/session_handler.h>
#include <rtcr/session_handler_factory.h>

/* Local includes */
#include "pd_session_handler.h"

using namespace Rtcr;

// factory for creating a Cpu_session_handler
class Pd_session_handler_factory : public Session_handler_factory
{
public:
  Session_handler* create(Genode::Allocator &md_alloc);
  char const* name();
};


#endif /* _RTCR_CPU_SESSION_HANDLER_H_ */
