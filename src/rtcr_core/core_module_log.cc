/*
 * \brief  Log implementation of core module
 * \author Johannes Fischer
 * \date   2019-04-12
 */

#include <rtcr_core/core_module_log.h>

using namespace Rtcr;


Core_module_log::Core_module_log(Genode::Env &env,
				 Genode::Allocator &md_alloc,
				 Genode::Entrypoint &ep)
  :
  _md_alloc(md_alloc),
  _env(env),
  _ep(ep)
{
}

void Core_module_log::_init(const char* label, bool &bootstrap)
{
#ifdef DEBUG
    Genode::log("\033[36m", __PRETTY_FUNCTION__, "\033[0m");
#endif
  
  _log_root = new (_md_alloc) Log_root(_env, _md_alloc, _ep, bootstrap);
  _log_service = new (_md_alloc) Genode::Local_service("LOG", _log_root);
}


Core_module_log::~Core_module_log()
{
    Genode::destroy(_md_alloc, _log_root);
    Genode::destroy(_md_alloc, _log_service);
}


void Core_module_log::_checkpoint(Target_state &state)
{
#ifdef DEBUG
    Genode::log("\033[36m", __PRETTY_FUNCTION__, "\033[0m");
#endif

  Genode::List<Stored_log_session_info> &stored_infos = state._stored_log_sessions;
  Genode::List<Log_session_component> &child_infos = log_root().session_infos();
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
	  Genode::addr_t childs_kcap = find_kcap_by_badge(child_info->cap().local_name());
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


void Core_module_log::_destroy_stored_log_session(Target_state &state,
						  Stored_log_session_info &stored_info)
{
  #ifdef VERBOSE
  Genode::log("Ckpt::\033[33m", __func__, "\033[0m(...)");
  #endif

  Genode::destroy(state._alloc, &stored_info);
}
