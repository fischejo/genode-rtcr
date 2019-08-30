/*
 * \brief  Intercepting timer session
 * \author Denis Huber
 * \author Johannes Fischer
 * \date   2019-08-29
 */

#ifndef _RTCR_TIMER_SESSION_H_
#define _RTCR_TIMER_SESSION_H_

/* Genode includes */
#include <timer_session/connection.h>
#include <root/component.h>
#include <base/allocator.h>
#include <util/list.h>

/* Rtcr includes */
#include <rtcr/checkpointable.h>

namespace Rtcr {
	class Timer_session;
	class Timer_root;
}

/**
 * Custom RPC session object to intercept its creation, modification, and
 * destruction through its interface
 */
class Rtcr::Timer_session : public Rtcr::Checkpointable,
							public Genode::Rpc_object<Timer::Session>,
							public Genode::List<Timer_session>::Element
{
public:
	/******************
	 ** COLD STORAGE **
	 ******************/

	Genode::String<160> ck_creation_args;
	Genode::String<160> ck_upgrade_args;
	bool ck_bootstrapped;
	unsigned ck_timeout;
	bool ck_periodic;
	Genode::uint16_t ck_badge;
	Genode::addr_t ck_kcap;
	Genode::uint16_t ck_sigh_badge;
  
protected:
	/*****************
	 ** HOT STORAGE **
	 *****************/

	const char* _upgrade_args;
	bool _bootstrapped;
	Genode::Signal_context_capability _sigh;
	unsigned _timeout;
	bool _periodic;

	/**
	 * Allocator for Rpc objects created by this session and also for monitoring
	 * structures
	 */
	Genode::Allocator  &_md_alloc;

	/**
	 * Entrypoint for managing created Rpc objects
	 */
	Genode::Entrypoint &_ep;

	/**
	 * Parent's session connection which is used by the intercepted methods
	 */
	Timer::Connection   _parent_timer;
	
public:
	using Genode::Rpc_object<Timer::Session>::cap;
	
	Timer_session(Genode::Env &env,
				  Genode::Allocator &md_alloc,
				  Genode::Entrypoint &ep,
				  const char *creation_args,
				  bool bootstrapped);

	~Timer_session() {};


	void print(Genode::Output &output) const {
		Genode::print(output,
					  "ck_sigh_badge ", ck_sigh_badge,
					  ", ck_timeout=", ck_timeout,
					  ", ck_periodic=", ck_periodic,
					  ", ck_bootstrapped=", ck_bootstrapped,
					  ", ck_cargs='", ck_creation_args, "'",
					  ", ck_uargs='", ck_upgrade_args, "'");
	}

	
	Timer::Session_capability parent_cap() { return _parent_timer.cap(); }

	void checkpoint() override;

	void upgrade(const char *upgrade_args) {
		_upgrade_args = upgrade_args;		
	}
	
	const char* upgrade_args() { return _upgrade_args; }

	Timer_session *find_by_badge(Genode::uint16_t badge);	

	/************************************
	 ** Timer session Rpc interface **
	 ************************************/

	void trigger_once(unsigned us) override;

	void trigger_periodic(unsigned us) override;

	void sigh(Genode::Signal_context_capability sigh) override;

	unsigned long elapsed_ms() const override;

	unsigned long now_us() const override;

	void msleep(unsigned ms) override;

	void usleep(unsigned us) override;
};


class Rtcr::Timer_root : public Genode::Root_component<Timer_session>
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
	bool               &_bootstrap_phase;

	/**
	 * Lock for infos list
	 */
	Genode::Lock        _objs_lock;

	/**
	 * List for monitoring session objects
	 */
	Genode::List<Timer_session> _session_rpc_objs;

protected:

	Timer_session *_create_session(const char *args);

	void _upgrade_session(Timer_session *session, const char *upgrade_args);

	void _destroy_session(Timer_session *session);

public:
  
	Timer_root(Genode::Env &env,
			   Genode::Allocator &md_alloc,
			   Genode::Entrypoint &session_ep,
			   bool &bootstrap_phase);

	~Timer_root();

	Timer_session *find_by_badge(Genode::uint16_t badge);

  
	Genode::List<Timer_session> &sessions() { return _session_rpc_objs;  }
};


#endif /* _RTCR_TIMER_SESSION_H_ */
