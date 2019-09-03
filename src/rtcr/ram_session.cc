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


Ram_session::Ram_session(Genode::Env &env,
					     Genode::Allocator &md_alloc,
					     const char *creation_args,
					     Child_info *child_info)
	:
	Checkpointable(env, "ram_session"),
	_env                (env),
	_md_alloc           (md_alloc),
	_parent_ram         (env, child_info->name.string()),
	_parent_rm          (env),
	_child_info (child_info),
	info (creation_args)
{
	DEBUG_THIS_CALL
	/* Donate ram quota to child */
	ref_account(env.ram_session_cap());
	// Note: transfer goes directly to parent's ram session
	Genode::size_t quota = _read_child_quota(child_info->name.string());
	env.ram().transfer_quota(parent_cap(), quota);
}


Ram_session::~Ram_session()
{
	while(Ram_dataspace *ds = _ram_dataspaces.first()) {
		_ram_dataspaces.remove(ds);
		Genode::destroy(_md_alloc, ds);
	}
}


Genode::size_t Ram_session::_read_child_quota(const char* child_name)
{
	Genode::size_t quota = 512*1024; // 512 kB
	try {
		Genode::Xml_node config_node = Genode::config()->xml_node();
		Genode::Xml_node ck_node = config_node.sub_node("child");
		Genode::String<30> node_name;
		while(Genode::strcmp(child_name, ck_node.attribute_value("name", node_name).string()))
			ck_node = ck_node.next("child");

		quota = ck_node.attribute_value<unsigned int>("quota", quota);
	}
	catch (...) {}
	return quota;
}


void Ram_session::mark_region_map_dataspace(Genode::Dataspace_capability cap)
{
	Ram_dataspace *dataspace = _ram_dataspaces.first();
	/* only iterate through the recently added dataspaces. */
	while(dataspace && dataspace != info.ram_dataspaces) {
		if (dataspace->info.cap == cap) {
			dataspace->is_region_map = true;
		}
		dataspace = dataspace->next();
	}
}


void Ram_session::_destroy_dataspace(Ram_dataspace &ds)
{
	/* detach */
	 _env.rm().detach(ds.dst);
	 _env.rm().detach(ds.src);

	 /* free */
	_parent_ram.free(ds.info.cap);
	_env.ram().free(ds.info.dst_cap);
	
	/* Destroy Ram_dataspace */
	Genode::destroy(_md_alloc, &ds);
}


void Ram_session::_copy_dataspace(Ram_dataspace &ds)
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL;

	Genode::memcpy(ds.dst, ds.src, ds.info.size);
}


void Ram_session::_alloc_dataspace(Ram_dataspace &ds)
{
	ds.info.dst_cap = _env.ram().alloc(ds.info.size);	
}

void Ram_session::_attach_dataspace(Ram_dataspace &ds)
{
	ds.dst = _env.rm().attach(ds.info.dst_cap);
	ds.src = _env.rm().attach(ds.info.cap);		
}


void Ram_session::checkpoint()
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL

	info.badge = cap().local_name();
	info.bootstrapped = _child_info->bootstrapped;
	info.upgrade_args = _upgrade_args;

	/* step 1: remove all destroyed dataspaces */
	Ram_dataspace *dataspace = nullptr;	
	while(dataspace = _destroyed_ram_dataspaces.dequeue()) {
		_ram_dataspaces.remove(dataspace);
		_destroy_dataspace(*dataspace);    
	}

	/* step 2: allocate cold dataspace for recently added dataspaces */
	dataspace = _ram_dataspaces.first();	
	while(dataspace && dataspace != info.ram_dataspaces) {
		if(!dataspace->is_region_map) {
			_alloc_dataspace(*dataspace);
			_attach_dataspace(*dataspace);
		}
		
		dataspace = dataspace->next();
	}

	/* step 3: copy memory of hot ds to cold ds */
	dataspace = _ram_dataspaces.first();
	while(dataspace) {
		dataspace->checkpoint();

		if(!dataspace->is_region_map)
			_copy_dataspace(*dataspace);
    
		dataspace = dataspace->next();
	}

	/* step 4: move pointer forward to update ck_ram_dataspaces */
	info.ram_dataspaces = _ram_dataspaces.first();
	
}


Genode::Ram_dataspace_capability Ram_session::alloc(Genode::size_t size, Genode::Cache_attribute cached)
{
	DEBUG_THIS_CALL;
	auto result_cap = _parent_ram.alloc(size, cached);

	/* Create a Ram_dataspace to monitor the newly created Ram_dataspace */
	Ram_dataspace *new_rds = new (_md_alloc) Ram_dataspace(result_cap,
														   size,
														   cached,
														   _child_info->bootstrapped);
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

Ram_session *Ram_root::_create_ram_session(Child_info *info, const char *args)
{
	return new (md_alloc()) Ram_session(_env, _md_alloc, args, info);
}


Ram_session *Ram_root::_create_session(const char *args)
{
	DEBUG_THIS_CALL;

	/* Revert ram_quota calculation, because the monitor needs the original
	 * session creation argument */
	char ram_quota_buf[32];
	char readjusted_args[160];
	Genode::strncpy(readjusted_args, args, sizeof(readjusted_args));

	Genode::size_t readjusted_ram_quota = Genode::Arg_string::find_arg(readjusted_args, "ram_quota").ulong_value(0);
	readjusted_ram_quota = readjusted_ram_quota + sizeof(Ram_session) + md_alloc()->overhead(sizeof(Ram_session));

	Genode::snprintf(ram_quota_buf, sizeof(ram_quota_buf), "%zu", readjusted_ram_quota);
	Genode::Arg_string::set_arg(readjusted_args, sizeof(readjusted_args), "ram_quota", ram_quota_buf);


	/* Extracting label from args */
	char label_buf[128];
	Genode::Arg label_arg = Genode::Arg_string::find_arg(args, "label");
	label_arg.string(label_buf, sizeof(label_buf), "");
	
	_childs_lock.lock();
	Child_info *info = _childs.first();
	if(info) info = info->find_by_name(label_buf);
	if(!info) {
		info = new(_md_alloc) Child_info(label_buf);
		_childs.insert(info);			
	}
	_childs_lock.unlock();

	/* Create custom RAM session */
	Ram_session *new_session = _create_ram_session(info, readjusted_args);

	info->ram_session = new_session;	
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

}


Ram_root::Ram_root(Genode::Env &env,
				   Genode::Allocator &md_alloc,
				   Genode::Entrypoint &session_ep,
				   Genode::Lock &childs_lock,
				   Genode::List<Child_info> &childs)
	:
	Root_component<Ram_session>(session_ep, md_alloc),
	_env              (env),
	_md_alloc         (md_alloc),
	_ep               (session_ep),
	_childs_lock(childs_lock),
	_childs(childs)
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL;
}


Ram_root::~Ram_root()
{
	Genode::Lock::Guard lock(_childs_lock);
	Child_info *info = _childs.first();
	while(info) {
		Genode::destroy(_md_alloc, info->ram_session);
		info->ram_session = nullptr;
		if(info->child_destroyed()) _childs.remove(info);
		info = info->next();
	}
}

