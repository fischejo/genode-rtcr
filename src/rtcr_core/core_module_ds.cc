/*
 * \brief  Dataspace implementation of Core_module
 * \author Johannes Fischer
 * \date   2019-06-19
 */

#include <rtcr_core/core_module_ds.h>

#ifdef PROFILE
#include <util/profiler.h>
#define PROFILE_THIS_CALL PROFILE_FUNCTION("blue");
#else
#define PROFILE_THIS_CALL
#endif

#if DEBUG 
#define DEBUG_THIS_CALL Genode::log("\033[36m", __PRETTY_FUNCTION__, "\033[0m");
#else
#define DEBUG_THIS_CALL
#endif


using namespace Rtcr;


Core_module_ds::Core_module_ds(Genode::Env &env, Genode::Allocator &alloc, Genode::Entrypoint &ep)
	: _alloc(alloc), _env(env), _ep(ep), _ds_module(nullptr)
{}


void Core_module_ds::checkpoint_dataspace(Genode::Ram_dataspace_capability ckpt_ds_cap,
					  Genode::Dataspace_capability resto_ds_cap,
					  Genode::size_t size)
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL

	Dataspace_translation_info *trans_info = _dataspace_translations.first();
	if(trans_info)
		trans_info = trans_info->find_by_resto_badge(resto_ds_cap.local_name());

	/* if the dataspace is not already in the list, it will be added and it
	 * will be handed over to the dataspace module which will do the
	 * copying */
	if(!trans_info) {
		trans_info = new (_alloc) Dataspace_translation_info(ckpt_ds_cap,
								     resto_ds_cap,
								     size);
		/* add dataspace to the list of checkpointed dataspaces */
		_dataspace_translations.insert(trans_info);
	}
}


void Core_module_ds::initialize(Genode::List<Module> &modules)
{
	Module *module = modules.first();
	while (!_ds_module && module) {
		_ds_module = dynamic_cast<Dataspace_module*>(module);
		module = module->next();
	}

	if(!_ds_module)
		Genode::error("No Dataspace_module loaded! ");
}


void Core_module_ds::_checkpoint()
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL	

	Dataspace_translation_info *memory_info = _dataspace_translations.first();

#ifdef DEBUG
	Genode::log("Dataspaces to checkpoint:");  
	while(memory_info) {
		Genode::log(" ", *memory_info);
		memory_info = memory_info->next();
	}
	memory_info = _dataspace_translations.first();	
#endif
	while(memory_info) {		
		_ds_module->checkpoint_dataspace(memory_info->ckpt_ds_cap,
						 memory_info->resto_ds_cap,
						 0,
						 memory_info->size);		
		memory_info = memory_info->next();
	}

	/* empty the list after checkpointing, otherwise the next checkpoint
	 * exclude some dataspaces */
	_destroy_list(_dataspace_translations);		
}


Core_module_ds::~Core_module_ds()
{
	DEBUG_THIS_CALL
	_destroy_list(_dataspace_translations);	

}

void Core_module_ds::_destroy_list(Genode::List<Dataspace_translation_info> &list)
{
	DEBUG_THIS_CALL	
	while(Dataspace_translation_info *elem = list.first()) {
		list.remove(elem);
		Genode::destroy(_alloc, elem);
	}
}
