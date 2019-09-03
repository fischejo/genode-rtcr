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

/* Create a static instance of the Init_module_factory. This registers the
 * module */
Rtcr::Base_module_factory _base_module_factory_instance;


Base_module::Base_module(Genode::Env &env, Genode::Allocator &alloc)
	:
	Init_module(env, alloc)
{
	DEBUG_THIS_CALL;
	init( new(alloc) Pd_root(env, alloc, _ep, _childs_lock, _childs));
	init( new(alloc) Cpu_root(env, alloc, _ep, _childs_lock, _childs));
	init( new(alloc) Ram_root(env, alloc, _ep, _childs_lock, _childs));
	init( new(alloc) Rm_root(env, alloc, _ep, _childs_lock, _childs));
	init( new(alloc) Rom_root(env, alloc, _ep, _childs_lock, _childs));
	init( new(alloc) Log_root(env, alloc, _ep, _childs_lock, _childs));
	init( new(alloc) Timer_root(env, alloc, _ep, _childs_lock, _childs));
}
