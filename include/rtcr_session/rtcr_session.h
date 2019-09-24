/*
 * \brief  Interface definition of the QA service
 * \author Johannes Fischer
 * \date   2019-09-24
 */



#ifndef _INCLUDE__RTCR_SESSION__RTCR_SESSION_H_
#define _INCLUDE__RTCR_SESSION__RTCR_SESSION_H_

#include <session/session.h>
#include <base/rpc.h>

namespace Rtcr { struct Session; }


struct Rtcr::Session : Genode::Session
{
	static const char *service_name() { return "Rtcr"; }

	virtual Genode::Dataspace_capability checkpoint() = 0;
	virtual Genode::size_t size() = 0;
	virtual void free() = 0;

	enum { CAP_QUOTA = 10 };

	/*******************
	 ** RPC interface **
	 *******************/

	GENODE_RPC(Rpc_checkpoint, Genode::Dataspace_capability, checkpoint);
	GENODE_RPC(Rpc_size, Genode::size_t, size);
	GENODE_RPC(Rpc_free, void, free);		

	GENODE_RPC_INTERFACE(Rpc_checkpoint, Rpc_size, Rpc_free);	
};

#endif /* _INCLUDE__RTCR_SESSION__RTCR_SESSION_H_ */
