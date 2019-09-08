/*
 * \brief  Intercepting Pd session
 * \author Denis Huber
 * \author Johannes Fischer
 * \date   2019-08-29
 */

#ifndef _RTCR_PD_SESSION_H_
#define _RTCR_PD_SESSION_H_

/* Genode includes */
#include <root/component.h>
#include <base/allocator.h>
#include <base/rpc_server.h>
#include <pd_session/connection.h>
#include <util/list.h>
#include <util/fifo.h>

/* Rtcr includes */
#include <rtcr/checkpointable.h>
#include <rtcr/rm/region_map.h>
#include <rtcr/pd/native_capability.h>
#include <rtcr/pd/signal_context.h>
#include <rtcr/pd/signal_source.h>
#include <rtcr/pd/pd_session_info.h>
#include <rtcr/child_info.h>

namespace Rtcr {
	class Capability_mapping;
	class Child_info;
	class Pd_session;
	class Pd_root;
}

/**
 * Custom RPC session object to intercept its creation, modification, and
 * destruction through its interface
 */
class Rtcr::Pd_session : public Rtcr::Checkpointable,
						 public Genode::Rpc_object<Genode::Pd_session>,
						 public Rtcr::Pd_session_info
{	
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
	 * List for monitoring the creation and destruction of Native_capabilities
	 */  
	Genode::Lock _native_caps_lock;
	Genode::List<Native_capability_info> _native_caps;
	Genode::Lock _destroyed_native_caps_lock;
	Genode::Fifo<Native_capability_info> _destroyed_native_caps;


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

public:
	using Genode::Rpc_object<Genode::Pd_session>::cap;
	
	Pd_session(Genode::Env &env,
			   Genode::Allocator &md_alloc,
			   Genode::Entrypoint &ep,
			   const char *creation_args,
			   Child_info *child_info);

  
	~Pd_session();

	void checkpoint() override;

	void upgrade(const char *upgrade_args) {
		_upgrade_args = upgrade_args;		
	}
	
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

};


/**
 * Custom root RPC object to intercept session RPC object creation,
 * modification, and destruction through the root interface
 */
class Rtcr::Pd_root : public Genode::Root_component<Pd_session>
{
private:
	/**
	 * Environment of Rtcr; is forwarded to a created session object
	 */
	Genode::Env &_env;
	/**
	 * Allocator for session objects and monitoring list elements
	 */
	Genode::Allocator &_md_alloc;
	/**
	 * Entrypoint for managing session objects
	 */
	Genode::Entrypoint &_ep;

	Genode::Lock &_childs_lock;
	Genode::List<Child_info> &_childs;
  
protected:

	/**
	 * Wrapper for creating a ram session
	 */
	virtual Pd_session *_create_pd_session(Child_info *info, const char *args);
	
	Pd_session *_create_session(const char *args);
	void _upgrade_session(Pd_session *session, const char *upgrade_args);
	void _destroy_session(Pd_session *session);
  
public:
	Pd_root(Genode::Env &env,
			Genode::Allocator &md_alloc,
			Genode::Entrypoint &session_ep,
			Genode::Lock &childs_lock,
			Genode::List<Child_info> &childs);
			
	~Pd_root();
};

#endif /* _RTCR_PD_SESSION_H_ */
