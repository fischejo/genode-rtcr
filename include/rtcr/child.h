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
	/**
	 * Child's unique name and filename of child's rom module
	 */
	const char*  _name;
	Genode::Env &_env;
	Genode::Allocator &_alloc;

	Genode::Pd_connection _pd_session;
  
	/**
	 * Registry for parent's services
	 */
	Genode::Registry<Genode::Registered<Genode::Parent_service>> &_parent_services;

	Base_module &_module;
	/**
	 * Rom dataspace holding the XML config 
	 */
	Genode::Attached_rom_dataspace _config;

	inline Genode::Cap_quota read_caps_quota();
  
	Genode::Cap_quota _caps_quota;

	/**
	 * Entrypoint for child's creation
	 */
	Genode::Entrypoint  _child_ep;

	/**
	 * Child object
	 */
	Genode::Child _child;





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
	      Genode::Registry<Genode::Registered<Genode::Parent_service>>  &parent_services,
	      Base_module &module);

	

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
};

#endif /* _RTCR_CHILD_H_ */
