/*
 * \brief  Container for all module states
 * \author Johannes Fischer
 * \date   2019-03-23
 */

#ifndef _RTCR_TARGET_STATE_H_
#define _RTCR_TARGET_STATE_H_

/* Genode includes */
#include <util/list.h>

/* Rtcr includes */
#include <rtcr/module_state.h>
#include <rtcr/module.h>
#include <base/lock.h>

namespace Rtcr {
	class Target_state;
}

using namespace Rtcr;


/**
 * Container for states of all modules
 */
class Rtcr::Target_state
{
private:
	Genode::Lock _write_lock;
	
	/** 
	 * This class is an internal container which links the name to a state of a
	 * module. So it is possible to find a stored state of a module by its
	 * module name.
	 */
	class Module_state_container : public Genode::List<Module_state_container>::Element
	{
	public:
		Module_state &state;
		Module_name name;
       
		Module_state_container(Module_state &_state, Module_name _name)
			:
			state(_state),
			name(_name)
		{}
	};

	/**
	 * List of all stored module states
	 */
	Genode::List<Module_state_container> _containers;
     
public:
    
	Genode::Env       &_env;
	Genode::Allocator &_alloc;

	Target_state(Genode::Env &env, Genode::Allocator &alloc);
	~Target_state();

	/**
	 * Print the internal data structures of this Target_state.
	 *
	 * Usage:
	 * ```C++
	 * Target_state &state = new Target_state();
	 * Genode::log(state);
	 */
	void print(Genode::Output &output) const;

	/**
	 * Find state of a module by module name
	 *
	 * \param name Name of module
	 * \return State of module
	 */
	Module_state *state(Module_name name);

	/**
	 * Store a Module_state object reference
	 *
	 * \param name Name of the module which provides the state
	 * \param state State of the module
	 */
	void store(Module_name name, Module_state &state);
};

#endif /* _RTCR_TARGET_STATE_H_ */
