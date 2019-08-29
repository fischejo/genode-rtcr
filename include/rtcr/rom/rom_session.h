/*
 * \brief  Intercepting ROM session
 * \author Denis Huber
 * \author Johannes Fischer
 * \date   2019-08-29
 */

#ifndef _RTCR_ROM_SESSION_COMPONENT_H_
#define _RTCR_ROM_SESSION_COMPONENT_H_

/* Genode includes */
#include <root/component.h>
#include <rom_session/connection.h>
#include <dataspace/client.h>
#include <base/rpc_server.h>
#include <base/entrypoint.h>
#include <base/allocator.h>

/* Rtcr includes */
#include <rtcr/checkpointable.h>

namespace Rtcr {
	class Rom_session;
	class Rom_root;
}

class Rtcr::Rom_session : public Rtcr::Checkpointable,
						  public Genode::Rpc_object<Genode::Rom_session>,
						  public Genode::List<Rom_session>::Element
{
public:
	/******************
	 ** COLD STORAGE **
	 ******************/
  
	Genode::String<160> ck_creation_args;
	Genode::String<160> ck_upgrade_args;
	bool ck_bootstrapped;
	Genode::uint16_t ck_badge;
	Genode::addr_t ck_kcap;
	Genode::uint16_t ck_dataspace_badge;
	Genode::uint16_t ck_sigh_badge;
  
protected:

	/*****************
	 ** HOT STORAGE **
	 *****************/
	
	char* _upgrade_args;
	bool _bootstrapped;
	Genode::Rom_dataspace_capability  _dataspace;
	Genode::size_t                    _size;
	Genode::Signal_context_capability _sigh;

	/**
	 * Environment of creator component (usually rtcr)
	 */
	Genode::Env &_env;

	/**
	 * Allocator
	 */
	Genode::Allocator &_md_alloc;

	/**
	 * Entrypoint
	 */
	Genode::Entrypoint &_ep;

	/**
	 * Connection to parent's ROM session
	 */
	Genode::Rom_connection _parent_rom;



	
public:
	using Genode::Rpc_object<Genode::Rom_session>::cap;
	
	Rom_session(Genode::Env &env,
				Genode::Allocator &md_alloc,
				Genode::Entrypoint &ep,
				const char *label,
				const char *creation_args,
				bool &bootstrap_phase,
				Genode::Xml_node *config);

	~Rom_session();
  
	void print(Genode::Output &output) const {
		using Genode::Hex;
		Genode::print(output, ", ck_dataspace_badge=", ck_dataspace_badge,
					  ", ck_sigh_badge=", ck_sigh_badge);    
	}

	void checkpoint() override;
	
	Genode::Rom_session_capability parent_cap() { return _parent_rom.cap(); }

	Rom_session *find_by_badge(Genode::uint16_t badge);

	/*******************************
	 ** Rom session Rpc interface **
	 *******************************/

	Genode::Rom_dataspace_capability dataspace() override;
	bool update() override;
	void sigh(Genode::Signal_context_capability sigh) override;
};

/**
 * Custom root RPC object to intercept session RPC object creation,
 * modification, and destruction through the root interface
 */
class Rtcr::Rom_root : public Genode::Root_component<Rom_session>
{
private:

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
	Genode::List<Rom_session> _session_rpc_objs;

protected:
	Rom_session *_create_session(const char *args);

	void _upgrade_session(Rom_session *session, const char *upgrade_args);

	void _destroy_session(Rom_session *session);

	Genode::Xml_node *_config;
public:
  
	Rom_root(Genode::Env &env,
			 Genode::Allocator &md_alloc,
			 Genode::Entrypoint &session_ep,
			 bool &bootstrap_phase,
			 Genode::Xml_node *config);

	~Rom_root();
  
	Rom_session *find_by_badge(Genode::uint16_t badge);
	Genode::List<Rom_session> &session_infos() { return _session_rpc_objs; }
};

#endif /* _RTCR_ROM_SESSION_COMPONENT_H_ */
