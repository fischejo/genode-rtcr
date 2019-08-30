/*
 * \brief  Threaded Container for a Checkpointable
 * \author Johannes Fischer
 * \date   2019-06-10
 */

#include <rtcr/checkpointable.h>

using namespace Rtcr;

Checkpointable::Checkpointable(Genode::Env &env,
							   Genode::Xml_node *config,
							   const char* name)
	:
	_affinity(_read_affinity(config, name)),
	Thread(env,
		   name,
		   64*1024,
		   _affinity,
		   Genode::Thread::Weight(),
		   env.cpu()),
	_running(true),
	_next_job(NONE)
{
	Thread::start();

#ifdef VERBOSE
	Genode::log("Checkpointable[",name,"] started on CPU ",
				"xpos=",_affinity.xpos(), " ypos=",_affinity.ypos());
#endif	
}


Genode::Affinity::Location Checkpointable::_read_affinity(Genode::Xml_node *config,
														  const char* name)
{
	try {
		Genode::Xml_node ck_node = config->sub_node("checkpointable");
		Genode::String<30> node_name;
		while(Genode::strcmp(name, ck_node.attribute_value("name", node_name).string()))
			ck_node = ck_node.next("checkpointable");

		long const xpos = ck_node.attribute_value<long>("xpos", 0);
		long const ypos = ck_node.attribute_value<long>("ypos", 0);
		return Genode::Affinity::Location(xpos, ypos, 1 ,1);
	}
	catch (...) { return Genode::Affinity::Location(0, 0, 1, 1);}
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

		switch(_current_job) {
		case CHECKPOINT:
			checkpoint();
			break;
			break;
		case NONE:
			break;
		}
		_job_finished.set();
	}
}


void Checkpointable::start_checkpoint()
{
	_next_job = CHECKPOINT;
	_job_finished.unset(); // must come before trigger next job.
	_next_event.set();
}


void Checkpointable::join_checkpoint()
{
	_job_finished.wait();
}
