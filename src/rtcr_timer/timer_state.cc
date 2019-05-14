/*
 * \brief  State of timer module
 * \author Johannes Fischer
 * \date   2019-04-23
 */

#include <rtcr_timer/timer_state.h>

using namespace Rtcr;

Timer_state::Timer_state(Genode::Allocator &alloc)
	: _alloc(alloc)
{}


Timer_state::~Timer_state()
{
	while(Stored_timer_session_info *timer_info = _stored_timer_sessions.first()) {
		_stored_timer_sessions.remove(timer_info);
		Genode::destroy(_alloc, timer_info);
	}	
}


void Timer_state::print(Genode::Output &output) const
{
	using Genode::Hex;
	using Genode::print;

	Genode::print(output, "Timer sessions:\n");
	Stored_timer_session_info const *timer_info = _stored_timer_sessions.first();
	if(!timer_info) Genode::print(output, " <empty>\n");
	while(timer_info) {
		Genode::print(output, " ", *timer_info, "\n");
		timer_info = timer_info->next();
	}
}

