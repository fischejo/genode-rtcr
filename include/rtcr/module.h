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
#include <base/service.h>
#include <rtcr/module_state.h>
#include <rtcr/module.h>

namespace Rtcr {
    class Module;
    typedef Genode::String<16> Module_name;

/* forward declaration */
class Module_state;
  
}

using namespace Rtcr;



class Rtcr::Module : public Genode::List<Module>::Element
{
public:
    virtual Module_name name() = 0;
  
    virtual void initialize(Genode::List<Module> &modules) {};
    virtual Module_state *checkpoint() = 0;
    virtual void restore(Module_state *state) = 0;

    virtual Genode::Service *resolve_session_request(const char *service_name, const char *args) = 0;
    /*
      Module *find_by_name(char const* name)
      {
      if(name == this->name())
      return this;
      Module *next = next();
      return next ? next->find_by_name(name) : 0;
      }
    */
};



#endif /* _RTCR_MODULE_H_ */
