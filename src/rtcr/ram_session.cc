/*
 * \brief  Intercepting Ram session
 * \author Denis Huber
 * \date   2016-08-12
 */

#include <rtcr/ram/ram_session.h>

using namespace Rtcr;



void Ram_session::_destroy_ramds_info(Ram_dataspace_info &ramds_info)
{

	Genode::Ram_dataspace_capability ds_cap =
		Genode::static_cap_cast<Genode::Ram_dataspace>(ramds_info.ck_cap);


	/* Destroy Ram_dataspace_info */
	Genode::destroy(_md_alloc, &ramds_info);

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
	if(verbose_debug) Genode::log("\033[33m", "Ram", "\033[0m(parent ",
				      _parent_ram, ")");
}


Ram_session::~Ram_session()
{
	/* Destroy all Ram_dataspace_infos */
	while(Ram_dataspace_info *rds_info = ck_ram_dataspaces.first()) {
		_destroy_ramds_info(*rds_info);
	}

	// TODO FJO: delete destroyed and new ds
	if(verbose_debug) Genode::log("\033[33m", "~Ram", "\033[0m ", _parent_ram);
}

void Ram_session::mark_region_map_dataspace(Genode::Dataspace_capability cap)
{
  Ram_dataspace_info *dataspace = _new_ram_dataspaces.first();
  while(dataspace) {
    if (dataspace->ck_cap == cap) {
      dataspace->is_region_map = true;
    }
    dataspace = dataspace->next();
  }
}

void Ram_session::copy_dataspace(Ram_dataspace_info &info)
{
	char *dst_addr_start = _env.rm().attach(info.ck_dst_cap);
	char *src_addr_start = _env.rm().attach(info.ck_cap);

	Genode::memcpy(dst_addr_start, src_addr_start, info.ck_size);
	_env.rm().detach(src_addr_start);
	_env.rm().detach(dst_addr_start);
}


void Ram_session::checkpoint()
{
  ck_badge = cap().local_name();
  ck_bootstrapped = _bootstrap_phase;
//  ck_upgrade_args = _upgrade_args.string();

  // TODO
  //  ck_kcap = _core_module->find_kcap_by_badge(ck_badge);

  Ram_dataspace_info *dataspace = nullptr;
  while(dataspace = _new_ram_dataspaces.first()) {
    ck_ram_dataspaces.insert(dataspace);
    _new_ram_dataspaces.remove(dataspace);

    if(!dataspace->is_region_map)
      dataspace->ck_dst_cap = _env.ram().alloc(dataspace->ck_size);
  }

  while(dataspace = _destroyed_ram_dataspaces.first()) {
    ck_ram_dataspaces.remove(dataspace);
    _destroyed_ram_dataspaces.remove(dataspace);
    _destroy_ramds_info(*dataspace);    
  }

 dataspace = ck_ram_dataspaces.first();
 while(dataspace) {
    dataspace->checkpoint();

    if(!dataspace->is_region_map)
      copy_dataspace(*dataspace);
    
    dataspace = dataspace->next();    
  }    
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
	if(verbose_debug) Genode::log("Ram::\033[33m", __func__,
				      "\033[0m(size=", Genode::Hex(size),")");

	auto result_cap = _parent_ram.alloc(size, cached);

	/* Create a Ram_dataspace_info to monitor the newly created Ram_dataspace */
	Ram_dataspace_info *new_rds_info = new (_md_alloc) Ram_dataspace_info(result_cap, size, cached, _bootstrap_phase);
	Genode::Lock::Guard guard(_new_ram_dataspaces_lock);
	_new_ram_dataspaces.insert(new_rds_info);

	if(verbose_debug) Genode::log("  result: ", result_cap);

	return result_cap;

}


void Ram_session::free(Genode::Ram_dataspace_capability ds_cap)
{
	if(verbose_debug) Genode::log("Ram::\033[33m", __func__, "\033[0m(", ds_cap, ")");

	Genode::Lock::Guard lock_guard(_destroyed_ram_dataspaces_lock);

	/* Find the Ram_dataspace_info which monitors the given Ram_dataspace */
	Ram_dataspace_info *rds_info = _new_ram_dataspaces.first();
	if(rds_info) rds_info = rds_info->find_by_badge(ds_cap.local_name());
	if(!rds_info) {
	  rds_info = _destroyed_ram_dataspaces.first();
	  if(rds_info) rds_info = rds_info->find_by_badge(ds_cap.local_name());
	} 	  
	
	/* Ram_dataspace_info found? */
	if(rds_info) {
	  _destroyed_ram_dataspaces.insert(rds_info);
	} else {
		Genode::warning(__func__, " Ram_dataspace_info not found for ", ds_cap);
		return;
	}
}


int Ram_session::ref_account(Genode::Ram_session_capability ram_session)
{
	if(verbose_debug) Genode::log("Ram::\033[33m", __func__, "\033[0m(ref=", ram_session, ")");

	auto result = _parent_ram.ref_account(ram_session);
	_ref_account_cap = ram_session;

	if(verbose_debug) Genode::log("  result: ", result);

	return result;
}


int Ram_session::transfer_quota(Genode::Ram_session_capability ram_session, Genode::size_t amount)
{
	if(verbose_debug) Genode::log("Ram::\033[33m", __func__, "\033[0m(to=", ram_session, ", size=", amount, ")");

	auto result = _parent_ram.transfer_quota(ram_session, amount);

	if(verbose_debug) Genode::log("  result: ", result);

	return result;
}


Genode::size_t Ram_session::quota()
{
	if(verbose_debug) Genode::log("Ram::\033[33m", __func__, "\033[0m()");

	auto result = _parent_ram.quota();

	if(verbose_debug) Genode::log("  result: ", result);

	return result;
}


Genode::size_t Ram_session::used()
{
	if(verbose_debug) Genode::log("Ram::\033[33m", __func__, "\033[0m()");

	auto result = _parent_ram.used();

	if(verbose_debug) Genode::log("  result: ", result);

	return result;
}


void Ram_session::set_label(char *label)
{
	// TODO verbose_debug
	_parent_ram.set_label(label);
}


Ram_session *Ram_root::_create_session(const char *args)
{
	if(verbose_debug) Genode::log("Ram_root::\033[33m", __func__, "\033[0m(", args,")");

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
	if(verbose_debug) Genode::log("Ram_root::\033[33m", __func__, "\033[0m(session ", session->cap(),", args=", upgrade_args,")");

	char ram_quota_buf[32];
	char new_upgrade_args[160];

//	Genode::strncpy(new_upgrade_args, session->parent_state().upgrade_args.string(), sizeof(new_upgrade_args));

	Genode::size_t ram_quota = Genode::Arg_string::find_arg(new_upgrade_args, "ram_quota").ulong_value(0);
	Genode::size_t extra_ram_quota = Genode::Arg_string::find_arg(upgrade_args, "ram_quota").ulong_value(0);
	ram_quota += extra_ram_quota;

	Genode::snprintf(ram_quota_buf, sizeof(ram_quota_buf), "%zu", ram_quota);
	Genode::Arg_string::set_arg(new_upgrade_args, sizeof(new_upgrade_args), "ram_quota", ram_quota_buf);

//	session->parent_state().upgrade_args = new_upgrade_args;

	_env.parent().upgrade(session->parent_cap(), upgrade_args);
}


void Ram_root::_destroy_session(Ram_session *session)
{
	if(verbose_debug) Genode::log("Ram_root::\033[33m", __func__, "\033[0m(session ", session->cap(),")");

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
	if(verbose_debug) Genode::log("\033[33m", __func__, "\033[0m");
}


Ram_root::~Ram_root()
{
	while(Ram_session *obj = _session_rpc_objs.first()) {
		_session_rpc_objs.remove(obj);
		Genode::destroy(_md_alloc, obj);
	}

	if(verbose_debug) Genode::log("\033[33m", __func__, "\033[0m");
}

