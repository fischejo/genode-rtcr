/*
 * \brief  State of timer module
 * \author Johannes Fischer
 * \date   2019-04-23
 */

#ifndef _RTCR_TIMER_STATE_H_
#define _RTCR_TIMER_STATE_H_

/* Genode includes */
#include <util/list.h>

/* Rtcr includes */
#include <rtcr/module_state.h>
#include <rtcr_timer/stored_timer_session_info.h>


namespace Rtcr {
    class Timer_state;
}

class Rtcr::Timer_state : public Module_state
{
public:
    Genode::List<Stored_timer_session_info> _stored_timer_sessions;
    
    void print(Genode::Output &output) const override;

    Timer_state();
    ~Timer_state();
};

#endif /* _RTCR_TIMER_STATE_H_ */
