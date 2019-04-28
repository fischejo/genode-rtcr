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
#include <rtcr/dataspace_translation_info.h>

namespace Rtcr {
	class Dataspace_module;
	class Dataspace_module_factory;
}

using namespace Rtcr;


/**
 * The module encapsulates the logic for checkpointing/restoring dataspaces. 
 *
 * At this time, it is not sure if this module will be integrated in to the core
 * module. Due to the strong coupling of the core module and this module, this
 * might make sense.
 */
class Rtcr::Dataspace_module : public virtual Module
{
protected:
	Genode::Env        &_env;
	Genode::Allocator  &_alloc;
	Genode::Entrypoint &_ep;

	/* list of dataspaces which should be checkpointed */
	Genode::List<Dataspace_translation_info> _dataspace_translations;

	/**
	 * Checkpoint dataspaces stored in `_dataspace_translations`.
	 *
	 * \param dst_ds_cap source dataspace
	 * \param src_ds_cap destination dataspace
	 * \param dst_offset copy start position in source dataspce
	 * \param size
	 */
	void _checkpoint_dataspace(Genode::Dataspace_capability dst_ds_cap,
				   Genode::Dataspace_capability src_ds_cap,
				   Genode::addr_t dst_offset,
				   Genode::size_t size);
public:

	Dataspace_module(Genode::Env &env,
			 Genode::Allocator &alloc,
			 Genode::Entrypoint &ep);

	~Dataspace_module();

	/**
	 * This method allows other modules to register dataspaces which should be
	 * checkpointed by this module later.
	 * 
	 * \param ckpt_ds_cap capability of source dataspace
	 * \param resto_ds_cap capability of destination datapace
	 * \param size
	 */
	void register_dataspace(Genode::Ram_dataspace_capability ckpt_ds_cap,
				Genode::Dataspace_capability resto_ds_cap,
				Genode::size_t size);

	/**
	 * Checkpoint all registered dataspaces
	 * 
	 * \return nullptr - This module does not provide a Module_state
	 */
	Module_state *checkpoint() override;

	/**
	 * This method will restore the Dataspaces. 
	 *
	 * Not yet implementend.
	 */ 
	void restore(Module_state *state) override;

	/**
	 * As this module does not provide a session, no session requests are
	 * resolved.
	 */
	Genode::Service *resolve_session_request(const char *service_name,
						 const char *args) override;

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
