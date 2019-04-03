/*
 * \brief  CPU Session Handler
 * \author Johannes Fischer
 * \date   2019-03-21
 */

#include <rtcr_cpu/cpu_session_handler.h>


using namespace Rtcr;


// This line registers the Cpu_session_handler during application start.
Rtcr::Cpu_session_handler_factory _cpu_session_handler_factory;


Cpu_session_handler::Cpu_session_handler(Genode::Env &env,
					 Genode::Allocator &md_alloc,
					 Genode::Entrypoint &ep,
					 Pd_session_handler &pd_handler,
					 const char* label,
					 bool &bootstrap)
    :
    _env(env),
    _md_alloc(md_alloc),
    _ep(ep),
    _pd_handler(pd_handler),
    _root(env, md_alloc, ep, pd_handler.root(), bootstrap),
    _service("CPU", _root),
    _session(_find_session(label, _root))
{
#ifdef DEBUG
    Genode::log("Ckpt::\033[33m", __func__, "\033[0m(...)");
#endif
}  


Cpu_session_component &Cpu_session_handler::_find_session(const char *label, Cpu_root &cpu_root)
{
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

    return *cpu_session;
}




Cpu_session_handler::~Cpu_session_handler()
{
}


void Cpu_session_handler::prepare_checkpoint(Target_state &state)
{
#ifdef DEBUG    
    Genode::log("CPU_session_handler::prepare_checkpoint()");
#endif
    _pause();
}

void Cpu_session_handler::checkpoint(Target_state &state)
{
#ifdef DEBUG
    Genode::log("CPU_session_handler::session_checkpoint()");
#endif
    _prepare_cpu_sessions(Genode::List<Stored_cpu_session_info> &stored_infos,
						Genode::List<Cpu_session_component> &child_infos)    
}


void Cpu_session_handler::_prepare_cpu_sessions(Target_state &state,
						Genode::List<Cpu_session_component> &child_infos)
{
#ifdef DEBUG
    Genode::log("Ckpt::\033[33m", __func__, "\033[0m(...)");
#endif
    Genode::List<Stored_cpu_session_info> &stored_infos = state._stored_cpu_sessions;
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
	    Genode::addr_t childs_kcap = _pd_handler.find_kcap_by_badge(child_info->cap().local_name());
	    stored_info = new (state._alloc) Stored_cpu_session_info(*child_info, childs_kcap);
	    stored_infos.insert(stored_info);
	}

	/* Update stored_info */
	stored_info->sigh_badge = child_info->parent_state().sigh.local_name();
	_prepare_cpu_threads(state, child_info->parent_state().cpu_threads);

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
	    _destroy_stored_cpu_session(state, *stored_info);
	}

	stored_info = next_info;
    }
}


void Cpu_session_handler::_destroy_stored_cpu_session(Target_state &state,
						      Stored_cpu_session_info &stored_info)
{
#ifdef DEBUG
    Genode::log("Ckpt::\033[33m", __func__, "\033[0m(...)");
#endif

    while(Stored_cpu_thread_info *info = stored_info.stored_cpu_thread_infos.first()) {
	stored_info.stored_cpu_thread_infos.remove(info);
	_destroy_stored_cpu_thread(state, *info);
    }
    Genode::destroy(state._alloc, &stored_info);
}


void Cpu_session_handler::_prepare_cpu_threads(Target_state &state,
					       Genode::List<Cpu_thread_component> &child_infos)
{
#ifdef DEBUG
    Genode::log("Ckpt::\033[33m", __func__, "\033[0m(...)");
#endif
    Genode::List<Stored_cpu_thread_info> &stored_infos = stored_info->stored_cpu_thread_infos;
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
	    Genode::addr_t childs_kcap = pd_handler.find_kcap_by_badge(child_info->cap().local_name());
	    stored_info = new (state._alloc) Stored_cpu_thread_info(*child_info, childs_kcap);
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
    while(stored_info)
	{
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


void Cpu_session_handler::_destroy_stored_cpu_thread(Target_state &state, Stored_cpu_thread_info &stored_info)
{
#ifdef DEBUG
    Genode::log("Ckpt::\033[33m", __func__, "\033[0m(...)");
#endif

    Genode::destroy(state._alloc, &stored_info);
}


void Cpu_session_handler::_pause()
{
#ifdef DEBUG
    Genode::log("Target_child::\033[33m", __func__, "\033[0m()");
#endif

    /* Pause all threads of all sessions */

    /* Iterate through every session */
    Cpu_session_component *cpu_session = _root->session_infos().first();
    while(cpu_session) {
	/* Iterate through every CPU thread */
	Cpu_thread_component *cpu_thread = _session->parent_state().cpu_threads.first();
	while(cpu_thread) {
	    /* Pause the CPU thread */
	    Genode::Cpu_thread_client client{cpu_thread->parent_cap()};
	    client.pause();

	    cpu_thread = cpu_thread->next();
	}

	cpu_session = cpu_session->next();
    }
}


void Cpu_session_handler::_resume()
{
#ifdef DEBUG
    Genode::log("Target_child::\033[33m", __func__, "\033[0m()");
#endif
    /* Pause all threads of all sessions */

    /* Iterate through every session */
    Cpu_session_component *cpu_session = _root->session_infos().first();
    while(cpu_session) {
	/* Iterate through every CPU thread */
	Cpu_thread_component *cpu_thread = _session->parent_state().cpu_threads.first();
	while(cpu_thread) {
	    /* Pause the CPU thread */
	    Genode::Cpu_thread_client client{cpu_thread->parent_cap()};
	    client.resume();

	    cpu_thread = cpu_thread->next();
	}

	cpu_session = cpu_session->next();
    }
}


void Cpu_session_handler::post_checkpoint()
{
#ifdef DEBUG
    Genode::log("Target_child::\033[33m", __func__, "\033[0m()");
#endif    

    _resume();
}


void Cpu_session_handler::session_restore()
{
    Genode::log("CPU_session_handler::session_restore()");
}






