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

/* Rtcr includes */
#include <rtcr/module.h>
#include <rtcr/cpu/cpu_session.h>
#include <rtcr/pd/pd_session.h>
#include <rtcr/ram/ram_session.h>
#include <rtcr/child_info.h>

namespace Rtcr {
	class Target_child;
	typedef Genode::String<32> Child_name;	
}

/**
 * Encapsulates the policy and creation of the child
 */
class Rtcr::Target_child : public Genode::Child_policy,
						   public Genode::List<Rtcr::Target_child>
{
private:
	/**
	 * Child's unique name and filename of child's rom module
	 */
	const char*  _name;

	/**
	 * Local environment
	 */
	Genode::Env        &_env;

	/**
	 * Local allocator
	 */
	Genode::Allocator &_alloc;

	Module &_module;
	
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

	
	Genode::Service &_ram_service;
	Genode::Service &_pd_service;
	Genode::Service &_cpu_service;
	
	Ram_session &_ram_session;
	Pd_session &_pd_session;
	Cpu_session &_cpu_session;

	Genode::Rom_connection _binary_rom;
	Genode::Dataspace_capability _binary_rom_ds;

	/**
	 * Needed for child's creation
	 */
	Genode::Child::Initial_thread  _initial_thread;
	/**
	 * Needed for child's creation
	 */
	Genode::Region_map_client _address_space;

	/**
	 * Child object
	 */
	Genode::Child _child;

	Cpu_session &create_cpu_session();
	Pd_session &create_pd_session();
	Ram_session &create_ram_session();  

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
				 const char *name,
				 Module &module);
	
	~Target_child() {};
  
	/**
	 * Start child from scratch
	 */
	void start();

	/****************************
	 ** Child-policy interface **
	 ****************************/

	const char *name() const { return _name; }
	Genode::Service *resolve_session_request(const char *service_name, const char *args);
	void filter_session_args(const char *service, char *args, Genode::size_t args_len);
	
};

#endif /* _RTCR_TARGET_CHILD_H_ */
