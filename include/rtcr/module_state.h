/*
 * \brief  State of a module
 * \author Johannes Fischer
 * \date   2019-04-23
 */

#ifndef _RTCR_MODULE_STATE_H_
#define _RTCR_MODULE_STATE_H_

/* Rtcr includes */
#include <rtcr/module.h>
#include <util/string.h>

namespace Rtcr {
    class Module_state;
}


class Rtcr::Module_state
{
public:
    virtual void print(Genode::Output &output) const = 0;
};

#endif /* _RTCR_MODULE_STATE_H_ */
