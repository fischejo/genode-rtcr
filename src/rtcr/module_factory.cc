/*
 * \brief Factory class for registring and creating a module at runtime.
 * \author Johannes Fischer
 * \date   2019-03-21
 */

#include <rtcr/module_factory.h>

using namespace Rtcr;

Module_factory* Module_factory::_head = nullptr;  /* initialize List */


Module_factory::~Module_factory() {}


Module_factory::Module_factory() {
	_next = _head;
	_head = this;  
}


void Module_factory::print()
{
	Module_factory* r = Module_factory::first();
	while(r) {
		Genode::log("Register Module Set: ", r->name());
		r = r->next();
	}
}


Module_factory* Module_factory::get(const Module_name name)
{
	Module_factory* r = Module_factory::first();
	while(r) {
		if(!Genode::strcmp(r->name().string(), name.string()))
			return r;
		r = r->next();
	}
	return nullptr;
}


Module_factory* Module_factory::first()
{
	return _head;
}


Module_factory* Module_factory::next()
{
	return _next;
}
