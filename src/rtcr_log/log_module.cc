/*
 * \brief  Log module
 * \description This module provides a log session
 * \author Johannes Fischer
 * \date   2019-04-19
 */

#include <rtcr_log/log_module.h>

using namespace Rtcr;

/* Create a static instance of the Log_module_factory. This registers the module */

Rtcr::Log_module_factory _log_module_factory_instance;


Log_module::Log_module(Genode::Env &env,
		       Genode::Allocator &md_alloc,
		       Genode::Entrypoint &ep,
		       bool &bootstrap)
  :
    _env(env),
    _md_alloc(md_alloc),
    _ep(ep),
    _bootstrap(bootstrap)
{}


Log_module::~Log_module()
{
    Genode::destroy(_md_alloc, _log_root);
    Genode::destroy(_md_alloc, _log_service);
}


void Log_module::initialize(Genode::List<Module> &modules)
{
  Module *module = modules.first();
  while (!_core_module && module) {
    _core_module = dynamic_cast<Core_module_abstract*>(module);
    module = module->next();
  }

  if(!_core_module)
    Genode::error("No Core_module loaded! ");
}

void Log_module::checkpoint(Target_state &state)
{
#ifdef DEBUG
    Genode::log("\033[36m", __PRETTY_FUNCTION__, "\033[0m");
#endif

  Genode::List<Stored_log_session_info> &stored_infos = state._stored_log_sessions;
  Genode::List<Log_session_component> &child_infos = _log_root->session_infos();
  Log_session_component *child_info = nullptr;
  Stored_log_session_info *stored_info = nullptr;

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
	  stored_info = new (state._alloc) Stored_log_session_info(*child_info, childs_kcap);
	  stored_infos.insert(stored_info);
	}

      /* Nothing to update in stored_info */

      child_info = child_info->next();
    }

  /* Delete old stored_infos, if the child misses corresponding infos in its
     list */
  stored_info = stored_infos.first();
  while(stored_info) {
      Stored_log_session_info *next_info = stored_info->next();

      /* Find corresponding child_info */
      child_info = child_infos.first();
      if(child_info)
	child_info = child_info->find_by_badge(stored_info->badge);

      /* No corresponding child_info => delete it */
      if(!child_info) {
	  stored_infos.remove(stored_info);
	  _destroy_stored_log_session(state, *stored_info);
	}

      stored_info = next_info;
    }
}


void Log_module::_destroy_stored_log_session(Target_state &state,
						  Stored_log_session_info &stored_info)
{
  #ifdef VERBOSE
  Genode::log("Ckpt::\033[33m", __func__, "\033[0m(...)");
  #endif

  Genode::destroy(state._alloc, &stored_info);
}



void Log_module::restore(Target_state &state)
{

}


Genode::Service *Log_module::resolve_session_request(const char *service_name,
						       const char *args)
{
  if(!Genode::strcmp(service_name, "LOG")) {
    if(!_log_root) {
      _log_root = new (_md_alloc) Log_root(_env, _md_alloc, _ep, _bootstrap);
      _log_service = new (_md_alloc) Genode::Local_service("LOG",_log_root);
    }
    return _log_service;
  }
  return 0;
}

