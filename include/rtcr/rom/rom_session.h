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
#include <rtcr/info_structs.h>
#include <rtcr/child_info.h>

namespace Rtcr {
	class Rom_session;
	class Rom_root;
	class Rom_session_info;
}


struct Rtcr::Rom_session_info : Session_info {
	Genode::uint16_t dataspace_badge;
	Genode::uint16_t sigh_badge;

	Rom_session_info(const char* creation_args) : Session_info(creation_args) {}
	
	void print(Genode::Output &output) const {
		Genode::print(output, " Rom session:\n  ");
		Session_info::print(output);
		Genode::print(output,
					  " dataspace_badge=", dataspace_badge,
					  ", sigh_badge=", sigh_badge, "'\n");
	}
};


class Rtcr::Rom_session : public Rtcr::Checkpointable,
						  public Genode::Rpc_object<Genode::Rom_session>
{
public:
	/******************
	 ** COLD STORAGE **
	 ******************/
  
	Rom_session_info info;
	
protected:

	/*****************
	 ** HOT STORAGE **
	 *****************/
	
	const char* _upgrade_args;

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

	Child_info *_child_info;
	
public:
	using Genode::Rpc_object<Genode::Rom_session>::cap;
	
	Rom_session(Genode::Env &env,
				Genode::Allocator &md_alloc,
				Genode::Entrypoint &ep,
				const char *creation_args,
				Child_info *child_info);

	~Rom_session() {};
  
	void checkpoint() override;

	void upgrade(const char *upgrade_args) {
		_upgrade_args = upgrade_args;		
	}
	
	const char* upgrade_args() { return _upgrade_args; }
	
	Genode::Rom_session_capability parent_cap() { return _parent_rom.cap(); }

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

	Genode::Lock &_childs_lock;
	Genode::List<Child_info> &_childs;

protected:


	/**
	 * Wrapper for creating a ram session
	 */
	virtual Rom_session *_create_rom_session(Child_info *info, const char *args);
	
	Rom_session *_create_session(const char *args);

	void _upgrade_session(Rom_session *session, const char *upgrade_args);

	void _destroy_session(Rom_session *session);

public:
  
	Rom_root(Genode::Env &env,
			 Genode::Allocator &md_alloc,
			 Genode::Entrypoint &session_ep,
			 Genode::Lock &childs_lock,
			 Genode::List<Child_info> &childs);

	~Rom_root();

};

#endif /* _RTCR_ROM_SESSION_COMPONENT_H_ */
