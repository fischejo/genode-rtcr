/*
 * \brief Abstract implementation of a Module class. This class provides the
 * necessary sessions which are required for creating a intercepted child.
 * \author Johannes Fischer
 * \date   2019-08-29
 */

#ifndef _RTCR_MODULE_SET_H_
#define _RTCR_MODULE_SET_H_

/* Genode includes */
#include <util/list.h>

/* Rtcr includes */
#include <rtcr/cpu/cpu_session.h>
#include <rtcr/pd/pd_session.h>
#include <rtcr/ram/ram_session.h>
#include <rtcr/rm/rm_session.h>
#include <rtcr/rom/rom_session.h>
#include <rtcr/timer/timer_session.h>
#include <rtcr/log/log_session.h>
#include <rtcr/child_info.h>

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
class Rtcr::Module : public Genode::List<Module>::Element
{
public:
	/**
	 * Name of your module.
	 *
	 * \return Name of your module
	 */
	virtual Module_name name() = 0;

	virtual Genode::Service &ram_service() = 0;
	virtual Genode::Service &cpu_service() = 0;
	virtual Genode::Service &pd_service() = 0;

	virtual Child_info *child_info(const char* name) = 0;
	virtual void pause(Child_info *info) = 0;
	virtual void resume(Child_info *info) = 0;
	virtual void checkpoint(Child_info *info) = 0;


	virtual Genode::Service *resolve_session_request(const char *service_name,
													 const char *args) = 0;
};



#endif /* _RTCR_MODULE_SET_H_ */
