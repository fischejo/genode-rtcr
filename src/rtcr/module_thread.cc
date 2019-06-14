/*
 * \brief  Threaded Container for a Module
 * \author Johannes Fischer
 * \date   2019-06-10
 */

#include <rtcr/module_thread.h>

using namespace Rtcr;

Module_thread::Module_thread(Genode::Env &env,
			     Module &module,
			     Genode::List<Module> &modules,
			     Genode::Affinity::Location location,
			     Genode::Cpu_session &cpu)
	:
	Thread(env,
	       module.name().string(),
	       64*1024,
	       location,
	       Genode::Thread::Weight(),
	       cpu),
	_module(module),
	_modules(modules),
	_running(true),
	_next_job(NONE)
{}


void Module_thread::initialize()
{
	start();
}


void Module_thread::stop()
{
	_running=false;
	// TODO: not sure if this works, if not than use _next_event.release()
	cancel_blocking(); 
}


void Module_thread::entry()
{
#ifdef DEBUG
	Genode::log("\e[1m\e[38;5;199m", "Module_thread[", name(),
		    "] started", "\033[0m");
#endif	

	/* initialize the module */
	_job_finished.unset();
	_module.initialize(_modules);
	_job_finished.set();
	
	while(_running) {
		/* wait for the next job */
		_next_event.wait();
		_next_event.unset();

		if(!_running)
			return;

		/* start next job */
		Job _current_job = _next_job;
		_next_job = NONE;
		Target_state *ts = _target_state;
		Module_state *module_state;
		switch(_current_job) {
		case CHECKPOINT:
			module_state = _module.checkpoint();
			/* store current module state in the target state */
			if(module_state) {
				ts->store(_module.name(), *module_state);
			}
			break;
			
		case RESTORE:
			/* lookup if target_state stores a state of the module */
			module_state = ts->state(_module.name());
			/* restore module based on the looked up state */
			_module.restore(module_state);
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
