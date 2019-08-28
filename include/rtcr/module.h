/*
 * \brief  Session Handler
 * \author Denis Huber
 * \author Johannes Fischer
 * \date   2019-03-21
 */

#ifndef _RTCR_MODULE_SET_H_
#define _RTCR_MODULE_SET_H_

/* Genode includes */
#include <util/list.h>
#include <base/heap.h>
#include <util/list.h>
#include <base/service.h>

namespace Rtcr {
	class Module;
	typedef Genode::String<16> Module_name;
}

using namespace Rtcr;

/**
 * Superclass of a module
 *
 * A module is loaded during startup of the Rtcr library by the `Target_child`
 * class. A module is a container for all kind of logic and provides a solution
 * for extending and configurating the Rtcr process.
 *
 * In order to provide your own module, create a class and inherit from `Rtcr::Module`.
 */
class Rtcr::Module : public Genode::List<Module_name>::Element
{
public:
	/**
	 * Name of your module.
	 *
	 * \return Name of your module
	 */
	virtual Module_name name() = 0;

	/**
	 * Called when this module should run a checkpoint
	 */
	virtual void checkpoint(bool resume) = 0;

	/**
	 * Resolves session requests for sessions which are provided by this module.
	 *
	 * \param service_name Name of the service which is looked up
	 * \param args
	 * \return Instance of a Service object.
	 */
	virtual Genode::Service *resolve_session_request(const char *service_name,
							 const char *args) = 0;

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
};



#endif /* _RTCR_MODULE_SET_H_ */
