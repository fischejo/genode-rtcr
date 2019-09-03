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

/* Rtcr includes */
#include <rtcr/checkpointable.h>
#include <rtcr/info_structs.h>
#include <rtcr/child_info.h>

namespace Rtcr {
	class Log_session;
	class Log_root;
	class Log_session_info;
}


struct Rtcr::Log_session_info : Session_info {
	Log_session_info(const char* creation_args) : Session_info(creation_args) {}
	
	void print(Genode::Output &output) const {
		Genode::print(output, " Log session:\n ");
		Session_info::print(output);
		
	}
};



/**
 * Custom RPC session object to intercept its creation, modification, and
 * destruction through its interface
 */
class Rtcr::Log_session : public Rtcr::Checkpointable,
						  public Genode::Rpc_object<Genode::Log_session>
{
public:
	/******************
	 ** COLD STORAGE **
	 ******************/

	Log_session_info info;

protected:
	/*****************
	 ** HOT STORAGE **
	 *****************/

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


/**
 * Custom root RPC object to intercept session RPC object creation,
 * modification, and destruction through the root interface
 */
class Rtcr::Log_root : public Genode::Root_component<Log_session>
{
private:
	/**
	 * Environment of Rtcr; is forwarded to a created session object
	 */
	Genode::Env        &_env;
	/**
	 * Allocator for session objects and monitoring list elements
	 */
	Genode::Allocator  &_md_alloc;
	/**
	 * Entrypoint for managing session objects
	 */
	Genode::Entrypoint &_ep;

	Genode::Lock &_childs_lock;
	Genode::List<Child_info> &_childs;
	
protected:

	/**
	 * Wrapper for creating a ram session
	 */
	virtual Log_session *_create_log_session(Child_info *info, const char *args);
	
	Log_session *_create_session(const char *args);
	void _upgrade_session(Log_session *session, const char *upgrade_args);
	void _destroy_session(Log_session *session);

public:
	Log_root(Genode::Env &env,
			 Genode::Allocator &md_alloc,
			 Genode::Entrypoint &session_ep,
			 Genode::Lock &childs_lock,
			 Genode::List<Child_info> &childs);
			 
	~Log_root();

};

#endif /* _RTCR_LOG_SESSION_H_ */
