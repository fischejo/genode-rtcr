/*
 * \brief  Base Module creates all intecepting sessions
 * \author Johannes Fischer
 * \date   2019-08-29
 */

#ifndef _RTCR_BASE_MODULE_H_
#define _RTCR_BASE_MODULE_H_

/* Genode includes */
#include <util/list.h>
#include <base/heap.h>
#include <util/list.h>
#include <os/config.h>
#include <base/service.h>
#include <rom_session/connection.h>

/* Local includes */
#include <rtcr/module.h>
#include <rtcr/module_factory.h>
#include <rtcr/cpu/cpu_session.h>
#include <rtcr/pd/pd_session.h>
#include <rtcr/ram/ram_session.h>
#include <rtcr/rm/rm_session.h>
#include <rtcr/log/log_session.h>
#include <rtcr/timer/timer_session.h>
#include <rtcr/rom/rom_session.h>
#include <rtcr/cap/capability_mapping.h>

namespace Rtcr {
	class Base_module;
	class Base_module_factory;
}

using namespace Rtcr;

/**
 * The class Rtcr::Base_module provides the intercepting sessions and handles
 * the communication between a target child and the corresponding sessions.
 */
class Rtcr::Base_module : public virtual Module
{
protected:
	Genode::Env        &_env;
	Genode::Allocator  &_alloc;
	Genode::Entrypoint &_ep;
	bool &_bootstrap;

	/* If enabled, all threads are executed in parallel during checkpoint */
	bool _parallel;
	
	/* pointer to the current XML configuration of this module */
	Genode::Xml_node *_config;
	
	/* intercepting PD session */
	Pd_root &_pd_root;
	Genode::Local_service _pd_service;
	Pd_session &_pd_session;

	/* intercepting CPU session */
	Cpu_root &_cpu_root;
	Genode::Local_service _cpu_service;
	Cpu_session &_cpu_session;

	/* intercepting RAM session */
	Ram_root &_ram_root;
	Genode::Local_service _ram_service;
	Ram_session &_ram_session;

	/* intercepting RM session */
	Rm_root &_rm_root;
	Genode::Local_service _rm_service;

	/* intercepting ROM session */
	Rom_root &_rom_root;
	Genode::Local_service _rom_service;
	Genode::Rom_connection _rom_connection;

	/* intercepting Log session */
	Log_root &_log_root;
	Genode::Local_service _log_service;

	/* intercepting Timer session */
	Timer_root &_timer_root;
	Genode::Local_service _timer_service;

	/* Thread for creating the capability mapping */
	Capability_mapping &_capability_mapping;
	
	/* Methods for initializing the intercepting session. In order to provide
	 * another custom session, inherit from Rtcr::Base_module and override these
	 * methods */
	virtual Pd_root &pd_root();
	virtual Cpu_root &cpu_root();  
	virtual Ram_root &ram_root();
	virtual Rm_root &rm_root();
	virtual Rom_root &rom_root();
	virtual Log_root &log_root();
	virtual Timer_root &timer_root();
	virtual Capability_mapping &capability_mapping();
  
	Cpu_session &_find_cpu_session(const char *label, Cpu_root &cpu_root);
	Pd_session &_find_pd_session(const char *label, Pd_root &pd_root);
	Ram_session &_find_ram_session(const char *label, Ram_root &ram_root);  

	/* methods for parsing the XML configuration */
	inline Genode::size_t _read_quota();
	inline bool _read_parallel();  
  
public:  

	Base_module(Genode::Env &env,
				Genode::Allocator &alloc,
				Genode::Entrypoint &ep,
				const char* label,
				bool &bootstrap,
				Genode::Xml_node *config);

	~Base_module();

	/**
	 * Checkpoint PD,RAM,ROM,RM,CPU sessions
	 *
	 * \param resume the child threads after checkpoint
	 */
	void checkpoint(bool resume) override;

	/**
	 * If the child requests a service of PD,RM,RAM,ROM or CPU, this module
	 * provides it.
	 */    
	Genode::Service *resolve_session_request(const char *service_name,
											 const char *args) override;

	Module_name name() override { return "base"; }

	/* following methods provide the necessary services and sessions which are
	 * required for creating a child */
	Genode::Local_service &pd_service() override { return _pd_service; }
	Genode::Local_service &rm_service() override  { return _rm_service; }
	Genode::Local_service &cpu_service() override  { return _cpu_service; }
	Genode::Local_service &ram_service() override  { return _ram_service; }
	Genode::Rpc_object<Genode::Cpu_session> &cpu_session() override { return _cpu_session; }
	Genode::Rpc_object<Genode::Ram_session> &ram_session() override { return _ram_session; }
	Genode::Rpc_object<Genode::Pd_session> &pd_session() override { return _pd_session; }
	Genode::Rom_connection &rom_connection() override  { return _rom_connection; }
};


/**
 * Factory class for creating the Rtcr::Base_module
 */
class Rtcr::Base_module_factory : public Module_factory
{
public:
	Module* create(Genode::Env &env,
				   Genode::Allocator &alloc,
				   Genode::Entrypoint &ep,
				   const char* label,
				   bool &bootstrap,
				   Genode::Xml_node *config) override
		{   
			return new (alloc) Base_module(env, alloc, ep, label, bootstrap, config);
		}
    
	Module_name name() override { return "base"; }
};

#endif /* _RTCR_BASE_MODULE_H_ */