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

namespace Rtcr {
	class Log_session;
	class Log_root;
}

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

	Genode::String<160> ck_creation_args;
	Genode::String<160> ck_upgrade_args;
	bool ck_bootstrapped;
	Genode::uint16_t ck_badge;
	Genode::addr_t ck_kcap;


protected:
	/*****************
	 ** HOT STORAGE **
	 *****************/

	char* _upgrade_args;
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
				bool bootstrapped,
				Genode::Xml_node *config);

	~Log_session();

	Genode::Log_session_capability parent_cap() { return _parent_log.cap(); }

	void print(Genode::Output &output) const {
		Genode::print(output,
					  ", ck_cargs='", ck_creation_args, "'",
					  ", ck_uargs='", ck_upgrade_args, "'");
	}

	void checkpoint() override;


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

	Genode::Xml_node *_config;

protected:
	Log_session *_create_session(const char *args);
	void _upgrade_session(Log_session *session, const char *upgrade_args);
	void _destroy_session(Log_session *session);

public:
	Log_root(Genode::Env &env,
			 Genode::Allocator &md_alloc,
			 Genode::Entrypoint &session_ep,
			 bool &bootstrap_phase,
			 Genode::Xml_node *config);
	~Log_root();

	Log_session *find_by_badge(Genode::uint16_t badge);
  
	Genode::List<Log_session> &sessions() { return _session_rpc_objs; }
};

#endif /* _RTCR_LOG_SESSION_H_ */
