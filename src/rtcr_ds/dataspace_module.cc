/*
 * \brief  Datapsace module
 * \description This module provides the dataspace handling
 * \author Johannes Fischer
 * \date   2019-04-13
 */

#include <rtcr_ds/dataspace_module.h>

using namespace Rtcr;

/* Create a static instance of the Dataspace_module_factory. This registers the module */
Rtcr::Dataspace_module_factory _dataspace_module_factory_instance;


Dataspace_module::Dataspace_module(Genode::Env &env,
				   Genode::Allocator &alloc,
				   Genode::Entrypoint &ep)
	: _env(env), _alloc(alloc), _ep(ep) {}


void Dataspace_module::checkpoint_dataspace(Genode::Dataspace_capability dst_ds_cap,
					     Genode::Dataspace_capability src_ds_cap,
					     Genode::addr_t dst_offset,
					     Genode::size_t size)
{
#ifdef DEBUG
	Genode::log("\e[38;5;204m", __PRETTY_FUNCTION__, "\033[0m");
#endif
    
	char *dst_addr_start = _env.rm().attach(dst_ds_cap);
	char *src_addr_start = _env.rm().attach(src_ds_cap);

	Genode::memcpy(dst_addr_start + dst_offset, src_addr_start, size);

	_env.rm().detach(src_addr_start);
	_env.rm().detach(dst_addr_start);
}
