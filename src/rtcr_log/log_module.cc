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
		       Genode::Allocator &alloc,
		       Genode::Entrypoint &ep,
		       bool &bootstrap)
  :
    _env(env),
    _alloc(alloc),
    _ep(ep),
    _bootstrap(bootstrap),
    _log_state(_initialize_state(alloc))
{}


Log_module::~Log_module()
{
  if(_log_root) Genode::destroy(_alloc, _log_root);
  if(_log_service) Genode::destroy(_alloc, _log_service);
}


Log_state &Log_module::_initialize_state(Genode::Allocator &alloc)
{
  return *new(alloc) Log_state();
}


void Log_module::initialize(Genode::List<Module> &modules)
{
#ifdef DEBUG
    Genode::log("\033[36m", __PRETTY_FUNCTION__, "\033[0m");
#endif
  
  Module *module = modules.first();
  while (!_core_module && module) {
    _core_module = dynamic_cast<Core_module_abstract*>(module);
    module = module->next();
  }

  if(!_core_module)
    Genode::error("No Core_module loaded! ");
}


Module_state *Log_module::checkpoint()
{
#ifdef DEBUG
    Genode::log("\033[36m", __PRETTY_FUNCTION__, "\033[0m");
#endif

  Genode::List<Stored_log_session_info> &stored_infos = _log_state._stored_log_sessions;
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
	  stored_info = new (_alloc) Stored_log_session_info(*child_info, childs_kcap);
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
	  _destroy_stored_log_session(*stored_info);
	}

      stored_info = next_info;
    }
  return &_log_state;
}


void Log_module::_destroy_stored_log_session(Stored_log_session_info &stored_info)
{
#ifdef DEBUG
    Genode::log("\033[36m", __PRETTY_FUNCTION__, "\033[0m");
#endif
  Genode::destroy(_alloc, &stored_info);
}



void Log_module::restore(Module_state *state)
{
#ifdef DEBUG
    Genode::log("\033[36m", __PRETTY_FUNCTION__, "\033[0m");
#endif

}


Genode::Service *Log_module::resolve_session_request(const char *service_name,
						       const char *args)
{
#ifdef DEBUG
    Genode::log("\033[36m", __PRETTY_FUNCTION__, "\033[0m");
#endif
  
  if(!Genode::strcmp(service_name, "LOG")) {
    if(!_log_root) {
      _log_root = new (_alloc) Log_root(_env, _alloc, _ep, _bootstrap);
      _log_service = new (_alloc) Genode::Local_service("LOG",_log_root);
    }
    return _log_service;
  }
  return 0;
}

