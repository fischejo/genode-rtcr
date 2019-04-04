/*
 * \brief  RAM Session Handler
 * \author Johannes Fischer
 * \date   2019-03-21
 */

#include <rtcr_core/core_module_rom.h>

using namespace Rtcr;


Core_module_rom::Core_module_rom(Genode::Env &env,
					Genode::Allocator &md_alloc,
					Genode::Entrypoint &ep,
					const char* label,
					bool &bootstrap)
  :
  _rom_root(env, md_alloc, ep, bootstrap),
  _rom_service("ROM", _rom_root),
  _rom_connection(env, label)
{

}

Core_module_rom::~Core_module_rom()
{

}
