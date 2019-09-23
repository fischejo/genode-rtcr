/*
 * \brief  Intercepting ROM session
 * \author Denis Huber
 * \author Johannes Fischer
 * \date   2019-08-29
 */

#ifndef _RTCR_ROM_SESSION_H_
#define _RTCR_ROM_SESSION_H_

/* Genode includes */
#include <rom_session/connection.h>
#include <dataspace/client.h>
#include <base/rpc_server.h>
#include <base/entrypoint.h>
#include <base/allocator.h>
#include <base/session_object.h>

/* Rtcr includes */
#include <rtcr/checkpointable.h>
#include <rtcr/rom/rom_session_info.h>
#include <rtcr/child_info.h>
#include <rtcr/root_component.h>

namespace Rtcr {
	class Rom_session;
	class Rom_root;
}

class Rtcr::Rom_session : public Rtcr::Checkpointable,
                          public Genode::Rpc_object<Genode::Rom_session>,
                          public Rtcr::Rom_session_info
{
protected:
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

	~Rom_session();

	void checkpoint() override;

	void upgrade(const char *upgrade_args);

	const char* upgrade_args() { return _upgrade_args; }

	Genode::Rom_session_capability parent_cap() { return _parent_rom.cap(); }

	/*******************************
	 ** Rom session Rpc interface **
	 *******************************/

	Genode::Rom_dataspace_capability dataspace() override;
	bool update() override;
	void sigh(Genode::Signal_context_capability sigh) override;
};


class Rtcr::Rom_root : public Root_component<Rom_session>
{
public:	
	Rtcr::Rom_session *_create_session(Child_info *info, const char *args) override
	{
		Rom_session *rom_session = new (_alloc) Rom_session(_env, _alloc, _ep, args, info);
		info->rom_session = rom_session;
		return rom_session;
	}

	void _destroy_session(Child_info *info, Rom_session *session) override
	{
		Genode::destroy(_alloc, session);
		info->rom_session = nullptr;
	}


	Rom_root(Genode::Env &env,
	         Genode::Allocator &alloc,
	         Genode::Entrypoint &ep,
	         Genode::Lock &childs_lock,
	         Genode::List<Child_info> &childs,
	         Genode::Registry<Genode::Service> &registry)
		:
		Root_component<Rom_session>(env, alloc, ep, childs_lock, childs, registry)
	{}
};


#endif /* _RTCR_ROM_SESSION_H_ */
