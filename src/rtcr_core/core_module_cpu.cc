/*
 * \brief  CPU implementation of core module
 * \author Johannes Fischer
 * \date   2019-03-21
 */

#include <rtcr_core/core_module_cpu.h>

using namespace Rtcr;


Core_module_cpu::Core_module_cpu(Genode::Env &env,
				 Genode::Allocator &md_alloc,
				 Genode::Entrypoint &ep)
    :
    _env(env),
    _md_alloc(md_alloc),
    _ep(ep)
{

}

void Core_module_cpu::_initialize_cpu_session(const char* label, bool &bootstrap)
{
#ifdef DEBUG
    Genode::log("\033[36m", __PRETTY_FUNCTION__, "\033[0m");
#endif
    _cpu_root = new (_md_alloc) Cpu_root(_env, _md_alloc, _ep, pd_root(), bootstrap);
    _cpu_service = new (_md_alloc) Genode::Local_service("CPU", _cpu_root);
    _cpu_session = _find_cpu_session(label, cpu_root());  
}

Cpu_session_component *Core_module_cpu::_find_cpu_session(const char *label, Cpu_root &cpu_root)
{
#ifdef DEBUG
    Genode::log("\033[36m", __PRETTY_FUNCTION__, "\033[0m");
#endif
  
    /* Preparing argument string */
    char args_buf[160];
    Genode::snprintf(args_buf, sizeof(args_buf),
		     "priority=0x%x, ram_quota=%u, label=\"%s\"",
		     Genode::Cpu_session::DEFAULT_PRIORITY, 128*1024, label);

    /* Issuing session method of Cpu_root */
    Genode::Session_capability cpu_cap = cpu_root.session(args_buf, Genode::Affinity());

    /* Find created RPC object in Cpu_root's list */
    Cpu_session_component *cpu_session = cpu_root.session_infos().first();
    if(cpu_session) cpu_session = cpu_session->find_by_badge(cpu_cap.local_name());
    if(!cpu_session) {
	Genode::error("Creating custom PD session failed: "
		      "Could not find PD session in PD root");
	throw Genode::Exception();
    }

    return cpu_session;
}


Core_module_cpu::~Core_module_cpu() {
   Genode::destroy(_md_alloc, _cpu_root);
   Genode::destroy(_md_alloc, _cpu_service);
}



void Core_module_cpu::_checkpoint()
{
#ifdef DEBUG
    Genode::log("\033[36m", __PRETTY_FUNCTION__, "\033[0m");
#endif
    Genode::List<Cpu_session_component> &child_infos =  cpu_root().session_infos();
    Genode::List<Stored_cpu_session_info> &stored_infos = state()._stored_cpu_sessions;
    Cpu_session_component *child_info = nullptr;
    Stored_cpu_session_info *stored_info = nullptr;

    /* Update state_info from child_info
     * If a child_info has no corresponding state_info, create it
     */
    child_info = child_infos.first();
    while(child_info) {
	/* Find corresponding state_info */
	stored_info = stored_infos.first();
	if(stored_info) stored_info = stored_info->find_by_badge(child_info->cap().local_name());
	
	/* No corresponding stored_info => create it */
	if(!stored_info) {
	    Genode::addr_t childs_kcap = find_kcap_by_badge(child_info->cap().local_name());
	    stored_info = new (_md_alloc) Stored_cpu_session_info(*child_info, childs_kcap);
	    stored_infos.insert(stored_info);
	}

	/* Update stored_info */
	stored_info->sigh_badge = child_info->parent_state().sigh.local_name();
	_prepare_cpu_threads(stored_info->stored_cpu_thread_infos,
			     child_info->parent_state().cpu_threads);

	child_info = child_info->next();
    }

    /* Delete old stored_infos, if the child misses corresponding infos in its list */
    stored_info = stored_infos.first();
    while(stored_info) {
	Stored_cpu_session_info *next_info = stored_info->next();

	/* Find corresponding child_info */
	child_info = child_infos.first();
	if(child_info) child_info = child_info->find_by_badge(stored_info->badge);

	/* No corresponding child_info => delete it */
	if(!child_info) {
	    stored_infos.remove(stored_info);
	    _destroy_stored_cpu_session(*stored_info);
	}

	stored_info = next_info;
    }
}


void Core_module_cpu::_destroy_stored_cpu_session(Stored_cpu_session_info &stored_info)
{
#ifdef DEBUG
    Genode::log("\033[36m", __PRETTY_FUNCTION__, "\033[0m");
#endif
    while(Stored_cpu_thread_info *info = stored_info.stored_cpu_thread_infos.first()) {
	stored_info.stored_cpu_thread_infos.remove(info);
	_destroy_stored_cpu_thread(*info);
    }
    Genode::destroy(_md_alloc, &stored_info);
}


void Core_module_cpu::_prepare_cpu_threads(Genode::List<Stored_cpu_thread_info> &stored_infos,
					   Genode::List<Cpu_thread_component> &child_infos)
{
#ifdef DEBUG
    Genode::log("\033[36m", __PRETTY_FUNCTION__, "\033[0m");
#endif
    Cpu_thread_component *child_info = nullptr;
    Stored_cpu_thread_info *stored_info = nullptr;

    /* Update state_info from child_info
     * If a child_info has no corresponding state_info, create it
     */

    child_info = child_infos.first();
    while(child_info) {
	/* Find corresponding state_info */
	stored_info = stored_infos.first();
	if(stored_info) stored_info = stored_info->find_by_badge(child_info->cap().local_name());

	/* No corresponding stored_info => create it */
	if(!stored_info) {
	    Genode::addr_t childs_kcap = find_kcap_by_badge(child_info->cap().local_name());
	    stored_info = new (_md_alloc) Stored_cpu_thread_info(*child_info, childs_kcap);
	    stored_infos.insert(stored_info);
	}

	/* Update stored_info */
	stored_info->started = child_info->parent_state().started;
	stored_info->paused = child_info->parent_state().paused;
	stored_info->single_step = child_info->parent_state().single_step;
	stored_info->affinity = child_info->parent_state().affinity;
	stored_info->sigh_badge = child_info->parent_state().sigh.local_name();
	/* XXX does not guarantee to return the current thread registers */
	stored_info->ts = Genode::Cpu_thread_client(child_info->parent_cap()).state();

	child_info = child_info->next();
    }

    /* Delete old stored_infos, if the child misses corresponding infos in its list */
    stored_info = stored_infos.first();
    while(stored_info) {
	    Stored_cpu_thread_info *next_info = stored_info->next();

	    /* Find corresponding child_info */
	    child_info = child_infos.first();
	    if(child_info) child_info = child_info->find_by_badge(stored_info->badge);

	    /* No corresponding child_info => delete it */
	    if(!child_info) {
		stored_infos.remove(stored_info);
		_destroy_stored_cpu_thread(*stored_info);
	    }

	    stored_info = next_info;
	}
}


void Core_module_cpu::_destroy_stored_cpu_thread(Stored_cpu_thread_info &stored_info)
{
    Genode::destroy(_md_alloc, &stored_info);
}


void Core_module_cpu::pause()
{
#ifdef DEBUG
    Genode::log("\033[36m", __PRETTY_FUNCTION__, "\033[0m");
#endif
    /* Iterate through every session */
    Cpu_session_component *cpu_session = _cpu_root->session_infos().first();
    while(cpu_session) {
	/* Iterate through every CPU thread */
	Cpu_thread_component *cpu_thread = _cpu_session->parent_state().cpu_threads.first();
	while(cpu_thread) {
	    /* Pause the CPU thread */
	    Genode::Cpu_thread_client client{cpu_thread->parent_cap()};
	    client.pause();

	    cpu_thread = cpu_thread->next();
	}

	cpu_session = cpu_session->next();
    }
}


void Core_module_cpu::resume()
{
#ifdef DEBUG
    Genode::log("\033[36m", __PRETTY_FUNCTION__, "\033[0m");
#endif
    /* Iterate through every session */
    Cpu_session_component *cpu_session = _cpu_root->session_infos().first();
    while(cpu_session) {
	/* Iterate through every CPU thread */
	Cpu_thread_component *cpu_thread = _cpu_session->parent_state().cpu_threads.first();
	while(cpu_thread) {
	    /* Pause the CPU thread */
	    Genode::Cpu_thread_client client{cpu_thread->parent_cap()};
	    client.resume();

	    cpu_thread = cpu_thread->next();
	    Genode::log("\033[36m", __PRETTY_FUNCTION__, " cpu_thread=",cpu_thread, "\033[0m");    	    
	}

	cpu_session = cpu_session->next();
	    Genode::log("\033[36m", __PRETTY_FUNCTION__, " cpu_session=",cpu_session, "\033[0m");    	    	
    }
    Genode::log("\033[36m", __PRETTY_FUNCTION__, " finished", "\033[0m");    
}



