/*
 * \brief Semaphore protected Fifo Implementation
 * \author Johannes Fischer
 * \date 2019-06-06
 */

#ifndef _RTCR_SYNCED_FIFO_H_
#define _RTCR_SYNCED_FIFO_H_

#include <util/fifo.h>
#include <base/semaphore.h>

namespace Rtcr {
	template<class T> class Synced_fifo;
}



template<class T> class Rtcr::Synced_fifo : public Genode::Fifo<T>
{
private:
	Genode::Semaphore _sema;
	Genode::Lock _meta_lock;

	struct Caller : Genode::Fifo<Caller>::Element
	{
		Genode::Lock lock { Genode::Lock::LOCKED };

		void block()   { lock.lock();   }
		void wake_up() { lock.unlock(); }
	};

	Genode::Fifo<Caller> _waiting_callers;

	void _release_callers()
	{
		while(Caller *caller = _waiting_callers.dequeue()) {
			caller->wake_up();
		}
	}

public:

	void wait_until_empty()
	{
		_meta_lock.lock();
		if(!Genode::Fifo<T>::empty()) {
			Caller caller;
			_waiting_callers.enqueue(&caller);
			_meta_lock.unlock();
			caller.block();
		}
	}

	/**
	 * Remove element explicitely from queue
	 */
	void remove(T *qe)
	{
		_sema.down();
		_meta_lock.lock();
		Genode::Fifo<T>::remove(qe);
		if(Genode::Fifo<T>::empty())
			_release_callers();
		_meta_lock.unlock();
	}

	/**
	 * Attach element at the end of the queue
	 */
	void enqueue(T *e)
	{
		Genode::Fifo<T>::enqueue(e);
		_sema.up();
	}

	/**
	 * Remove head element from queue
	 * 
	 * \param block block current thread, if no element is in queue.
	 *
	 * \return  head element or 0 if queue is empty
	 */
	T *dequeue(bool block=true)
	{
		if(!block && Genode::Fifo<T>::empty())
			return 0;

		_sema.down();
		_meta_lock.lock();
		T *head = Genode::Fifo<T>::dequeue();
		if(Genode::Fifo<T>::empty())
			_release_callers();
		_meta_lock.unlock();
		return head;
	}
};


#endif /* _RTCR_SYNCED_FIFO_H_ */
