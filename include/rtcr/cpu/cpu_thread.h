/*
 * \brief  Intercepting Cpu thread
 * \author Denis Huber
 * \date   2016-10-21
 */

#ifndef _RTCR_CPU_THREAD_H_
#define _RTCR_CPU_THREAD_H_

/* Genode inlcudes */
#include <cpu_thread/client.h>
#include <base/allocator.h>
#include <base/rpc_server.h>
#include <base/entrypoint.h>
#include <util/fifo.h>
#include <pd_session/connection.h>
#include <base/session_object.h>

/* Rtcr includes */
#include <rtcr/cpu/cpu_thread_info.h>

namespace Rtcr {
	class Cpu_thread;
}


class Rtcr::Cpu_thread : public Genode::Rpc_object<Genode::Cpu_thread>,
                         public Cpu_thread_info

{
public:
	bool bootstrapped;
	
protected:

	// Modifiable state
	bool _started;
	bool _paused;
	bool _single_step;
	Genode::Affinity::Location        _affinity; // Is also used for creation
	Genode::Signal_context_capability _sigh;
	Genode::Pd_session_capability _pd_session_cap;

private:
	/**
	 * Allocator for Region map's attachments
	 */
	Genode::Allocator &_md_alloc;

	Genode::Entrypoint &_ep;	
	/**
	 * Wrapped region map from parent, usually core
	 */
	Genode::Cpu_thread_client  _parent_cpu_thread;
	Genode::Capability<Genode::Cpu_thread> _parent_cpu_thread_cap;	
	
public:
	
	Cpu_thread(Genode::Allocator &md_alloc,
	           Genode::Capability<Genode::Cpu_thread> cpu_thread_cap,
	           Genode::Pd_session_capability pd_session_cap,
	           const char *name,
	           Genode::Cpu_session::Weight weight,
	           Genode::addr_t utcb,
	           Genode::Affinity::Location affinity,
	           bool bootstrapped,
	           Genode::Entrypoint &ep);

	~Cpu_thread();

	void checkpoint();
	
	/**
	 * Called by the corresponding CPU session for silently pausing this
	 * thread. Calling this method will not lead to updated information in the
	 * hot storage.
	 */
	void silent_pause();

	/**
	 * Called by the corresponding CPU session for silently resuming this
	 * thread. Calling this method will not lead to updated information in the
	 * hot storage.
	 */	
	void silent_resume();
  
	Genode::Capability<Genode::Cpu_thread> parent_cap() { return _parent_cpu_thread_cap; }


	/******************************
	 ** Cpu thread Rpc interface **
	 ******************************/

	Genode::Dataspace_capability utcb () override;
	void start(Genode::addr_t ip, Genode::addr_t sp) override;
	void pause() override;
	void resume() override;
	void cancel_blocking() override;
	Genode::Thread_state state() override;
	void state(Genode::Thread_state const &state) override;
	void exception_sigh(Genode::Signal_context_capability handler) override;
	void single_step(bool enabled) override;
	void affinity(Genode::Affinity::Location location) override;
	unsigned trace_control_index() override;
	Genode::Dataspace_capability trace_buffer() override;
	Genode::Dataspace_capability trace_policy() override;
};

#endif /* _RTCR_CPU_THREAD_H_ */
