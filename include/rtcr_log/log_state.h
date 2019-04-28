/*
 * \brief  State of log module
 * \author Johannes Fischer
 * \date   2019-04-23
 */

#ifndef _RTCR_LOG_STATE_H_
#define _RTCR_LOG_STATE_H_

/* Genode includes */
#include <util/list.h>

/* Rtcr includes */
#include <rtcr/module_state.h>
#include <rtcr_log/stored_log_session_info.h>


namespace Rtcr {
	class Log_state;
}


class Rtcr::Log_state : public Module_state
{
protected:
	Genode::Allocator &_alloc;
public:
	Log_state(Genode::Allocator &alloc);
	~Log_state();

	Genode::List<Stored_log_session_info> _stored_log_sessions;

	void print(Genode::Output &output) const override;
};

#endif /* _RTCR_LOG_STATE_H_ */
