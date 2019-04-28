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

/**
 * Container for a checkpointed state of a module
 */
class Rtcr::Module_state
{
public:
	/**
	 * Print the internal data structures of this Module_state.
	 *
	 * Usage:
	 * ```C++
	 * Module_state &state = new Module_state();
	 * Genode::log(state);
	 */
	virtual void print(Genode::Output &output) const = 0;
};

#endif /* _RTCR_MODULE_STATE_H_ */
