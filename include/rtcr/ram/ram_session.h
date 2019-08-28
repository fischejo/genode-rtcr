/*
 * \brief  Intercepting Ram session
 * \author Denis Huber
 * \date   2016-08-12
 */

#ifndef _RTCR_RAM_SESSION_H_
#define _RTCR_RAM_SESSION_H_

/* Genode includes */
#include <root/component.h>
#include <base/allocator.h>
#include <base/rpc_server.h>
#include <ram_session/connection.h>
#include <rm_session/connection.h>
#include <region_map/client.h>
#include <dataspace/client.h>
#include <util/retry.h>
#include <util/misc_math.h>

/* Rtcr includes */
#include <rtcr/ram/ram_dataspace_info.h>
#include <rtcr/checkpointable.h>

namespace Rtcr {

	class Ram_session_component;
	class Ram_root;

	constexpr bool fh_verbose_debug = false;
	constexpr bool ram_verbose_debug = false;
	constexpr bool ram_root_verbose_debug = false;
}

/**
 * Custom RAM session to monitor the allocation, freeing, and ram quota transfers
 *
 * Instead of providing ordinary RAM dataspaces, it can provide managed dataspaces,
 * which are used to monitor the access to the provided RAM dataspaces
 */
class Rtcr::Ram_session_component : public Rtcr::Checkpointable,
				    public Genode::Rpc_object<Genode::Ram_session>,
                                    public Genode::List<Ram_session_component>::Element
{
public:
  using Genode::Rpc_object<Genode::Ram_session>::cap;
  
  Genode::String<160> ck_creation_args;
  Genode::String<160> ck_upgrade_args;
  bool ck_bootstrapped;
  Genode::uint16_t ck_badge;
  Genode::addr_t ck_kcap;
  Genode::List<Ram_dataspace_info> ck_ram_dataspaces;
  Genode::Ram_session_capability   ck_ref_account_cap;
  
  void print(Genode::Output &output) const
  {
    Genode::print(output,
		  ", ck_cargs='", ck_creation_args, "'",
		  ", ck_uargs='", ck_upgrade_args, "'");
  }


  void checkpoint() override;

  void mark_region_map_dataspace(Genode::Dataspace_capability ds_cap);
  
protected:

  char* _upgrade_args;
  Genode::Ram_session_capability   _ref_account_cap;
  /**
   * List of allocated ram dataspaces
   */
  Genode::Lock                     _new_ram_dataspaces_lock;
  Genode::List<Ram_dataspace_info> _new_ram_dataspaces;

  Genode::Lock                     _destroyed_ram_dataspaces_lock;  
  Genode::List<Ram_dataspace_info> _destroyed_ram_dataspaces; 

  
	/**
	 * Enable log output for debugging
	 */
	static constexpr bool verbose_debug = ram_verbose_debug;

	/**
	 * Environment of Rtcr; needed to upgrade RAM quota of the RM session
	 */
	Genode::Env             &_env;
	/**
	 * Allocator for structures monitoring the allocated Ram dataspaces
	 */
	Genode::Allocator       &_md_alloc;
	/**
	 * Reference to Target_child's bootstrap phase
	 */
	bool                    &_bootstrap_phase;
	/**
	 * Connection to the parent Ram session (usually core's Ram session)
	 */
	Genode::Ram_connection   _parent_ram;
	/**
	 * Connection to the parent Rm session for creating new Region_maps (usually core's Rm session)
	 */
	Genode::Rm_connection    _parent_rm;
	/**
	 * Destroy rds_info and all its sub infos)
	 */
	void _destroy_ramds_info(Ram_dataspace_info &rds_info);

  void copy_dataspace(Ram_dataspace_info &info);
  Genode::Xml_node *_config;

public:
	Ram_session_component(Genode::Env &env,
			      Genode::Allocator &md_alloc,
			      const char *label,
			      const char *creation_args,
			      bool &bootstrap_phase,
			      Genode::Xml_node *config);
  
	~Ram_session_component();

	Genode::Ram_session_capability parent_cap() { return _parent_ram.cap(); }

	Ram_session_component *find_by_badge(Genode::uint16_t badge);

	/***************************
	 ** Ram_session interface **
	 ***************************/

	/**
	 * Allocate a Ram_dataspace
	 */
	Genode::Ram_dataspace_capability alloc(Genode::size_t size, Genode::Cache_attribute cached) override;
	/**
	 * Frees the Ram_dataspace and destroys all monitoring structures
	 */
	void free(Genode::Ram_dataspace_capability ds_cap) override;
	int ref_account(Genode::Ram_session_capability ram_session) override;
	int transfer_quota(Genode::Ram_session_capability ram_session, Genode::size_t amount) override;
	Genode::size_t quota() override;
	Genode::size_t used() override;

	/*
	 * KIA4SM method
	 */
	void set_label(char *label) override;

};


/**
 * Custom root RPC object to intercept session RPC object creation, modification, and destruction through the root interface
 */
class Rtcr::Ram_root : public Genode::Root_component<Ram_session_component>
{
protected:
	/**
	 * Enable log output for debugging
	 */
	static constexpr bool verbose_debug = ram_root_verbose_debug;

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
	Genode::List<Ram_session_component> _session_rpc_objs;


	Ram_session_component *_create_session(const char *args);
	void _upgrade_session(Ram_session_component *session, const char *upgrade_args);
	void _destroy_session(Ram_session_component *session);

	Genode::Xml_node *_config;
public:
	Ram_root(Genode::Env &env,
		 Genode::Allocator &md_alloc,
		 Genode::Entrypoint &session_ep,
		 bool &bootstrap_phase,
		 Genode::Xml_node *config);
	~Ram_root();

	Genode::List<Ram_session_component> &session_infos() { return _session_rpc_objs; }
};


#endif /* _RTCR_RAM_SESSION_H_ */
