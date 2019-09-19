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
#include <base/session_object.h>

/* Rtcr includes */
#include <rtcr/checkpointable.h>
#include <rtcr/timer/timer_session_info.h>
#include <rtcr/child_info.h>

namespace Rtcr {
	class Timer_session;
	class Timer_factory;
}


/**
 * Custom RPC session object to intercept its creation, modification, and
 * destruction through its interface
 */
class Rtcr::Timer_session : public Rtcr::Checkpointable,
                            public Genode::Rpc_object<Timer::Session>,
                            public Rtcr::Timer_session_info
{

protected:
	const char* _upgrade_args;
	Genode::Signal_context_capability _sigh;
	unsigned _timeout;
	bool _periodic;

	/**
	 * Allocator for Rpc objects created by this session and also for
	 * monitoring structures
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

	~Timer_session();

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

	unsigned long elapsed_us() const override;

	void msleep(unsigned ms) override;

	void usleep(unsigned us) override;
};


class Rtcr::Timer_factory : public Genode::Local_service<Rtcr::Timer_session>::Factory
{
private:
	Genode::Env &_env;
	Genode::Allocator &_md_alloc;
	Genode::Entrypoint &_ep;

	Genode::Lock &_childs_lock;
	Genode::List<Child_info> &_childs;

	Genode::Local_service<Timer_session> _service;
	Genode::Session::Diag _diag;

protected:

	Timer_session *_create(Child_info *info, const char *args);

public:
	Timer_factory(Genode::Env &env,
	              Genode::Allocator &md_alloc,
	              Genode::Entrypoint &ep,
	              Genode::Lock &childs_lock,
	              Genode::List<Child_info> &childs);

	Timer_session &create(Genode::Session_state::Args const &args,
	                      Genode::Affinity) override;

	void upgrade(Timer_session&,
	             Genode::Session_state::Args const &) override;

	void destroy(Timer_session&) override;

	Genode::Service *service() { return &_service; }
};


#endif /* _RTCR_TIMER_SESSION_H_ */
