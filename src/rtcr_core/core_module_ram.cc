/*
 * \brief  Ram implementation of core module
 * \author Johannes Fischer
 * \date   2019-04-04
 */

#include <rtcr_core/core_module_ram.h>

using namespace Rtcr;


Core_module_ram::Core_module_ram(Genode::Env &env,
				 Genode::Allocator &alloc,
				 Genode::Entrypoint &ep)
	:
	_env(env),
	_alloc(alloc),
	_ep(ep)
{}  


void Core_module_ram::_initialize_ram_session(const char* label, bool &bootstrap)
{
	_ram_root = new (_alloc) Ram_root(_env, _alloc, _ep, bootstrap);
	_ram_service = new (_alloc) Genode::Local_service("RAM", _ram_root);
	_ram_session = _find_ram_session(label, ram_root());

	// Donate ram quota to child
	// TODO Replace static quota donation with the amount of quota, the child needs
	Genode::size_t donate_quota = 512*1024;
	_ram_session->ref_account(_env.ram_session_cap());
	// Note: transfer goes directly to parent's ram session
	_env.ram().transfer_quota(_ram_session->parent_cap(), donate_quota);
}


Ram_session_component *Core_module_ram::_find_ram_session(const char *label, Ram_root &ram_root)
{ 
#ifdef DEBUG
	Genode::log("\033[36m", __PRETTY_FUNCTION__, "\033[0m");
#endif
   
	/* Preparing argument string */
	char args_buf[160];
	Genode::snprintf(args_buf, sizeof(args_buf),
			 "ram_quota=%u, phys_start=0x%lx, phys_size=0x%lx, label=\"%s\"",
			 4*1024*sizeof(long), 0UL, 0UL, label);

	/* Issuing session method of Ram_root */
	Genode::Session_capability ram_cap = ram_root.session(args_buf, Genode::Affinity());

	/* Find created RPC object in Ram_root's list */
	Ram_session_component *ram_session = ram_root.session_infos().first();
	if(ram_session) ram_session = ram_session->find_by_badge(ram_cap.local_name());
	if(!ram_session) {
		Genode::error("Creating custom RAM session failed: Could not find RAM session in RAM root");
		throw Genode::Exception();
	}

	return ram_session;
}


Core_module_ram::~Core_module_ram()
{

}


void Core_module_ram::_checkpoint()
{ 
#ifdef DEBUG
	Genode::log("\033[36m", __PRETTY_FUNCTION__, "\033[0m");
#endif
	Genode::List<Ram_session_component> &child_infos =  _ram_root->session_infos();
	Genode::List<Stored_ram_session_info> &stored_infos = state()._stored_ram_sessions;    
	Ram_session_component *child_info = nullptr;
	Stored_ram_session_info *stored_info = nullptr;
	/* Update state_info from child_info */
	/* If a child_info has no corresponding state_info, create it */
	child_info = child_infos.first();
	while(child_info) {
		/* Find corresponding state_info */
		stored_info = stored_infos.first();
		if(stored_info) stored_info = stored_info->find_by_badge(child_info->cap().local_name());

		/* No corresponding stored_info => create it */
		if(!stored_info) {
			Genode::addr_t childs_kcap = find_kcap_by_badge(child_info->cap().local_name());
			stored_info = new (_alloc) Stored_ram_session_info(*child_info, childs_kcap);
			stored_infos.insert(stored_info);
		}

		/* Update stored_info */
		_prepare_ram_dataspaces(stored_info->stored_ramds_infos,
					child_info->parent_state().ram_dataspaces);

		child_info = child_info->next();
	}

	/* Delete old stored_infos, if the child misses corresponding infos in its list */
	stored_info = stored_infos.first();
	while(stored_info) {
		Stored_ram_session_info *next_info = stored_info->next();

		/* Find corresponding child_info */
		child_info = child_infos.first();
		if(child_info) child_info = child_info->find_by_badge(stored_info->badge);

		/* No corresponding child_info => delete it */
		if(!child_info) {
			stored_infos.remove(stored_info);
			_destroy_stored_ram_session(*stored_info);
		}

		stored_info = next_info;
	}
}


void Core_module_ram::_destroy_stored_ram_session(Stored_ram_session_info &stored_info)
{
	while(Stored_ram_dataspace_info *info = stored_info.stored_ramds_infos.first()) {
		stored_info.stored_ramds_infos.remove(info);
		_destroy_stored_ram_dataspace(*info);
	}
	Genode::destroy(_alloc, &stored_info);
}


void Core_module_ram::_prepare_ram_dataspaces(Genode::List<Stored_ram_dataspace_info> &stored_infos,
					      Genode::List<Ram_dataspace_info> &child_infos)
{
#ifdef DEBUG
	Genode::log("\033[36m", __PRETTY_FUNCTION__, "\033[0m");
#endif

	Ram_dataspace_info *child_info = nullptr;
	Stored_ram_dataspace_info *stored_info = nullptr;

	/* Update state_info from child_info
	 * If a child_info has no corresponding state_info, create it */
	child_info = child_infos.first();
	while(child_info) {
		/* Find corresponding state_info */
		stored_info = stored_infos.first();
		if(stored_info) stored_info = stored_info->find_by_badge(child_info->cap.local_name());

		/* No corresponding stored_info => create it */
		if(!stored_info) {
			stored_info = &_create_stored_ram_dataspace(*child_info);
			stored_infos.insert(stored_info);
		}

		/* register dataspace for checkpointing */
		ds_module().register_dataspace(stored_info->memory_content,
					       child_info->cap,
					       child_info->size);

		child_info = child_info->next();
	}

	/* Delete old stored_infos, if the child misses corresponding infos in its list */
	stored_info = stored_infos.first();
	while(stored_info) {
		Stored_ram_dataspace_info *next_info = stored_info->next();

		/* Find corresponding child_info */
		child_info = child_infos.first();
		if(child_info) child_info = child_info->find_by_badge(stored_info->badge);

		/* No corresponding child_info => delete it */
		if(!child_info) {
			stored_infos.remove(stored_info);
			_destroy_stored_ram_dataspace(*stored_info);
		}

		stored_info = next_info;
	}
}


Stored_ram_dataspace_info &Core_module_ram::_create_stored_ram_dataspace(Ram_dataspace_info &child_info)
{
#ifdef DEBUG
	Genode::log("\033[36m", __PRETTY_FUNCTION__, "\033[0m");
#endif
	/* The dataspace with the memory content of the ram dataspace will be
	 * referenced by the stored ram dataspace */
	Genode::Ram_dataspace_capability ramds_cap;

	/* Exclude dataspaces which are known region maps (except managed dataspaces
	 * from the incremental checkpoint mechanism) */
	Ref_badge_info *region_map_dataspace = find_region_map_by_badge(child_info.cap.local_name());

	if(region_map_dataspace) {
		Genode::log("Dataspace ", child_info.cap, " is a region map.");
	} else {
		/* Check whether the dataspace is somewhere in the stored session RPC objects */
		ramds_cap = Genode::reinterpret_cap_cast<Genode::Ram_dataspace>(state().find_stored_dataspace(
											child_info.cap.local_name()));

		if(!ramds_cap.valid()) {
			Genode::log("Dataspace ", child_info.cap, " is not known. "
				    "Creating dataspace with size ", Genode::Hex(child_info.size));
			ramds_cap = _env.ram().alloc(child_info.size);
		} else {
			Genode::log("Dataspace ", child_info.cap, " is known from last checkpoint.");
		}
	}

	Genode::addr_t childs_kcap = find_kcap_by_badge(child_info.cap.local_name());
	return *new (_alloc) Stored_ram_dataspace_info(child_info, childs_kcap, ramds_cap);
}


void Core_module_ram::_destroy_stored_ram_dataspace(Stored_ram_dataspace_info &stored_info)
{
#ifdef DEBUG
	Genode::log("\033[36m", __PRETTY_FUNCTION__, "\033[0m");
#endif
	/* Pre-condition: This stored object is removed from its list, thus, a
	 * search for a stored dataspace will not return its memory content
	 * dataspace */
	Genode::Dataspace_capability stored_ds_cap = state().find_stored_dataspace(stored_info.badge);
	if(!stored_ds_cap.valid()) {
		_env.ram().free(stored_info.memory_content);
	}

	Genode::destroy(_alloc, &stored_info);
}




