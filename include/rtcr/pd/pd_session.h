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
#include <rtcr/ram/ram_session.h>

namespace Rtcr {
	class Pd_session;
	class Pd_root;
	class Pd_session_info;
}

struct Rtcr::Pd_session_info {
	Genode::String<160> creation_args;
	Genode::String<160> upgrade_args;
	bool bootstrapped;
	Genode::uint16_t badge;
	Genode::addr_t kcap;

	Region_map *region_maps;

	Signal_source* signal_sources;
	Signal_context* signal_contexts;
	Native_capability* native_caps;

	Region_map_info &address_space_info;
	Region_map_info &stack_area_info;
	Region_map_info &linker_area_info;
	
	Pd_session_info(const char* creation_args,
					Region_map_info &address_space_info,
					Region_map_info &stack_area_info,
					Region_map_info &linker_area_info)
		:
		creation_args(creation_args),
		address_space_info(address_space_info),
		stack_area_info(stack_area_info),
		linker_area_info(linker_area_info)
		{}
	
	void print(Genode::Output &output) const {
		Genode::print(output, " PD session:\n");
		Genode::print(output,
					  "  bootstrapped=", bootstrapped,
					  ", cargs='", creation_args, "'",
					  ", uargs='", upgrade_args, "'\n");		

		/* Signal contexts */
		Genode::print(output, "  Signal contexts:\n");
		Signal_context *context_info = signal_contexts;
		if(!context_info) Genode::print(output, "   <empty>\n");
		while(context_info) {
			Genode::print(output, "   ", *context_info, "\n");
			context_info = context_info->next();
		}

		/* Signal sources */
		Genode::print(output, "  Signal sources:\n");
		Signal_source *source_info = signal_sources;
		if(!source_info) Genode::print(output, "   <empty>\n");
		while(source_info) {
			Genode::print(output, "   ", *source_info, "\n");
			source_info = source_info->next();
		}

		/* Address space */
		Genode::print(output, "  Address space: \n");
		Genode::print(output, "   ", address_space_info);
		Genode::print(output, "  Stack area: \n");		
		Genode::print(output, "   ", stack_area_info);		
		Genode::print(output, "  Linker area: \n");
		Genode::print(output, "   ", linker_area_info);				
	}
};


/**
 * Custom RPC session object to intercept its creation, modification, and
 * destruction through its interface
 */
class Rtcr::Pd_session : public Rtcr::Checkpointable,
						 public Genode::Rpc_object<Genode::Pd_session>,
						 public Genode::List<Pd_session>::Element
{	
protected:
  
	const char* _upgrade_args;
	bool _bootstrapped;

	/**
	 * List for monitoring the creation and destruction of
	 * Signal_source_capabilities
	 */  
	Genode::Lock _signal_sources_lock;
	Genode::List<Signal_source> _signal_sources;
	Genode::Lock _destroyed_signal_sources_lock;
	Genode::Fifo<Signal_source> _destroyed_signal_sources;  

	/**
	 * List for monitoring the creation and destruction of
	 * Signal_context_capabilities
	 */  
	Genode::Lock _signal_contexts_lock;
	Genode::List<Signal_context> _signal_contexts;
	Genode::Lock _destroyed_signal_contexts_lock;
	Genode::Fifo<Signal_context> _destroyed_signal_contexts;  

	/**
	 * List for monitoring the creation and destruction of Native_capabilities
	 */  
	Genode::Lock _native_caps_lock;
	Genode::List<Native_capability> _native_caps;
	Genode::Lock _destroyed_native_caps_lock;
	Genode::Fifo<Native_capability> _destroyed_native_caps;


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
	 * Reference to Target_child's bootstrap phase
	 */
	bool &_bootstrap_phase;
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

	
	void _checkpoint_signal_sources();
	void _checkpoint_signal_contexts();  	
	void _checkpoint_native_capabilities();  


public:
	/******************
	 ** COLD STORAGE **
	 ******************/

	Pd_session_info info;
	
	
public:
	using Genode::Rpc_object<Genode::Pd_session>::cap;
	
	Pd_session(Genode::Env &env,
			   Genode::Allocator &md_alloc,
			   Genode::Entrypoint &ep,
			   const char *label,
			   const char *creation_args,
			   Ram_root &ram_root,
			   bool &bootstrap_phase);
  
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


	Pd_session *find_by_badge(Genode::uint16_t badge);

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
	/**
	 * Reference to Target_child's bootstrap phase
	 */
	bool &_bootstrap_phase;
	/**
	 * Lock for infos list
	 */
	Genode::Lock _objs_lock;
	/**
	 * List for monitoring session objects
	 */
	Genode::List<Pd_session> _session_rpc_objs;

	Ram_root &_ram_root;  

  
protected:
	Pd_session *_create_session(const char *args);
	void _upgrade_session(Pd_session *session, const char *upgrade_args);
	void _destroy_session(Pd_session *session);
  
public:
	Pd_root(Genode::Env &env,
			Genode::Allocator &md_alloc,
			Genode::Entrypoint &session_ep,
			Ram_root &ram_root,
			bool &bootstrap_phase);
	~Pd_root();

	Genode::List<Pd_session> &sessions() { return _session_rpc_objs; }
};

#endif /* _RTCR_PD_SESSION_H_ */
