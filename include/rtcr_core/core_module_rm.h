/*
 * \brief  RAM Session Handler
 * \author Johannes Fischer
 * \date   2019-03-21
 */

#ifndef _RTCR_CORE_MODULE_RM_H_
#define _RTCR_CORE_MODULE_RM_H_

/* Genode includes */
#include <base/component.h>
#include <base/log.h>
#include <base/service.h>
#include <base/env.h>
#include <base/child.h>
#include <base/service.h>
#include <base/snprintf.h>
#include <region_map/client.h>

/* Rtcr includes */
#include <rtcr/module_state.h>

/* Core module includes */
#include <rtcr_core/core_module_base.h>
#include <rtcr_core/rm/rm_session.h>

namespace Rtcr {
	class Core_module_rm;
}

using namespace Rtcr;

class Rtcr::Core_module_rm: public virtual Core_module_base
{
private:
	Genode::Env        &_env;
	Genode::Allocator  &_alloc;
	Genode::Entrypoint &_ep;

protected:
	Rm_root *_rm_root;
	Genode::Local_service *_rm_service;

	/**
	 * List of dataspace badges which are (known) managed dataspaces
	 * These dataspaces are not needed to be copied
	 */
	Genode::List<Ref_badge_info> _region_maps;

	void _prepare_region_maps(Genode::List<Stored_region_map_info> &stored_infos,
				  Genode::List<Region_map_component> &child_infos);

	void _prepare_attached_regions(Genode::List<Stored_attached_region_info> &stored_infos,
				       Genode::List<Attached_region_info> &child_infos);

	Stored_attached_region_info &_create_stored_attached_region(Attached_region_info &child_info);
    
	void _destroy_stored_rm_session(Stored_rm_session_info &stored_info);

	void _destroy_stored_region_map(Stored_region_map_info &stored_info);

	void _destroy_stored_attached_region(Stored_attached_region_info &stored_info);

	void _create_region_map_dataspaces_list();

	void _checkpoint();
	void _initialize_rm_session(const char* label, bool &bootstrap);

	/**
	 * Find a region map based on a badge
	 * \param badge
	 * \return Reference to a badge info object
	 */
	Ref_badge_info *find_region_map_by_badge(Genode::uint16_t badge) override;

public:    
	Core_module_rm(Genode::Env &env,
		       Genode::Allocator &alloc,
		       Genode::Entrypoint &ep);
	
	~Core_module_rm();

    
	virtual Genode::Local_service &rm_service() override
	{
		return *_rm_service;
	}

	Rm_root &rm_root() override
	{
		return *_rm_root;
	}
};


#endif /* _RTCR_CORE_MODULE_RM_H_ */
