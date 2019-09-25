/*
 * \brief  Rtcr Service
 * \author Johannes Fischer
 * \date   2019-09-24
 */



#ifndef _RTCR_SESSION_COMPONENT_H_
#define _RTCR_SESSION_COMPONENT_H_

#include <session/session.h>
#include <base/rpc.h>
#include <base/rpc_server.h>
#include <base/component.h>
#include <root/component.h>

#include <rtcr/init_module.h>
#include <rtcr_serializer/serializer.h>

#include <rtcr_session/rtcr_session.h>

namespace Init_rtcr {
	class Session_component;
	class Root_component;	
}

class Init_rtcr::Session_component : public Genode::Rpc_object<Rtcr::Session>
{
private:
	Genode::Env &_env;
	Genode::Allocator &_alloc;
	Init_module &_module;
	Serializer _serializer;
	Genode::Ram_dataspace_capability _serialized_cap;
	Genode::size_t _serialized_size;
	bool _free = true;
	
public:
	Session_component(Genode::Env &env,
					  Genode::Allocator &alloc,
					  Init_module &module)
		:
		_env(env),
		_alloc(alloc),
		_module(module),
		_serializer(env, alloc)
	{}
	
	Genode::Dataspace_capability checkpoint() {
		if(!_free) {
			Genode::warning("Last checkpoint dataspace is not yet free!");
			free();
		}

		_module.checkpoint();
		
		_serialized_cap = _serializer.serialize(_module.child_info(), &_serialized_size);
		_free = false;
		return _serialized_cap;
	}

	void free() {
		if(!_free) {
			_env.ram().free(_serialized_cap);
			_free = true;
		}
	}

	Genode::size_t size() {
		return _serialized_size;
	}
};


class Init_rtcr::Root_component : public Genode::Root_component<Init_rtcr::Session_component>
{
	Genode::Env &_env;
	Genode::Allocator &_alloc;
	Init_module &_module;
	
protected:

	Session_component *_create_session(const char *args)
	{
		return new (md_alloc()) Session_component(_env, _alloc, _module);
	}

public:

	Root_component(Genode::Env &env,
				   Genode::Allocator &alloc,
				   Init_module &module)
		:
		Genode::Root_component<Init_rtcr::Session_component>(env.ep(), alloc),
		_env(env),
		_alloc(alloc),
		_module(module)		
	{
		env.parent().announce(env.ep().manage(*this));
	}
};

#endif /* _SESSION_COMPONENT_H_ */
