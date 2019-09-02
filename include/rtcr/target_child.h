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
#include <rtcr/rm/rm_session.h>
#include <rtcr/log/log_session.h>
#include <rtcr/timer/timer_session.h>
#include <rtcr/rom/rom_session.h>
#include <rtcr/cap/capability_mapping.h>
#include <rtcr/session_service.h>

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
	Child_name  _name;

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
	
	Genode::Local_service _ram_service;
	Ram_session &_ram_session;

	Genode::Local_service _pd_service;
	Pd_session &_pd_session;
	
	Genode::Local_service _cpu_service;
	Cpu_session &_cpu_session;

	Genode::Rom_connection _binary_rom;
	Genode::Dataspace_capability _binary_rom_ds;

	Session_service<Rm_session,Rm_root> _rm_service;
	Session_service<Log_session,Log_root> _log_service;
	Session_service<Rom_session,Rom_root> _rom_service;
	Session_service<Timer_session,Timer_root> _timer_service;

	Capability_mapping _capability_mapping;
	
	/**
	 * Indicator whether child was bootstraped or not
	 */
	bool _in_bootstrap;

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


	Cpu_session &_find_cpu_session(const char *label, Cpu_root &cpu_root);
	Pd_session &_find_pd_session(const char *label, Pd_root &pd_root);
	Ram_session &_find_ram_session(const char *label, Ram_root &ram_root);  

	bool _parallel;

	inline const char* read_binary_name(const char* name);
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
				 const char *label,
				 Module &module);

	Cpu_session &cpu_session() { return _cpu_session; }
	Ram_session &ram_session() { return _ram_session; }
	Pd_session &pd_session() { return _pd_session; }	

	Timer_session *timer_session() { return _timer_service.session(); }
	Log_session *log_session() { return _log_service.session(); }
	Rm_session *rm_session() { return _rm_service.session(); }	
	Rom_session *rom_session() { return _rom_service.session(); }

	Capability_mapping &capability_mapping() { return _capability_mapping; }
	
	~Target_child() {};
  
	/**
	 * Start child from scratch
	 */
	void start();

	void pause();

	void resume();

	void checkpoint();

	void print(Genode::Output &output) const;

	/****************************
	 ** Child-policy interface **
	 ****************************/

	const char *name() const { return _name.string(); }
	Genode::Service *resolve_session_request(const char *service_name, const char *args);
	void filter_session_args(const char *service, char *args, Genode::size_t args_len);
	
};

#endif /* _RTCR_TARGET_CHILD_H_ */
