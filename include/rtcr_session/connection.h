/*
 * \brief  Connection to Rtcr service
 * \author Johannes Fischer
 * \date   2019-09-24
 */


#ifndef _INCLUDE__RTCR_SESSION__CONNECTION_H_
#define _INCLUDE__RTCR_SESSION__CONNECTION_H_

#include <rtcr_session/client.h>
#include <base/connection.h>

namespace Rtcr { struct Connection; }


struct Rtcr::Connection : Genode::Connection<Rtcr::Session>, Rtcr::Session_client
{
	Connection(Genode::Env &env)
	:
		Genode::Connection<Rtcr::Session>(env, session(env.parent(), "ram_quota=4K")),
		Rtcr::Session_client(cap()) { }
};

#endif /* _INCLUDE__RTCR_SESSION__CONNECTION_H_ */
