/*
 * \brief  Child creation
 * \author Denis Huber
 * \author Johannes Fischer
 * \date   2019-08-29
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
#include <cap_session/connection.h>
#include <util/list.h>

/* Local includes */
#include <rtcr/module.h>
#include <rtcr/module_factory.h>

namespace Rtcr {
	class Target_child;
	typedef Genode::String<32> Child_name;	
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
	Child_name  _name;

	/**
	 * Local environment
	 */
	Genode::Env        &_env;

	/**
	 * Local allocator
	 */
	Genode::Allocator &_alloc;

	/**
	 * Entrypoint for managing child's resource-sessions (PD, CPU, RAM)
	 */
	Genode::Entrypoint _resources_ep;

	/**
	 * Entrypoint for child's creation
	 *
	 * Entry point used for serving the parent interface and the
	 * locally provided ROM sessions for the 'config' and 'binary'
	 * files.
	 */
	enum { ENTRYPOINT_STACK_SIZE = 12*1024 };
	Genode::Cap_connection _cap_session;
	Genode::Affinity::Location _affinity_location;
	Genode::Rpc_entrypoint _entrypoint;

	/**
	 * Indicator whether child was bootstraped or not
	 */
	bool _in_bootstrap;

	/**
	 * In order to create a child process, a few sessions are necessary. These
	 * are provided by a module implementation.
	 */
	Module &_module;

	/**
	 * Needed for child's creation
	 */
	Genode::Child::Initial_thread  _initial_thread;
	/**
	 * Needed for child's creation
	 */
	Genode::Region_map_client _address_space;
	/**
	 * Registry for parent's services (parent of RTCR component). It is shared between all children.
	 */
	Genode::Service_registry &_parent_services;
	/**
	 * Child object
	 */
	Genode::Child _child;

	
	/**
	 * Parse name of child component from XML configuration. 
	 *
	 * The name can be defined as following:
	 * ```XML
	 * <config>
	 *   <child name="sheep_counter" />
	 * </config>
	 */
	inline Child_name _read_name();
	/**
	 * Parse name of child component from XML configuration. 
	 *
	 * The name can be defined as following:
	 * ```XML
	 * <config>
	 *   <module name="inc" />
	 * </config>
	 */
	inline Module_name _read_module_name();

	Module &_load_module(Module_name name);
	
public:

	/**
	 * Create a child process
	 *
	 * \param env               Environment
	 * \param alloc             Heap Allocator
	 * \param parent_services   Services which are already provided by the parents
	 * \param name              Name of child component
	 */
	Target_child(Genode::Env &env,
				 Genode::Allocator &alloc,
				 Genode::Service_registry &parent_services,
				 Child_name name);

	/**
	 * Create a child process
	 *
	 * The name of the child component is parsed from the XML Configuration.
	 *
	 * \param env               Environment
	 * \param alloc             Heap Allocator
	 * \param parent_services   Services which are already provided by the parents
	 */
	Target_child(Genode::Env &env,
				 Genode::Allocator &alloc,
				 Genode::Service_registry &parent_services);

  
	~Target_child();
  
	/**
	 * Start child from scratch
	 */
	void start();

	/**
	 * Checkpoint the child
	 *
	 * \param resume child after checkpointing (default: true)
	 */
	void checkpoint(bool resume = true);
	
	/****************************
	 ** Child-policy interface **
	 ****************************/

	const char *name() const { return _name.string(); }
	Genode::Service *resolve_session_request(const char *service_name, const char *args);
	void filter_session_args(const char *service, char *args, Genode::size_t args_len);
  
};

#endif /* _RTCR_TARGET_CHILD_H_ */
