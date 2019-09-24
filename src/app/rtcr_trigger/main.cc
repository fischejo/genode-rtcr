/*
 * \brief  Triggers a Checkpoint
 * \author Johannes Fischer
 * \date   2016-09-24
 */

#include <base/component.h>
#include <rtcr_session/connection.h>
#include <timer_session/connection.h>
#include <rtcr_serializer/serializer.h>


/* Libc includes */
#include <libc/component.h>


Genode::size_t Component::stack_size() { return 16*1024; }

void Libc::Component::construct(Libc::Env &env)
{
	Genode::Heap heap { env.ram(), env.rm() };	
	Timer::Connection timer(env);
	Rtcr::Connection rtcr(env);	
	Serializer serializer(env, heap);
		
	Genode::log("sleeping for 2 seconds");
	timer.msleep(2000);
	Genode::log("Trigger checkpoint 1");	
	Genode::Dataspace_capability ds_cap_0 = rtcr.checkpoint();
	rtcr.free();
	
	Genode::log("sleeping for 2 seconds");	
	timer.msleep(2000);
	Genode::log("Trigger checkpoint 2");
	Genode::Dataspace_capability ds_cap_1 = rtcr.checkpoint();

	Genode::List<Child_info> *child_infos = serializer.parse(ds_cap_1);
	Genode::log(*child_infos->first());
	rtcr.free();
}
