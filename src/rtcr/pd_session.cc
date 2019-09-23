/*
 * \brief  Intercepting Pd session
 * \author Denis Huber
 * \author Johannes Fischer
 * \date   2019-08-29
 */

#include <rtcr/pd/pd_session.h>

#include <rtcr/cap/capability_mapping.h>

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
                       const char *creation_args,
                       Child_info *child_info)
	:
	Pd_session_info(creation_args, cap().local_name()),
	pd_checkpointable(env, this),
	ram_checkpointable(env, this),
	_env (env),
	_md_alloc (md_alloc),
	_ep (ep),
	_child_info (child_info),
	_parent_pd (env, child_info->name.string()),
	_address_space (_md_alloc,
	                _parent_pd.address_space(),
	                0,
	                "address_space",
	                child_info->bootstrapped,
	                ep),
	_stack_area (_md_alloc,
	             _parent_pd.stack_area(),
	             0,
	             "stack_area",
	             child_info->bootstrapped,
	             ep),
	_linker_area (_md_alloc,
	              _parent_pd.linker_area(),
	              0,
	              "linker_area",
	              child_info->bootstrapped,
	              ep)
{
	DEBUG_THIS_CALL;

	_ep.rpc_ep().manage(this);

	i_address_space = &_address_space;
	i_stack_area = &_stack_area;
	i_linker_area = &_linker_area;

	/* init capability mapping */
	child_info->capability_mapping = new(md_alloc) Capability_mapping(env, md_alloc, this);
}


Pd_session::~Pd_session()
{
	_ep.rpc_ep().dissolve(this);

	while(Signal_context_info *sc = _signal_contexts.first()) {
		_signal_contexts.remove(sc);
		Genode::destroy(_md_alloc, sc);
	}


	while(Signal_source_info *ss = _signal_sources.first()) {
		_signal_sources.remove(ss);
		Genode::destroy(_md_alloc, ss);
	}


	while(Native_capability_info *nc = _native_caps.first()) {
		_native_caps.remove(nc);
		Genode::destroy(_md_alloc, nc);
	}

	while(Ram_dataspace_info *ds = _ram_dataspaces.first()) {
		_ram_dataspaces.remove(ds);
		Genode::destroy(_md_alloc, ds);
	}	
}



void Pd_session::_checkpoint_signal_contexts()
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL;

	Signal_context_info *sc = nullptr;
	while(sc = _destroyed_signal_contexts.dequeue()) {
		_signal_contexts.remove(sc);
		Genode::destroy(_md_alloc, &sc);
	}

	sc = _signal_contexts.first();
	while(sc) {
		static_cast<Signal_context*>(sc)->checkpoint();
		sc = sc->next();
	}

	i_signal_contexts = _signal_contexts.first();
}


void Pd_session::_checkpoint_signal_sources()
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL;

	Signal_source_info *ss = nullptr;
	while(ss = _destroyed_signal_sources.dequeue()) {
		_signal_sources.remove(ss);
		Genode::destroy(_md_alloc, &ss);
	}

	/* Signal_source only stores const values. No need for checkpoint() */

	i_signal_sources = _signal_sources.first();
}


void Pd_session::_checkpoint_native_capabilities()
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL;

	Native_capability_info *nc = nullptr;
	while(nc = _destroyed_native_caps.dequeue()) {
		_native_caps.remove(nc);
		Genode::destroy(_md_alloc, &nc);
	}

	/* Native_capability only stores const values. No need for
	   checkpoint() */

	i_native_caps = _native_caps.first();
}


void Pd_session::_checkpoint_ram_dataspaces()
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL
		i_upgrade_args = _upgrade_args;

	/* step 1: remove all destroyed dataspaces */
	Ram_dataspace_info *dataspace = nullptr;
	while(dataspace = _destroyed_ram_dataspaces.dequeue()) {
		_ram_dataspaces.remove(dataspace);
		_destroy_dataspace(static_cast<Ram_dataspace*>(dataspace));
	}

	/* step 2: allocate cold dataspace for recently added dataspaces */
	dataspace = _ram_dataspaces.first();
	while(dataspace && dataspace != i_ram_dataspaces) {
		_alloc_dataspace(static_cast<Ram_dataspace*>(dataspace));
		_attach_dataspace(static_cast<Ram_dataspace*>(dataspace));
		dataspace = dataspace->next();
	}

	/* step 3: copy memory of hot ds to cold ds */
	dataspace = _ram_dataspaces.first();
	while(dataspace) {
		static_cast<Ram_dataspace*>(dataspace)->checkpoint();
		_copy_dataspace(static_cast<Ram_dataspace*>(dataspace));

		dataspace = dataspace->next();
	}

	/* step 4: move pointer forward to update ck_ram_dataspaces */
	i_ram_dataspaces = _ram_dataspaces.first();
}


void Pd_session::Pd_checkpointable::checkpoint()
{
	_pd->i_upgrade_args = _pd->_upgrade_args;
	
	_pd->_checkpoint_native_capabilities();
	_pd->_checkpoint_signal_sources();
	_pd->_checkpoint_signal_contexts();

	_pd->_address_space.checkpoint();
	_pd->_stack_area.checkpoint();
	_pd->_linker_area.checkpoint();
}


void Pd_session::Ram_checkpointable::checkpoint()
{
	_pd->_checkpoint_ram_dataspaces();	
}


void Pd_session::_destroy_dataspace(Ram_dataspace *ds)
{
	/* detach */
	_env.rm().detach(ds->dst);
	_env.rm().detach(ds->src);

	/* free */
	_parent_pd.free(ds->i_src_cap);
	_env.ram().free(ds->i_dst_cap);

	/* Destroy Ram_dataspace */
	Genode::destroy(_md_alloc, ds);
}


void Pd_session::_copy_dataspace(Ram_dataspace *ds)
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL;

	Genode::memcpy(ds->dst, ds->src, ds->i_size);
}


void Pd_session::_alloc_dataspace(Ram_dataspace *ds)
{
	ds->i_dst_cap = _env.ram().alloc(ds->i_size);
}

void Pd_session::_attach_dataspace(Ram_dataspace *ds)
{
	ds->dst = _env.rm().attach(ds->i_dst_cap);
	ds->src = _env.rm().attach(ds->i_src_cap);
}




void Pd_session::assign_parent(Genode::Capability<Genode::Parent> parent)
{
	DEBUG_THIS_CALL;
	_parent_pd.assign_parent(parent);
}


bool Pd_session::assign_pci(Genode::addr_t addr, Genode::uint16_t bdf)
{
	DEBUG_THIS_CALL;
	return _parent_pd.assign_pci(addr, bdf);
}


Genode::Capability<Genode::Signal_source> Pd_session::alloc_signal_source()
{
	DEBUG_THIS_CALL;
	auto result_cap = _parent_pd.alloc_signal_source();

	/* Create and insert list element to monitor this signal source */
	Signal_source *new_ss = new (_md_alloc) Signal_source(result_cap,
	                                                      _child_info->bootstrapped);

	Genode::Lock::Guard guard(_signal_sources_lock);
	_signal_sources.insert(new_ss);

	return result_cap;
}


void Pd_session::free_signal_source(Genode::Capability<Genode::Signal_source> cap)
{
	DEBUG_THIS_CALL;
	/* Find list element */
	Genode::Lock::Guard guard(_signal_sources_lock);
	Signal_source_info *ss = _signal_sources.first();
	if(ss) ss = ss->find_by_badge(cap.local_name());
	if(ss) {
		/* Free signal source */
		_parent_pd.free_signal_source(cap);
		_destroyed_signal_sources.enqueue(ss);
	} else {
		Genode::error("No list element found!");
	}
}


Genode::Signal_context_capability Pd_session::alloc_context(Signal_source_capability source,
                                                            unsigned long imprint)
{
	DEBUG_THIS_CALL;
	auto result_cap = _parent_pd.alloc_context(source, imprint);

	/* Create and insert list element to monitor this signal context */
	Signal_context *new_sc = new (_md_alloc) Signal_context(result_cap,
	                                                        source,
	                                                        imprint,
	                                                        _child_info->bootstrapped);

	Genode::Lock::Guard guard(_signal_contexts_lock);
	_signal_contexts.insert(new_sc);
	return result_cap;
}


void Pd_session::free_context(Genode::Signal_context_capability cap)
{
	/* Find list element */
	Genode::Lock::Guard guard(_signal_contexts_lock);
	Signal_context_info *sc = _signal_contexts.first();
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
	Native_capability *new_nc = new (_md_alloc) Native_capability(result_cap, ep,
	                                                              _child_info->bootstrapped);

	Genode::Lock::Guard guard(_native_caps_lock);
	_native_caps.insert(new_nc);
	return result_cap;
}


void Pd_session::free_rpc_cap(Genode::Native_capability cap)
{
	/* Find list element */
	Genode::Lock::Guard guard(_native_caps_lock);
	Native_capability_info *nc = _native_caps.first();
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


void Pd_session::map(Genode::addr_t _addr, Genode::addr_t __addr)
{
	_parent_pd.map(_addr, __addr);
}


void Pd_session::ref_account(Genode::Capability<Genode::Pd_session> cap)
{
	_ref_account_cap = cap;	
	return _parent_pd.ref_account(cap);
}


void Pd_session::transfer_quota(Genode::Capability<Genode::Pd_session> cap,
                                Genode::Cap_quota quota)
{
	_parent_pd.transfer_quota(cap, quota);
}


void Pd_session::transfer_quota(Genode::Capability<Genode::Pd_session> cap,
                                Genode::Ram_quota quota)
{
	_parent_pd.transfer_quota(cap, quota);
}


Genode::Cap_quota Pd_session::cap_quota() const
{
	return _parent_pd.cap_quota();
}


Genode::Cap_quota Pd_session::used_caps() const
{
	return _parent_pd.used_caps();
}


Genode::Ram_quota Pd_session::ram_quota() const
{
	return _parent_pd.ram_quota();
}


Genode::Ram_quota Pd_session::used_ram() const
{
	return _parent_pd.used_ram();
}


Genode::Ram_dataspace_capability Pd_session::alloc(Genode::size_t size,
                                                   Genode::Cache_attribute cached)
{
	DEBUG_THIS_CALL;

	Genode::Ram_dataspace_capability src_cap = _parent_pd.alloc(size, cached);

	/* Create a Ram_dataspace to monitor the newly created Ram_dataspace */
	Ram_dataspace *ds = new (_md_alloc) Ram_dataspace(src_cap,
	                                                  size,
	                                                  cached,
	                                                  _child_info->bootstrapped);
	Genode::Lock::Guard guard(_ram_dataspaces_lock);
	_ram_dataspaces.insert(ds);

	return src_cap;
}


void Pd_session::free(Genode::Ram_dataspace_capability ds_cap)
{
	DEBUG_THIS_CALL;	
	/* Find the Ram_dataspace which monitors the given Ram_dataspace */
	Ram_dataspace_info *rds = _ram_dataspaces.first();
	if(rds) rds = rds->find_by_badge(ds_cap.local_name());
	if(rds) {
		Genode::Lock::Guard lock_guard(_destroyed_ram_dataspaces_lock);
		_destroyed_ram_dataspaces.enqueue(rds);
	} else {
		Genode::warning(__func__, " Ram_dataspace not found for ", ds_cap);
		return;
	}	
}


Genode::size_t Pd_session::dataspace_size(Genode::Ram_dataspace_capability cap) const
{
	return _parent_pd.dataspace_size(cap);
}


void Pd_session::upgrade(const char *upgrade_args)
{
	/* instead of upgrading the intercepting session, the
	   intercepted session is upgraded */
	_env.parent().upgrade(Genode::Parent::Env::pd(), upgrade_args);
	_upgrade_args = upgrade_args;
}
