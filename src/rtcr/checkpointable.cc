/*
 * \brief  Threaded Container for a Checkpointable
 * \author Johannes Fischer
 * \date   2019-06-10
 */

#include <rtcr/checkpointable.h>

using namespace Rtcr;

Checkpointable::Checkpointable(Genode::Env &env, const char* name, bool ready)
	:
	_timer(env),
	_affinity(_read_affinity(name)),
	_config(env, "config"),
	Thread(env,
		   name,
		   64*1024,
		   _affinity,
		   Genode::Thread::Weight(),
		   env.cpu()),
	_running(true),
	_next_job(NONE),
	_ready_event(ready) // not ready by default
{
	Thread::start();

#ifdef VERBOSE
	Genode::log("Checkpointable[",name,"] started on CPU ",
				"xpos=",_affinity.xpos(), " ypos=",_affinity.ypos());
#endif	
}


Genode::Affinity::Location Checkpointable::_read_affinity(const char* name)
{
	try {
		Genode::Xml_node config_node = _config.xml();
		Genode::Xml_node ck_node = config_node.sub_node("checkpointable");
		Genode::String<30> node_name;
		while(Genode::strcmp(name, ck_node.attribute_value("name", node_name).string()))
			ck_node = ck_node.next("checkpointable");

		long const xpos = ck_node.attribute_value<long>("xpos", 0);
		long const ypos = ck_node.attribute_value<long>("ypos", 0);
		return Genode::Affinity::Location(xpos, ypos, 1 ,1);
	}
	catch (...) { return Genode::Affinity::Location(0, 0, 1, 1);}
	return Genode::Affinity::Location(0, 0, 1, 1);  
}


void Checkpointable::stop()
{
	_running=false;
	// TODO: not sure if this works, if not than use _next_event.release()
	cancel_blocking(); 
}


void Checkpointable::entry()
{
	while(_running) {
		/* wait for the next job */
		_next_event.wait();
		_next_event.unset();

		if(!_running)
			return;

		/* start next job */
		Job _current_job = _next_job;
		_next_job = NONE;
		if(CHECKPOINT == _current_job) {
			/* now busy */
			_ready_event.unset();

			/* do checkpoint */
			unsigned long long start = _timer.elapsed_ms();
			checkpoint();
			_checkpoint_time =  _timer.elapsed_ms() - start;
			_checkpoint_finished.set();

			/* do post checkpoint */
			post_checkpoint();

			/* not busy */
			_ready_event.set();
		}
	}
}


void Checkpointable::start_checkpoint()
{
	_ready_event.wait(); // wait until a checkpoint is possible.
	_next_job = CHECKPOINT;
	_checkpoint_finished.unset(); // must come before trigger next job.
	_next_event.set();
}


void Checkpointable::join_checkpoint()
{
	_checkpoint_finished.wait();
}


void Checkpointable::wait_ready()
{
	_ready_event.wait();
}


void Checkpointable::ready()
{
	_ready_event.set();
}



