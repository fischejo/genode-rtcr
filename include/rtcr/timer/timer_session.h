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
#include <base/allocator.h>
#include <util/list.h>
#include <base/session_object.h>

/* Rtcr includes */
#include <rtcr/checkpointable.h>
#include <rtcr/timer/timer_session_info.h>
#include <rtcr/child_info.h>

namespace Rtcr {
	class Timer_session;
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

	Genode::Env &_env;
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

	void upgrade(const char *upgrade_args);

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

#endif /* _RTCR_TIMER_SESSION_H_ */
