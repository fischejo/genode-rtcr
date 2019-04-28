/*
 * \brief  Session Handler
 * \author Denis Huber
 * \author Johannes Fischer
 * \date   2019-03-21
 */

#ifndef _RTCR_MODULE_H_
#define _RTCR_MODULE_H_

/* Genode includes */
#include <util/list.h>
#include <base/heap.h>
#include <util/list.h>
#include <base/service.h>
#include <rtcr/module_state.h>
#include <rtcr/module.h>

namespace Rtcr {
	class Module;
	typedef Genode::String<16> Module_name;

/* forward declaration */
	class Module_state;
  
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
class Rtcr::Module : public Genode::List<Module>::Element
{
public:
	/**
	 * Name of your module.
	 *
	 * \return Name of your module
	 */
	virtual Module_name name() = 0;

	/**
	 * Called when all modules are loaded. If your module depends on other
	 * modules, this is the right place to search for them.
	 * 
	 * \param modules List of modules which are loaded.
	 */
	virtual void initialize(Genode::List<Module> &modules) {};

	/**
	 * Called when this module should run a checkpoint
	 *
	 * \return Module_state which stores the checkpointed data.
	 */
	virtual Module_state *checkpoint() = 0;

	/**
	 * Restore module from a checkpointed state.
	 *
	 * \param state A object of type Module_state which provides the
	 *              checkpointed state.
	 */
	virtual void restore(Module_state *state) = 0;

	/**
	 * Resolves session requests for sessions which are provided by this module.
	 *
	 * \param service_name Name of the service which is looked up
	 * \param args
	 * \return Instance of a Service object.
	 */
	virtual Genode::Service *resolve_session_request(const char *service_name,
							 const char *args) = 0;
};



#endif /* _RTCR_MODULE_H_ */
