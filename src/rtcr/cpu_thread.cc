/*
 * \brief  Intercepting Cpu thread
 * \author Denis Huber
 * \date   2016-10-21
 */

#include <rtcr/cpu/cpu_thread.h>

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
	ck_name (name),
	ck_pd_session_badge (pd_session_cap.local_name()),
	ck_weight (weight),
	ck_utcb (utcb),
	_bootstrapped(bootstrap_phase),
	_affinity(affinity)
{
	if(verbose_debug) Genode::log("\033[33m", "Thread", "\033[0m<\033[35m", ck_name,
				      "\033[0m>(parent ", _parent_cpu_thread,")");
}


Cpu_thread::~Cpu_thread()
{
	if(verbose_debug) Genode::log("\033[33m", "~Thread", "\033[0m<\033[35m", ck_name,
				      "\033[0m> ", _parent_cpu_thread);
}

void Cpu_thread::checkpoint()
{
  ck_badge = cap().local_name();

  // TODO
  //  ck_kcap = _core_module->find_kcap_by_badge(ck_badge);
  
  ck_started = _started;
  ck_paused = _paused;
  ck_single_step = _single_step;
  ck_affinity = _affinity; // TODO FJO: clone it
  ck_sigh_badge = _sigh.local_name();
  ck_bootstrapped = _bootstrapped;

  /* XXX does not guarantee to return the current thread registers */
  ck_ts = _parent_cpu_thread.state();
}


void Cpu_thread::print(Genode::Output &output) const
{
  using Genode::Hex;

  Genode::print(output,
		", pd_session_badge=", ck_pd_session_badge,
		", name=", ck_name,
		", weight=", ck_weight.value,
		", utcb=", Hex(ck_utcb));
  
  Genode::print(output,
		", started=", ck_started,
		", paused=", ck_paused,
		", single_step=", ck_single_step);
  
  Genode::print(output,
		", affinity=(", ck_affinity.xpos(),
		"x", ck_affinity.ypos(),
		", ", ck_affinity.width(),
		"x", ck_affinity.height(), ")");
  
  Genode::print(output, ", sigh_badge=", ck_sigh_badge, "\n");

  Genode::print(output, "  r0-r4: ", Hex(ck_ts.r0, Hex::PREFIX, Hex::PAD), " ",
		Hex(ck_ts.r1, Hex::PREFIX, Hex::PAD), " ", Hex(ck_ts.r2, Hex::PREFIX, Hex::PAD), " ",
		Hex(ck_ts.r3, Hex::PREFIX, Hex::PAD), " ", Hex(ck_ts.r4, Hex::PREFIX, Hex::PAD), "\n");
  
  Genode::print(output, "  r5-r9: ", Hex(ck_ts.r5, Hex::PREFIX, Hex::PAD), " ",
		Hex(ck_ts.r6, Hex::PREFIX, Hex::PAD), " ", Hex(ck_ts.r7, Hex::PREFIX, Hex::PAD), " ",
		Hex(ck_ts.r8, Hex::PREFIX, Hex::PAD), " ", Hex(ck_ts.r9, Hex::PREFIX, Hex::PAD), "\n");
		Genode::print(output, "  r10-r12: ", Hex(ck_ts.r10, Hex::PREFIX, Hex::PAD), " ",
			      Hex(ck_ts.r11, Hex::PREFIX, Hex::PAD), " ", Hex(ck_ts.r12, Hex::PREFIX, Hex::PAD), "\n");
		Genode::print(output, "  sp, lr, ip, cpsr, cpu_e: ", Hex(ck_ts.sp, Hex::PREFIX, Hex::PAD), " ",
			      Hex(ck_ts.lr, Hex::PREFIX, Hex::PAD), " ", Hex(ck_ts.ip, Hex::PREFIX, Hex::PAD), " ",
			      Hex(ck_ts.cpsr, Hex::PREFIX, Hex::PAD), " ", Hex(ck_ts.cpu_exception, Hex::PREFIX, Hex::PAD));
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
	if(!Genode::strcmp(name, ck_name.string()))
		return this;
	Cpu_thread *obj = next();
	return obj ? obj->find_by_name(name) : 0;
}


Genode::Dataspace_capability Cpu_thread::utcb()
{
	if(verbose_debug) Genode::log("Thread<\033[35m", ck_name, "\033[0m>::\033[33m",
				      __func__, "\033[0m()");
	auto result = _parent_cpu_thread.utcb();
	if(verbose_debug) Genode::log("  result: ", result);

	return result;
}


void Cpu_thread::start(Genode::addr_t ip, Genode::addr_t sp)
{
	if(verbose_debug) Genode::log("Thread<\033[35m", ck_name, "\033[0m>::\033[33m",
				      __func__, "\033[0m(ip=",
				      Genode::Hex(ip), ", sp=", Genode::Hex(sp), ")");
	_parent_cpu_thread.start(ip, sp);
	_started = true;

}


void Cpu_thread::pause()
{
	if(verbose_debug) Genode::log("Thread<\033[35m", ck_name, "\033[0m>::\033[33m",
				      __func__, "\033[0m()");
	_parent_cpu_thread.pause();
	_paused = true;
}


void Cpu_thread::resume()
{
	if(verbose_debug) Genode::log("Thread<\033[35m", ck_name, "\033[0m>::\033[33m",
				      __func__, "\033[0m()");
	_parent_cpu_thread.resume();
	_paused = false;
}

void Cpu_thread::silent_pause()
{
	if(verbose_debug) Genode::log("Thread<\033[35m", ck_name, "\033[0m>::\033[33m",
				      __func__, "\033[0m()");
	_parent_cpu_thread.pause();
}


void Cpu_thread::silent_resume()
{
	if(verbose_debug) Genode::log("Thread<\033[35m", ck_name, "\033[0m>::\033[33m",
				      __func__, "\033[0m()");
	_parent_cpu_thread.resume();
}


void Cpu_thread::cancel_blocking()
{
	if(verbose_debug) Genode::log("Thread<\033[35m", ck_name, "\033[0m>::\033[33m",
				      __func__, "\033[0m()");
	_parent_cpu_thread.cancel_blocking();
}


Genode::Thread_state Cpu_thread::state()
{
	if(verbose_debug) Genode::log("Thread<\033[35m", ck_name, "\033[0m>::\033[33m",
				      __func__, "\033[0m()");
	auto result = _parent_cpu_thread.state();
	if(verbose_debug) Genode::log("  result: state<kcap=",
				      Genode::Hex(result.kcap), ", id=", result.id, ">");

	return result;
}


void Cpu_thread::state(const Genode::Thread_state& state)
{
	if(verbose_debug) Genode::log("Thread<\033[35m", ck_name, "\033[0m>::\033[33m",
				      __func__, "\033[0m(state<kcap=",
				      Genode::Hex(state.kcap), ", id=", state.id, ">)");
	_parent_cpu_thread.state(state);
}


void Cpu_thread::exception_sigh(Genode::Signal_context_capability handler)
{
	if(verbose_debug) Genode::log("Thread<\033[35m", ck_name, "\033[0m>::\033[33m",
				      __func__, "\033[0m(sigh=", handler, ")");
	_parent_cpu_thread.exception_sigh(handler);
	_sigh = handler;
}


void Cpu_thread::single_step(bool enabled)
{
	if(verbose_debug) Genode::log("Thread<\033[35m", ck_name, "\033[0m>::\033[33m",
				      __func__, "\033[0m(", enabled, ")");
	_parent_cpu_thread.single_step(enabled);
	_single_step = enabled;
}


void Cpu_thread::affinity(Genode::Affinity::Location location)
{
	if(verbose_debug) Genode::log("Thread<\033[35m", ck_name, "\033[0m>::\033[33m",
				      __func__, "\033[0m(loc<pos=",
				      location.xpos(), "x", location.ypos(), ", dim=",
				      location.width(), "x", location.height(), ")");
	_parent_cpu_thread.affinity(location);
	_affinity = location;
}


unsigned Cpu_thread::trace_control_index()
{
	if(verbose_debug) Genode::log("Thread<\033[35m", ck_name, "\033[0m>::\033[33m",
				      __func__, "\033[0m()");
	auto result = _parent_cpu_thread.trace_control_index();
	if(verbose_debug) Genode::log("  result: ", result);
	return result;
}


Genode::Dataspace_capability Cpu_thread::trace_buffer()
{
	if(verbose_debug) Genode::log("Thread<\033[35m", ck_name, "\033[0m>::\033[33m",
				      __func__, "\033[0m()");
	auto result = _parent_cpu_thread.trace_buffer();
	if(verbose_debug) Genode::log("  result: ", result);
	return result;
}


Genode::Dataspace_capability Cpu_thread::trace_policy()
{
	if(verbose_debug) Genode::log("Thread<\033[35m", ck_name, "\033[0m>::\033[33m",
				      __func__, "\033[0m()");
	auto result = _parent_cpu_thread.trace_policy();
	if(verbose_debug) Genode::log("  result: ", result);
	return result;
}
