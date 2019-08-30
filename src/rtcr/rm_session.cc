/*
 * \brief  Intercepting Rm session
 * \author Denis Huber
 * \author Johannes Fischer
 * \date   2019-08-29
 */

#include <rtcr/rm/rm_session.h>

#ifdef PROFILE
#include <util/profiler.h>
#define PROFILE_THIS_CALL PROFILE_FUNCTION("magenta");
#else
#define PROFILE_THIS_CALL
#endif

#if DEBUG 
#define DEBUG_THIS_CALL Genode::log("\e[38;5;207m", __PRETTY_FUNCTION__, "\033[0m");
#else
#define DEBUG_THIS_CALL
#endif

using namespace Rtcr;

Region_map &Rm_session::_create(Genode::size_t size)
{
	/* Create real Region map from parent */
	auto parent_cap = _parent_rm.create(size);

	/* Create custom Region map */
	Region_map *new_region_map = new (_md_alloc) Region_map(
		_md_alloc, parent_cap, size, "custom", _bootstrap_phase);

	/* Manage custom Region map */
	_ep.manage(*new_region_map);

	/* Insert custom Region map into list */
	Genode::Lock::Guard lock(_region_maps_lock);
	_region_maps.insert(new_region_map);
	_ram_session.mark_region_map_dataspace(new_region_map->dataspace());
	return *new_region_map;
}


void Rm_session::_destroy(Region_map &region_map)
{
	/* Reverse order as in _create */
	auto parent_cap = region_map.parent_cap();

	/* Dissolve custom RPC object */
	_ep.dissolve(region_map);

	/* Destroy real Region map from parent */
	_parent_rm.destroy(parent_cap);
}


Rm_session::Rm_session(Genode::Env &env,
					   Genode::Allocator &md_alloc,
					   Genode::Entrypoint &ep,
					   const char *creation_args,
					   Ram_session &ram_session,
					   bool &bootstrap_phase,
					   Genode::Xml_node *config)
	:
	Checkpointable(env, config, "rm_session"),
	_md_alloc         (md_alloc),
	_ep               (ep),
	_bootstrap_phase  (bootstrap_phase),
	_parent_rm        (env),
	_ram_session (ram_session),
	ck_creation_args (creation_args)
{
	DEBUG_THIS_CALL
}


Rm_session::~Rm_session()
{

}




void Rm_session::checkpoint()
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL
	ck_badge = cap().local_name();
	ck_bootstrapped = _bootstrap_phase;
//  ck_upgrade_args = _upgrade_args.string();

	// TODO
	//  ck_kcap = _core_module->find_kcap_by_badge(ck_badge);

	Region_map *region_map = nullptr;
	while(region_map = _destroyed_region_maps.dequeue()) {
		_region_maps.remove(region_map);
		Genode::destroy(_md_alloc, &region_map);
	}

	region_map = _region_maps.first();
	while(region_map) {
		region_map->checkpoint();
		region_map = region_map->next();
	}

	ck_region_maps = _region_maps.first();
}


Genode::Capability<Genode::Region_map> Rm_session::create(Genode::size_t size)
{
	/* Create custom Region map */
	Region_map &new_region_map = _create(size);
	return new_region_map.cap();
}


void Rm_session::destroy(Genode::Capability<Genode::Region_map> region_map_cap)
{
	/* Find RPC object for the given Capability */
	Region_map *region_map = _region_maps.first();
	if(region_map) region_map = region_map->find_by_badge(region_map_cap.local_name());
	if(region_map) {
		Genode::error("Issuing Rm_session::destroy, which is bugged and hangs up.");

		Genode::Lock::Guard lock(_destroyed_region_maps_lock);
		_destroyed_region_maps.enqueue(region_map);
		
		_destroy(*region_map);
	} else {
		Genode::error("No Region map with ", region_map_cap, " found!");
	}
}


Rm_session *Rm_root::_create_session(const char *args)
{
	/* Revert ram_quota calculation, because the monitor needs the original
	 * session creation argument */
	char ram_quota_buf[32];
	char readjusted_args[160];
	Genode::strncpy(readjusted_args, args, sizeof(readjusted_args));

	Genode::size_t readjusted_ram_quota = Genode::Arg_string::find_arg(readjusted_args, "ram_quota").ulong_value(0);
	readjusted_ram_quota = readjusted_ram_quota + sizeof(Rm_session) + md_alloc()->overhead(sizeof(Rm_session));

	Genode::snprintf(ram_quota_buf, sizeof(ram_quota_buf), "%zu", readjusted_ram_quota);
	Genode::Arg_string::set_arg(readjusted_args, sizeof(readjusted_args), "ram_quota", ram_quota_buf);

	/* Create custom Rm_session */
	Rm_session *new_session =
		new (md_alloc()) Rm_session(_env,
									_md_alloc,
									_ep,
									readjusted_args,
									_ram_session,
									_bootstrap_phase,
									_config);

	Genode::Lock::Guard lock(_objs_lock);
	_session_rpc_objs.insert(new_session);

	return new_session;
}


void Rm_root::_upgrade_session(Rm_session *session, const char *upgrade_args)
{
	char ram_quota_buf[32];
	char new_upgrade_args[160];

//	Genode::strncpy(new_upgrade_args, session->parent_state().upgrade_args.string(), sizeof(new_upgrade_args));

	Genode::size_t ram_quota = Genode::Arg_string::find_arg(new_upgrade_args, "ram_quota").ulong_value(0);
	Genode::size_t extra_ram_quota = Genode::Arg_string::find_arg(upgrade_args, "ram_quota").ulong_value(0);
	ram_quota += extra_ram_quota;

	Genode::snprintf(ram_quota_buf, sizeof(ram_quota_buf), "%zu", ram_quota);
	Genode::Arg_string::set_arg(new_upgrade_args, sizeof(new_upgrade_args), "ram_quota", ram_quota_buf);


	// TODO
	// session->parent_state().upgrade_args = new_upgrade_args;

	_env.parent().upgrade(session->parent_cap(), upgrade_args);
}


void Rm_root::_destroy_session(Rm_session *session)
{
	_session_rpc_objs.remove(session);
	Genode::destroy(_md_alloc, session);
}


Rm_root::Rm_root(Genode::Env &env,
				 Genode::Allocator &md_alloc,
				 Genode::Entrypoint &session_ep,
				 Ram_session &ram_session,		 
				 bool &bootstrap_phase,
				 Genode::Xml_node *config)
	:
	Root_component<Rm_session>(session_ep, md_alloc),
	_env              (env),
	_md_alloc         (md_alloc),
	_ep               (session_ep),
	_bootstrap_phase  (bootstrap_phase),
	_ram_session (ram_session),
	_objs_lock        (),
	_session_rpc_objs (),
	_config (config)
{
}

Rm_root::~Rm_root()
{
	while(Rm_session *obj = _session_rpc_objs.first()) {
		_session_rpc_objs.remove(obj);
		Genode::destroy(_md_alloc, obj);
	}
}


Rm_session *Rm_root::find_by_badge(Genode::uint16_t badge)
{
	Rm_session *obj = _session_rpc_objs.first();
	while(obj) { 
		if(badge == obj->cap().local_name())
			return obj;
		obj = obj->next();
	}
	return 0;
}
