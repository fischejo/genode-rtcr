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

/* Local includes */
#include <rtcr_core/core_module_base.h>
#include <rtcr/module_state.h>
#include <rtcr_core/rm/rm_session.h>


namespace Rtcr {
    class Core_module_rm;
}

using namespace Rtcr;

class Rtcr::Core_module_rm: public virtual Core_module_base
{
private:
    Genode::Env        &_env;
    Genode::Allocator  &_md_alloc;
    Genode::Entrypoint &_ep;

protected:
    Rm_root *_rm_root;
    Genode::Local_service *_rm_service;

    /**
     * List of dataspace badges which are (known) managed dataspaces
     * These dataspaces are not needed to be copied
     */
    Genode::List<Ref_badge_info> _region_maps;

    // level 2.1
    void _prepare_region_maps(Genode::List<Stored_region_map_info> &stored_infos,
			      Genode::List<Region_map_component> &child_infos);

    // level 2.1.1
    void _prepare_attached_regions(Genode::List<Stored_attached_region_info> &stored_infos,
				   Genode::List<Attached_region_info> &child_infos);

    // level 2.1.1.1
    Stored_attached_region_info &_create_stored_attached_region(Attached_region_info &child_info);
    
    // level 2.2
    void _destroy_stored_rm_session(Stored_rm_session_info &stored_info);

    // level 2.2.1
    void _destroy_stored_region_map(Stored_region_map_info &stored_info);

    // level 2.2.1.1
    void _destroy_stored_attached_region(Stored_attached_region_info &stored_info);

    // level 1
    void _create_region_map_dataspaces_list();

    // level 2
    void _checkpoint();
    void _initialize_rm_session(const char* label, bool &bootstrap);
    Ref_badge_info *find_region_map_by_badge(Genode::uint16_t badge);
    


public:    
    
    virtual Genode::Local_service &rm_service() {
	return *_rm_service;
    }


    

    Core_module_rm(Genode::Env &env,
		       Genode::Allocator &md_alloc,
		   Genode::Entrypoint &ep);

	
    ~Core_module_rm();

    /* implement virtual methods of Core_module_base */
    Rm_root &rm_root() {
	return *_rm_root;
    }

    
};


#endif /* _RTCR_CORE_MODULE_RM_H_ */
