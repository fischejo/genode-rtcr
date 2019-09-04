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
#include <rtcr/info_structs.h>
#include <rtcr/child_info.h>

namespace Rtcr {
	class Timer_session;
	class Timer_root;
	class Timer_session_info;
}

struct Rtcr::Timer_session_info : Session_info {
	unsigned timeout;
	bool periodic;
	Genode::uint16_t sigh_badge;

	Timer_session_info(const char* creation_args) : Session_info(creation_args) {}
	
	void print(Genode::Output &output) const {
		Genode::print(output, " Timer session:\n  ");
		Session_info::print(output);
		Genode::print(output,
					  ", sigh_badge ", sigh_badge,
					  ", timeout=", timeout,
					  ", periodic=", periodic, "\n");
	}
};



/**
 * Custom RPC session object to intercept its creation, modification, and
 * destruction through its interface
 */
class Rtcr::Timer_session : public Rtcr::Checkpointable,
							public Genode::Rpc_object<Timer::Session>
{
public:
	/******************
	 ** COLD STORAGE **
	 ******************/
	Timer_session_info info;
  
protected:
	/*****************
	 ** HOT STORAGE **
	 *****************/

	const char* _upgrade_args;
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

	Child_info *_child_info;
	
public:
	using Genode::Rpc_object<Timer::Session>::cap;
	
	Timer_session(Genode::Env &env,
				  Genode::Allocator &md_alloc,
				  Genode::Entrypoint &ep,
				  const char *creation_args,
				  Child_info *child_info);

	~Timer_session() {};
	
	Timer::Session_capability parent_cap() { return _parent_timer.cap(); }

	void checkpoint() override;

	void upgrade(const char *upgrade_args) {
		_upgrade_args = upgrade_args;		
	}
	
	const char* upgrade_args() { return _upgrade_args; }

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

	Genode::Lock &_childs_lock;
	Genode::List<Child_info> &_childs;

protected:

	/**
	 * Wrapper for creating a ram session
	 */
	virtual Timer_session *_create_timer_session(Child_info *info, const char *args);
	
	Timer_session *_create_session(const char *args);

	void _upgrade_session(Timer_session *session, const char *upgrade_args);

	void _destroy_session(Timer_session *session);

public:
  
	Timer_root(Genode::Env &env,
			   Genode::Allocator &md_alloc,
			   Genode::Entrypoint &session_ep,
			   Genode::Lock &childs_lock,
			   Genode::List<Child_info> &childs);

	~Timer_root();

};


#endif /* _RTCR_TIMER_SESSION_H_ */
