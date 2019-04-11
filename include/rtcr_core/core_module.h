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
#include <rtcr/target_state.h>
#include <rtcr_core/core_module_base.h>
#include <rtcr_core/core_module_cpu.h>
#include <rtcr_core/core_module_ram.h>
#include <rtcr_core/core_module_pd.h>
#include <rtcr_core/core_module_rm.h>
#include <rtcr_core/core_module_rom.h>


namespace Rtcr {
    class Core_module;
}

using namespace Rtcr;

class Rtcr::Core_module : public Core_module_pd,
			  public Core_module_cpu,
			  public Core_module_ram,
			  public Core_module_rm,
			  public Core_module_rom
{
public:
  Core_module(Genode::Env &env,
	      Genode::Allocator &md_alloc,
	      Genode::Entrypoint &ep,
	      const char* label,
	      bool &bootstrap);

  ~Core_module();

  
    /* These methods are implemented for the Target_child */
    void pause();
    void resume();
    void checkpoint(Target_state &state);
    void restore(Target_state &state);

};



#endif /* _RTCR_CORE_MODULE_H_ */
