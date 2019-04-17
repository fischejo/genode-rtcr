/*
  * \brief  CPU Session Handler
  * \author Johannes Fischer
  * \date   2019-03-21
  */

#include <rtcr_core/core_module_pd.h>
#include <base/internal/cap_map.h>
#include <base/internal/cap_alloc.h>

using namespace Rtcr;


Core_module_pd::Core_module_pd(Genode::Env &env,
			       Genode::Allocator &md_alloc,
			       Genode::Entrypoint &ep)
  :
    _env(env),
    _md_alloc(md_alloc),
    _ep(ep)
{
}

void Core_module_pd::_init(const char* label, bool &bootstrap)
{
#ifdef DEBUG
    Genode::log("\033[36m", __PRETTY_FUNCTION__, "\033[0m");
#endif
    _pd_root = new (_md_alloc) Pd_root(_env, _md_alloc, _ep, bootstrap);
    _pd_service = new (_md_alloc) Genode::Local_service("PD", _pd_root);
    _pd_session = _find_session(label, pd_root());
}

Pd_session_component *Core_module_pd::_find_session(const char *label, Pd_root &pd_root)
{
#ifdef DEBUG
    Genode::log("\033[36m", __PRETTY_FUNCTION__, "\033[0m");
#endif
  
    /* Preparing argument string */
    char args_buf[160];
    Genode::snprintf(args_buf, sizeof(args_buf), "ram_quota=%u, label=\"%s\"", 20*1024*sizeof(long), label);

    /* Issuing session method of pd_root */
    Genode::Session_capability pd_cap = pd_root.session(args_buf, Genode::Affinity());

    /* Find created RPC object in pd_root's list */
    Pd_session_component *pd_session = pd_root.session_infos().first();
    if(pd_session) pd_session = pd_session->find_by_badge(pd_cap.local_name());
    if(!pd_session) {
	Genode::error("Creating custom PD session failed: Could not find PD session in PD root");
	throw Genode::Exception();
    }

    return pd_session;
}


Core_module_pd::~Core_module_pd()
{
    // this should be cleaned right after a checkpoint.
    _destroy_list(_kcap_mappings);
    Genode::destroy(_md_alloc, _pd_root);
    Genode::destroy(_md_alloc, _pd_service);    
}  


Pd_root &Core_module_pd::pd_root() {
  return *_pd_root;
};
  



void Core_module_pd::_create_kcap_mappings(Target_state &state)
{
#ifdef DEBUG
    Genode::log("\033[36m", __PRETTY_FUNCTION__, "\033[0m");
#endif
  
    using Genode::log;
    using Genode::Hex;
    using Genode::addr_t;
    using Genode::size_t;
    using Genode::uint16_t;
	
    Genode::List<Kcap_badge_info> result;

    /* Retrieve cap_idx_alloc_addr */
    Genode::Pd_session_client pd_client(pd_session().parent_cap());
    addr_t const cap_idx_alloc_addr = Genode::Foc_native_pd_client(
	pd_client.native_pd()).cap_map_info();
    state._cap_idx_alloc_addr = cap_idx_alloc_addr;

#ifdef DEBUG
    Genode::log("Address of cap_idx_alloc = ", Hex(cap_idx_alloc_addr));
#endif
	
    /* Find child's dataspace containing the capability map
     * It is found via cap_idx_alloc_addr */
    Attached_region_info *ar_info = pd_session()
      .address_space_component()
      .parent_state()
      .attached_regions
      .first();
    
    if(ar_info) ar_info = ar_info->find_by_addr(cap_idx_alloc_addr);
    if(!ar_info) {
	Genode::error("No dataspace found for cap_idx_alloc's datastructure at ",
		      Hex(cap_idx_alloc_addr));
	throw Genode::Exception();
    }

    /* If ar_info is a managed Ram_dataspace_info, mark detached
     * Designated_dataspace_infos and attach them, thus, the Checkpointer does
     * not trigger page faults which mark accessed regions */
    Genode::List<Ref_badge_info> marked_badge_infos =
	_mark_and_attach_designated_dataspaces(*ar_info);

    /* Create new badge_kcap list */
    size_t const struct_size    = sizeof(Genode::Cap_index_allocator_tpl<Genode::Cap_index,4096>);
    size_t const array_ele_size = sizeof(Genode::Cap_index);
    size_t const array_size     = array_ele_size*4096;

    addr_t const child_ds_start     = ar_info->rel_addr;
    addr_t const child_ds_end       = child_ds_start + ar_info->size;
    addr_t const child_struct_start = cap_idx_alloc_addr;
    addr_t const child_struct_end   = child_struct_start + struct_size;
    addr_t const child_array_start  = child_struct_start + 8;
    addr_t const child_array_end    = child_array_start + array_size;

    addr_t const local_ds_start     = state._env.rm().attach(ar_info->attached_ds_cap, ar_info->size, ar_info->offset);
    addr_t const local_ds_end       = local_ds_start + ar_info->size;
    addr_t const local_struct_start = local_ds_start + (cap_idx_alloc_addr - child_ds_start);
    addr_t const local_struct_end   = local_struct_start + struct_size;
    addr_t const local_array_start  = local_struct_start + 8;
    addr_t const local_array_end    = local_array_start + array_size;

#ifdef DEBUG
    log("child_ds_start:     ", Hex(child_ds_start));
    log("child_struct_start: ", Hex(child_struct_start));
    log("child_array_start:  ", Hex(child_array_start));
    log("child_array_end:    ", Hex(child_array_end));
    log("child_struct_end:   ", Hex(child_struct_end));
    log("child_ds_end:       ", Hex(child_ds_end));

    log("local_ds_start:     ", Hex(local_ds_start));
    log("local_struct_start: ", Hex(local_struct_start));
    log("local_array_start:  ", Hex(local_array_start));
    log("local_array_end:    ", Hex(local_array_end));
    log("local_struct_end:   ", Hex(local_struct_end));
    log("local_ds_end:       ", Hex(local_ds_end));
#endif 

    //dump_mem((void*)local_array_start, 0x1200);

    enum { UNUSED = 0, INVALID_ID = 0xffff };
    for(addr_t curr = local_array_start; curr < local_array_end; curr += array_ele_size) {

	size_t const badge_offset = 6;

	/* Convert address to pointer and dereference it */
	uint16_t const badge = *(uint16_t*)(curr + badge_offset);
	/* Current capability map slot = Compute distance from start to current
	 * address of array and divide it by the element size; kcap = current
	 * capability map slot shifted by 12 bits to the left (last 12 bits are
	 * used by Fiasco.OC for parameters for IPC calls) */
	addr_t const kcap  = ((curr - local_array_start) / array_ele_size) << 12;

	if(badge != UNUSED && badge != INVALID_ID) {
	    Kcap_badge_info *state_info = new (_md_alloc) Kcap_badge_info(kcap, badge);
	    result.insert(state_info);

#ifdef DEBUG
	    log("+ ", Hex(kcap), ": ", badge, " (", Hex(badge), ")");
#endif
	}
    }

    state._env.rm().detach(local_ds_start);

    /* Detach the previously attached Designated_dataspace_infos and delete the
     * list containing marked Designated_dataspace_infos */
    _detach_and_unmark_designated_dataspaces(marked_badge_infos, *ar_info);

#ifdef DEBUG
    Genode::log("Capability map:");
    Kcap_badge_info const *info = result.first();
    if(!info) Genode::log(" <empty>\n");
    while(info)
	{
	    Genode::log(" ", *info);
	    info = info->next();
	}
#endif

    // FJO: not sure if this might work...
    _kcap_mappings = result;
}



Genode::List<Ref_badge_info> Core_module_pd::_mark_and_attach_designated_dataspaces(Attached_region_info &ar_info)
{
#ifdef DEBUG
    Genode::log("\033[36m", __PRETTY_FUNCTION__, "\033[0m");
#endif
    
    Genode::List<Ref_badge_info> result_infos;

    Managed_region_map_info *mrm_info = ar_info.managed_dataspace(ram_session().parent_state().ram_dataspaces);
    if(mrm_info) {
	Designated_dataspace_info *dd_info = mrm_info->dd_infos.first();
	while(dd_info) {
	    if(!dd_info->attached) {
		dd_info->attach();

		Ref_badge_info *new_info = new (_md_alloc) Ref_badge_info(dd_info->cap.local_name());
		result_infos.insert(new_info);
	    }

	    dd_info = dd_info->next();
	}
    }

    return result_infos;
}


void Core_module_pd::_detach_and_unmark_designated_dataspaces(Genode::List<Ref_badge_info> &badge_infos,
							      Attached_region_info &ar_info)
{
#ifdef DEBUG
    Genode::log("\033[36m", __PRETTY_FUNCTION__, "\033[0m");
#endif
  
    Managed_region_map_info *mrm_info = ar_info.managed_dataspace(ram_session().parent_state().ram_dataspaces);
    if(mrm_info && badge_infos.first()) {
	Designated_dataspace_info *dd_info = mrm_info->dd_infos.first();
	while(dd_info) {
	    if(badge_infos.first()->find_by_badge(dd_info->cap.local_name())) {
		dd_info->detach();
	    }

	    dd_info = dd_info->next();
	}
    }

    /* Delete list elements from badge_infos */
    _destroy_list(badge_infos);
}



void Core_module_pd::_checkpoint(Target_state &state)
{
#ifdef DEBUG
    Genode::log("\033[36m", __PRETTY_FUNCTION__, "\033[0m");
#endif
  
    Genode::List<Stored_pd_session_info> &stored_infos = state._stored_pd_sessions;
    Genode::List<Pd_session_component> &child_infos = pd_root().session_infos();

    Pd_session_component *child_info = nullptr;
    Stored_pd_session_info *stored_info = nullptr;
    
    /* Update state_info from child_info If a child_info has no corresponding
     * state_info, create it */
    child_info = child_infos.first();
    while(child_info) {
	/* Find corresponding state_info */
	stored_info = stored_infos.first();
	if(stored_info) stored_info = stored_info->find_by_badge(child_info->cap().local_name());

	/* No corresponding stored_info => create it */
	if(!stored_info) {
	    Genode::addr_t childs_pd_kcap  = find_kcap_by_badge(child_info->cap().local_name());
	    Genode::addr_t childs_add_kcap = find_kcap_by_badge(child_info->address_space_component().cap().local_name());
	    Genode::addr_t childs_sta_kcap = find_kcap_by_badge(child_info->stack_area_component().cap().local_name());
	    Genode::addr_t childs_lin_kcap = find_kcap_by_badge(child_info->linker_area_component().cap().local_name());
	    stored_info = new (state._alloc) Stored_pd_session_info(*child_info,
								    childs_pd_kcap,
								    childs_add_kcap,
								    childs_sta_kcap,
								    childs_lin_kcap);
	    stored_infos.insert(stored_info);
	}

	/* Wrap Region_maps of child's and checkpointer's PD session in lists
	 * for reusing _prepare_region_maps The linked list pointers of the
	 * three regions maps are usually not used gloablly */
	Genode::List<Stored_region_map_info> temp_stored;
	temp_stored.insert(&stored_info->stored_linker_area);
	temp_stored.insert(&stored_info->stored_stack_area);
	temp_stored.insert(&stored_info->stored_address_space);
	Genode::List<Region_map_component> temp_child;
	temp_child.insert(&child_info->linker_area_component());
	temp_child.insert(&child_info->stack_area_component());
	temp_child.insert(&child_info->address_space_component());
	/* Update stored_info */

	_prepare_native_caps(state,
			     stored_info->stored_native_cap_infos,
			     child_info->parent_state().native_caps);
	
	_prepare_signal_sources(state,
				stored_info->stored_source_infos,
				child_info->parent_state().signal_sources);

	_prepare_signal_contexts(state,
				 stored_info->stored_context_infos,
				 child_info->parent_state().signal_contexts);

	// reuse the function from core_module_rm...
	_prepare_region_maps(state, temp_stored, temp_child);

	child_info = child_info->next();
    }

    /* Delete old stored_infos, if the child misses corresponding infos in its list */
    stored_info = stored_infos.first();
    while(stored_info) {
	Stored_pd_session_info *next_info = stored_info->next();

	/* Find corresponding child_info */
	child_info = child_infos.first();
	if(child_info) child_info = child_info->find_by_badge(stored_info->badge);

	/* No corresponding child_info => delete it */
	if(!child_info) {
	    stored_infos.remove(stored_info);
	    _destroy_stored_pd_session(state, *stored_info);
	}

	stored_info = next_info;
    }
}


void Core_module_pd::_destroy_stored_pd_session(Target_state &state,
						Stored_pd_session_info &stored_info)
{
#ifdef DEBUG
    Genode::log("\033[36m", __PRETTY_FUNCTION__, "\033[0m");
#endif

    while(Stored_signal_context_info *info = stored_info.stored_context_infos.first()) {
	stored_info.stored_context_infos.remove(info);
	_destroy_stored_signal_context(state, *info);
    }

    while(Stored_signal_source_info *info = stored_info.stored_source_infos.first()) {
	stored_info.stored_source_infos.remove(info);
	_destroy_stored_signal_source(state, *info);
    }

    while(Stored_native_capability_info *info = stored_info.stored_native_cap_infos.first()) {
	stored_info.stored_native_cap_infos.remove(info);
	_destroy_stored_native_cap(state, *info);
    }

    _destroy_stored_region_map(state, stored_info.stored_linker_area);
    _destroy_stored_region_map(state, stored_info.stored_stack_area);
    _destroy_stored_region_map(state, stored_info.stored_address_space);

    Genode::destroy(state._alloc, &stored_info);
}


void Core_module_pd::_destroy_stored_region_map(Target_state &state, Stored_region_map_info &stored_info)
{
#ifdef DEBUG
    Genode::log("\033[36m", __PRETTY_FUNCTION__, "\033[0m");
#endif

    while(Stored_attached_region_info *info = stored_info.stored_attached_region_infos.first()) {
	stored_info.stored_attached_region_infos.remove(info);
	_destroy_stored_attached_region(state, *info);
    }
    Genode::destroy(state._alloc, &stored_info);
}


void Core_module_pd::_destroy_stored_attached_region(Target_state &state,
						     Stored_attached_region_info &stored_info)
{
#ifdef DEBUG
    Genode::log("\033[36m", __PRETTY_FUNCTION__, "\033[0m");
#endif
    /* Pre-condition: This stored object is removed from its list, thus, a
     * search for a stored dataspace will not return its memory content
     * dataspace */
    Genode::Dataspace_capability stored_ds_cap = state.find_stored_dataspace(
	stored_info.attached_ds_badge);
    
    if(!stored_ds_cap.valid()) {
	state._env.ram().free(stored_info.memory_content);
    }

    Genode::destroy(state._alloc, &stored_info);
}



void Core_module_pd::_prepare_native_caps(Target_state &state,
					  Genode::List<Stored_native_capability_info> &stored_infos,
					  Genode::List<Native_capability_info> &child_infos)
{
#ifdef DEBUG
    Genode::log("\033[36m", __PRETTY_FUNCTION__, "\033[0m");
#endif
    
    Native_capability_info *child_info = nullptr;
    Stored_native_capability_info *stored_info = nullptr;

    /* Update state_info from child_info If a child_info has no corresponding
     * state_info, create it */
    child_info = child_infos.first();
    while(child_info) {
	/* Find corresponding state_info */
	stored_info = stored_infos.first();
	if(stored_info) stored_info = stored_info->find_by_badge(child_info->cap.local_name());

	/* No corresponding stored_info => create it */
	if(!stored_info) {
	    Genode::addr_t childs_kcap = find_kcap_by_badge(child_info->cap.local_name());
	    stored_info = new (state._alloc) Stored_native_capability_info(*child_info, childs_kcap);
	    stored_infos.insert(stored_info);
	}

	/* Nothing to update in stored_info */

	child_info = child_info->next();
    }

    /* Delete old stored_infos, if the child misses corresponding infos in its list */
    stored_info = stored_infos.first();
    while(stored_info) {
	Stored_native_capability_info *next_info = stored_info->next();

	/* Find corresponding child_info */
	child_info = child_infos.first();
	if(child_info) child_info = child_info->find_by_native_badge(stored_info->badge);

	/* No corresponding child_info => delete it */
	if(!child_info) {
	    stored_infos.remove(stored_info);
	    _destroy_stored_native_cap(state, *stored_info);
	}

	stored_info = next_info;
    }
}


void Core_module_pd::_destroy_stored_native_cap(Target_state &state, Stored_native_capability_info &stored_info)
{
#ifdef DEBUG
    Genode::log("\033[36m", __PRETTY_FUNCTION__, "\033[0m");
#endif

    Genode::destroy(state._alloc, &stored_info);
}


void Core_module_pd::_prepare_signal_sources(Target_state &state,
					     Genode::List<Stored_signal_source_info> &stored_infos,
					     Genode::List<Signal_source_info> &child_infos)
{
#ifdef DEBUG
    Genode::log("\033[36m", __PRETTY_FUNCTION__, "\033[0m");
#endif

    Signal_source_info *child_info = nullptr;
    Stored_signal_source_info *stored_info = nullptr;

    /* Update state_info from child_info If a child_info has no corresponding
     * state_info, create it */
    child_info = child_infos.first();
    while(child_info) {
	/* Find corresponding state_info */
	stored_info = stored_infos.first();
	if(stored_info) stored_info = stored_info->find_by_badge(child_info->cap.local_name());

	/* No corresponding stored_info => create it */
	if(!stored_info) {
	    Genode::addr_t childs_kcap = find_kcap_by_badge(child_info->cap.local_name());
	    stored_info = new (state._alloc) Stored_signal_source_info(*child_info, childs_kcap);
	    stored_infos.insert(stored_info);
	}

	/* Nothing to update in stored_info */

	child_info = child_info->next();
    }

    /* Delete old stored_infos, if the child misses corresponding infos in its list */
    stored_info = stored_infos.first();
    while(stored_info) {
	Stored_signal_source_info *next_info = stored_info->next();

	/* Find corresponding child_info */
	child_info = child_infos.first();
	if(child_info) child_info = child_info->find_by_badge(stored_info->badge);

	/* No corresponding child_info => delete it */
	if(!child_info) {
	    stored_infos.remove(stored_info);
	    _destroy_stored_signal_source(state, *stored_info);
	}

	stored_info = next_info;
    }
}


void Core_module_pd::_destroy_stored_signal_source(Target_state &state,
						   Stored_signal_source_info &stored_info)
{
#ifdef DEBUG
    Genode::log("\033[36m", __PRETTY_FUNCTION__, "\033[0m");
#endif
    Genode::destroy(state._alloc, &stored_info);
}


void Core_module_pd::_prepare_signal_contexts(Target_state &state,
					      Genode::List<Stored_signal_context_info> &stored_infos,
					      Genode::List<Signal_context_info> &child_infos)
{
#ifdef DEBUG
    Genode::log("\033[36m", __PRETTY_FUNCTION__, "\033[0m");
#endif
    Signal_context_info *child_info = nullptr;
    Stored_signal_context_info *stored_info = nullptr;

    /* Update state_info from child_info. If a child_info has no corresponding
     * state_info, create it */
    child_info = child_infos.first();
    while(child_info) {
	/* Find corresponding state_info */
	stored_info = stored_infos.first();
	if(stored_info) stored_info = stored_info->find_by_badge(child_info->cap.local_name());

	/* No corresponding stored_info => create it */
	if(!stored_info) {
	    Genode::addr_t childs_kcap = find_kcap_by_badge(child_info->cap.local_name());
	    stored_info = new (state._alloc) Stored_signal_context_info(*child_info, childs_kcap);
	    stored_infos.insert(stored_info);
	}

	/* Nothing to update in stored_info */

	child_info = child_info->next();
    }

    /* Delete old stored_infos, if the child misses corresponding infos in its
     * list */
    stored_info = stored_infos.first();
    while(stored_info) {
	Stored_signal_context_info *next_info = stored_info->next();

	/* Find corresponding child_info */
	child_info = child_infos.first();
	if(child_info) child_info = child_info->find_by_badge(stored_info->badge);

	/* No corresponding child_info => delete it */
	if(!child_info) {
	    stored_infos.remove(stored_info);
	    _destroy_stored_signal_context(state, *stored_info);
	}

	stored_info = next_info;
    }
}


void Core_module_pd::_destroy_stored_signal_context(Target_state &state,
						    Stored_signal_context_info &stored_info)
{  
    Genode::destroy(state._alloc, &stored_info);
}


Genode::addr_t Core_module_pd::find_kcap_by_badge(Genode::uint16_t badge)
{
    Genode::addr_t kcap = 0;

    Kcap_badge_info *info = _kcap_mappings.first();
    if(info) info = info->find_by_badge(badge);
    if(info) kcap = info->kcap;

    return kcap;
}


void Core_module_pd::_destroy_list(Genode::List<Kcap_badge_info> &list)
{
    while(Kcap_badge_info *elem = list.first()) {
	list.remove(elem);
	Genode::destroy(_md_alloc, elem);
    }
}

void Core_module_pd::_destroy_list(Genode::List<Ref_badge_info> &list)
{
  while(Ref_badge_info *elem = list.first())
    {
      list.remove(elem);
      Genode::destroy(_md_alloc, elem);
    }
}
