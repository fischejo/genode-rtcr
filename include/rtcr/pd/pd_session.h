/*
 * \brief  Intercepting Pd session
 * \author Denis Huber
 * \date   2016-08-03
 */

#ifndef _RTCR_PD_SESSION_H_
#define _RTCR_PD_SESSION_H_

/* Genode includes */
#include <root/component.h>
#include <base/allocator.h>
#include <base/rpc_server.h>
#include <pd_session/connection.h>

/* Rtcr includes */
#include <rtcr/checkpointable.h>
#include <rtcr/rm/region_map_component.h>
#include <rtcr/pd/native_capability_info.h>
#include <rtcr/pd/signal_context_info.h>
#include <rtcr/pd/signal_source_info.h>
#include <rtcr/ram/ram_session.h>

namespace Rtcr {
	class Pd_session_component;
	class Pd_root;

	constexpr bool pd_verbose_debug = true;
	constexpr bool pd_root_verbose_debug = false;
}

/**
 * Custom RPC session object to intercept its creation, modification, and destruction through its interface
 */
class Rtcr::Pd_session_component : public Rtcr::Checkpointable,
				   public Genode::Rpc_object<Genode::Pd_session>,
                                   public Genode::List<Pd_session_component>::Element
{
public:
  using Genode::Rpc_object<Genode::Pd_session>::cap;
  
  Genode::String<160> ck_creation_args;
  Genode::String<160> ck_update_args;
  bool ck_bootstrapped;
  Genode::uint16_t ck_badge;
  Genode::addr_t ck_kcap;

  void print(Genode::Output &output) const;
  void checkpoint() override;

  Genode::List<Signal_source_info>     ck_signal_sources;
  Genode::List<Signal_context_info>    ck_signal_contexts;
  Genode::List<Native_capability_info> ck_native_caps;

  /**
   * Custom address space for monitoring the attachments of the Region map
   */
  Region_map_component   _address_space;
  /**
   * Custom stack area for monitoring the attachments of the Region map
   */
  Region_map_component   _stack_area;
  /**
   * Custom linker area for monitoring the attachments of the Region map
   */
  Region_map_component   _linker_area;



  
protected:
  
  char* _upgrade_args;
  bool _bootstrapped;

  /**
   * List for monitoring the creation and destruction of Signal_source_capabilities
   */  
  Genode::Lock                         _new_signal_sources_lock;
  Genode::List<Signal_source_info>     _new_signal_sources;
  Genode::Lock                         _destroyed_signal_sources_lock;
  Genode::List<Signal_source_info>     _destroyed_signal_sources;  
  void _checkpoint_signal_sources();
    
  /**
   * List for monitoring the creation and destruction of Signal_context_capabilities
   */  
  Genode::Lock                         _new_signal_contexts_lock;
  Genode::List<Signal_context_info>    _new_signal_contexts;
  Genode::Lock                         _destroyed_signal_contexts_lock;
  Genode::List<Signal_context_info>    _destroyed_signal_contexts;  
  void _checkpoint_signal_contexts();  

  /**
   * List for monitoring the creation and destruction of Native_capabilities
   */  
  Genode::Lock                         _new_native_caps_lock;
  Genode::List<Native_capability_info> _new_native_caps;
  Genode::Lock                         _destroyed_native_caps_lock;
  Genode::List<Native_capability_info> _destroyed_native_caps;
  
  void _checkpoint_native_capabilities();  

private:
	/**
	 * Enable log output for debugging
	 */
	static constexpr bool verbose_debug = pd_verbose_debug;

	/**
	 * TODO Needed?
	 */
	Genode::Env           &_env;
	/**
	 * Allocator for list elements which monitor the Signal_source,
	 * Signal_context and Native_capability creation and destruction
	 */
	Genode::Allocator     &_md_alloc;
	/**
	 * Entrypoint to manage itself
	 */
	Genode::Entrypoint    &_ep;
	/**
	 * Reference to Target_child's bootstrap phase
	 */
	bool                  &_bootstrap_phase;
	/**
	 * Connection to parent's pd session, usually from core
	 */
	Genode::Pd_connection  _parent_pd;


public:
	Pd_session_component(Genode::Env &env,
			     Genode::Allocator &md_alloc,
			     Genode::Entrypoint &ep,
			     const char *label,
			     const char *creation_args,
			     Ram_session_component &ram_session,
			     bool &bootstrap_phase,
			     Genode::Xml_node *config);
  
	~Pd_session_component();

	Genode::Pd_session_capability parent_cap() { return _parent_pd.cap(); }

  Region_map_component &address_space_component() { return _address_space; }
	// Region_map_component const &address_space_component() const { return _address_space; }

	// Region_map_component &stack_area_component() { return _stack_area; }
	// Region_map_component const &stack_area_component() const { return _stack_area; }

	// Region_map_component &linker_area_component() { return _linker_area; }
	// Region_map_component const &linker_area_component() const { return _linker_area; }


	Pd_session_component *find_by_badge(Genode::uint16_t badge);

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
 * Custom root RPC object to intercept session RPC object creation, modification, and destruction through the root interface
 */
class Rtcr::Pd_root : public Genode::Root_component<Pd_session_component>
{
private:
	/**
	 * Enable log output for debugging
	 */
	static constexpr bool verbose_debug = pd_root_verbose_debug;

	/**
	 * Environment of Rtcr; is forwarded to a created session object
	 */
	Genode::Env        &_env;
	/**
	 * Allocator for session objects and monitoring list elements
	 */
	Genode::Allocator  &_md_alloc;
	/**
	 * Entrypoint for managing session objects
	 */
	Genode::Entrypoint &_ep;
	/**
	 * Reference to Target_child's bootstrap phase
	 */
	bool               &_bootstrap_phase;
	/**
	 * Lock for infos list
	 */
	Genode::Lock        _objs_lock;
	/**
	 * List for monitoring session objects
	 */
	Genode::List<Pd_session_component> _session_rpc_objs;

        Ram_session_component &_ram_session;  
        Genode::Xml_node *_config;
  
protected:
	Pd_session_component *_create_session(const char *args);
	void _upgrade_session(Pd_session_component *session, const char *upgrade_args);
	void _destroy_session(Pd_session_component *session);


  
public:
	Pd_root(Genode::Env &env,
		Genode::Allocator &md_alloc,
		Genode::Entrypoint &session_ep,
		Ram_session_component &ram_session,
		bool &bootstrap_phase,
		Genode::Xml_node *config);
	~Pd_root();

	Genode::List<Pd_session_component> &session_infos() { return _session_rpc_objs; }
};

#endif /* _RTCR_PD_SESSION_H_ */
