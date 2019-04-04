/*
 * \brief  Core module
 * \description This module provides a minimal implementation for a sucessful checkpoint/restore.
 * \author Johannes Fischer
 * \date   2019-04-04
 */

#include <rtcr_core/core_module.h>
#include <rtcr_core/core_module_pd.h>
#include <rtcr_core/core_module_cpu.h>
#include <rtcr_core/core_module_rm.h>
#include <rtcr_core/core_module_ram.h>
#include <rtcr_core/core_module_rom.h>

using namespace Rtcr;


Core_module::Core_module(Genode::Env &env,
			 Genode::Allocator &md_alloc,
			 Genode::Entrypoint &ep,
			 const char* label,
			 bool &bootstrap)
    :
    Core_module_pd(env, md_alloc, ep, label, bootstrap),
    Core_module_cpu(env, md_alloc, ep, label, bootstrap), /* depends on Core_module_pd::Core_module_pd */
    Core_module_rm(env, md_alloc, ep, label, bootstrap), /* depends on Core_module_pd::Core_module_pd */
    Core_module_ram(env, md_alloc, ep, label, 0, bootstrap), /* depends on Core_module_pd::Core_module_pd */
    Core_module_rom(env, md_alloc, ep, label, bootstrap)
{
  
}


void Core_module::checkpoint(Target_state &state)
{  
    /* Preparations */
    Core_module_pd::_create_kcap_mappings(state);  /* Now find_kcap_by_badge is initialized */
    Core_module_rm::_create_region_map_dataspace_list(state); /* Now ... */

    /* Checkpointing */
    Core_module_pd::_checkpoint(state); /* depends on Core_module_rm::_create_region_map_dataspace_list */
    Core_module_cpu::_checkpoint(state); /* depends on Core_module_rm::_create_region_map_dataspace_list */
    Core_module_rm::_checkpoint(state); /* depends on Core_module_rm::_create_region_map_dataspace_list */
    Core_module_ram::_checkpoint(state); /* depends on Core_module_rm::_create_region_map_dataspace_list */
}


void Core_module::restore(Target_state &state)
{

}

void pause()
{
    Core_module_cpu::_pause();
}


void resume()
{
    Core_module_cpu::_resume();
}



TODO in all classes:
_find_kcap_by_badge -> Core_module_pd::find_kcap_by_badge
