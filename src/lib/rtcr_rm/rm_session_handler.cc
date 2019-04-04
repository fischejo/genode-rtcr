/*
 * \brief  RAM Session Handler
 * \author Johannes Fischer
 * \date   2019-03-21
 */

#include <rtcr_rm/rm_session_handler.h>


using namespace Rtcr;

// This line registers the Cpu_session_handler during application start.
Rtcr::Rm_session_handler_factory _rm_session_handler_factory;


Rm_session_handler::Rm_session_handler(Genode::Env &env,
				       Genode::Allocator &md_alloc,
				       Genode::Entrypoint &ep,
				       const char* label,
				       bool &bootstrap,
				       Pd_root &pd_root)
    :
    _env(env),
    _md_alloc(md_alloc),
    _ep(ep),
    _root(env, md_alloc, ep, bootstrap),
    _pd_root(pd_root),
    _service("RM", _root)
{
#ifdef DEBUG
    Genode::log("Ckpt::\033[33m", __func__, "\033[0m()");
#endif
}  


Rm_session_handler::~Rm_session_handler()
{
}


void Rm_session_handler::prepare_checkpoint(Target_state &state)
{
#ifdef DEBUG
    Genode::log("Ckpt::\033[33m", __func__, "\033[0m()");
#endif  

    /* Create a list of region map dataspaces which are known to child
     * These dataspaces are ignored when creating copy dataspaces
     * For new intercepted sessions which trade managed dataspaces between child and themselves,
     * the region map dataspace capability has to be inserted into this list
     */
    Genode::List<Rm_session_component> *rm_sessions = nullptr;
    if(_root) rm_sessions = &_root->session_infos();

    _region_maps = _create_region_map_dataspaces_list(_pd_root->session_infos(), rm_sessions);

    
#ifdef DEBUG
    Genode::log("Region map dataspaces:");
    Ref_badge_info const *info = _region_maps.first();
    while(info) {
	Genode::log(" ", *info);
	info = info->next();
    }
#endif
}


Genode::List<Ref_badge_info> Rm_session_handler::_create_region_map_dataspaces_list(
    Genode::List<Pd_session_component> &pd_sessions, Genode::List<Rm_session_component> *rm_sessions)
{
#ifdef DEBUG
    Genode::log("Ckpt::\033[33m", __func__, "\033[0m()");
#endif  

    Genode::List<Ref_badge_info> result_list;

    /* Region maps of PD session */
    Pd_session_component *pd_session = pd_sessions.first();
    while(pd_session) {
	Ref_badge_info *new_ref = nullptr;

	/* Address space */
	new_ref = new (_alloc) Ref_badge_info(pd_session->address_space_component().parent_state().ds_cap.local_name());
	result_list.insert(new_ref);

	/* Stack area  */
	new_ref = new (_alloc) Ref_badge_info(pd_session->stack_area_component().parent_state().ds_cap.local_name());
	result_list.insert(new_ref);

	/* Linker area */
	new_ref = new (_alloc) Ref_badge_info(pd_session->linker_area_component().parent_state().ds_cap.local_name());
	result_list.insert(new_ref);

	pd_session = pd_session->next();
    }

    /* Region maps of RM session, if there are any */
    if(rm_sessions) {
	Rm_session_component *rm_session = rm_sessions->first();
	while(rm_session) {
	    Region_map_component *region_map = rm_session->parent_state().region_maps.first();
	    while(region_map) {
		Ref_badge_info *new_ref = new (_alloc) Ref_badge_info(region_map->parent_state().ds_cap.local_name());
		result_list.insert(new_ref);

		region_map = region_map->next();
	    }

	    rm_session = rm_session->next();
	}
    }

    return result_list;
}


void Rm_session_handler::checkpoint(Target_state &state)
{
#ifdef DEBUG
    Genode::log("Ckpt::\033[33m", __func__, "\033[0m()");
#endif

    _prepare_rm_sessions(state, _root->session_infos());
}


void Rm_session_handler::_prepare_rm_sessions(Target_state &state,
					      Genode::List<Rm_session_component> &child_infos)
{
#ifdef DEBUG
    Genode::log("Ckpt::\033[33m", __func__, "\033[0m()");
#endif      
    Genode::List<Stored_rm_session_info> &stored_infos = state._stored_rm_sessions;
    Rm_session_component *child_info = nullptr;
    Stored_rm_session_info *stored_info = nullptr;

    /* Update state_info from child_info
     * If a child_info has no corresponding state_info, create it
     */
    child_info = child_infos.first();
    while(child_info) {
	/* Find corresponding state_info */
	stored_info = stored_infos.first();
	if(stored_info) stored_info = stored_info->find_by_badge(child_info->cap().local_name());

	/* No corresponding stored_info => create it */
	if(!stored_info) {
	    Genode::addr_t childs_kcap = _pd_handler.find_kcap_by_badge(child_info->cap().local_name());
	    stored_info = new (state._alloc) Stored_rm_session_info(*child_info, childs_kcap);
	    stored_infos.insert(stored_info);
	}

	/* Update stored_info */
	_prepare_region_maps(state,
			     stored_info->stored_region_map_infos,
			     child_info->parent_state().region_maps);

	child_info = child_info->next();
    }

    /* Delete old stored_infos, if the child misses corresponding infos in its list */
    stored_info = stored_infos.first();
    while(stored_info) {
	Stored_rm_session_info *next_info = stored_info->next();

	/* Find corresponding child_info */
	child_info = child_infos.first();
	if(child_info) child_info = child_info->find_by_badge(stored_info->badge);

	/* No corresponding child_info => delete it */
	if(!child_info) {
	    stored_infos.remove(stored_info);
	    _destroy_stored_rm_session(state, *stored_info);
	}

	stored_info = next_info;
    }
}


void Rm_session_handler::_destroy_stored_rm_session(Target_state &state, Stored_rm_session_info &stored_info)
{
#ifdef DEBUG
    Genode::log("Ckpt::\033[33m", __func__, "\033[0m()");
#endif      

    while(Stored_region_map_info *info = stored_info.stored_region_map_infos.first()) {
	stored_info.stored_region_map_infos.remove(info);
	_destroy_stored_region_map(*info);
    }
    Genode::destroy(_state._alloc, &stored_info);
}


void Rm_session_handler::_prepare_region_maps(Target_state &state,
					      Genode::List<Stored_region_map_info> &stored_infos,
					      Genode::List<Region_map_component> &child_infos)
{
#ifdef DEBUG
    Genode::log("Ckpt::\033[33m", __func__, "\033[0m()");
#endif      


    Region_map_component *child_info = nullptr;
    Stored_region_map_info *stored_info = nullptr;

    /* Update stored_info from child_info
     * If a child_info has no corresponding stored_info, create it
     */
    child_info = child_infos.first();
    while(child_info) {
	stored_info = stored_infos.first();
	if(stored_info) stored_info = stored_info->find_by_badge(child_info->cap().local_name());

	/* No corresponding stored_info => create it */
	if(!stored_info) {
	    Genode::addr_t childs_kcap = _pd_handler.find_kcap_by_badge(child_info->cap().local_name());
	    stored_info = new (state._alloc) Stored_region_map_info(*child_info, childs_kcap);
	    stored_infos.insert(stored_info);
	}

	/* Update stored_info */
	stored_info->sigh_badge = child_info->parent_state().sigh.local_name();

	_prepare_attached_regions(state,
				  stored_info->stored_attached_region_infos,
				  child_info->parent_state().attached_regions);

	/* Remeber region map's dataspace badge to remove the dataspace from _memory_to_checkpoint later */
	Ref_badge_info *ref_badge = new (_alloc) Ref_badge_info(child_info->parent_state().ds_cap.local_name());
	_region_maps.insert(ref_badge);

	child_info = child_info->next();
    }

    /* Delete old stored_infos, if the child misses corresponding infos in its list */
    stored_info = stored_infos.first();
    while(stored_info) {
	Stored_region_map_info *next_info = stored_info->next();

	/* Find corresponding child_info */
	child_info = child_infos.first();
	if(child_info) child_info = child_info->find_by_badge(stored_info->badge);

	/* No corresponding child_info => delete it */
	if(!child_info) {
	    stored_infos.remove(stored_info);
	    _destroy_stored_region_map(state, *stored_info);
	}

	stored_info = next_info;
    }
}


void Rm_session_handler::_destroy_stored_region_map(Target_state &state, Stored_region_map_info &stored_info)
{
#ifdef DEBUG
    Genode::log("Ckpt::\033[33m", __func__, "\033[0m()");
#endif      


    while(Stored_attached_region_info *info = stored_info.stored_attached_region_infos.first()) {
	stored_info.stored_attached_region_infos.remove(info);
	_destroy_stored_attached_region(*info);
    }
    Genode::destroy(state._alloc, &stored_info);
}


void Rm_session_handler::_prepare_attached_regions(Target_state &state,
						   Genode::List<Stored_attached_region_info> &stored_infos,
						   Genode::List<Attached_region_info> &child_infos)
{
#ifdef DEBUG
    Genode::log("Ckpt::\033[33m", __func__, "\033[0m()");
#endif      

    Attached_region_info *child_info = nullptr;
    Stored_attached_region_info *stored_info = nullptr;

    /* Update stored_info from child_info
     * If a child_info has no corresponding stored_info, create it
     */
    child_info = child_infos.first();
    while(child_info) {
	stored_info = stored_infos.first();
	if(stored_info) stored_info = stored_find->info_by_addr(child_info->rel_addr);

	/* No corresponding stored_info => create it */
	if(!stored_info) {
	    stored_info = &_create_stored_attached_region(state, *child_info);
	    stored_infos.insert(stored_info);
	}

	/* No need to update stored_info */

	/* Remeber this dataspace for checkpoint, if not already in list */
	Dataspace_translation_info *trans_info = _dataspace_translations.first();
	if(trans_info) trans_info = trans_info->find_by_resto_badge(child_info->attached_ds_cap.local_name());
	if(!trans_info) {
	    Ref_badge_info *badge_info = _region_maps.first();
	    if(badge_info) badge_info = badge_info->find_by_badge(child_info->attached_ds_cap.local_name());
	    if(!badge_info) {
		trans_info = new (_alloc) Dataspace_translation_info(stored_info->memory_content,
								     child_info->attached_ds_cap,
								     child_info->size);
		_dataspace_translations.insert(trans_info);
	    }
	}

	child_info = child_info->next();
    }

    /* Delete old stored_infos, if the child misses corresponding infos in its list */
    stored_info = stored_infos.first();
    while(stored_info) {
	Stored_attached_region_info *next_info = stored_info->next();

	/* Find corresponding child_info */
	child_info = child_infos.first();
	if(child_info) child_info = child_info->find_by_addr(stored_info->rel_addr);

	/* No corresponding child_info => delete it */
	if(!child_info) {
	    stored_infos.remove(stored_info);
	    _destroy_stored_attached_region(state, *stored_info);
	}

	stored_info = next_info;
    }
}


Stored_attached_region_info &Rm_session_handler::_create_stored_attached_region(
    Target_state &state, Attached_region_info &child_info)
{
#ifdef DEBUG
    Genode::log("Ckpt::\033[33m", __func__, "\033[0m()");
#endif      

    /* The dataspace with the memory content of the ram dataspace will be
     * referenced by the stored ram dataspace */
    Genode::Ram_dataspace_capability ramds_cap;

    /* Exclude dataspaces which are known region maps (except managed dataspaces
     * from the incremental checkpoint mechanism) */
    Ref_badge_info *region_map_dataspace = _region_maps.first();
    if(region_map_dataspace) region_map_dataspace = region_map_dataspace->find_by_badge(child_info.attached_ds_cap.local_name());
    if(region_map_dataspace) {
#ifdef DEBUG
	Genode::log("Dataspace ", child_info.attached_ds_cap, " is a region map.");
#endif
    } else {
	/* Check whether the dataspace is somewhere in the stored session RPC objects */
	ramds_cap = Genode::reinterpret_cap_cast<Genode::Ram_dataspace>(
	    _find_stored_dataspace(child_info.attached_ds_cap.local_name()));

	if(!ramds_cap.valid()) {
	    ramds_cap = state._env.ram().alloc(child_info.size);
	} 

#ifdef DEBUG
	if(!ramds_cap.valid()) {
	    Genode::log("Dataspace ", child_info.attached_ds_cap, " is not known. "
			"Creating dataspace with size ", Genode::Hex(child_info.size));
	} else {
	    Genode::log("Dataspace ", child_info.attached_ds_cap, " is known from last checkpoint.");
	}
#endif	
    }

    Genode::addr_t childs_kcap = _pd_handler.find_kcap_by_badge(child_info.attached_ds_cap.local_name());
    return *new (state._alloc) Stored_attached_region_info(child_info, childs_kcap, ramds_cap);
}


void Rm_session_handler::_destroy_stored_attached_region(Target_state &state,
							 Stored_attached_region_info &stored_info)
{
#ifdef DEBUG
    Genode::log("Ckpt::\033[33m", __func__, "\033[0m()");
#endif      

    /* Pre-condition: This stored object is removed from its list, thus, a
     * search for a stored dataspace will not return its memory content
     * dataspace */
    Genode::Dataspace_capability stored_ds_cap = _find_stored_dataspace(stored_info.attached_ds_badge);
    if(!stored_ds_cap.valid()) {
	state._env.ram().free(stored_info.memory_content);
    }

    Genode::destroy(state._alloc, &stored_info);
}


void Rm_session_handler::session_restore()
{
    Genode::log("Rm_session_handler::session_restore()");
}


Genode::Local_service &Rm_session_handler::service()
{
    return _service;
}


Rm_root &Rm_session_handler::root()
{
    return _root;
}



