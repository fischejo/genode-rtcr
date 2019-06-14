/*
 * \brief  Container for all module states
 * \author Johannes Fischer
 * \date   2019-03-23
 */

#include <rtcr/target_state.h>

using namespace Rtcr;


Target_state::Target_state(Genode::Env &env, Genode::Allocator &alloc)
	:
	_env   (env),
	_alloc (alloc)
{ }


Target_state::~Target_state()
{
	while(Module_state_container *container = _containers.first()) {
		_containers.remove(container);
		Genode::destroy(_alloc, container);
	}
}


void Target_state::store(Module_name name, Module_state &state)
{
	_write_lock.lock();
	/* if state is already stored, search for it and update it. */
	Module_state_container *container = _containers.first();
	while(container) {
		if(!Genode::strcmp(container->name.string(), name.string())) {
			container->state = state;
			_write_lock.unlock();
			return;
		}
		container = container->next();
	}

	/* otherwise register new container */
	_containers.insert(new(_alloc) Module_state_container(state, name));
	_write_lock.unlock();
}


Module_state *Target_state::state(Module_name name)
{
	Module_state_container *container = _containers.first();
	while(container) {
		if(!Genode::strcmp(container->name.string(), name.string()))
			return &container->state;
		container = container->next();
	}
	return nullptr;
}


void Target_state::print(Genode::Output &output) const
{
	Genode::print(output, "##########################\n");
	Genode::print(output, "###    Target_state    ###\n");
	Genode::print(output, "##########################\n");

	/* Module states */
	Module_state_container const *container = _containers.first();
	while(container) {
		Genode::print(output, container->state, "\n");
		container = container->next();
	}

}
