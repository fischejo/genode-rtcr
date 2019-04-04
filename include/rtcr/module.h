/*
 * \brief  Session Handler
 * \author Denis Huber
 * \author Johannes Fischer
 * \date   2019-03-21
 */

#ifndef _RTCR_MODULE_H_
#define _RTCR_MODULE_H_

/* Genode includes */
#include <util/list.h>
#include <base/heap.h>
#include <util/list.h>


namespace Rtcr {
    class Module;
}

class Rtcr::Module : public Genode::List<Module>::Element
{
public:
    virtual void checkpoint(Target_state &state) = 0;
    virtual void restore(Target_state &state) = 0;

  Module *find_by_name(char const* name)
	{
	    if(name == this->name())
		return this;
	    Module *next = next();
	    return next ? next->find_by_name(name) : 0;
	}
};



#endif /* _RTCR_MODULE_H_ */