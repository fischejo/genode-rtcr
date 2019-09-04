/*
 * \brief  Monitoring ram dataspace creation/destruction
 * \author Denis Huber
 * \author Johannes Fischer
 * \date   2019-08-29
 */

#ifndef _RTCR_RAM_DATASPACE_INFO_H_
#define _RTCR_RAM_DATASPACE_INFO_H_

/* Genode includes */
#include <util/list.h>
#include <util/fifo.h>
#include <ram_session/ram_session.h>
#include <region_map/client.h>

/* Rtcr includes */
#include <rtcr/info_structs.h>

namespace Rtcr {
	struct Ram_dataspace;
	struct Ram_dataspace_info;
}

struct Rtcr::Ram_dataspace_info : Normal_info {
	Genode::Ram_dataspace_capability dst_cap;
	Genode::size_t timestamp;
	Genode::Ram_dataspace_capability const cap;
	Genode::size_t                   const size;
	Genode::Cache_attribute          const cached;

	Ram_dataspace_info(Genode::Ram_dataspace_capability const cap,
					   Genode::size_t const size,
					   Genode::Cache_attribute const cached)
		: cap(cap), size(size), cached(cached) {}

	void print(Genode::Output &output) const {
		using Genode::Hex;
		Genode::print(output,
					  "   cap=", cap,
					  ", badge=", cap.local_name(),
					  ", size=", Hex(size),
					  ", cached=", static_cast<unsigned>(cached),
					  ", timestamp=", timestamp, "\n");
	}
};

/**
 * Monitors allocated Ram dataspaces
 */
struct Rtcr::Ram_dataspace : private Simple_counter<Ram_dataspace>,
			Genode::List<Ram_dataspace>::Element,
			Genode::Fifo<Ram_dataspace>::Element
{
	/******************
	 ** COLD STORAGE **
	 ******************/
	
	Ram_dataspace_info info;
	
	/*****************
	 ** HOT STORAGE **
	 *****************/

	/* pointers where ds are attached */
	void *src = nullptr;
	void *dst = nullptr;
	
	
	/**
	 * List and Fifo provide a next() method. In general, you want to use the
	 * list implementation.
	 */
	using Genode::List<Ram_dataspace>::Element::next;
	
	bool _bootstrapped;
	bool is_region_map;

	void checkpoint() {
		info.bootstrapped = _bootstrapped;
		info.timestamp = timestamp();
	}

	/**
	 * Pointer to extra data. This can be used by an extending
	 * implementation in order to prevent an inheritance of this class.
	 */
	void *storage;


	Ram_dataspace(Genode::Ram_dataspace_capability ds_cap,
					   Genode::size_t size,
					   Genode::Cache_attribute cached,
					   bool bootstrapped)
		:
		Ram_dataspace(ds_cap, size, cached, bootstrapped, nullptr)
		{ }

	Ram_dataspace(Genode::Ram_dataspace_capability ds_cap,
					   Genode::size_t size,
					   Genode::Cache_attribute cached,
					   bool bootstrapped,
					   void *storage)
		:
		_bootstrapped (bootstrapped),
		storage (storage),
		info(ds_cap, size, cached)
		{ }

	
	/* one method should be virtual, otherwise this class is not polymorphic
	 * and therefore dynamic_cast can't be applied. */
	virtual ~Ram_dataspace() {};
		
	Ram_dataspace *find_by_badge(Genode::uint16_t badge) {
		if(badge == info.cap.local_name())
			return this;
		Ram_dataspace *info = next();
		return info ? info->find_by_badge(badge) : 0;
	}

	Ram_dataspace *find_by_timestamp(Genode::size_t timestamp) {
		if(timestamp == this->timestamp())
			return this;
		Ram_dataspace *info = next();
		return info ? info->find_by_timestamp(timestamp) : 0;
	}

	Genode::size_t timestamp() const {
		return Simple_counter<Ram_dataspace>::id();
	}


};



#endif /* _RTCR_RAM_DATASPACE_INFO_H_ */
