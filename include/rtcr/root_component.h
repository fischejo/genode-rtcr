/*
 * \brief  Intercepting Pd session
 * \author Denis Huber
 * \author Johannes Fischer
 * \date   2019-08-29
 */

#ifndef _RTCR_ROOT_COMPONENT_H_
#define _RTCR_ROOT_COMPONENT_H_

/* Genode includes */
#include <base/service.h>
#include <root/component.h>
#include <util/list.h>
#include <base/allocator.h>
#include <base/registry.h>

/* Rtcr includes */
#include <rtcr/child_info.h>

namespace Rtcr {
	template <typename> class Root_component;
}

using namespace Rtcr;

template <typename SESSION>
class Rtcr::Root_component : public Genode::Root_component<SESSION>
{
private:
	Genode::Lock &_childs_lock;
	Genode::List<Child_info> &_childs;
	Genode::Local_service<SESSION> _service;
	Genode::Registry<Genode::Service>::Element _registered_service;
	
protected:
	Genode::Env &_env;
	Genode::Allocator &_alloc;
	Genode::Entrypoint &_ep;

public:

	SESSION *_create_session(const char *args) override
	{		
		/* get child label from args */
		Genode::Session_label label = Genode::label_from_args(args);

		/* there are seveal ways how a label is formed:
		 * 
		 * * `sheep_counter` used by CPU, PD
		 * * `sheep_counter -> ` used by Timer
		 * * `sheep_counter -> config` used by ROM
		 *
		 * The ROM formed label is not supported yet.  Following lines filter
		 * the name of the component from CPU, PD & Timer formed labels.
		 */
		Genode::Session_label prefix = label.prefix();
		Genode::Session_label name = prefix == "" ? label : prefix;

		/* find child based on name */
		_childs_lock.lock();
		Child_info *info = _childs.first();
		if(info) info = info->find_by_name(name.string());

		/* child_info does not exist, let's create it */
		if(!info) {
			info = new(_alloc) Child_info(name.string());
			_childs.insert(info);
		}
		_childs_lock.unlock();

		/* create session */
		return new(_alloc) SESSION(_env, _alloc, _ep, args, info);
	}

	
	void _upgrade_session(SESSION *session, const char *upgrade_args) override
	{
		/* the intercepting session does the upgrade by itself */
		session->upgrade(upgrade_args);		
 	}

	void _destroy_session(SESSION *session) override {
		Genode::destroy(_alloc, session);
	}

	Root_component(Genode::Env &env,
	               Genode::Allocator &alloc,
	               Genode::Entrypoint &ep,
	               Genode::Lock &childs_lock,
	               Genode::List<Child_info> &childs,
	               Genode::Registry<Genode::Service> &registry)
		:
		Genode::Root_component<SESSION>(ep, alloc),
		_env(env),
		_alloc(alloc),
		_ep(ep),
		_childs_lock(childs_lock),
		_childs(childs),
		_service(*this),
		_registered_service(registry, _service)
	{
		env.parent().announce(env.ep().manage(*this));
	}

	~Root_component() {}
};

#endif
