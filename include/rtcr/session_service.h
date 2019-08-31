/*
 * \brief  Service management framework
 * \author Johannes Fischer
 * \date   2019-08-30
 */

#ifndef _INCLUDE__RTCR_SESSION_SERVICE_H_
#define _INCLUDE__RTCR_SESSION_SERVICE_H_

#include <base/service.h>
#include <root/client.h>
#include <base/log.h>
#include <util/list.h>
#include <ram_session/client.h>
#include <base/env.h>


namespace Rtcr {

}

/**
 * Representation of a locally implemented service
 */
template <class T, class R>
class Session_service : public Genode::Local_service
{
private:
	Genode::Session_capability _cap;
	R *_root;
	T *_session;
	
public:

	Session_service(const char *name, R *root)
		: Genode::Local_service(name, root), _root(root) { }

	Genode::Session_capability session(const char *args, Genode::Affinity const &affinity) override
		{
			_cap = Local_service::session(args, affinity);
			T *session = _root->sessions().first();
			_session = session->find_by_badge(_cap.local_name());

#ifdef DEBUG
			Genode::log("Monitored session creation name=", name(), " cap=",_cap);
#endif
			
			return _cap;
		}

	Genode::Session_capability cap() { return _cap; }
	T *session() { return _session; }
	const T *session() const { return _session; }	
};

#endif
