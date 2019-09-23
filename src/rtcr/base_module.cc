/*
 * \brief Base module
 * \author Johannes Fischer
 * \date   2019-08-29
 */
#include <rtcr/base_module.h>
#include <base/service.h>
#include <base/registry.h>

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

/* Create a static instance of the Init_module_factory. This registers the
 * module */
Rtcr::Base_module_factory _base_module_factory_instance;


Base_module::Base_module(Genode::Env &env, Genode::Allocator &alloc)
	:
	Init_module(env, alloc),
	_ep(env, 16*1024, "resources ep"),
	_pd(env, alloc, _ep, _childs_lock, _childs, _services),
	_cpu(env, alloc, _ep, _childs_lock, _childs, _services),
	_log(env, alloc, _ep, _childs_lock, _childs, _services),
	_timer(env, alloc, _ep, _childs_lock, _childs, _services),
	_rom(env, alloc, _ep, _childs_lock, _childs, _services),
	_rm(env, alloc, _ep, _childs_lock, _childs, _services)
{
	DEBUG_THIS_CALL;
}
