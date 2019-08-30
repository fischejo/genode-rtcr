/*
 * \brief Base module
 * \author Johannes Fischer
 * \date   2019-08-29
 */
#include <rtcr/base_module.h>

#ifdef PROFILE
#include <util/profiler.h>
#define PROFILE_THIS_CALL PROFILE_FUNCTION("violet");
#else
#define PROFILE_THIS_CALL
#endif

#if DEBUG 
#define DEBUG_THIS_CALL Genode::log("\e[38;5;207m", __PRETTY_FUNCTION__, "\033[0m");
#else
#define DEBUG_THIS_CALL
#endif

using namespace Rtcr;

/* Create a static instance of the Base_module_factory. This registers the
 * module */
Rtcr::Base_module_factory _base_module_factory_instance;


Base_module::Base_module(Genode::Env &env, Genode::Allocator &alloc)
	:
	_bootstrap(true),
	_ep(env, 16*1024, "resources ep"),
	_ram_root(env, alloc, _ep, _bootstrap),
	_pd_root(env, alloc, _ep, _ram_root, _bootstrap),
	_cpu_root(env, alloc, _ep, _pd_root, _bootstrap),	
	_rm_root(env, alloc, _ep, _ram_root, _bootstrap),
	_rom_root(env, alloc, _ep, _bootstrap),
	_log_root(env, alloc, _ep, _bootstrap),
	_timer_root(env, alloc, _ep, _bootstrap)
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL
}

