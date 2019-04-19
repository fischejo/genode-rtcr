/*
 * \brief  RAM Session Handler
 * \author Johannes Fischer
 * \date   2019-03-21
 */

#include <rtcr_core/core_module_rom.h>

using namespace Rtcr;


Core_module_rom::Core_module_rom(Genode::Env &env,
				 Genode::Allocator &md_alloc,
				 Genode::Entrypoint &ep)
  :
  _md_alloc(md_alloc),
  _env(env),
  _ep(ep)
{
}

void Core_module_rom::_initialize_rom_session(const char* label, bool &bootstrap)
{
#ifdef DEBUG
    Genode::log("\033[36m", __PRETTY_FUNCTION__, "\033[0m");
#endif
  
  _rom_root = new (_md_alloc) Rom_root(_env, _md_alloc, _ep, bootstrap);
  _rom_service = new (_md_alloc) Genode::Local_service("ROM", _rom_root);
  _rom_connection = new(_md_alloc) Genode::Rom_connection(_env, label);
}


Core_module_rom::~Core_module_rom()
{
    Genode::destroy(_md_alloc, _rom_root);
    Genode::destroy(_md_alloc, _rom_service);
    Genode::destroy(_md_alloc, _rom_connection);        
}
