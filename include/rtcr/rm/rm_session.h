/*
 * \brief  Intercepting Rm session
 * \author Denis Huber
 * \date   2016-10-02
 */

#ifndef _RTCR_RM_SESSION_H_
#define _RTCR_RM_SESSION_H_

/* Genode includes */
#include <rm_session/connection.h>
#include <base/allocator.h>
#include <root/component.h>
#include <util/list.h>

/* Rtcr includes */
#include <rtcr/checkpointable.h>
#include <rtcr/rm/region_map_component.h>
#include <rtcr/ram/ram_session.h>

namespace Rtcr {
	class Rm_session_component;
	class Rm_root;

	constexpr bool rm_verbose_debug = true;
	constexpr bool rm_root_verbose_debug = true;
}

/**
 * Custom RPC session object to intercept its creation, modification, and
 * destruction through its interface
 */
class Rtcr::Rm_session_component : public Rtcr::Checkpointable,
				   public Genode::Rpc_object<Genode::Rm_session>,
                                   public Genode::List<Rm_session_component>::Element
{

  
public:

  using Genode::Rpc_object<Genode::Rm_session>::cap;  
  Genode::String<160> ck_creation_args;
  Genode::String<160> ck_upgrade_args;
  bool ck_bootstrapped;
  Genode::uint16_t ck_badge;
  Genode::addr_t ck_kcap;
  Genode::List<Region_map_component> ck_region_maps;
  
  void checkpoint() override;

  void print(Genode::Output &output) const
  {
    Genode::print(output,
		  ", ck_cargs='", ck_creation_args, "'",
		  ", ck_uargs='", ck_upgrade_args, "'");    
  }
  
  
protected:

  char* _upgrade_args;
  /**
   * Lists for monitoring Rpc object
   */  
  Genode::Lock _new_region_maps_lock;
  Genode::Lock _destroyed_region_maps_lock;  
  Genode::List<Region_map_component> _new_region_maps;
  Genode::List<Region_map_component> _destroyed_region_maps;



private:
	/**
	 * Enable log output for debugging
	 */
	static constexpr bool verbose_debug = rm_verbose_debug;

	/**
	 * Allocator for Rpc objects created by this session and also for monitoring structures
	 */
	Genode::Allocator     &_md_alloc;
	/**
	 * Entrypoint for managing created Rpc objects
	 */
	Genode::Entrypoint    &_ep;
	/**
	 * Reference to Target_child's bootstrap phase
	 */
	bool                  &_bootstrap_phase;
	/**
	 * Parent's session connection which is used by the intercepted methods
	 */
	Genode::Rm_connection  _parent_rm;


	Region_map_component &_create(Genode::size_t size);
	void _destroy(Region_map_component &region_map);

	Ram_session_component &_ram_session;	

public:

  Rm_session_component(Genode::Env &env,
		       Genode::Allocator &md_alloc,
		       Genode::Entrypoint &ep,
		       const char *creation_args,
		       Ram_session_component &ram_session,
		       bool &bootstrap_phase,
		       Genode::Xml_node *config);
  
	~Rm_session_component();

	Genode::Rm_session_capability parent_cap() { return _parent_rm.cap(); }




	/******************************
	 ** Rm session Rpc interface **
	 ******************************/

	/**
	 * Create a virtual Region map, its real counter part and a list element to manage them
	 */
	Genode::Capability<Genode::Region_map> create(Genode::size_t size) override;
	/**
	 * Destroying the virtual Region map, its real counter part, and the list element it was managed in
	 *
	 * XXX Untested! During the implementation time, the destroy method was not working.
	 */
	void destroy(Genode::Capability<Genode::Region_map> region_map_cap) override;
};



/**
 * Custom root RPC object to intercept session RPC object creation, modification, and destruction through the root interface
 */
class Rtcr::Rm_root : public Genode::Root_component<Rm_session_component>
{
private:
	/**
	 * Enable log output for debugging
	 */
	static constexpr bool verbose_debug = rm_root_verbose_debug;

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
	Genode::List<Rm_session_component> _session_rpc_objs;

protected:
	Rm_session_component *_create_session(const char *args);
	void _upgrade_session(Rm_session_component *session, const char *upgrade_args);
	void _destroy_session(Rm_session_component *session);

	Ram_session_component &_ram_session;

  Genode::Xml_node *_config;
  
public:
	Rm_root(Genode::Env &env,
		Genode::Allocator &md_alloc,
		Genode::Entrypoint &session_ep,
		Ram_session_component &ram_session,		
		bool &bootstrap_phase,
		Genode::Xml_node *config);
  
	~Rm_root();
	Rm_session_component *find_by_badge(Genode::uint16_t badge);
	Genode::List<Rm_session_component> &session_infos() { return _session_rpc_objs; }
};

#endif /* _RTCR_RM_SESSION_H_ */
