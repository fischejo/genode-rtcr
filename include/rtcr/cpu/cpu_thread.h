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

/* Rtcr includes */
#include <rtcr/info_structs.h>

namespace Rtcr {
	class Cpu_thread;
	class Cpu_thread_info;
}

struct Rtcr::Cpu_thread_info : Normal_info {
	Genode::uint16_t pd_session_badge;
	Genode::Cpu_session::Name name;
	Genode::Cpu_session::Weight weight;
	Genode::addr_t utcb;
	bool started;
	bool paused;
	bool single_step;
	Genode::Affinity::Location affinity;
	Genode::uint16_t sigh_badge;
	Genode::Thread_state ts;
	
	Cpu_thread_info(const char *name,
					Genode::Cpu_session::Weight weight,
					Genode::addr_t utcb)
		: name(name), weight(weight), utcb(utcb) {}
		

	void print(Genode::Output &output) const {
		using Genode::Hex;
		Genode::print(output, "Thread:\n");
		Genode::print(output,
					  "   pd_session_badge=", pd_session_badge,
					  ", name=", name,
					  ", weight=", weight.value,
					  ", utcb=", Hex(utcb));
  
		Genode::print(output,
					  ", started=", started,
					  ", paused=", paused,
					  ", single_step=", single_step);
  
		Genode::print(output,
					  ", affinity=(", affinity.xpos(),
					  "x", affinity.ypos(),
					  ", ", affinity.width(),
					  "x", affinity.height(), ")");
  
		Genode::print(output, ", sigh_badge=", sigh_badge, "\n");

		Genode::print(output, "   r0-r4: ",
					  Hex(ts.r0, Hex::PREFIX, Hex::PAD), " ",
					  Hex(ts.r1, Hex::PREFIX, Hex::PAD), " ",
					  Hex(ts.r2, Hex::PREFIX, Hex::PAD), " ",
					  Hex(ts.r3, Hex::PREFIX, Hex::PAD), " ",
					  Hex(ts.r4, Hex::PREFIX, Hex::PAD), "\n");
  
		Genode::print(output, "   r5-r9: ",
					  Hex(ts.r5, Hex::PREFIX, Hex::PAD), " ",
					  Hex(ts.r6, Hex::PREFIX, Hex::PAD), " ",
					  Hex(ts.r7, Hex::PREFIX, Hex::PAD), " ",
					  Hex(ts.r8, Hex::PREFIX, Hex::PAD), " ",
					  Hex(ts.r9, Hex::PREFIX, Hex::PAD), "\n");

		Genode::print(output, "   r10-r12: ",
					  Hex(ts.r10, Hex::PREFIX, Hex::PAD), " ",
					  Hex(ts.r11, Hex::PREFIX, Hex::PAD), " ",
					  Hex(ts.r12, Hex::PREFIX, Hex::PAD), "\n");

		Genode::print(output, "   sp, lr, ip, cpsr, cpu_e: ",
					  Hex(ts.sp, Hex::PREFIX, Hex::PAD), " ",
					  Hex(ts.lr, Hex::PREFIX, Hex::PAD), " ",
					  Hex(ts.ip, Hex::PREFIX, Hex::PAD), " ",
					  Hex(ts.cpsr, Hex::PREFIX, Hex::PAD), " ",
					  Hex(ts.cpu_exception, Hex::PREFIX, Hex::PAD));
		Genode::print(output, "\n");		
	}
};




class Rtcr::Cpu_thread : public Genode::Rpc_object<Genode::Cpu_thread>,
						 public Genode::List<Cpu_thread>::Element,
						 public Genode::Fifo<Cpu_thread>::Element		   
{
public:
	/******************
	 ** COLD STORAGE **
	 ******************/

	Cpu_thread_info info;
  
protected:

	// Modifiable state
	bool _started;
	bool _paused;
	bool _single_step;
	Genode::Affinity::Location        _affinity; // Is also used for creation
	Genode::Signal_context_capability _sigh;
	bool &_bootstrapped;  
	Genode::Pd_session_capability _pd_session_cap;

private:
	/**
	 * Allocator for Region map's attachments
	 */
	Genode::Allocator &_md_alloc;
	/**
	 * Wrapped region map from parent, usually core
	 */
	Genode::Cpu_thread_client  _parent_cpu_thread;

public:
	/**
	 * List and Fifo provide a next() method. In general, you want to use the
	 * list implementation.
	 */	
	using Genode::List<Cpu_thread>::Element::next;
	
	Cpu_thread(Genode::Allocator &md_alloc,
			   Genode::Capability<Genode::Cpu_thread> cpu_thread_cap,
			   Genode::Pd_session_capability pd_session_cap,
			   const char *name,
			   Genode::Cpu_session::Weight weight,
			   Genode::addr_t utcb,
			   Genode::Affinity::Location affinity,
			   bool &bootstrap_phase);

	~Cpu_thread() {}

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
  
	Genode::Capability<Genode::Cpu_thread> parent_cap() { return _parent_cpu_thread; }

	Cpu_thread *find_by_badge(Genode::uint16_t badge);
	Cpu_thread *find_by_name(const char* name);

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
