/*
 * \brief  Core Module
 * \author Johannes Fischer
 * \date   2019-04-04
 */

#ifndef _RTCR_CORE_MODULE_BASE_H_
#define _RTCR_CORE_MODULE_BASE_H_

/* Genode includes */
#include <util/list.h>


#include <rtcr/ref_badge_info.h>
#include <rtcr_core/rm/rm_session.h>
#include <rtcr_core/cpu/cpu_session.h>
#include <rtcr_core/pd/pd_session.h>
#include <rtcr_core/rom/rom_session.h>
#include <rtcr_core/ram/ram_session.h>
#include <rtcr/target_state.h>
#include <base/service.h>

namespace Rtcr {
    class Core_module_base;
}

using namespace Rtcr;

class Rtcr::Core_module_base
{
 protected:
    /**
     * \brief Return the kcap for a given badge from _capability_map_infos
     * Refactored from `checkpointer.h`
     * Return the kcap for a given badge. If there is no, return 0.
     *
     * As every module requires this method, it is public to othe rmodules.
     */
    virtual Genode::addr_t find_kcap_by_badge(Genode::uint16_t badge) = 0;
    virtual Ref_badge_info *find_region_map_by_badge(Genode::uint16_t badge) = 0;

  // implemented in core_module_rm
  // used by core_module_pd
    virtual void _prepare_region_maps(Target_state &state,
				      Genode::List<Stored_region_map_info> &stored_infos,
				      Genode::List<Region_map_component> &child_infos) = 0;
public:
    /* Methods required by Target_child */
    virtual Pd_root &pd_root() = 0;
    virtual Ram_root &ram_root() = 0;
    virtual Cpu_root &cpu_root() = 0;
    virtual Rm_root &rm_root() = 0;
  
    virtual Genode::Local_service &pd_service() = 0;
    virtual Genode::Local_service &rm_service() = 0;
    virtual Genode::Local_service &cpu_service() = 0;
    virtual Genode::Local_service &ram_service() = 0;

    virtual Cpu_session_component &cpu_session() = 0;
    virtual Ram_session_component &ram_session() = 0;
    virtual Pd_session_component &pd_session() = 0;
};



#endif /* _RTCR_CORE_MODULE_BASE_H_ */



