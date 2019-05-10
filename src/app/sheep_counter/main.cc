/*
 * \brief  Testprogram which just counts sheeps
 * \author Denis Huber
 * \date   2016-08-04
 *
 * This program is a target for the rtcr service. It counts a sheep,
 * prints the number of the sheep and goes to sleep between each 
 * iteration. This component will be checkpointed serialized, 
 * (transfered), deserialized, and restored. It does not know that it is
 * being checkpointed.
 */

#include <base/component.h>
#include <timer_session/connection.h>
#include <base/log.h>
#include <rm_session/connection.h>
#include <region_map/client.h>

Genode::size_t Component::stack_size() { return 16*1024; }

void Component::construct(Genode::Env &env)
{
	//enter_kdebug("before restore");
	using namespace Genode;
	log("Creating Timer session.");
	Timer::Connection timer(env);


	//	env.parent().upgrade(timer, "ram_quota=8K");
	//	env.parent().upgrade(env.ram_session_cap(), "ram_quota=24K");
	int n  = 0;
	while(1) {
		log("1 sheep. zzZ");
		n++;

		/*
		 * Busy waiting is used here to avoid pausing this component during an RPC call to the timer component.
		 * This could cause a resume call to this component to fail.
		 */
		for(int i = 0; i < 100000000; i++)
			__asm__("NOP");		
	}
}
