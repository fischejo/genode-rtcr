/*
 * \brief  Container for all module states
 * \author Johannes Fischer
 * \date   2019-03-23
 */

#ifndef _RTCR_TARGET_STATE_H_
#define _RTCR_TARGET_STATE_H_

/* Genode includes */
#include <util/list.h>

/* Rtcr includes */
#include <rtcr/module_state.h>
#include <rtcr/module.h>

namespace Rtcr {
    class Target_state;
}

using namespace Rtcr;

class Rtcr::Target_state
{

private:

  class Module_state_container : public Genode::List<Module_state_container>::Element
     {
     public:
       Module_state &state;
       Module_name name;
       
       Module_state_container(Module_state &_state, Module_name _name)
	 :
	 state(_state),
	 name(_name)
       {}
     };

  Genode::List<Module_state_container> _containers;
     
public:

    
    Genode::Env       &_env;
    Genode::Allocator &_alloc;

    Target_state(Genode::Env &env, Genode::Allocator &alloc);
    ~Target_state();
    
    void print(Genode::Output &output) const;
    Module_state *state(Module_name name);
    void store(Module_name name, Module_state &state);
    
};

#endif /* _RTCR_TARGET_STATE_H_ */
