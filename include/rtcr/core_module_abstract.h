/*
 * \brief  Abstract implementation of a Core Module
 * \author Johannes Fischer
 * \date   2019-04-04
 */

#ifndef _RTCR_CORE_MODULE_ABSTRACT_H_
#define _RTCR_CORE_MODULE_ABSTRACT_H_

/* Genode includes */
#include <base/service.h>
#include <ram_session/capability.h>
#include <region_map/client.h>
#include <pd_session/client.h>
#include <cpu_session/client.h>

/* Rtcr includes */
#include <rtcr/module.h>

namespace Rtcr {
	class Core_module_abstract;
}

using namespace Rtcr;

class Rtcr::Core_module_abstract : public virtual Module
{
public:
	/**
	 * Return the kcap for a given badge from _capability_map_infos Refactored
	 * from `checkpointer.h` Return the kcap for a given badge. If there is no,
	 * return 0.
	 *
	 * As every module requires this method, it is public to other modules.
	 *
	 * If an external module call this method, it's thread will be blocked
	 * until the core module can response to the query.
	 */
	virtual Genode::addr_t find_kcap_by_badge(Genode::uint16_t badge) = 0;

	/**
	 * Methods required by Target_child for creating a Genode::Child
	 *
	 * These methods are implemented in derived classes. If a derived class
	 * calls one of these methods, but does not implement it on its one, the
	 * call is delegated to a sister class
	 */
	virtual Genode::Local_service &pd_service() = 0;
	virtual Genode::Local_service &rm_service() = 0;
	virtual Genode::Local_service &cpu_service() = 0;
	virtual Genode::Local_service &ram_service() = 0;

	virtual Genode::Rpc_object<Genode::Cpu_session> &cpu_session() = 0;
	virtual Genode::Rpc_object<Genode::Ram_session> &ram_session() = 0;
	virtual Genode::Rpc_object<Genode::Pd_session> &pd_session() = 0;

	virtual Genode::Rom_connection &rom_connection() = 0;

	/**
	 * Pause all threads of the child
	 */
	virtual void pause() = 0;

	/**
	 * Resume all threads of the child
	 */    
	virtual void resume() = 0;
};

#endif /* _RTCR_CORE_MODULE_ABSTRACT_H_ */



