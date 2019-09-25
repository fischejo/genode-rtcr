/*
 * \brief  Triggers a Checkpoint
 * \author Johannes Fischer
 * \date   2016-09-24
 */

#include <base/component.h>
#include <rtcr_session/connection.h>
#include <timer_session/connection.h>
#include <rtcr_serializer/serializer.h>
#include <base/env.h>


/* Libc includes */
#include <libc/component.h>


namespace Trigger {
	struct Main;
}


Genode::size_t Component::stack_size() { return 16*1024; }


struct Trigger::Main
{
	Genode::Env &_env;
	
	Genode::Heap _heap { _env.ram(), _env.rm() };
	
	Timer::Connection _timer { _env };
	Rtcr::Connection _rtcr { _env };
	Rtcr::Serializer _serializer { _env, _heap };

	Main(Genode::Env &env) : _env(env)
	{
		Genode::log("sleeping for 2 seconds");
		_timer.msleep(2000);

		/* checkpoint 1 */
		Genode::log("Trigger checkpoint 1");
		Genode::Dataspace_capability ds_cap_0 = _rtcr.checkpoint();
		_rtcr.free();
	
		Genode::log("sleeping for 2 seconds");	
		_timer.msleep(2000);

		/* checkpoint 2 */
		Genode::log("Trigger checkpoint 2");
		Genode::Dataspace_capability ds_cap_1 = _rtcr.checkpoint();
		Genode::List<Child_info> *child_infos = _serializer.parse(ds_cap_1);
		Genode::log(*child_infos->first());
		_rtcr.free();
	}
};


void Libc::Component::construct(Libc::Env &env)
{
	static Trigger::Main main(env);
}
	
