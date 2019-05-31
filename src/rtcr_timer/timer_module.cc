/*
 * \brief  Timer module
 * \description This module provides a timer session
 * \author Johannes Fischer
 * \date   2019-04-12
 */

#include <rtcr_timer/timer_module.h>

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

/* Create a static instance of the Core_module_factory. This registers the module */
Rtcr::Timer_module_factory _timer_module_factory_instance;


Timer_module::Timer_module(Genode::Env &env,
			   Genode::Allocator &alloc,
			   Genode::Entrypoint &ep,
			   bool &bootstrap)
	:
	_env(env),
	_alloc(alloc),
	_ep(ep),
	_bootstrap(bootstrap),
	_timer_state(_initialize_state(alloc))
{}


Timer_module::~Timer_module()
{
	if(_timer_root) Genode::destroy(_alloc, _timer_root);
	if(_timer_service) Genode::destroy(_alloc, _timer_service);
}


void Timer_module::initialize(Genode::List<Module> &modules)
{
	DEBUG_THIS_CALL
  
	Module *module = modules.first();
	while (!_core_module && module) {
		_core_module = dynamic_cast<Core_module_abstract*>(module);
		module = module->next();
	}

	if(!_core_module)
		Genode::error("No Core_module loaded! ");
}


Timer_state &Timer_module::_initialize_state(Genode::Allocator &_alloc)
{
	return *new(_alloc) Timer_state(_alloc);
}


Module_state *Timer_module::checkpoint()
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL

	Genode::List<Stored_timer_session_info> &stored_infos = _timer_state._stored_timer_sessions;
	Genode::List<Timer_session_component> &child_infos = _timer_root->session_infos();
	Timer_session_component *child_info = nullptr;
	Stored_timer_session_info *stored_info=  nullptr;

	/* Update state_info from child_info If a child_info has no corresponding
	 * state_info, create it */
	child_info = child_infos.first();
	while(child_info) {
		/* Find corresponding state_info */
		stored_info = stored_infos.first();
		if(stored_info)
			stored_info = stored_info->find_by_badge(child_info->cap().local_name());

		/* No corresponding stored_info => create it */
		if(!stored_info) {
			Genode::addr_t childs_kcap = _core_module->find_kcap_by_badge(child_info->cap().local_name());
			stored_info = new (_alloc) Stored_timer_session_info(*child_info, childs_kcap);
			stored_infos.insert(stored_info);
		}

		/* Update stored_info */
		stored_info->sigh_badge = child_info->parent_state().sigh.local_name();
		stored_info->timeout = child_info->parent_state().timeout;
		stored_info->periodic = child_info->parent_state().periodic;

		child_info = child_info->next();
	}

	/* Delete old stored_infos, if the child misses corresponding infos in its list */
	stored_info = stored_infos.first();
	while(stored_info) {
		Stored_timer_session_info *next_info = stored_info->next();

		/* Find corresponding child_info */
		child_info = child_infos.first();
		if(child_info) child_info = child_info->find_by_badge(stored_info->badge);

		/* No corresponding child_info => delete it */
		if(!child_info) {
			stored_infos.remove(stored_info);
			_destroy_stored_timer_session(*stored_info);
		}

		stored_info = next_info;
	}
	return &_timer_state;
}


void Timer_module::_destroy_stored_timer_session(Stored_timer_session_info &stored_info)
{
	DEBUG_THIS_CALL

	Genode::destroy(_alloc, &stored_info);
}



void Timer_module::restore(Module_state *state)
{
	DEBUG_THIS_CALL
}


Genode::Service *Timer_module::resolve_session_request(const char *service_name,
						       const char *args)
{
	DEBUG_THIS_CALL

  
	if(!Genode::strcmp(service_name, "Timer")) {
		if(!_timer_root) {
			_timer_root = new (_alloc) Timer_root(_env, _alloc, _ep, _bootstrap);
			_timer_service = new (_alloc) Genode::Local_service("Timer",_timer_root);
		}
		return _timer_service;
	}
	return 0;
}

