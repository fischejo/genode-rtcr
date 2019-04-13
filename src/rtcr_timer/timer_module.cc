/*
 * \brief  Timer module
 * \description This module provides a timer session
 * \author Johannes Fischer
 * \date   2019-04-12
 */

#include <rtcr_timer/timer_module.h>

using namespace Rtcr;

/* Create a static instance of the Core_module_factory. This registers the module */

Rtcr::Timer_module_factory _timer_module_factory_instance;


Timer_module::Timer_module(Genode::Env &env,
			   Genode::Allocator &md_alloc,
			   Genode::Entrypoint &ep,
			   const char* label,
			   bool &bootstrap,
			   Core_module &core)
  :
    _env(env),
    _md_alloc(md_alloc),
    _ep(ep),
    _bootstrap(bootstrap),
    _core(core)
{

}



void Timer_module::checkpoint(Target_state &state)
{
  #ifdef VERBOSE
  Genode::log("Ckpt::\033[33m", __func__, "\033[0m(...)");
  #endif

  Genode::List<Stored_timer_session_info> &stored_infos = state._stored_timer_sessions;
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
			Genode::addr_t childs_kcap = _core.find_kcap_by_badge(child_info->cap().local_name());
			stored_info = new (state._alloc) Stored_timer_session_info(*child_info, childs_kcap);
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
		  _destroy_stored_timer_session(state, *stored_info);
		}

		stored_info = next_info;
	}
}
void Timer_module::_destroy_stored_timer_session(Target_state &state,
						 Stored_timer_session_info &stored_info)
{
  #ifdef VERBOSE
  Genode::log("Ckpt::\033[33m", __func__, "\033[0m(...)");
  #endif
  
	Genode::destroy(state._alloc, &stored_info);
}



void Timer_module::restore(Target_state &state)
{

}


Genode::Service *Timer_module::resolve_session_request(const char *service_name,
						       const char *args)
{
  if(!_timer_root) {
    _timer_root = new (_md_alloc) Timer_root(_env,
					     _md_alloc,
					     _ep,
					     _bootstrap);

    _timer_service = new (_md_alloc) Genode::Local_service("Timer",
							   _timer_root);
  }

  return _timer_service;
}


