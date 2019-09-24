/*
 * \brief  Client-side interface of the RTCR service
 * \author Johannes Fischer
 * \date   2019-09-24
 */


#ifndef _INCLUDE__RTCR_SESSION_H__CLIENT_H_
#define _INCLUDE__RTCR_SESSION_H__CLIENT_H_

#include <base/rpc_client.h>
#include <rtcr_session/rtcr_session.h>

namespace Rtcr { struct Session_client; }


struct Rtcr::Session_client : Genode::Rpc_client<Rtcr::Session>
{
	Session_client(Genode::Capability<Rtcr::Session> cap)
		:
		Genode::Rpc_client<Session>(cap)
		{ }

	Genode::Dataspace_capability checkpoint()
	{
		return call<Rpc_checkpoint>();
	}

	Genode::size_t size()
	{
	  return call<Rpc_size>();
	}

	void free()
	{
	  call<Rpc_free>();
	}
	
};

#endif /* _INCLUDE__RTCR_SESSION_H__CLIENT_H_ */
