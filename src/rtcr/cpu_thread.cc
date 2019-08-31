/*
 * \brief  Intercepting Cpu thread
 * \author Denis Huber
 * \author Johannes Fischer
 * \date   2019-08-29
 */

#include <rtcr/cpu/cpu_thread.h>

#ifdef PROFILE
#include <util/profiler.h>
#define PROFILE_THIS_CALL PROFILE_FUNCTION("gold");
#else
#define PROFILE_THIS_CALL
#endif

#if DEBUG 
#define DEBUG_THIS_CALL Genode::log("\e[38;5;215m", __PRETTY_FUNCTION__, "\033[0m");
#else
#define DEBUG_THIS_CALL
#endif


using namespace Rtcr;

Cpu_thread::Cpu_thread(Genode::Allocator &md_alloc,
					   Genode::Capability<Genode::Cpu_thread> cpu_thread_cap,
					   Genode::Pd_session_capability pd_session_cap,
					   const char *name,
					   Genode::Cpu_session::Weight weight,
					   Genode::addr_t utcb,
					   Genode::Affinity::Location affinity,
					   bool &bootstrap_phase)
	:
	_md_alloc (md_alloc),
	_parent_cpu_thread (cpu_thread_cap),
	_pd_session_cap(pd_session_cap),
	info(name, weight, utcb),
	_bootstrapped(bootstrap_phase),
	_affinity(affinity)
{
	DEBUG_THIS_CALL
}


void Cpu_thread::checkpoint()
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL	
	info.badge = cap().local_name();

	// TODO
	//  ck_kcap = _core_module->find_kcap_by_badge(ck_badge);
  
	info.started = _started;
	info.paused = _paused;
	info.single_step = _single_step;
	info.affinity = _affinity; // TODO FJO: clone it
	info.sigh_badge = _sigh.local_name();
	info.bootstrapped = _bootstrapped;
	info.pd_session_badge = _pd_session_cap.local_name();

	/* XXX does not guarantee to return the current thread registers */
	info.ts = _parent_cpu_thread.state();
}


Cpu_thread *Cpu_thread::find_by_badge(Genode::uint16_t badge)
{
	if(badge == cap().local_name())
		return this;
	Cpu_thread *obj = next();
	return obj ? obj->find_by_badge(badge) : 0;
}


Cpu_thread *Cpu_thread::find_by_name(const char* name)
{
	if(!Genode::strcmp(name, info.name.string()))
		return this;
	Cpu_thread *obj = next();
	return obj ? obj->find_by_name(name) : 0;
}


Genode::Dataspace_capability Cpu_thread::utcb()
{
	return _parent_cpu_thread.utcb();
}


void Cpu_thread::start(Genode::addr_t ip, Genode::addr_t sp)
{
	_parent_cpu_thread.start(ip, sp);
	_started = true;

}


void Cpu_thread::pause()
{
	_parent_cpu_thread.pause();
	_paused = true;
}


void Cpu_thread::resume()
{
	_parent_cpu_thread.resume();
	_paused = false;
}

void Cpu_thread::silent_pause()
{
	DEBUG_THIS_CALL
	_parent_cpu_thread.pause();
}


void Cpu_thread::silent_resume()
{
	DEBUG_THIS_CALL
	_parent_cpu_thread.resume();
}


void Cpu_thread::cancel_blocking()
{
	_parent_cpu_thread.cancel_blocking();
}


Genode::Thread_state Cpu_thread::state()
{
	return _parent_cpu_thread.state();
}


void Cpu_thread::state(const Genode::Thread_state& state)
{
	_parent_cpu_thread.state(state);
}


void Cpu_thread::exception_sigh(Genode::Signal_context_capability handler)
{
	_parent_cpu_thread.exception_sigh(handler);
	_sigh = handler;
}


void Cpu_thread::single_step(bool enabled)
{
	_parent_cpu_thread.single_step(enabled);
	_single_step = enabled;
}


void Cpu_thread::affinity(Genode::Affinity::Location location)
{
	_parent_cpu_thread.affinity(location);
	_affinity = location;
}


unsigned Cpu_thread::trace_control_index()
{
	return _parent_cpu_thread.trace_control_index();
}


Genode::Dataspace_capability Cpu_thread::trace_buffer()
{
	return _parent_cpu_thread.trace_buffer();
}


Genode::Dataspace_capability Cpu_thread::trace_policy()
{
	return _parent_cpu_thread.trace_policy();
}
