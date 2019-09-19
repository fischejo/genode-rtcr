/*
 * \brief  Intercepting ROM session
 * \author Denis Huber
 * \author Johannes Fischer
 * \date   2019-08-29
 */

#ifndef _RTCR_ROM_SESSION_H_
#define _RTCR_ROM_SESSION_H_

/* Genode includes */
#include <root/component.h>
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

namespace Rtcr {
	class Rom_session;
	class Rom_factory;
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
	            const char *label,
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



class Rtcr::Rom_factory : public Genode::Local_service<Rtcr::Rom_session>::Factory
{
private:
	Genode::Env &_env;
	Genode::Allocator &_md_alloc;
	Genode::Entrypoint &_ep;

	Genode::Lock &_childs_lock;
	Genode::List<Child_info> &_childs;

	Genode::Local_service<Rom_session> _service;
	Genode::Session::Diag _diag;

protected:

	Rom_session *_create(Child_info *info, const char *args);

public:
	Rom_factory(Genode::Env &env,
	            Genode::Allocator &md_alloc,
	            Genode::Entrypoint &ep,
	            Genode::Lock &childs_lock,
	            Genode::List<Child_info> &childs);

	Rom_session &create(Genode::Session_state::Args const &args, Genode::Affinity) override;
	void upgrade(Rom_session&, Genode::Session_state::Args const &) override;
	void destroy(Rom_session&) override;

	Genode::Service *service() { return &_service; }
};

#endif /* _RTCR_ROM_SESSION_H_ */
