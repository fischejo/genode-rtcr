/*
 * \brief  Monitoring ram dataspace creation/destruction
 * \author Denis Huber
 * \author Johannes Fischer
 * \date   2019-08-29
 */

#ifndef _RTCR_RAM_DATASPACE_H_
#define _RTCR_RAM_DATASPACE_H_

/* Genode includes */
#include <util/list.h>
#include <util/fifo.h>
#include <ram_session/ram_session.h>
#include <region_map/client.h>

/* Rtcr includes */
#include <rtcr/ram/ram_dataspace_info.h>
#include <rtcr/info_structs.h>

namespace Rtcr {
	class Ram_dataspace;
}


/**
 * Monitors allocated Ram dataspaces
 */
class Rtcr::Ram_dataspace : private Simple_counter<Ram_dataspace>,
							public Ram_dataspace_info
{
public:	
	/* pointers where ds are attached */
	void *src = nullptr;
	void *dst = nullptr;
		
	bool _bootstrapped;

	void checkpoint() {
		i_bootstrapped = _bootstrapped;
//		i_timestamp = timestamp();
	}

	/**
	 * Pointer to extra data. This can be used by an extending
	 * implementation in order to prevent an inheritance of this class.
	 */
	void *storage;


	Ram_dataspace(Genode::Ram_dataspace_capability src_cap,
				  Genode::size_t size,
				  Genode::Cache_attribute cached,
				  bool bootstrapped)
		:
		Ram_dataspace(src_cap, size, cached, bootstrapped, nullptr)
		{ }

	Ram_dataspace(Genode::Ram_dataspace_capability src_cap,
				  Genode::size_t size,
				  Genode::Cache_attribute cached,
				  bool bootstrapped,
				  void *storage)
		:
		Ram_dataspace_info(src_cap, size, cached),
		_bootstrapped (bootstrapped),		
		storage (storage)
		{ }

	~Ram_dataspace() {};

};



#endif /* _RTCR_RAM_DATASPACE_INFO_H_ */
