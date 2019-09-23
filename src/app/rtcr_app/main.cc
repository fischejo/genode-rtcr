/*
 * \brief  Test client for the Hello RPC interface
 * \author Björn Döbel
 * \author Norman Feske
 * \date   2008-03-20
 */

/*
 * Copyright (C) 2008-2016 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

/* genode includes */
#include <base/component.h>
#include <base/log.h>
#include <base/component.h>
#include <base/signal.h>
#include <base/sleep.h>
#include <base/log.h>
#include <timer_session/connection.h>
#include <os/static_parent_services.h>
#include <base/registry.h>

/* Libc includes */
#include <libc/component.h>

/* Rtcr includes */
#include <rtcr/child.h>
#include <rtcr/module_factory.h>
#include <rtcr/base_module.h>
#include <rtcr_serializer/serializer.h>

#include <rtcr/pd/pd_session.h>
#include <rtcr/cpu/cpu_session.h>
#include <rtcr/rm/rm_session.h>
#include <rtcr/rom/rom_session.h>
#include <rtcr/log/log_session.h>
#include <rtcr/timer/timer_session.h>

Genode::size_t Component::stack_size() { return 512*1024; }

using namespace Rtcr;

namespace Rtcr {
	struct Main;
}

struct Rtcr::Main
{
	enum { ROOT_STACK_SIZE = 512*1024 };
	Genode::Env              &env;
	Genode::Heap              heap            { env.ram(), env.rm() };
	Genode::Static_parent_services<Rtcr::Pd_session,
	                               Rtcr::Cpu_session,
	                               Rtcr::Rm_session,	                               
	                               Rtcr::Rom_session,
	                               Rtcr::Log_session,
	                               Rtcr::Timer_session> parent_services { };
	
	Genode::Attached_rom_dataspace config {env, "config" };

	Main(Genode::Env &env_) : env(env_)
	{
		Module_name module_name = config.xml().sub_node("module")
			.attribute_value("name", Module_name());

		Init_module &module = *Module_factory::get(module_name)->create(env, heap);
		
		/* timer instance for sleeping */
		Timer::Connection timer(env);

		/* create serializer */
		Serializer serializer(env, heap);

		/* create a single child */
		/* Note: multiple childs are not yet fully supported by Rtcr */
		Child sheep (env, heap, "sheep_counter", parent_services, module.services());

		/* sleep a moment until child is running */
		timer.msleep(2000);

		/* Pause, checkpoint, Resume all childs */
		module.pause();
		module.checkpoint();
		module.resume();

		/* Print all information of the *_info objects. These represents the
		 * last checkpoint state */
		Child_info *sheep_info = module.child_info("sheep_counter");
		Genode::log("Child_info before serializing:");
		Genode::log(*sheep_info);

		// /* Serialize the last checkpoint state */
		Genode::size_t size;
		Genode::List<Child_info> *child_infos = module.child_info();
		Genode::Dataspace_capability ds_cap = serializer.serialize(child_infos, &size);
		Genode::log("Serialized Size: ", size);
	  
		/* Parse serialized dataspace*/
		child_infos = serializer.parse(ds_cap);
		Genode::log("Child_info after serializing:");
		Genode::log(*child_infos->first());

		/* done */
		Genode::log("test completed");		
		Genode::sleep_forever();
	}
};


void Libc::Component::construct(Libc::Env &env) 
{
	static Rtcr::Main main(env);
}



