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
	 * Indicator if the current checkpoint is finished
	 */
	Event _checkpoint_finished;

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
	 * event for signaling that a checkpoint is possible
	 */
	Event _ready_event;
	
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

protected:
	void ready();

	/**
	 * Abstract method which is called for a checkpoint by the thread. This
	 * method must be implemented by the inheriting class.
	 *
	 * /return true, if the next checkpoint is directly possible. If false is
	 * returned, the thread has to call ready().
	 */
	virtual void checkpoint() = 0;

	/**
	 * Abstract method which is called after a checkpoint by the thread.
	 */
	virtual void post_checkpoint() { }
	
	
public:
	/**
	 * \param env reference to a Genode environment
	 * \param name which represents the * thread name and the name attribute in
	 * \param ready is set by default and notifies that this thread is able to
	 *        directly start a checkpoint.
	 * the XML configuration.
	 */
	Checkpointable(Genode::Env &env, const char* name, bool ready = true);

	/**
	 * Pause the calling thread until this thread finished its current job.
	 *
	 * If no job is in progress, this method will directly return.
	 */
	void join_checkpoint();
		
	/**
	 * Starts a checkpoint
	 */
	void start_checkpoint();

	/**
	 * stop this thread
	 */
	void stop();

	/** 
	 * wait until this checkpointable is ready for a checkpoint 
	 */
	void wait_ready();

	/**
	 * \return true, if thread is ready to checkpoint
	 */
	bool is_ready() { return _ready_event.is_set(); }
};


#endif /* _RTCR_MODULE_H_ */
