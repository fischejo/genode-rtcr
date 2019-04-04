
void Checkpointer::_prepare_timer_sessions(Genode::List<Stored_timer_session_info> &stored_infos,
		Genode::List<Timer_session_component> &child_infos)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(...)");

	Timer_session_component *child_info = nullptr;
	Stored_timer_session_info *stored_info = nullptr;

	// Update state_info from child_info
	// If a child_info has no corresponding state_info, create it
	child_info = child_infos.first();
	while(child_info)
	{
		// Find corresponding state_info
		stored_info = stored_infos.first();
		if(stored_info) stored_info = stored_info->find_by_badge(child_info->cap().local_name());

		// No corresponding stored_info => create it
		if(!stored_info)
		{
			Genode::addr_t childs_kcap = _find_kcap_by_badge(child_info->cap().local_name());
			stored_info = new (_state._alloc) Stored_timer_session_info(*child_info, childs_kcap);
			stored_infos.insert(stored_info);
		}

		// Update stored_info
		stored_info->sigh_badge = child_info->parent_state().sigh.local_name();
		stored_info->timeout = child_info->parent_state().timeout;
		stored_info->periodic = child_info->parent_state().periodic;

		child_info = child_info->next();
	}

	// Delete old stored_infos, if the child misses corresponding infos in its list
	stored_info = stored_infos.first();
	while(stored_info)
	{
		Stored_timer_session_info *next_info = stored_info->next();

		// Find corresponding child_info
		child_info = child_infos.first();
		if(child_info) child_info = child_info->find_by_badge(stored_info->badge);

		// No corresponding child_info => delete it
		if(!child_info)
		{
			stored_infos.remove(stored_info);
			_destroy_stored_timer_session(*stored_info);
		}

		stored_info = next_info;
	}
}
void Checkpointer::_destroy_stored_timer_session(Stored_timer_session_info &stored_info)
{
	if(verbose_debug) Genode::log("Ckpt::\033[33m", __func__, "\033[0m(...)");

	Genode::destroy(_state._alloc, &stored_info);
}
