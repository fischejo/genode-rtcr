/*
 * \brief  Monitoring ram dataspace creation/destruction
 * \author Denis Huber
 * \date   2016-10-06
 */

#ifndef _RTCR_RAM_DATASPACE_INFO_H_
#define _RTCR_RAM_DATASPACE_INFO_H_

/* Genode includes */
#include <util/list.h>
#include <ram_session/ram_session.h>
#include <region_map/client.h>

/* Rtcr includes */
#include <rtcr/info_structs.h>

namespace Rtcr {
	struct Ram_dataspace_info;

	constexpr bool dd_verbose_debug = false;
}

/**
 * Monitors allocated Ram dataspaces
 */
struct Rtcr::Ram_dataspace_info : private Simple_counter<Ram_dataspace_info>,
	    Genode::List<Ram_dataspace_info>::Element
{
  bool ck_bootstrapped;
  
  
  Genode::Ram_dataspace_capability ck_dst_cap;
  Genode::size_t ck_timestamp;
  bool _bootstrapped;
  bool is_region_map;

  void checkpoint()
  {

    ck_bootstrapped = _bootstrapped;
  }
	/**
	 * Pointer to extra data. This can be used by an extending
	 * implementation in order to prevent an inheritance of this class.
	 */
	void *storage;

	/**
	 * Allocated Ram dataspace
	 */
	Genode::Ram_dataspace_capability const ck_cap;
	Genode::size_t                   const ck_size;
	Genode::Cache_attribute          const ck_cached;


  
	Ram_dataspace_info(Genode::Ram_dataspace_capability ds_cap,
			   Genode::size_t size,
			   Genode::Cache_attribute cached,
			   bool bootstrapped)
		:
	  _bootstrapped (bootstrapped),
		ck_cap      (ds_cap),
		ck_size     (size),
		ck_cached   (cached),
	    is_region_map(false),
	  storage (nullptr)
	{ }

	Ram_dataspace_info(Genode::Ram_dataspace_capability ds_cap,
			   Genode::size_t size,
			   Genode::Cache_attribute cached,
			   bool bootstrapped,
			   void *storage)
		:
		_bootstrapped (bootstrapped),
		  ck_cap      (ds_cap),
		ck_size     (size),
		ck_cached   (cached),
		storage (storage)
	{ }

	
	/* one method should be virtual, otherwise this class is not polymorphic
	 * and therefore dynamic_cast can't be applied. */
	virtual ~Ram_dataspace_info() {};
		
	Ram_dataspace_info *find_by_badge(Genode::uint16_t badge)
	{
		if(badge == ck_cap.local_name())
			return this;
		Ram_dataspace_info *info = next();
		return info ? info->find_by_badge(badge) : 0;
	}

	Ram_dataspace_info *find_by_timestamp(Genode::size_t timestamp)
	{
		if(timestamp == this->timestamp())
			return this;
		Ram_dataspace_info *info = next();
		return info ? info->find_by_timestamp(timestamp) : 0;
	}

	Genode::size_t timestamp() const
	{
		return Simple_counter<Ram_dataspace_info>::id();
	}

	void print(Genode::Output &output) const
	{
		using Genode::Hex;

		Genode::print(output, ck_cap, ", size=", Hex(ck_size), ", cached=", static_cast<unsigned>(ck_cached),
			      ", timestamp=", timestamp(), ", ");
	}
};



#endif /* _RTCR_RAM_DATASPACE_INFO_H_ */
