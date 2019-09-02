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

#include <base/component.h>
#include <base/log.h>
#include <base/component.h>
#include <base/signal.h>
#include <base/sleep.h>
#include <base/log.h>
#include <timer_session/connection.h>

#include <os/config.h>

#include <rtcr/target_child.h>
#include <rtcr/base_module.h>
#include <rtcr_serializer/serializer.h>


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

  	Main(Genode::Env &env_) : env(env_)
	{
	  Timer::Connection timer(env);

	  Base_module module(env, heap);
	  Serializer serializer(env, heap);
	  
	   Target_child sheep (env, heap, "sheep_counter",module);
	  sheep.start();
	  timer.msleep(2000);

	  Child_info *info = module.child_info("sheep_counter");
	  module.pause(info);
	  module.checkpoint(info);
	  module.resume(info);


	  Genode::size_t serialized_size;
	  Genode::Ram_dataspace_capability ds_cap =
		  serializer.serialize(info, &serialized_size);
	  
	  // Genode::log("moin");
	  // serializer.parse(ds_cap);
	  Genode::sleep_forever();
	}
};


void Component::construct(Genode::Env &env)
{
	static Rtcr::Main main(env);
	
}



