/*
 * \brief  Intercepting Pd session
 * \author Denis Huber
 * \author Johannes Fischer
 * \date   2019-08-29
 */

#ifndef _RTCR_PD_SESSION_H_
#define _RTCR_PD_SESSION_H_

/* Genode includes */
#include <base/allocator.h>
#include <base/rpc_server.h>
#include <pd_session/connection.h>
#include <util/list.h>
#include <util/fifo.h>
#include <util/arg_string.h>
#include <util/reconstructible.h>
#include <base/allocator_guard.h>
#include <base/session_object.h>
#include <base/registry.h>
#include <root/component.h>
#include <base/service.h>
#include <base/session_state.h>
#include <util/retry.h>

/* Rtcr includes */
#include <rtcr/checkpointable.h>
#include <rtcr/rm/region_map.h>
#include <rtcr/pd/native_capability.h>
#include <rtcr/pd/signal_context.h>
#include <rtcr/pd/signal_source.h>
#include <rtcr/pd/pd_session_info.h>
#include <rtcr/pd/ram_dataspace.h>
#include <rtcr/pd/ram_dataspace_info.h>
#include <rtcr/child_info.h>

namespace Rtcr {
	class Pd_session;
}

/**
 * Custom RPC session object to intercept its creation, modification, and
 * destruction through its interface
 */
class Rtcr::Pd_session : public Genode::Rpc_object<Genode::Pd_session>,
                         public Rtcr::Pd_session_info
{
public:

	struct Pd_checkpointable : Rtcr::Checkpointable {
		Pd_session *_pd;

		Pd_checkpointable(Genode::Env &env, Pd_session *pd)
			:
			Checkpointable(env, "pd_session"),
			_pd(pd) {};

		void checkpoint() override;
	} pd_checkpointable;


	struct Ram_checkpointable : Rtcr::Checkpointable {
		Pd_session *_pd;

		Ram_checkpointable(Genode::Env &env, Pd_session *pd)
			:
			Checkpointable(env, "ram_dataspaces"),
			_pd(pd) {};

		void checkpoint() override;
	} ram_checkpointable;

protected:
	
	const char* _upgrade_args;

	/**
	 * List for monitoring the creation and destruction of
	 * Signal_source_capabilities
	 */
	Genode::Lock _signal_sources_lock;
	Genode::List<Signal_source_info> _signal_sources;
	Genode::Lock _destroyed_signal_sources_lock;
	Genode::Fifo<Signal_source_info> _destroyed_signal_sources;

	/**
	 * List for monitoring the creation and destruction of
	 * Signal_context_capabilities
	 */
	Genode::Lock _signal_contexts_lock;
	Genode::List<Signal_context_info> _signal_contexts;
	Genode::Lock _destroyed_signal_contexts_lock;
	Genode::Fifo<Signal_context_info> _destroyed_signal_contexts;

	/**
	 * List for monitoring the creation and destruction of
	 * Native_capabilities
	 */
	Genode::Lock _native_caps_lock;
	Genode::List<Native_capability_info> _native_caps;
	Genode::Lock _destroyed_native_caps_lock;
	Genode::Fifo<Native_capability_info> _destroyed_native_caps;

	Genode::Pd_session_capability _ref_account_cap;

	/**
	 * List of allocated ram dataspaces
	 */
	Genode::Lock _ram_dataspaces_lock;
	Genode::List<Ram_dataspace_info> _ram_dataspaces;

	Genode::Lock _destroyed_ram_dataspaces_lock;
	Genode::Fifo<Ram_dataspace_info> _destroyed_ram_dataspaces;


	Genode::Env &_env;
	/**
	 * Allocator for list elements which monitor the Signal_source,
	 * Signal_context and Native_capability creation and destruction
	 */
	Genode::Allocator &_md_alloc;
	/**
	 * Entrypoint to manage itself
	 */
	Genode::Entrypoint &_ep;

	/**
	 * Connection to parent's pd session, usually from core
	 */
	Genode::Pd_connection  _parent_pd;

	/**
	 * Custom address space for monitoring the attachments of the Region map
	 */
	Region_map   _address_space;
	/**
	 * Custom stack area for monitoring the attachments of the Region map
	 */
	Region_map   _stack_area;
	/**
	 * Custom linker area for monitoring the attachments of the Region map
	 */
	Region_map   _linker_area;

	Child_info *_child_info;

	void _checkpoint_signal_sources();
	void _checkpoint_signal_contexts();
	void _checkpoint_native_capabilities();
	void _checkpoint_ram_dataspaces();	


	virtual void _destroy_dataspace(Ram_dataspace *ds);
	virtual void _attach_dataspace(Ram_dataspace *ds);
	virtual void _alloc_dataspace(Ram_dataspace *ds);
	virtual void _copy_dataspace(Ram_dataspace *ds);


public:
	using Genode::Rpc_object<Genode::Pd_session>::cap;

	Pd_session(Genode::Env &env,
	           Genode::Allocator &md_alloc,
	           Genode::Entrypoint &ep,
	           const char *creation_args,
	           Child_info *child_info);

	~Pd_session();
	
	void upgrade(const char *upgrade_args);

	const char* upgrade_args() { return _upgrade_args; }

	Genode::Pd_session_capability parent_cap() { return _parent_pd.cap(); }


	Region_map &address_space_component() { return _address_space; }

	// Region_map const &address_space_component() const { return _address_space; }

	// Region_map &stack_area_component() { return _stack_area; }
	// Region_map const &stack_area_component() const { return _stack_area; }

	// Region_map &linker_area_component() { return _linker_area; }
	// Region_map const &linker_area_component() const { return _linker_area; }


	/**************************
	 ** Pd_session interface **
	 **************************/

	void assign_parent(Genode::Capability<Genode::Parent> parent) override;

	bool assign_pci(Genode::addr_t addr, Genode::uint16_t bdf) override;

	Signal_source_capability alloc_signal_source() override;
	void free_signal_source(Signal_source_capability cap) override;

	Genode::Signal_context_capability alloc_context(Signal_source_capability source,
	                                                unsigned long imprint) override;
	void free_context(Genode::Signal_context_capability cap) override;

	void submit(Genode::Signal_context_capability context, unsigned cnt) override;

	Genode::Native_capability alloc_rpc_cap(Genode::Native_capability ep) override;
	void free_rpc_cap(Genode::Native_capability cap) override;

	/**
	 * Return custom address space
	 */
	Genode::Capability<Genode::Region_map> address_space() override;
	/**
	 * Return custom stack area
	 */
	Genode::Capability<Genode::Region_map> stack_area() override;
	/**
	 * Return custom linker area
	 */
	Genode::Capability<Genode::Region_map> linker_area() override;
	Genode::Capability<Native_pd> native_pd() override;


	void map(Genode::addr_t _addr, Genode::addr_t __addr) override;

	void ref_account(Genode::Capability<Genode::Pd_session>) override;

	void transfer_quota(Genode::Capability<Genode::Pd_session>, Genode::Cap_quota) override;
	void transfer_quota(Genode::Capability<Genode::Pd_session>, Genode::Ram_quota) override;

	Genode::Cap_quota cap_quota() const override;

	Genode::Cap_quota used_caps() const override;

	Genode::Ram_quota ram_quota() const override;

	Genode::Ram_quota used_ram() const override;

	Genode::Ram_dataspace_capability alloc(Genode::size_t, Genode::Cache_attribute) override;

	void free(Genode::Ram_dataspace_capability) override;

	Genode::size_t dataspace_size(Genode::Ram_dataspace_capability) const override;
};



#endif /* _RTCR_PD_SESSION_H_ */
