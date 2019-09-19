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
	Init_module(env, alloc),
	_ep(env, 16*1024, "resources ep"),
	_pd_factory(env, alloc, _ep, _childs_lock, _childs),
	_cpu_factory(env, alloc, _ep, _childs_lock, _childs),
	_log_factory(env, alloc, _ep, _childs_lock, _childs),
	_timer_factory(env, alloc, _ep, _childs_lock, _childs),
	_rom_factory(env, alloc, _ep, _childs_lock, _childs),
	_rm_factory(env, alloc, _ep, _childs_lock, _childs)    
{
	DEBUG_THIS_CALL;


}


Genode::Service *Base_module::resolve_session_request(const char *service_name,
                                                      const char *args)
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL;

	if(!Genode::strcmp(service_name, "PD")) {
		return _pd_factory.service();
	} else if(!Genode::strcmp(service_name, "CPU")) {
		return _cpu_factory.service();
	} else if(!Genode::strcmp(service_name, "LOG")) {
		return _log_factory.service();
	} else if(!Genode::strcmp(service_name, "Timer")) {
		return _timer_factory.service();
	} else if(!Genode::strcmp(service_name, "RM")) {
		return _rm_factory.service();
	} else if(!Genode::strcmp(service_name, "ROM")) {
		return _rom_factory.service();
	}

	return 0;
}
