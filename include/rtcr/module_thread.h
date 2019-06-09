/*
 * \brief  Threaded Container for a Module
 * \author Denis Huber
 * \author Johannes Fischer
 * \date   2019-03-21
 */

#ifndef _RTCR_MODULE_THREAD_H_
#define _RTCR_MODULE_THREAD_H_

/* Genode includes */
#include <util/list.h>
#include <base/heap.h>
#include <util/list.h>
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
 * 
 */
class Rtcr::Module_thread : public Genode::List<Module_thread>::Element, public Genode::Thread
{
private:

	Event _next_event;
	enum Job { CHECKPOINT, RESTORE, NONE };
	Job _next_job;
	Event _job_finished;
	bool _running;
	Target_state *_target_state;
public:
	Module &_module;
	
	Module_thread(Genode::Env &env,
		      Module &module,
		      Genode::Affinity::Location location,
		      Genode::Cpu_session &cpu);


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

	void entry();

	void stop();	
};


#endif /* _RTCR_MODULE_THREAD_H_ */
