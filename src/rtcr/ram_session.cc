/*
 * \brief  Intercepting Ram session
 * \author Denis Huber
 * \author Johannes Fischer
 * \date   2019-08-29
 */

#include <rtcr/ram/ram_session.h>

using namespace Rtcr;

#ifdef PROFILE
#include <util/profiler.h>
#define PROFILE_THIS_CALL PROFILE_FUNCTION("cyan");
#else
#define PROFILE_THIS_CALL
#endif

#if DEBUG 
#define DEBUG_THIS_CALL Genode::log("\e[38;5;87m", __PRETTY_FUNCTION__, "\033[0m");
#else
#define DEBUG_THIS_CALL
#endif


void Ram_session::_destroy_ramds(Ram_dataspace &ramds)
{

	Genode::Ram_dataspace_capability ds_cap =
		Genode::static_cap_cast<Genode::Ram_dataspace>(ramds.ck_cap);

	/* Destroy Ram_dataspace */
	Genode::destroy(_md_alloc, &ramds);

	/* Free from parent */
	_parent_ram.free(ds_cap);
}


Ram_session::Ram_session(Genode::Env &env,
					     Genode::Allocator &md_alloc,
					     const char *label,
					     const char *creation_args,
					     bool &bootstrap_phase,
					     Genode::Xml_node *config)
	:
	Checkpointable(env, config, "ram_session"),
	_env                (env),
	_md_alloc           (md_alloc),
	_bootstrap_phase    (bootstrap_phase),
	_parent_ram         (env, label),
	_parent_rm          (env),
	ck_creation_args (creation_args)
{
	DEBUG_THIS_CALL
		}


Ram_session::~Ram_session()
{
	while(Ram_dataspace *ds = _ram_dataspaces.first()) {
		_ram_dataspaces.remove(ds);
		Genode::destroy(_md_alloc, ds);
	}
}


void Ram_session::mark_region_map_dataspace(Genode::Dataspace_capability cap)
{
	Ram_dataspace *dataspace = _ram_dataspaces.first();
	/* only iterate through the recently added dataspaces. */
	while(dataspace && dataspace != ck_ram_dataspaces) {
		if (dataspace->ck_cap == cap) {
			dataspace->is_region_map = true;
		}
		dataspace = dataspace->next();
	}
}


void Ram_session::copy_dataspace(Ram_dataspace &info)
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL
		char *dst_addr_start = _env.rm().attach(info.ck_dst_cap);
	char *src_addr_start = _env.rm().attach(info.ck_cap);

	Genode::memcpy(dst_addr_start, src_addr_start, info.ck_size);
	_env.rm().detach(src_addr_start);
	_env.rm().detach(dst_addr_start);
}


void Ram_session::checkpoint()
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL
		ck_badge = cap().local_name();
	ck_bootstrapped = _bootstrap_phase;
	ck_upgrade_args = _upgrade_args;

	// TODO
	//  ck_kcap = _core_module->find_kcap_by_badge(ck_badge);

	/* step 1: remove all destroyed dataspaces */
	Ram_dataspace *dataspace = nullptr;	
	while(dataspace = _destroyed_ram_dataspaces.dequeue()) {
		_ram_dataspaces.remove(dataspace);
		_destroy_ramds(*dataspace);    
	}

	/* step 2: allocate cold dataspace for recently added dataspaces */
	dataspace = _ram_dataspaces.first();	
	while(dataspace && dataspace != ck_ram_dataspaces) {
		if(!dataspace->is_region_map)
			dataspace->ck_dst_cap = _env.ram().alloc(dataspace->ck_size);
		dataspace = dataspace->next();
	}

	/* step 3: copy memory of hot ds to cold ds */
	dataspace = _ram_dataspaces.first();
	while(dataspace) {
		dataspace->checkpoint();

		if(!dataspace->is_region_map)
			copy_dataspace(*dataspace);
    
		dataspace = dataspace->next();
	}

	/* step 4: move pointer forward to update ck_ram_dataspaces */
	ck_ram_dataspaces = _ram_dataspaces.first();
	
}


Ram_session *Ram_session::find_by_badge(Genode::uint16_t badge)
{
	if(badge == cap().local_name())
		return this;
	Ram_session *obj = next();
	return obj ? obj->find_by_badge(badge) : 0;
}


Genode::Ram_dataspace_capability Ram_session::alloc(Genode::size_t size, Genode::Cache_attribute cached)
{
	auto result_cap = _parent_ram.alloc(size, cached);

	/* Create a Ram_dataspace to monitor the newly created Ram_dataspace */
	Ram_dataspace *new_rds = new (_md_alloc) Ram_dataspace(result_cap, size, cached, _bootstrap_phase);
	Genode::Lock::Guard guard(_ram_dataspaces_lock);
	_ram_dataspaces.insert(new_rds);
	return result_cap;

}


void Ram_session::free(Genode::Ram_dataspace_capability ds_cap)
{
	/* Find the Ram_dataspace which monitors the given Ram_dataspace */
	Ram_dataspace *rds = _ram_dataspaces.first();
	if(rds) rds = rds->find_by_badge(ds_cap.local_name());
	if(rds) {
		Genode::Lock::Guard lock_guard(_destroyed_ram_dataspaces_lock);		
		_destroyed_ram_dataspaces.enqueue(rds);
	} else {
		Genode::warning(__func__, " Ram_dataspace not found for ", ds_cap);
		return;
	}
}


int Ram_session::ref_account(Genode::Ram_session_capability ram_session)
{
	auto result = _parent_ram.ref_account(ram_session);
	_ref_account_cap = ram_session;
	return result;
}


int Ram_session::transfer_quota(Genode::Ram_session_capability ram_session, Genode::size_t amount)
{
	return _parent_ram.transfer_quota(ram_session, amount);
}


Genode::size_t Ram_session::quota()
{
	return _parent_ram.quota();
}


Genode::size_t Ram_session::used()
{
	return _parent_ram.used();
}


void Ram_session::set_label(char *label)
{
	_parent_ram.set_label(label);
}


Ram_session *Ram_root::_create_session(const char *args)
{
	/* Extracting label from args */
	char label_buf[128];
	Genode::Arg label_arg = Genode::Arg_string::find_arg(args, "label");
	label_arg.string(label_buf, sizeof(label_buf), "");

	/* Revert ram_quota calculation, because the monitor needs the original
	 * session creation argument */
	char ram_quota_buf[32];
	char readjusted_args[160];
	Genode::strncpy(readjusted_args, args, sizeof(readjusted_args));

	Genode::size_t readjusted_ram_quota = Genode::Arg_string::find_arg(readjusted_args, "ram_quota").ulong_value(0);
	readjusted_ram_quota = readjusted_ram_quota + sizeof(Ram_session) + md_alloc()->overhead(sizeof(Ram_session));

	Genode::snprintf(ram_quota_buf, sizeof(ram_quota_buf), "%zu", readjusted_ram_quota);
	Genode::Arg_string::set_arg(readjusted_args, sizeof(readjusted_args), "ram_quota", ram_quota_buf);

	/* Create custom RAM session */
	Ram_session *new_session =
		new (md_alloc()) Ram_session(_env,
									 _md_alloc,
									 label_buf,
									 readjusted_args,
									 _bootstrap_phase,
									 _config);

	Genode::Lock::Guard lock(_objs_lock);
	_session_rpc_objs.insert(new_session);

	return new_session;
}


void Ram_root::_upgrade_session(Ram_session *session, const char *upgrade_args)
{
	char ram_quota_buf[32];
	char new_upgrade_args[160];

	Genode::strncpy(new_upgrade_args, session->upgrade_args(), sizeof(new_upgrade_args));
	Genode::size_t ram_quota = Genode::Arg_string::find_arg(new_upgrade_args, "ram_quota").ulong_value(0);
	Genode::size_t extra_ram_quota = Genode::Arg_string::find_arg(upgrade_args, "ram_quota").ulong_value(0);
	ram_quota += extra_ram_quota;

	Genode::snprintf(ram_quota_buf, sizeof(ram_quota_buf), "%zu", ram_quota);
	Genode::Arg_string::set_arg(new_upgrade_args, sizeof(new_upgrade_args), "ram_quota", ram_quota_buf);

	_env.parent().upgrade(session->parent_cap(), upgrade_args);
	session->upgrade(upgrade_args);
}


void Ram_root::_destroy_session(Ram_session *session)
{
	_session_rpc_objs.remove(session);
	Genode::destroy(_md_alloc, session);
}


Ram_root::Ram_root(Genode::Env &env,
				   Genode::Allocator &md_alloc,
				   Genode::Entrypoint &session_ep,
				   bool &bootstrap_phase,
				   Genode::Xml_node *config)
	:
	Root_component<Ram_session>(session_ep, md_alloc),
	_env              (env),
	_md_alloc         (md_alloc),
	_ep               (session_ep),
	_bootstrap_phase  (bootstrap_phase),
	_objs_lock        (),
	_session_rpc_objs (),
	_config(config)
{
}


Ram_root::~Ram_root()
{
	while(Ram_session *obj = _session_rpc_objs.first()) {
		_session_rpc_objs.remove(obj);
		Genode::destroy(_md_alloc, obj);
	}
}

