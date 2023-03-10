/*
 * \brief  Child creation
 * \author Denis Huber
 * \author Johannes Fischer
 * \date   2019-08-29
 */

#ifndef _RTCR_CHILD_H_
#define _RTCR_CHILD_H_

/* Genode includes */
#include <base/env.h>
#include <base/child.h>
#include <base/service.h>
#include <base/snprintf.h>
#include <rom_session/connection.h>
#include <cpu_session/cpu_session.h>
#include <util/list.h>
#include <os/session_requester.h>
#include <base/attached_rom_dataspace.h>

/* Rtcr includes */
#include <rtcr/base_module.h>
#include <rtcr/cpu/cpu_session.h>
#include <rtcr/pd/pd_session.h>
#include <rtcr/child_info.h>


namespace Rtcr {
	class Child;	
	typedef Genode::String<32> Child_name;	
}

	
/**
 * Encapsulates the policy and creation of the child
 */
class Rtcr::Child : public Genode::Child_policy,
                    public Genode::List<Rtcr::Child>
{
private:
	Genode::Env &_env;
	Genode::Allocator &_alloc;
	

	/**
	 * Child's unique name and filename of child's rom module
	 */
	const char*  _name;

	/**
	 * Module which provides intercepting services and a PD factory
	 */
	Init_module &_module;

	/**
	 * Pd_{session, service, factory} for child
	 */
    Pd_session &_pd_session;
	Genode::Pd_session_capability _pd_session_cap;
	Genode::Local_service<Pd_session>::Single_session_factory _pd_factory;	
	Genode::Local_service<Pd_session> _pd_service;
	
	/**
	 * Registry for parent's services
	 */
	Genode::Registry<Genode::Registered<Genode::Parent_service>> &_parent_services;

	/**
	 * Rom dataspace holding the XML config 
	 */
	Genode::Attached_rom_dataspace _config;

	inline Genode::Cap_quota read_cap_quota();
	inline Genode::Ram_quota read_ram_quota();	
  
	Genode::Cap_quota _caps_quota;
	Genode::Ram_quota _ram_quota;	

	/**
	 * Child object
	 */
	Genode::Entrypoint _child_ep;		
	Genode::Child _child;

	/**
	 * create pd session for child
	 */
	Pd_session &create_pd_session();
	
public:

	/**
	 * Create a child process
	 *
	 * \param env               Environment
	 * \param alloc             Heap Allocator
	 * \param parent_services   Services which are already provided by the parents
	 * \param name              Name of child component
	 */
	Child(Genode::Env &env,        
	      Genode::Allocator &alloc,
	      const char *name,
		  Genode::Registry<Genode::Registered<Genode::Parent_service>> &parent_services,
	      Init_module &module);

	~Child() {};
	
	/****************************
	 ** Child-policy interface **
	 ****************************/

	Genode::Child_policy::Name name() const { return _name; }
	Genode::Child_policy::Route resolve_session_request(Genode::Service::Name const &,
	                                                    Genode::Session_label const &) override;
	void init(Genode::Pd_session &, Genode::Capability<Genode::Pd_session>) override;
	void init(Genode::Cpu_session &, Genode::Capability<Genode::Cpu_session>) override;
  
	Genode::Pd_session &ref_pd() override;
	Genode::Pd_session_capability ref_pd_cap() const override;

	void resource_request(Genode::Parent::Resource_args const &args) override;	
};

#endif /* _RTCR_CHILD_H_ */
