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
					   Ram_root &ram_root,
					   bool &bootstrap_phase)
	:
	Checkpointable(env, "pd_session"),
	_env (env),
	_md_alloc (md_alloc),
	_ep (ep),
	_bootstrap_phase (bootstrap_phase),
	_parent_pd (env, label),
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
				  _bootstrap_phase),
	info (creation_args,
		  _address_space.info,
		  _stack_area.info,
		  _linker_area.info)
{
	DEBUG_THIS_CALL

	_ep.manage(_address_space);
	_ep.manage(_stack_area);
	_ep.manage(_linker_area);

//	ram_session.mark_region_map_dataspace(_address_space.dataspace());
//	ram_session.mark_region_map_dataspace(_stack_area.dataspace());
//	ram_session.mark_region_map_dataspace(_linker_area.dataspace());	
}


Pd_session::~Pd_session()
{
	_ep.dissolve(_linker_area);
	_ep.dissolve(_stack_area);
	_ep.dissolve(_address_space);

	while(Signal_context *sc = _signal_contexts.first()) {
		_signal_contexts.remove(sc);
		Genode::destroy(_md_alloc, sc);
	}


	while(Signal_source *ss = _signal_sources.first()) {
		_signal_sources.remove(ss);
		Genode::destroy(_md_alloc, ss);
	}

	
	while(Native_capability *nc = _native_caps.first()) {
		_native_caps.remove(nc);
		Genode::destroy(_md_alloc, nc);
	} 
}



void Pd_session::_checkpoint_signal_contexts()
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL	

		Signal_context *sc = nullptr;
	while(sc = _destroyed_signal_contexts.dequeue()) {
		_signal_contexts.remove(sc);
		Genode::destroy(_md_alloc, &sc);
	}

	sc = _signal_contexts.first();
	while(sc) {
		sc->checkpoint();
		sc = sc->next();
	}

	info.signal_contexts = _signal_contexts.first();
}


void Pd_session::_checkpoint_signal_sources()
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL	

		Signal_source *ss = nullptr;
	while(ss = _destroyed_signal_sources.dequeue()) {
		_signal_sources.remove(ss);
		Genode::destroy(_md_alloc, &ss);
	}

	ss = _signal_sources.first();
	while(ss) {
		ss->checkpoint();
		ss = ss->next();
	}

	info.signal_sources = _signal_sources.first();
}


void Pd_session::_checkpoint_native_capabilities()
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL	

		Native_capability *nc = nullptr;
	while(nc = _destroyed_native_caps.dequeue()) {
		_native_caps.remove(nc);
		Genode::destroy(_md_alloc, &nc);
	}

	nc = _native_caps.first();
	while(nc) {
		nc->checkpoint();
		nc = nc->next();
	}

	info.native_caps = _native_caps.first();
}
 

void Pd_session::checkpoint()
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL

	info.badge = cap().local_name();
	info.bootstrapped = _bootstrapped;
	info.upgrade_args = _upgrade_args;

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
	Signal_source *new_ss = new (_md_alloc) Signal_source(result_cap, _bootstrap_phase);
	Genode::Lock::Guard guard(_signal_sources_lock);
	_signal_sources.insert(new_ss);

	return result_cap;
}


void Pd_session::free_signal_source(Genode::Capability<Genode::Signal_source> cap)
{
	/* Find list element */
	Genode::Lock::Guard guard(_signal_sources_lock);
	Signal_source *ss = _signal_sources.first();
	if(ss) ss = ss->find_by_badge(cap.local_name());
	if(ss) {		
		/* Free signal source */
		_parent_pd.free_signal_source(cap);
		_destroyed_signal_sources.enqueue(ss);		
	} else {
		Genode::error("No list element found!");
	}
}


Genode::Signal_context_capability Pd_session::alloc_context(
	Signal_source_capability source, unsigned long imprint)
{
	auto result_cap = _parent_pd.alloc_context(source, imprint);

	/* Create and insert list element to monitor this signal context */
	Signal_context *new_sc = new (_md_alloc) Signal_context(result_cap,
															source,
															imprint,
															_bootstrap_phase);
	Genode::Lock::Guard guard(_signal_contexts_lock);
	_signal_contexts.insert(new_sc);
	return result_cap;
}


void Pd_session::free_context(Genode::Signal_context_capability cap)
{
	/* Find list element */
	Genode::Lock::Guard guard(_signal_contexts_lock);
	Signal_context *sc = _signal_contexts.first();
	if(sc) sc = sc->find_by_badge(cap.local_name());
	if(sc) {
		/* Free signal context */
		_parent_pd.free_context(cap);
		_destroyed_signal_contexts.enqueue(sc);
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
	Native_capability *new_nc = new (_md_alloc) Native_capability(result_cap,
																  ep,
																  _bootstrap_phase);
	Genode::Lock::Guard guard(_native_caps_lock);
	_native_caps.insert(new_nc);
	return result_cap;
}


void Pd_session::free_rpc_cap(Genode::Native_capability cap)
{
	/* Find list element */
	Genode::Lock::Guard guard(_native_caps_lock);
	Native_capability *nc = _native_caps.first();
	if(nc) nc = nc->find_by_native_badge(cap.local_name());
	if(nc) {
		/* Free native capability */
		_parent_pd.free_rpc_cap(cap);
		_destroyed_native_caps.enqueue(nc);		
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
									_ram_root,
									_bootstrap_phase);

	Genode::Lock::Guard lock(_objs_lock);
	_session_rpc_objs.insert(new_session);

	return new_session;
}


void Pd_root::_upgrade_session(Pd_session *session, const char *upgrade_args)
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


void Pd_root::_destroy_session(Pd_session *session)
{
	_session_rpc_objs.remove(session);
	Genode::destroy(_md_alloc, session);
}


Pd_root::Pd_root(Genode::Env &env,
				 Genode::Allocator &md_alloc,
				 Genode::Entrypoint &session_ep,
				 Ram_root &ram_root,		 
				 bool &bootstrap_phase)
	:
	Root_component<Pd_session>(session_ep, md_alloc),
	_env              (env),
	_md_alloc         (md_alloc),
	_ep               (session_ep),
	_bootstrap_phase  (bootstrap_phase),
	_objs_lock        (),
	_session_rpc_objs (),
	_ram_root(ram_root)
{
}


Pd_root::~Pd_root()
{
	while(Pd_session *obj = _session_rpc_objs.first()) {
		_session_rpc_objs.remove(obj);
		Genode::destroy(_md_alloc, obj);
	}
}
