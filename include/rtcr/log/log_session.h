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
						  public Genode::Rpc_object<Genode::Log_session>,
						  public Genode::List<Log_session>::Element
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
	bool _bootstrapped;

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




public:
    using Genode::Rpc_object<Genode::Log_session>::cap;	
	
	Log_session(Genode::Env &env,
				Genode::Allocator &md_alloc,
				Genode::Entrypoint &ep,
				const char *label,
				const char *creation_args,
				bool bootstrapped);

	~Log_session();

	Genode::Log_session_capability parent_cap() { return _parent_log.cap(); }

	void checkpoint() override;

	void upgrade(const char *upgrade_args) {
		_upgrade_args = upgrade_args;		
	}
	
	const char* upgrade_args() { return _upgrade_args; }

	Log_session *find_by_badge(Genode::uint16_t badge);		

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
	/**
	 * Reference to Target_child's bootstrap phase
	 */
	bool &_bootstrap_phase;
	/**
	 * Lock for infos list
	 */
	Genode::Lock        _objs_lock;
	/**
	 * List for monitoring session RPC objects
	 */
	Genode::List<Log_session> _session_rpc_objs;

protected:
	Log_session *_create_session(const char *args);
	void _upgrade_session(Log_session *session, const char *upgrade_args);
	void _destroy_session(Log_session *session);

public:
	Log_root(Genode::Env &env,
			 Genode::Allocator &md_alloc,
			 Genode::Entrypoint &session_ep,
			 bool &bootstrap_phase);
	~Log_root();


  
	Genode::List<Log_session> &sessions() { return _session_rpc_objs; }
};

#endif /* _RTCR_LOG_SESSION_H_ */
