/*
 * \brief  Dataspace Submodule
 * \author Johannes Fischer
 * \date   2019-05-19
 */

#ifndef _RTCR_CORE_MODULE_DS_H_
#define _RTCR_CORE_MODULE_DS_H_

/* Genode includes */
#include <base/component.h>
#include <base/log.h>
#include <base/service.h>
#include <base/env.h>
#include <base/child.h>
#include <base/service.h>
#include <base/snprintf.h>

/* Core module includes */
#include <rtcr_core/core_module_base.h>
#include <rtcr_core/ds/dataspace_translation_info.h>

/* Dataspace module includes */
#include <rtcr_ds/dataspace_module.h>

namespace Rtcr {
	class Core_module_ds;
}

using namespace Rtcr;

class Rtcr::Core_module_ds: public virtual Core_module_base
{
private:
	Genode::Env        &_env;
	Genode::Allocator  &_alloc;
	Genode::Entrypoint &_ep;

	void _destroy_list(Genode::List<Dataspace_translation_info> &list);
	
protected:
	/* list of dataspaces which should be checkpointed */
	Genode::List<Dataspace_translation_info> _dataspace_translations;
	Dataspace_module *_ds_module;
	
	/**
	 * Call the method for copying a dataspace
	 *
	 * \param ckpt_ds_cap capability of source dataspace
	 * \param resto_ds_cap capability of destination datapace
	 * \param size
	 */ 
	virtual void checkpoint_dataspace(Genode::Ram_dataspace_capability ckpt_ds_cap,
					  Genode::Dataspace_capability resto_ds_cap,
					  Genode::size_t size) override;
	
	/**
	 * Called by the Target_child when all modules are initialized.
	 *
	 * \param modules which were loaded. 
	 */
	void initialize(Genode::List<Module> &modules) override;

	void _checkpoint();

	Dataspace_module &ds_module() { return *_ds_module; };
	
public:
	Core_module_ds(Genode::Env &env, Genode::Allocator &alloc, Genode::Entrypoint &ep);
	~Core_module_ds();
};


#endif /* _RTCR_CORE_MODULE_DS_H_ */
