/*
 * \brief One thread signals an event and other threads wait for it.
 * \author Johannes Fischer
 * \date 2019-06-08
 */

#ifndef _RTCR_EVENT_H_
#define _RTCR_EVENT_H_

#include <base/lock.h>
#include <util/fifo.h>

namespace Rtcr {
	class Event;
}


class Rtcr::Event
{
private:
	struct Caller : Genode::Fifo<Caller>::Element
	{
		Genode::Lock lock { Genode::Lock::LOCKED };

		void block()   { lock.lock();   }
		void wake_up() { lock.unlock(); }
	};

	Genode::Fifo<Caller> _waiting_callers;
	Genode::Lock _meta_lock;
	bool _set;

public:
	Event() :_set(false) {}
	Event(bool set) :_set(set) {}
	~Event() { release(); }

	void release()
	{
		_meta_lock.lock();
		_waiting_callers.dequeue_all([] (Caller &caller) {
				caller.wake_up();
			});
		_meta_lock.unlock();
	}

	void wait()
	{
		if(_set) return;

		_meta_lock.lock();
		Caller caller;
		_waiting_callers.enqueue(caller);
		_meta_lock.unlock();
		caller.block();
	}

	void set()
	{
		_set = true;
		release();
	}

	void unset()
	{
		_set = false;
	}

	bool is_set() {
		return _set;
	}
};


#endif /* _RTCR_EVENT_H_ */
