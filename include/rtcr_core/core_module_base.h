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
#include <rtcr_core/core_state.h>
#include <base/service.h>
#include <rtcr_ds/dataspace_module.h>

#include <rtcr/core_module_abstract.h>

namespace Rtcr {
    class Core_module_base;
}

using namespace Rtcr;

class Rtcr::Core_module_base : public virtual Core_module_abstract
{
 protected:
    virtual Ref_badge_info *find_region_map_by_badge(Genode::uint16_t badge) = 0;

  // implemented in core_module_rm
  // used by core_module_pd
    virtual void _prepare_region_maps(Genode::List<Stored_region_map_info> &stored_infos,
				      Genode::List<Region_map_component> &child_infos) = 0;

  virtual Dataspace_module &ds_module() = 0;
  virtual Core_state &state() = 0;
  
public:
    /* Methods required by Target_child */
    virtual Pd_root &pd_root() = 0;
    virtual Ram_root &ram_root() = 0;
    virtual Cpu_root &cpu_root() = 0;
    virtual Rm_root &rm_root() = 0;

};



#endif /* _RTCR_CORE_MODULE_BASE_H_ */



