/*
 * \brief  Threaded Container for a Checkpointable
 * \author Johannes Fischer
 * \date   2019-06-10
 */

#ifndef _RTCR_CHECKPOINTABLE_H_
#define _RTCR_CHECKPOINTABLE_H_

/* Genode includes */
#include <util/list.h>
#include <base/heap.h>
#include <base/service.h>
#include <cpu_session/connection.h>
#include <cpu_thread/client.h>
#include <util/xml_node.h>

/* Rtcr includes */
#include <util/event.h>

namespace Rtcr {
  class Checkpointable;
}

using namespace Rtcr;


/**
 * This class is a container around the module and executes `initialize()`,
 * `checkpoint()` and `restore()` in a seperate thread.
 */
class Rtcr::Checkpointable : private Genode::Thread
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
	enum Job { CHECKPOINT, NONE };

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
	 * Entrypoint of the started thread.
	 */
	void entry();

	inline Genode::Affinity::Location _read_affinity(Genode::Xml_node *config,
						   const char* node_name);

  
public:
	/**
	 * \param env reference to a Genode environment 
	 * \param module which should be wrapped and executed in a separate thread
	 * \param optional XML config of module (necessary to parse affinity of thread)
	 * \param cpu of the thread
	 */
        Checkpointable(Genode::Env &env,
			 Genode::Xml_node *config,
			 const char* name);


	/**
	 * Pause the calling thread until this thread finished its current job.
	 *
	 * If no job is in progress, this method will directly return.
	 */
	virtual void wait_until_finished();

	/**
	 * Called when this module should run a checkpoint
	 */
	virtual void checkpoint() = 0;

  
	/**
	 * Called when this module should run a checkpoint
	 *
	 * \return Checkpointable_state which stores the checkpointed data.
	 */
	void start_checkpoint();


	/**
	 * stop this thread
	 */
	void stop();
};


#endif /* _RTCR_MODULE_H_ */
