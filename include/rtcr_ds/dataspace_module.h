/*
 * \brief  Dataspace Module
 * \author Johannes Fischer
 * \date   2019-04-13
 */

#ifndef _RTCR_DATASPACE_MODULE_H_
#define _RTCR_DATASPACE_MODULE_H_

/* Genode includes */
#include <util/list.h>
#include <base/heap.h>
#include <util/list.h>
#include <os/config.h>
#include <base/service.h>

/* Rtcr includes */
#include <rtcr/module.h>
#include <rtcr/module_factory.h>
#include <rtcr/module_state.h>

namespace Rtcr {
	class Dataspace_module;
	class Dataspace_module_factory;
}

using namespace Rtcr;


/**
 * The module encapsulates the low level logic for checkpointing/restoring
 * dataspaces. As the logic of dataspaces is strongly encapsulated with the
 * Core_module, the Core_module also implements a `Core_module_ds` class. 
 */
class Rtcr::Dataspace_module : public virtual Module
{
protected:
	Genode::Env        &_env;
	Genode::Allocator  &_alloc;
	Genode::Entrypoint &_ep;

public:

	Dataspace_module(Genode::Env &env,
			 Genode::Allocator &alloc,
			 Genode::Entrypoint &ep);

	~Dataspace_module() {};

	/**
	 * Checkpoint dataspaces
	 *
	 * Pay attention, duplicates of dataspaces will not be filtered
	 * out. Every dataspace which is handed over, will be copied.
	 *
	 * \param dst_ds_cap source dataspace
	 * \param src_ds_cap destination dataspace
	 * \param dst_offset copy start position in source dataspce
	 * \param size
	 */
	void checkpoint_dataspace(Genode::Dataspace_capability dst_ds_cap,
				   Genode::Dataspace_capability src_ds_cap,
				   Genode::addr_t dst_offset,
				   Genode::size_t size);
	/**
	 * As every dataspace is copied directly, no extra functionality is
	 * required in `checkpoint()`.
	 */
	Module_state *checkpoint() override { return nullptr; };

	void restore(Module_state *state) override {};

	/**
	 * As this module does not provide a session, no session requests are
	 * resolved.
	 */
	Genode::Service *resolve_session_request(const char *service_name, const char *args) override { return 0; }

	Module_name name() override { return "ds"; }
};


/**
 * Factory class for creating the Rtcr::Dataspace_module
 */
class Rtcr::Dataspace_module_factory : public Module_factory
{
public:
	Module* create(Genode::Env &env,
		       Genode::Allocator &alloc,
		       Genode::Entrypoint &ep,
		       const char* label,
		       bool &bootstrap,
		       Genode::Xml_node *config) override
	{
		return new (alloc) Dataspace_module(env, alloc, ep);
	}
    
	Module_name name() override { return "ds"; }  
};

#endif /* _RTCR_DATASPACE_MODULE_H_ */
