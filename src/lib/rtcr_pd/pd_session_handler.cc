/*
 * \brief  CPU Session Handler
 * \author Johannes Fischer
 * \date   2019-03-21
 */

#include <rtcr_pd/pd_session_handler.h>


using namespace Rtcr;

// This line registers the Cpu_session_handler during application start.
Rtcr::Pd_session_handler_factory _pd_session_handler_factory;


Pd_session_handler::Pd_session_handler(Genode::Env &env,
				       Genode::Allocator &md_alloc,
				       Genode::Entrypoint &ep,
				       const char* label,
				       bool &bootstrap)
:
  _env(env),
  _md_alloc(md_alloc),
  _ep(ep)

{
  pd_root  = new (_md_alloc) Pd_root(_env,
				     _md_alloc,
				     _ep,
				     bootstrap);
  
  pd_service = new (_md_alloc) Genode::Local_service("PD", pd_root);

  // Preparing argument string
  char args_buf[160];
  Genode::snprintf(args_buf,
		   sizeof(args_buf),
		   "ram_quota=%u, label=\"%s\"",
		   20*1024*sizeof(long),
		   label);

  // Issuing session method of pd_root
  Genode::Session_capability pd_cap = pd_root->session(args_buf, Genode::Affinity());

  // Find created RPC object in pd_root's list
  pd_session = pd_root->session_infos().first();
  if(pd_session) pd_session = pd_session->find_by_badge(pd_cap.local_name());
  if(!pd_session)
    {
      Genode::error("Creating custom PD session failed: "
		    "Could not find PD session in PD root");
      throw Genode::Exception();
    }
}



Pd_session_handler::~Pd_session_handler()
{
  if(pd_root)    Genode::destroy(_md_alloc, pd_root);
  if(pd_service) Genode::destroy(_md_alloc, pd_service);  
}  

void Pd_session_handler::session_init()
{
  Genode::log("CPU_session_handler::session_init()");
}
void Pd_session_handler::session_checkpoint()
{
  Genode::log("CPU_session_handler::session_checkpoint()");
}
void Pd_session_handler::session_restore()
{
  Genode::log("CPU_session_handler::session_restore()");
}


