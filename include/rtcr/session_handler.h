/*
 * \brief  Session Handler
 * \author Denis Huber
 * \author Johannes Fischer
 * \date   2019-03-21
 */

#ifndef _RTCR_SESSION_HANDLER_H_
#define _RTCR_SESSION_HANDLER_H_

/* Genode includes */
#include <util/list.h>
#include <base/heap.h>
#include <util/list.h>


namespace Rtcr {
	class Session_handler;
}

class Rtcr::Session_handler : public Genode::List<Session_handler>::Element
{
public:
	virtual void session_init() = 0;
	virtual void session_checkpoint() = 0;
	virtual void session_restore() = 0;
	//	virtual char const* name() = 0;	
};



#endif /* _RTCR_SESSION_HANDLER_H_ */
