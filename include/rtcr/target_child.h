/*
 * \brief  Child creation
 * \author Denis Huber
 * \author Johannes Fischer
 * \date   2019-03-23
 */

#ifndef _RTCR_TARGET_CHILD_H_
#define _RTCR_TARGET_CHILD_H_

/* Genode includes */
#include <base/env.h>
#include <base/child.h>
#include <base/service.h>
#include <base/snprintf.h>
#include <rom_session/connection.h>
#include <cpu_session/cpu_session.h>
#include <util/list.h>

/* Local includes */
#include <rtcr/core_module_abstract.h>
#include <rtcr/module.h>
#include <rtcr/module_factory.h>
#include <rtcr/target_state.h>

namespace Rtcr {
	class Target_child;
}

/**
 * Encapsulates the policy and creation of the child
 */
class Rtcr::Target_child : public Genode::Child_policy
{
private:
	/**
	 * Child's unique name and filename of child's rom module
	 */
	Genode::String<32>  _name;
	/**
	 * Local environment
	 */
	Genode::Env        &_env;
	/**
	 * Local allocator
	 */
	Genode::Allocator  &_alloc;
	/**
	 * Entrypoint for managing child's resource-sessions (PD, CPU, RAM)
	 */
	Genode::Entrypoint  _resources_ep;
	/**
	 * Entrypoint for child's creation
	 */
	Genode::Entrypoint  _child_ep;
	/**
	 * Indicator whether child was bootstraped or not
	 */
	bool                _in_bootstrap;

	/**
	 * Needed for child's creation
	 */
	Genode::Child::Initial_thread  *_initial_thread;
	/**
	 * Needed for child's creation
	 */
	Genode::Region_map_client      *_address_space;
	/**
	 * Registry for parent's services (parent of RTCR component). It is shared between all children.
	 */
	Genode::Service_registry      &_parent_services;
	/**
	 * Child object
	 */
	Genode::Child                 *_child;

	/**
	 * In order to create a child process, a few sessions are necessary. These
	 * are provided by a core module implementation.
	 */
	Core_module_abstract *core;

	/**
	 * list of all currently loaded modules.
	 */
	Genode::List<Module> modules;
	
public:

	/**
	 * Create a child process
	 *
	 * \param env               Environment
	 * \param alloc             Heap Allocator
	 * \param parent_services   Services which are already provided by the parents
	 * \param name              Name of ?
	 */
	Target_child(Genode::Env &env,
		     Genode::Allocator &alloc,
		     Genode::Service_registry &parent_services,
		     const char *name);

	~Target_child();
  
	/**
	 * Start child from scratch
	 */
	void start();

	/**
	 * Checkpoint the child
	 *
	 * \param state which stores the current state of the child
	 * \param resume child after checkpointing (default: true)
	 */
	void checkpoint(Target_state &state, bool resume = true);

	/**
	 * Restore child to a specific state
	 *
	 * \param state which provides the state for the child
	 */
	void restore(Target_state &state);
	
	/****************************
	 ** Child-policy interface **
	 ****************************/

	const char *name() const { return _name.string(); }
	Genode::Service *resolve_session_request(const char *service_name, const char *args);
	void filter_session_args(const char *service, char *args, Genode::size_t args_len);
  
};

#endif /* _RTCR_TARGET_CHILD_H_ */
