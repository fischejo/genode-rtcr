/*
 * \brief  Threaded Container for a Module
 * \author Johannes Fischer
 * \date   2019-06-10
 */

#ifndef _RTCR_MODULE_THREAD_H_
#define _RTCR_MODULE_THREAD_H_

/* Genode includes */
#include <util/list.h>
#include <base/heap.h>
#include <base/service.h>
#include <cpu_session/connection.h>
#include <cpu_thread/client.h>

/* Rtcr includes */
#include <rtcr/module_state.h>
#include <rtcr/target_state.h>
#include <rtcr/module.h>
#include <util/event.h>


namespace Rtcr {
	class Module_thread;
}

using namespace Rtcr;

/**
 * This class is a container around the module and executes `initialize()`,
 * `checkpoint()` and `restore()` in a seperate thread.
 */
class Rtcr::Module_thread : public Genode::List<Module_thread>::Element, private Genode::Thread
{
private:
	/**
	 * Indicator which pause this thread until a new job
	 * (checkpoint,restore) are triggered.
	 */
	Event _next_event;
	
	/**
	 * Types of jobs
	 */
	enum Job { CHECKPOINT, RESTORE, NONE };

	/**
	 * Defines the next job which should be processed.
	 * * If CHECKPOINT than the next job will be a checkpoint of `module`
	 * * If RESTORE than the next job will be a restore of `module`
	 * * If NONE, no job is registered right now.
	 */
	Job _next_job;

	/**
	 * Indicator if the current job is finished
	 */
	Event _job_finished;

	/**
	 * If `_running` is false, no further jobs are processed and the
	 * `entry()` method will be left. The thread stops.
	 */
	bool _running;

	/**
	 * Container for storing the checkpointed state of the module
	 */
	Target_state *_target_state;

	Genode::List<Module> &_modules;

	/**
	 * The module which is wrapped by this class.
	 */
	Module &_module;
	
public:


	/**
	 * \param env reference to a Genode environment 
	 * \param module which should be wrapped and executed in a separate thread
	 * \param affinity of the thread
	 * \param cpu of the thread
	 */
	Module_thread(Genode::Env &env,
		      Module &module,
		      Genode::List<Module> &modules,
		      Genode::Affinity::Location location,
		      Genode::Cpu_session &cpu);

	/**
	 * Pause the calling thread until this thread finished its current job.
	 *
	 * If no job is in progress, this method will directly return.
	 */
	void wait_until_finished();

	/**
	 * Called when this module should run a checkpoint
	 *
	 * \return Module_state which stores the checkpointed data.
	 */
	void checkpoint(Target_state &target_state);

	/**
	 * Restore module from a checkpointed state.
	 *
	 * \param state A object of type Module_state which provides the
	 *              checkpointed state.
	 */
	void restore(Target_state &target_state);

	/**
	 * Entrypoint of the started thread.
	 */
	void entry();

	/**
	 * stop this thread
	 */
	void stop();

	/**
	 * Starts the thread and initializes the module.
	 * 
	 * Pay attention: Call this method only once after all modules are added
	 * to the module list. This list was handed over in the constructor.
	 */
	void initialize();

	/**
	 * Name of the thread (same as the name of the module).
	 */
	Module_name name() { return _module.name(); };
};


#endif /* _RTCR_MODULE_THREAD_H_ */
