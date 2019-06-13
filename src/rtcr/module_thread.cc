/*
 * \brief  Child creation
 * \author Denis Huber
 * \date   2016-08-04
 */

#include <rtcr/module_thread.h>

#ifdef PROFILE
#include <util/profiler.h>
#define PROFILE_THIS_CALL PROFILE_FUNCTION("blue");
#else
#define PROFILE_THIS_CALL
#endif

#if DEBUG 
#define DEBUG_THIS_CALL Genode::log("\033[36m", __PRETTY_FUNCTION__, "\033[0m");
#else
#define DEBUG_THIS_CALL
#endif


using namespace Rtcr;

Module_thread::Module_thread(Genode::Env &env,
			     Module &_module,
			     Genode::Affinity::Location location,
			     Genode::Cpu_session &cpu)
	:
	Thread(env, _module.name().string(), 64*1024, location, Genode::Thread::Weight(), cpu),
	module(_module),
	_running(true),
	_next_job(NONE)
{
	
}


void Module_thread::stop()
{
	_running=false;
	cancel_blocking(); // not sure if this works, if not than use _next_event.release()
}


void Module_thread::entry()
{
	Genode::log("\e[1m\e[38;5;199m", "Thread[", module.name(),"] started", "\033[0m");
	while(_running) {

		/* wait for the next job */
		_next_event.wait();
		_next_event.unset();
		
		Genode::log("\e[1m\e[38;5;199m", "Thread[", module.name(),"] got job", "\033[0m");
		if(!_running)
			return;

		/* start next job */
		Job _current_job = _next_job;
		_next_job = NONE;
		Target_state *ts = _target_state;
		Module_state *module_state;
		switch(_current_job) {
		case CHECKPOINT:
			Genode::log("\e[1m\e[38;5;199m", "Thread[", module.name(),"] checkpoint", "\033[0m");
			module_state = module.checkpoint();
			/* store current module state in the target state */
			if(module_state) {
				ts->store(module.name(), *module_state);
			}
			Genode::log("\e[1m\e[38;5;199m", "Thread[", module.name(),"] checkpoint finished", "\033[0m");			
			break;
			
		case RESTORE:
			/* lookup if target_state stores a state of the module */
			module_state = ts->state(module.name());
			/* restore module based on the looked up state */
			module.restore(module_state);
			break;
		}
		_job_finished.set();
	}
}



void Module_thread::checkpoint(Target_state &target_state)
{
	_target_state = &target_state;
	_next_job = CHECKPOINT;
	_job_finished.unset(); // must come before trigger next job.	
	_next_event.set();
}


void Module_thread::wait_until_finished()
{
	_job_finished.wait();
}

void Module_thread::restore(Target_state &target_state)
{
	_target_state = &target_state;	
	_next_job = RESTORE;
	_job_finished.unset(); // must come before trigger next job.		
	_next_event.set();
}

