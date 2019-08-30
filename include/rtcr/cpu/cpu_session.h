/*
 * \brief  Intercepting Cpu session
 * \author Denis Huber
 * \author Johannes Fischer
 * \date   2019-08-29
 */

#ifndef _RTCR_CPU_SESSION_H_
#define _RTCR_CPU_SESSION_H_

/* Genode includes */
#include <util/list.h>
#include <util/fifo.h>
#include <root/component.h>
#include <base/allocator.h>
#include <base/rpc_server.h>
#include <cpu_session/connection.h>
#include <cpu_thread/client.h>

/* Rtcr includes */
#include <rtcr/cpu/cpu_thread.h>
#include <rtcr/pd/pd_session.h>
#include <rtcr/checkpointable.h>

namespace Rtcr {
	class Cpu_session;
	class Cpu_root;
}



/**
 * This custom Cpu session intercepts the creation and destruction of threads by
 * the client
 */
class Rtcr::Cpu_session : public virtual Rtcr::Checkpointable,
						  public Genode::Rpc_object<Genode::Cpu_session>,
						  public Genode::List<Cpu_session>::Element
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
	Genode::List<Cpu_thread>::Element *ck_cpu_threads;
	Genode::uint16_t ck_sigh_badge;
  
protected:

	const char* _upgrade_args;
	bool _bootstrapped;
	bool &_bootstrap_phase;
	
	Genode::Signal_context_capability _sigh;
	/**
	 * List of client's thread capabilities
	 */
	Genode::Lock _cpu_threads_lock;
	Genode::Lock _destroyed_cpu_threads_lock;
	Genode::List<Cpu_thread> _cpu_threads;
	Genode::Fifo<Cpu_thread> _destroyed_cpu_threads;

	/**
	 * Environment of creator component (usually rtcr)
	 */
	Genode::Env        &_env;
	/**
	 * Allocator for objects belonging to the monitoring of threads (e.g. Thread)
	 */
	Genode::Allocator  &_md_alloc;
	/**
	 * Entrypoint
	 */
	Genode::Entrypoint &_ep;

	/**
	 * PD root for the list of all PD sessions known to child
	 *
	 * Is used to translate child's known PD session (= custom PD session) to parent's PD session.
	 */
	Pd_root            &_pd_root;
	/**
	 * Connection to parent's Cpu session, usually from core; this class wraps this session
	 */
	Genode::Cpu_connection _parent_cpu;

	Cpu_thread &_create_thread(Genode::Pd_session_capability child_pd_cap,
							   Genode::Pd_session_capability parent_pd_cap,
							   Genode::Cpu_session::Name const &name,
							   Genode::Affinity::Location affinity,
							   Genode::Cpu_session::Weight weight,
							   Genode::addr_t utcb);
	
	void _kill_thread(Cpu_thread &cpu_thread);

	/**
	 * Each child thread is directly assigned to a core. This affinity
	 * defines the core.
	 */
	Genode::Affinity::Location _child_affinity;

	/** 
	 * Read the child affinity from the XML config
	 *
	 * Example Configuration
	 * ```XML
	 * <child name="sheep_counter" xpos="1" ypos="0" />
	 * ```
	 */
	Genode::Affinity::Location _read_child_affinity(Genode::Xml_node *config,
													const char* child_name);

	/*
	 * KIA4SM method
	 */
	Cpu_thread &_create_fp_edf_thread(Genode::Pd_session_capability child_pd_cap,
									  Genode::Pd_session_capability parent_pd_cap,
									  Genode::Cpu_session::Name const &name,
									  Genode::Affinity::Location affinity,
									  Genode::Cpu_session::Weight weight,
									  Genode::addr_t utcb,
									  unsigned priority,
									  unsigned deadline);

private	:
	using Genode::Rpc_object<Genode::Cpu_session>::cap;
	
public:

	
	Cpu_session(Genode::Env &env,
				Genode::Allocator &md_alloc,
				Genode::Entrypoint &ep,
				Pd_root &pd_root,
				const char *label,
				const char *creation_args,
				bool &bootstrap_phase,
				Genode::Xml_node *config);
	
	~Cpu_session();

	/**
	 * Pause all child threads of this session 
	 */
	void pause();

	/**
	 * Resume all child threads of this session 
	 */	
	void resume();

  
	void print(Genode::Output &output) const {
		Genode::print(output,
					  ", ck_creation_args='", ck_creation_args, "'",
					  ", ck_update_args='", ck_upgrade_args, "'");
	}

	void checkpoint() override;


	void upgrade(const char *upgrade_args) {
		_upgrade_args = upgrade_args;		
	}

	const char* upgrade_args() { return _upgrade_args; }
	
	Genode::Cpu_session_capability parent_cap() { return _parent_cpu.cap(); }

	Cpu_session *find_by_badge(Genode::uint16_t badge);

	/***************************
	 ** Cpu_session interface **
	 ***************************/

	Genode::Thread_capability create_thread(Genode::Pd_session_capability pd_cap,
											Genode::Cpu_session::Name const &name,
											Genode::Affinity::Location affinity,
											Genode::Cpu_session::Weight weight,
											Genode::addr_t utcb) override;

	void kill_thread(Genode::Thread_capability thread_cap) override;
	void exception_sigh(Genode::Signal_context_capability handler) override;
	Genode::Affinity::Space affinity_space() const override;
	Genode::Dataspace_capability trace_control() override;
	Quota quota() override;
	int ref_account(Genode::Cpu_session_capability c) override;
	int transfer_quota(Genode::Cpu_session_capability c, Genode::size_t q) override;
	Genode::Capability<Native_cpu> native_cpu() override;

	/*
	 * KIA4SM methods
	 */

	int set_sched_type(unsigned core, unsigned sched_type) override;
	int get_sched_type(unsigned core) override;
	void set(Genode::Ram_session_capability ram_cap) override;
	void deploy_queue(Genode::Dataspace_capability ds) override;
	void rq(Genode::Dataspace_capability ds) override;
	void dead(Genode::Dataspace_capability ds) override;

	void killed() override;
};


/**
 * Custom root RPC object to intercept session RPC object creation, modification, and destruction through the root interface
 */
class Rtcr::Cpu_root : public Genode::Root_component<Cpu_session>
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
	 * Monitor's PD root for the list of all PD sessions known to the child
	 *
	 * The PD sessions are used to translate child's PD sessions to parent's PD sessions.
	 * For creating a CPU thread, child needs to pass a PD session capability. Because the
	 * custom CPU session uses parent's CPU session (e.g. core's CPU session), it also has
	 * to pass a PD session which is known by the parent.
	 */
	Pd_root            &_pd_root;
	/**
	 * Lock for infos list
	 */
	Genode::Lock        _objs_lock;
	/**
	 * List for monitoring session objects
	 */
	Genode::List<Cpu_session> _session_rpc_objs;

	Genode::Xml_node *_config;
  
protected:
	Cpu_session *_create_session(const char *args);
	void _upgrade_session(Cpu_session *session, const char *upgrade_args);
	void _destroy_session(Cpu_session *session);

	inline Genode::Affinity::Location _read_child_affinity(Genode::Xml_node *config,
														   const char* child_name);
  
public:
	Cpu_root(Genode::Env &env,
			 Genode::Allocator &md_alloc,
			 Genode::Entrypoint &session_ep,
			 Pd_root &pd_root,
			 bool &bootstrap_phase,
			 Genode::Xml_node *config);

	
	~Cpu_root();


	Genode::List<Cpu_session> &sessions() { return _session_rpc_objs; }
};

#endif /* _RTCR_CPU_SESSION_H_ */
