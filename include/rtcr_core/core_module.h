/*
 * \brief  Core Module
 * \author Johannes Fischer
 * \date   2019-04-04
 */

#ifndef _RTCR_CORE_MODULE_H_
#define _RTCR_CORE_MODULE_H_

/* Genode includes */
#include <util/list.h>
#include <base/heap.h>
#include <util/list.h>


/* Local includes */
#include <rtcr/module.h>
#include <rtcr_core/core_module_cpu.h>
#include <rtcr_core/core_module_ram.h>
#include <rtcr_core/core_module_pd.h>
#include <rtcr_core/core_module_rm.h>
#include <rtcr_core/core_module_rom.h>


namespace Rtcr {
    class Core_module;
    class Core_base;
}


class Rtcr::Core_module_base :
{
public:
    /**
     * \brief Return the kcap for a given badge from _capability_map_infos
     * Refactored from `checkpointer.h`
     * Return the kcap for a given badge. If there is no, return 0.
     *
     * As every module requires this method, it is public to othe rmodules.
     */
    virtual Genode::addr_t find_kcap_by_badge(Genode::uint16_t badge) = 0;


    /* Methods required by Target_child */
    virtual Pd_root &pd_root() = 0;
    virtual Ram_root &ram_root() = 0;
    virtual Cpu_root &cpu_root() = 0;
    virtual Rm_root & rm_root() = 0;
  
    virtual Genode::Local_service &pd_service() = 0;
    virtual Genode::Local_service &rm_service() = 0;
    virtual Genode::Local_service &cpu_service() = 0;
    virtual Genode::Local_service &ram_service() = 0;

    virtual Cpu_session_component &cpu_session() = 0;
    virtual Ram_session_component &ram_session() = 0;
    virtual Pd_session_component &pd_session() = 0;
};


class Rtcr::Core_module : public Module,
			  public Core_module_cpu,
			  public Core_module_ram,
			  public Core_module_pd,
			  public Core_module_rm,
			  public Core_module_rom
{
public:

    /* These methods are implemented for the Target_child */
    void pause();
    void resume();
    void checkpoint(Target_state &state);
    void restore(Target_state &state);
};



#endif /* _RTCR_CORE_MODULE_H_ */
