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

	virtual Pd_root &pd_root() = 0;
	virtual Cpu_root &cpu_root() = 0;
	virtual Ram_root &ram_root() = 0;
	virtual Rm_root &rm_root() = 0;
	virtual Rom_root &rom_root() = 0;
	virtual Timer_root &timer_root() = 0;
	virtual Log_root &log_root() = 0;	

};



#endif /* _RTCR_MODULE_SET_H_ */
