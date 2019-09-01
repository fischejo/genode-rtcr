/*
 * \brief  Intercepting Rm session
 * \author Denis Huber
 * \author Johannes Fischer
 * \date   2019-08-29
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
#include <rtcr/rm/region_map.h>
#include <rtcr/ram/ram_session.h>
#include <rtcr/info_structs.h>

namespace Rtcr {
	class Rm_session;
	class Rm_root;
	class Rm_session_info;
}

struct Rtcr::Rm_session_info : Session_info {
	Region_map *region_maps;

	Rm_session_info(const char* creation_args) : Session_info(creation_args) {}
	
	void print(Genode::Output &output) const {
		Genode::print(output, " RM session:\n");
		Genode::print(output,
					  "  bootstrapped=", bootstrapped,
					  ", cargs='", creation_args, "'",
					  ", uargs='", upgrade_args, "'\n");
		
		const Region_map *rm = region_maps;
		if(!rm) Genode::print(output, "   <empty>\n");
		while(rm) {
			Genode::print(output, "   ", rm->info);
			rm = rm->next();
		}
	}
};


/**
 * Custom RPC session object to intercept its creation, modification, and
 * destruction through its interface
 */
class Rtcr::Rm_session : public Rtcr::Checkpointable,
						 public Genode::Rpc_object<Genode::Rm_session>,
						 public Genode::List<Rm_session>::Element
{

public:
	/******************
	 ** COLD STORAGE **
	 ******************/
	Rm_session_info info;
   
protected:
	/*****************
	 ** HOT STORAGE **
	 *****************/
	
	const char* _upgrade_args;
	Genode::Lock _region_maps_lock;
	Genode::Lock _destroyed_region_maps_lock;  
	Genode::List<Region_map> _region_maps;
	Genode::Fifo<Region_map> _destroyed_region_maps;

	/**
	 * Allocator for Rpc objects created by this session and also for monitoring structures
	 */
	Genode::Allocator &_md_alloc;

	/**
	 * Entrypoint for managing created Rpc objects
	 */
	Genode::Entrypoint &_ep;

	/**
	 * Reference to Target_child's bootstrap phase
	 */
	bool &_bootstrap_phase;

	/**
	 * Parent's session connection which is used by the intercepted methods
	 */
	Genode::Rm_connection  _parent_rm;

	Region_map &_create(Genode::size_t size);
	void _destroy(Region_map &region_map);

	Ram_root &_ram_root;

	
public:
	using Genode::Rpc_object<Genode::Rm_session>::cap;
	
	Rm_session(Genode::Env &env,
		       Genode::Allocator &md_alloc,
		       Genode::Entrypoint &ep,
		       const char *creation_args,
		       Ram_root &ram_root,
		       bool &bootstrap_phase);
  
	~Rm_session();

	Genode::Rm_session_capability parent_cap() { return _parent_rm.cap(); }

	void checkpoint() override;

	void upgrade(const char *upgrade_args) {
		_upgrade_args = upgrade_args;		
	}
	
	const char* upgrade_args() { return _upgrade_args; }
	

	Rm_session *find_by_badge(Genode::uint16_t badge);
	
	/******************************
	 ** Rm session Rpc interface **
	 ******************************/

	/**
	 * Create a virtual Region map, its real counter part and a list element to
	 * manage them
	 */
	Genode::Capability<Genode::Region_map> create(Genode::size_t size) override;
	/**
	 * Destroying the virtual Region map, its real counter part, and the list
	 * element it was managed in
	 *
	 * XXX Untested! During the implementation time, the destroy method was not working.
	 */
	void destroy(Genode::Capability<Genode::Region_map> region_map_cap) override;
};



/**
 * Custom root RPC object to intercept session RPC object creation,
 * modification, and destruction through the root interface
 */
class Rtcr::Rm_root : public Genode::Root_component<Rm_session>
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
	Genode::List<Rm_session> _session_rpc_objs;

protected:
	Rm_session *_create_session(const char *args);
	void _upgrade_session(Rm_session *session, const char *upgrade_args);
	void _destroy_session(Rm_session *session);

	Ram_root &_ram_root;
  
public:
	Rm_root(Genode::Env &env,
			Genode::Allocator &md_alloc,
			Genode::Entrypoint &session_ep,
			Ram_root &ram_root,		
			bool &bootstrap_phase);
  
	~Rm_root();

	Genode::List<Rm_session> &sessions() { return _session_rpc_objs; }
};

#endif /* _RTCR_RM_SESSION_H_ */
