/*
 * \brief  Intercepting timer session
 * \author Denis Huber
 * \date   2016-10-05
 */

#ifndef _RTCR_TIMER_SESSION_H_
#define _RTCR_TIMER_SESSION_H_

/* Genode includes */
#include <timer_session/connection.h>
#include <root/component.h>
#include <base/allocator.h>
#include <util/list.h>

#include <rtcr/checkpointable.h>

namespace Rtcr {
  class Timer_session;
  class Timer_root;
  constexpr bool timer_root_verbose_debug = false;
  constexpr bool timer_verbose_debug = false;
}

/**
 * Custom RPC session object to intercept its creation, modification, and destruction through its interface
 */
class Rtcr::Timer_session : public Rtcr::Checkpointable,
				      public Genode::Rpc_object<Timer::Session>,
                                      public Genode::List<Timer_session>::Element
{
public:
  using Genode::Rpc_object<Timer::Session>::cap;
  
  /* checkpointed variables */
  Genode::String<160> ck_creation_args;
  Genode::String<160> ck_upgrade_args;
  bool ck_bootstrapped;
  unsigned ck_timeout;
  bool ck_periodic;
  Genode::uint16_t ck_badge;
  Genode::addr_t ck_kcap;
  Genode::uint16_t ck_sigh_badge;
  
protected:

  char* _upgrade_args;
  bool _bootstrapped;
  Genode::Signal_context_capability _sigh;
  unsigned _timeout;
  bool _periodic;


  void print(Genode::Output &output) const
  {
    Genode::print(output,
		  "ck_sigh_badge ", ck_sigh_badge,
		  ", ck_timeout=", ck_timeout,
		  ", ck_periodic=", ck_periodic,
		  ", ck_bootstrapped=", ck_bootstrapped,
		  ", ck_cargs='", ck_creation_args, "'",
		  ", ck_uargs='", ck_upgrade_args, "'");
  }

  /**
   * Enable log output for debugging
   */
  static constexpr bool verbose_debug = timer_verbose_debug;
  /**
   * Allocator for Rpc objects created by this session and also for monitoring structures
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

  Timer_session(Genode::Env &env,
			  Genode::Allocator &md_alloc,
			  Genode::Entrypoint &ep,
			  const char *creation_args,
			  bool bootstrapped,
			  Genode::Xml_node *config);

  ~Timer_session();

  Timer::Session_capability parent_cap() { return _parent_timer.cap(); }

  void checkpoint() override;

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
   * Enable log output for debugging
   */
  static constexpr bool verbose_debug = timer_root_verbose_debug;

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
  Genode::Xml_node *_config;
 public:
  
  Timer_root(Genode::Env &env,
	     Genode::Allocator &md_alloc,
	     Genode::Entrypoint &session_ep,
	     bool &bootstrap_phase,
	     Genode::Xml_node *config);

  ~Timer_root();

  Timer_session *find_by_badge(Genode::uint16_t badge);

  
  Genode::List<Timer_session> &session_infos() { return _session_rpc_objs;  }
};


#endif /* _RTCR_TIMER_SESSION_H_ */
