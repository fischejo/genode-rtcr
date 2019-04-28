/*
 * \brief  State of log module
 * \author Johannes Fischer
 * \date   2019-04-23
 */

#include <rtcr_log/log_state.h>

using namespace Rtcr;

Log_state::Log_state()
{

}


Log_state::~Log_state()
{
	// TODO delete all list elements
}


void Log_state::print(Genode::Output &output) const
{
	using Genode::Hex;
	using Genode::print;
	Genode::print(output, "LOG sessions:\n");
	Stored_log_session_info const *log_info = _stored_log_sessions.first();
	if(!log_info) Genode::print(output, " <empty>\n");
	while(log_info) {
		Genode::print(output, " ", *log_info, "\n");
		log_info = log_info->next();
	}
}


