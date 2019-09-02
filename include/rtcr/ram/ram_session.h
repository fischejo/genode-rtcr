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
#include <util/list.h>
#include <util/fifo.h>
#include <util/misc_math.h>

/* Rtcr includes */
#include <rtcr/ram/ram_dataspace.h>
#include <rtcr/checkpointable.h>
#include <rtcr/info_structs.h>
#include <rtcr/child_info.h>

namespace Rtcr {
	class Child_info;
	class Ram_session;
	class Ram_root;
	class Ram_session_info;
}


struct Rtcr::Ram_session_info : Session_info {
	Ram_dataspace *ram_dataspaces;
	Genode::Ram_session_capability   ref_account_cap;

	Ram_session_info(const char* creation_args) : Session_info(creation_args) {}
	
	void print(Genode::Output &output) const {
		Genode::print(output, " Ram session:\n ");
		Session_info::print(output);
		
		Ram_dataspace *ds = ram_dataspaces;
		if(!ds) Genode::print(output, "  <empty>\n");
		while(ds) {
			Genode::print(output, "  ", ds->info);
			ds = ds->next();
		}
	}
};


/**
 * Custom RAM session to monitor the allocation, freeing, and ram quota
 * transfers
 *
 * Instead of providing ordinary RAM dataspaces, it can provide managed
 * dataspaces, which are used to monitor the access to the provided RAM
 * dataspaces
 */
class Rtcr::Ram_session : public Rtcr::Checkpointable,
						  public Genode::Rpc_object<Genode::Ram_session>
{
public:
	/******************
	 ** COLD STORAGE **
	 ******************/
  
	Ram_session_info info;
	
protected:
	/*****************
	 ** HOT STORAGE **
	 *****************/
	
	const char* _upgrade_args;
	Genode::Ram_session_capability _ref_account_cap;
	/**
	 * List of allocated ram dataspaces
	 */
	Genode::Lock _ram_dataspaces_lock;
	Genode::List<Ram_dataspace> _ram_dataspaces;

	Genode::Lock _destroyed_ram_dataspaces_lock;  
	Genode::Fifo<Ram_dataspace> _destroyed_ram_dataspaces; 

	/**
	 * Environment of Rtcr; needed to upgrade RAM quota of the RM session
	 */
	Genode::Env &_env;

	/**
	 * Allocator for structures monitoring the allocated Ram dataspaces
	 */
	Genode::Allocator &_md_alloc;

	/**
	 * Connection to the parent Ram session (usually core's Ram session)
	 */
	Genode::Ram_connection _parent_ram;

	/**
	 * Connection to the parent Rm session for creating new Region_maps (usually
	 * core's Rm session)
	 */
	Genode::Rm_connection _parent_rm;

	/**
	 * Destroy rds and all its sub infos)
	 */
	void _destroy_ramds(Ram_dataspace &rds);

	void copy_dataspace(Ram_dataspace &info);

	Genode::Xml_node *_config;

	Genode::size_t _read_child_quota(const char* child_name);

	Child_info *_child_info;
	
public:
	using Genode::Rpc_object<Genode::Ram_session>::cap;
	
	Ram_session(Genode::Env &env,
				Genode::Allocator &md_alloc,
				const char *label,
				const char *creation_args,
				Child_info *child_info);
  
	~Ram_session();

	void checkpoint() override;

	void upgrade(const char *upgrade_args) {
		_upgrade_args = upgrade_args;		
	}
	
	const char* upgrade_args() { return _upgrade_args; }

	void mark_region_map_dataspace(Genode::Dataspace_capability ds_cap);
	
	Genode::Ram_session_capability parent_cap() { return _parent_ram.cap(); }


	/***************************
	 ** Ram_session interface **
	 ***************************/

	/**
	 * Allocate a Ram_dataspace
	 */
	Genode::Ram_dataspace_capability alloc(Genode::size_t size,
										   Genode::Cache_attribute cached) override;
	/**
	 * Frees the Ram_dataspace and destroys all monitoring structures
	 */
	void free(Genode::Ram_dataspace_capability ds_cap) override;
	int ref_account(Genode::Ram_session_capability ram_session) override;
	int transfer_quota(Genode::Ram_session_capability ram_session,
					   Genode::size_t amount) override;
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
class Rtcr::Ram_root : public Genode::Root_component<Ram_session>
{
protected:
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

	Genode::Lock &_childs_lock;
	Genode::List<Child_info> &_childs;
	

	Ram_session *_create_session(const char *args);
	void _upgrade_session(Ram_session *session, const char *upgrade_args);
	void _destroy_session(Ram_session *session);

public:

	Ram_root(Genode::Env &env,
			 Genode::Allocator &md_alloc,
			 Genode::Entrypoint &session_ep,
			 Genode::Lock &childs_lock,
			 Genode::List<Child_info> &childs);
			 
	~Ram_root();

};


#endif /* _RTCR_RAM_SESSION_H_ */
