/*
 * \brief  Intercepting Log session
 * \author Denis Huber
 * \author Johannes Fischer
 * \date   2019-08-29
 */

#ifndef _RTCR_LOG_SESSION_H_
#define _RTCR_LOG_SESSION_H_

/* Genode includes */
#include <log_session/connection.h>
#include <base/allocator.h>
#include <root/component.h>
#include <util/list.h>
#include <base/session_object.h>

/* Rtcr includes */
#include <rtcr/checkpointable.h>
#include <rtcr/log/log_session_info.h>
#include <rtcr/child_info.h>

namespace Rtcr {
	class Log_session;
	class Log_factory;
}


/**
 * Custom RPC session object to intercept its creation, modification, and
 * destruction through its interface
 */
class Rtcr::Log_session : public Rtcr::Checkpointable,
						  public Genode::Rpc_object<Genode::Log_session>,
						  public Rtcr::Log_session_info
{
protected:

	const char* _upgrade_args;


	/**
	 * Allocator for Rpc objects created by this session and also for monitoring
	 * list elements
	 */
	Genode::Allocator &_md_alloc;

	/**
	 * Entrypoint for managing created Rpc objects
	 */
	Genode::Entrypoint &_ep;

	/**
	 * Parent's session connection which is used by the intercepted methods
	 */
	Genode::Log_connection _parent_log;

	Child_info *_child_info;


public:
    using Genode::Rpc_object<Genode::Log_session>::cap;	
	
	Log_session(Genode::Env &env,
				Genode::Allocator &md_alloc,
				Genode::Entrypoint &ep,
				const char *creation_args,
				Child_info *child_info);

	~Log_session();

	Genode::Log_session_capability parent_cap() { return _parent_log.cap(); }

	void checkpoint() override;

	void upgrade(const char *upgrade_args) {
		_upgrade_args = upgrade_args;		
	}
	
	const char* upgrade_args() { return _upgrade_args; }


	/*******************************
	 ** Log session Rpc interface **
	 *******************************/

	Genode::size_t write(String const &string) override;
};



class Rtcr::Log_factory : public Genode::Local_service<Rtcr::Log_session>::Factory
{
private:
	Genode::Env &_env;
	Genode::Allocator &_md_alloc;
	Genode::Entrypoint &_ep;

	Genode::Lock &_childs_lock;
	Genode::List<Child_info> &_childs;

        Genode::Local_service<Log_session> _service;  
	Genode::Session::Diag _diag;

protected:

        Log_session *_create(Child_info *info, const char *args);
  
public:
	Log_factory(Genode::Env &env,
		   Genode::Allocator &md_alloc,
		   Genode::Entrypoint &ep,
		   Genode::Lock &childs_lock,
		   Genode::List<Child_info> &childs);


  
  Log_session &create(Genode::Session_state::Args const &args, Genode::Affinity) override;
  void upgrade(Log_session&, Genode::Session_state::Args const &) override;
  void destroy(Log_session&) override;

  Genode::Service *service() { return &_service; }
};




#endif /* _RTCR_LOG_SESSION_H_ */
