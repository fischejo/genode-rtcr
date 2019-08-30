/*
 * \brief  Intercepting Pd session
 * \author Denis Huber
 * \author Johannes Fischer
 * \date   2019-08-29
 */

#include <rtcr/pd/pd_session.h>

#ifdef PROFILE
#include <util/profiler.h>
#define PROFILE_THIS_CALL PROFILE_FUNCTION("red");
#else
#define PROFILE_THIS_CALL
#endif

#if DEBUG 
#define DEBUG_THIS_CALL Genode::log("\e[38;5;196m", __PRETTY_FUNCTION__, "\033[0m");
#else
#define DEBUG_THIS_CALL
#endif


using namespace Rtcr;

Pd_session::Pd_session(Genode::Env &env,
					   Genode::Allocator &md_alloc,
					   Genode::Entrypoint &ep,
					   const char *label,
					   const char *creation_args,
					   Ram_session &ram_session,
					   bool &bootstrap_phase,
					   Genode::Xml_node *config)
	:
	Checkpointable(env, config, "pd_session"),
	_env (env),
	_md_alloc (md_alloc),
	_ep (ep),
	_bootstrap_phase (bootstrap_phase),
	_parent_pd (env, label),
	ck_creation_args (creation_args),
	_address_space (_md_alloc,
					_parent_pd.address_space(),
					0,
					"address_space",
					_bootstrap_phase),
	_stack_area (_md_alloc,
				 _parent_pd.stack_area(),
				 0,
				 "stack_area",
				 _bootstrap_phase),
	_linker_area (_md_alloc,
				  _parent_pd.linker_area(),
				  0,
				  "linker_area",
				  _bootstrap_phase)
{
	DEBUG_THIS_CALL
		_ep.manage(_address_space);
	_ep.manage(_stack_area);
	_ep.manage(_linker_area);

	ram_session.mark_region_map_dataspace(_address_space.dataspace());
	ram_session.mark_region_map_dataspace(_stack_area.dataspace());
	ram_session.mark_region_map_dataspace(_linker_area.dataspace());	
}


Pd_session::~Pd_session()
{
	_ep.dissolve(_linker_area);
	_ep.dissolve(_stack_area);
	_ep.dissolve(_address_space);
}


void Pd_session::_checkpoint_signal_contexts()
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL	

		Signal_context_info *sc_info = nullptr;
	while(sc_info = _destroyed_signal_contexts.dequeue()) {
		_signal_contexts.remove(sc_info);
		Genode::destroy(_md_alloc, &sc_info);
	}

	sc_info = _signal_contexts.first();
	while(sc_info) {
		sc_info->checkpoint();
		sc_info = sc_info->next();
	}

	ck_signal_contexts = _signal_contexts.first();
}


void Pd_session::_checkpoint_signal_sources()
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL	

		Signal_source_info *ss_info = nullptr;
	while(ss_info = _destroyed_signal_sources.dequeue()) {
		_signal_sources.remove(ss_info);
		Genode::destroy(_md_alloc, &ss_info);
	}

	ss_info = _signal_sources.first();
	while(ss_info) {
		ss_info->checkpoint();
		ss_info = ss_info->next();
	}

	ck_signal_sources = _signal_sources.first();
}


void Pd_session::_checkpoint_native_capabilities()
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL	

		Native_capability_info *nc_info = nullptr;
	while(nc_info = _destroyed_native_caps.dequeue()) {
		_native_caps.remove(nc_info);
		Genode::destroy(_md_alloc, &nc_info);
	}

	nc_info = _native_caps.first();
	while(nc_info) {
		nc_info->checkpoint();
		nc_info = nc_info->next();
	}

	ck_native_caps = _native_caps.first();
}
 

void Pd_session::checkpoint()
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL
		ck_badge = cap().local_name();
	ck_bootstrapped = _bootstrapped;
//  ck_upgrade_args = _upgrade_args.string();

	// TODO
	//  ck_kcap = _core_module->find_kcap_by_badge(ck_badge);
  
	_address_space.checkpoint();
	_stack_area.checkpoint();
	_linker_area.checkpoint();

	_checkpoint_native_capabilities();
	_checkpoint_signal_sources();
	_checkpoint_signal_contexts();
}


Pd_session *Pd_session::find_by_badge(Genode::uint16_t badge)
{
	if(badge == cap().local_name())
		return this;
	Pd_session *obj = next();
	return obj ? obj->find_by_badge(badge) : 0;
}


void Pd_session::assign_parent(Genode::Capability<Genode::Parent> parent)
{
	_parent_pd.assign_parent(parent);
}


bool Pd_session::assign_pci(Genode::addr_t addr, Genode::uint16_t bdf)
{
	return _parent_pd.assign_pci(addr, bdf);
}


Genode::Capability<Genode::Signal_source> Pd_session::alloc_signal_source()
{
	auto result_cap = _parent_pd.alloc_signal_source();

	/* Create and insert list element to monitor this signal source */
	Signal_source_info *new_ss_info = new (_md_alloc) Signal_source_info(result_cap, _bootstrap_phase);
	Genode::Lock::Guard guard(_signal_sources_lock);
	_signal_sources.insert(new_ss_info);

	return result_cap;
}


void Pd_session::free_signal_source(Genode::Capability<Genode::Signal_source> cap)
{
	/* Find list element */
	Genode::Lock::Guard guard(_signal_sources_lock);
	Signal_source_info *ss_info = _signal_sources.first();
	if(ss_info) ss_info = ss_info->find_by_badge(cap.local_name());
	if(ss_info) {		
		/* Free signal source */
		_parent_pd.free_signal_source(cap);
		_destroyed_signal_sources.enqueue(ss_info);		
	} else {
		Genode::error("No list element found!");
	}
}


Genode::Signal_context_capability Pd_session::alloc_context(
	Signal_source_capability source, unsigned long imprint)
{
	auto result_cap = _parent_pd.alloc_context(source, imprint);

	/* Create and insert list element to monitor this signal context */
	Signal_context_info *new_sc_info = new (_md_alloc)
		Signal_context_info(result_cap, source, imprint, _bootstrap_phase);
	Genode::Lock::Guard guard(_signal_contexts_lock);
	_signal_contexts.insert(new_sc_info);
	return result_cap;
}


void Pd_session::free_context(Genode::Signal_context_capability cap)
{
	/* Find list element */
	Genode::Lock::Guard guard(_signal_contexts_lock);
	Signal_context_info *sc_info = _signal_contexts.first();
	if(sc_info) sc_info = sc_info->find_by_badge(cap.local_name());
	if(sc_info) {
		/* Free signal context */
		_parent_pd.free_context(cap);
		_destroyed_signal_contexts.enqueue(sc_info);
	} else {
		Genode::error("No list element found!");
	}
}


void Pd_session::submit(Genode::Signal_context_capability context, unsigned cnt)
{
	_parent_pd.submit(context, cnt);
}


Genode::Native_capability Pd_session::alloc_rpc_cap(Genode::Native_capability ep)
{
	auto result_cap = _parent_pd.alloc_rpc_cap(ep);

	/* Create and insert list element to monitor this native_capability */
	Native_capability_info *new_nc_info = new (_md_alloc) Native_capability_info(result_cap, ep, _bootstrap_phase);
	Genode::Lock::Guard guard(_native_caps_lock);
	_native_caps.insert(new_nc_info);
	return result_cap;
}


void Pd_session::free_rpc_cap(Genode::Native_capability cap)
{
	/* Find list element */
	Genode::Lock::Guard guard(_native_caps_lock);
	Native_capability_info *nc_info = _native_caps.first();
	if(nc_info) nc_info = nc_info->find_by_native_badge(cap.local_name());
	if(nc_info) {
		/* Free native capability */
		_parent_pd.free_rpc_cap(cap);
		_destroyed_native_caps.enqueue(nc_info);		
	} else {
		Genode::error("No list element found!");
	}
}


Genode::Capability<Genode::Region_map> Pd_session::address_space()
{
	return _address_space.Rpc_object<Genode::Region_map>::cap();
}


Genode::Capability<Genode::Region_map> Pd_session::stack_area()
{
	return _stack_area.Rpc_object<Genode::Region_map>::cap();
}


Genode::Capability<Genode::Region_map> Pd_session::linker_area()
{
	return _linker_area.Rpc_object<Genode::Region_map>::cap();
}


Genode::Capability<Genode::Pd_session::Native_pd> Pd_session::native_pd()
{
	return _parent_pd.native_pd();
}


Pd_session *Pd_root::_create_session(const char *args)
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
	readjusted_ram_quota = readjusted_ram_quota + sizeof(Pd_session) + md_alloc()->overhead(sizeof(Pd_session));

	Genode::snprintf(ram_quota_buf, sizeof(ram_quota_buf), "%zu", readjusted_ram_quota);
	Genode::Arg_string::set_arg(readjusted_args, sizeof(readjusted_args), "ram_quota", ram_quota_buf);

	/* Create custom Pd_session */
	Pd_session *new_session =
		new (md_alloc()) Pd_session(_env,
									_md_alloc,
									_ep,
									label_buf,
									readjusted_args,
									_ram_session,
									_bootstrap_phase,
									_config);

	Genode::Lock::Guard lock(_objs_lock);
	_session_rpc_objs.insert(new_session);

	return new_session;
}


void Pd_root::_upgrade_session(Pd_session *session, const char *upgrade_args)
{
	char ram_quota_buf[32];
	char new_upgrade_args[160];

//	Genode::strncpy(new_upgrade_args, session->parent_state().upgrade_args.string(), sizeof(new_upgrade_args));

	Genode::size_t ram_quota = Genode::Arg_string::find_arg(new_upgrade_args, "ram_quota").ulong_value(0);
	Genode::size_t extra_ram_quota = Genode::Arg_string::find_arg(upgrade_args, "ram_quota").ulong_value(0);
	ram_quota += extra_ram_quota;

	Genode::snprintf(ram_quota_buf, sizeof(ram_quota_buf), "%zu", ram_quota);
	Genode::Arg_string::set_arg(new_upgrade_args, sizeof(new_upgrade_args), "ram_quota", ram_quota_buf);

	// TODO FJO:
	//	session->parent_state().upgrade_args = new_upgrade_args;

	_env.parent().upgrade(session->parent_cap(), upgrade_args);
}


void Pd_root::_destroy_session(Pd_session *session)
{
	_session_rpc_objs.remove(session);
	Genode::destroy(_md_alloc, session);
}


Pd_root::Pd_root(Genode::Env &env,
				 Genode::Allocator &md_alloc,
				 Genode::Entrypoint &session_ep,
				 Ram_session &ram_session,		 
				 bool &bootstrap_phase,
				 Genode::Xml_node *config)
	:
	Root_component<Pd_session>(session_ep, md_alloc),
	_env              (env),
	_md_alloc         (md_alloc),
	_ep               (session_ep),
	_bootstrap_phase  (bootstrap_phase),
	_objs_lock        (),
	_session_rpc_objs (),
	_config (config),
	_ram_session(ram_session)
{
}


Pd_root::~Pd_root()
{
	while(Pd_session *obj = _session_rpc_objs.first()) {
		_session_rpc_objs.remove(obj);
		Genode::destroy(_md_alloc, obj);
	}
}
