/*
 * \brief  Core Module
 * \author Johannes Fischer
 * \date   2019-04-04
 */

#ifndef _RTCR_CORE_MODULE_BASE_H_
#define _RTCR_CORE_MODULE_BASE_H_

/* Genode includes */
#include <util/list.h>

/* Rtcr includes */
#include <rtcr/ref_badge_info.h>
#include <rtcr_core/rm/rm_session.h>
#include <rtcr_core/cpu/cpu_session.h>
#include <rtcr_core/pd/pd_session.h>
#include <rtcr_core/rom/rom_session.h>
#include <rtcr_core/ram/ram_session.h>
#include <rtcr_core/core_state.h>
#include <base/service.h>
#include <rtcr_ds/dataspace_module.h>

/* Core module includes */
#include <rtcr/core_module_abstract.h>

namespace Rtcr {
	class Core_module_base;
}

using namespace Rtcr;

/**
 * The core module provides the core implementation of the Rtcr.
 *
 * It contains:
 * - CPU Session and Checkpointing/Restoring
 * - RAM Session and Checkpointing/Restoring
 * - ROM Session and Checkpointing/Restoring
 * - PD Session and Checkpointing/Restoring
 * - RM Session and Checkpointing/Restoring
 *
 * Due to the strong coupling between PD, RAM and RM sessions checkpointing,
 * this module is implemented with a diamand hierarchy. Where all classes in
 * level 2 inherits from level 1 and class Core_module (level 3) inherits from
 * all classes in level 2. This concept allows to seperate logic in container
 * classes (level 2) but there is still the possibility to share methods between
 * container classes.
 *
 *                      +---------------------+
 *                      |  Core_module_base   |                      [ LEVEL 1 ]
 *                      +---------------------+
 *
 *    +-----+       +----+      +-----+      +-----+      +----+
 *    | cpu |       | pd |      | ram |      | rom |      | rm |     [ LEVEL 2 ]
 *    +-----+       +----+      +-----+      +-----+      +----+
 *
 *                          +-------------+
 *                          | Core_module |                          [ LEVEL 3 ]
 *                          +-------------+
 *
 * The Core_module_base (level 1) defines methods which are used by one of the
 * classes in level 2-3. These methods are also defined in level 2 and 3. If
 * Core_module_cpu calls `pd_root()`, its call will be delegated to the
 * Core_module_pd class where `pd_root()` is implemented. 
 */
class Rtcr::Core_module_base : public virtual Core_module_abstract
{
protected:
	/**
	 * Find a region map based on a badge
	 * \param badge
	 * \return Reference to a badge info object
	 */
	virtual Ref_badge_info *find_region_map_by_badge(Genode::uint16_t badge) = 0;

	/**
	 * This method is implemented in core_module_rm, but also used by the core_module_pd.
	 */
	virtual void _prepare_region_maps(Genode::List<Stored_region_map_info> &stored_infos,
					  Genode::List<Region_map_component> &child_infos) = 0;

	/**
	 * Get the Dataspace module
	 *
	 * \return Dataspace Module
	 */ 
	virtual Dataspace_module &ds_module() = 0;

	/**
	 * Get the module state
	 *
	 * \return state of the module
	 */ 
	virtual Core_state &state() = 0;

	/**
	 * Provide session root components to all other classes.
	 */
	virtual Pd_root &pd_root() = 0;
	virtual Ram_root &ram_root() = 0;
	virtual Cpu_root &cpu_root() = 0;
	virtual Rm_root &rm_root() = 0;
	virtual Rom_root &rom_root() = 0;
};

#endif /* _RTCR_CORE_MODULE_BASE_H_ */
