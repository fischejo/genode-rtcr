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
#include <os/config.h>

/* Rtcr includes */
#include <util/event.h>

namespace Rtcr {
	class Checkpointable;
}

using namespace Rtcr;


/**
 * This class provides an interface for checkpointing in a seperate thread.
 */
class Rtcr::Checkpointable : private Genode::Thread
{
private:

	Genode::Affinity::Location _affinity;	
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

	/**
	 * Reads the affinity of the thread from the XML config
	 *
	 * Example configuration:
	 *
	 * ```XML
	 * <checkpointable name="cpu_session" xpos="1" ypos="0" />
	 * ```
	 */
	inline Genode::Affinity::Location _read_affinity(const char* node_name);

  
public:
	/**
	 * \param env reference to a Genode environment
	 * \param name which represents the * thread name and the name attribute in
	 * the XML configuration.
	 */
	Checkpointable(Genode::Env &env, const char* name);


	/**
	 * Pause the calling thread until this thread finished its current job.
	 *
	 * If no job is in progress, this method will directly return.
	 */
	virtual void join_checkpoint();

	/**
	 * Abstract method which is called for a checkpoint by the thread. This
	 * method must be implemented by the inheriting class.
	 */
	virtual void checkpoint() = 0;

  
	/**
	 * Starts a checkpoint
	 */
	void start_checkpoint();

	/**
	 * stop this thread
	 */
	void stop();
};


#endif /* _RTCR_MODULE_H_ */
