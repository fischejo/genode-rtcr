/*
 * \brief  RAM Session Handler
 * \author Johannes Fischer
 * \date   2019-03-21
 */

#include <rtcr_core/core_module_rom.h>

#ifdef PROFILE
#include <util/profiler.h>
#define PROFILE_THIS_CALL PROFILE_FUNCTION("blue");
#else
#define PROFILE_THIS_CALL
#endif

#if DEBUG 
#define DEBUG_THIS_CALL Genode::log("\033[36m", __PRETTY_FUNCTION__, "\033[0m");
#else
#define DEBUG_THIS_CALL
#endif


using namespace Rtcr;


Core_module_rom::Core_module_rom(Genode::Env &env,
				 Genode::Allocator &alloc,
				 Genode::Entrypoint &ep)
	:
	_alloc(alloc),
	_env(env),
	_ep(ep)
{}


void Core_module_rom::_initialize_rom_session(const char* label, bool &bootstrap)
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL
		
	_rom_root = new (_alloc) Rom_root(_env, _alloc, _ep, bootstrap);
	_rom_service = new (_alloc) Genode::Local_service("ROM", _rom_root);
	_rom_connection = new(_alloc) Genode::Rom_connection(_env, label);
}


Core_module_rom::~Core_module_rom()
{
	DEBUG_THIS_CALL
	Genode::destroy(_alloc, _rom_root);
	Genode::destroy(_alloc, _rom_service);
	Genode::destroy(_alloc, _rom_connection);        
}
