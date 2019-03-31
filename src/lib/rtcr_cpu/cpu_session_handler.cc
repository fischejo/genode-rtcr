/*
 * \brief  CPU Session Handler
 * \author Johannes Fischer
 * \date   2019-03-21
 */

#include <rtcr_cpu/cpu_session_handler.h>


using namespace Rtcr;


// This line registers the Cpu_session_handler during application start.
Rtcr::Cpu_session_handler_factory _cpu_session_handler_factory;


Cpu_session_handler::Cpu_session_handler(Genode::Env &env,
					 Genode::Allocator &md_alloc,
					 Genode::Entrypoint &ep,
					 Pd_root &pd_root,
					 const char* label,
					 bool &bootstrap)
  :
  _env(env),
  _md_alloc(md_alloc),
  _ep(ep)
{
  cpu_root = new (_md_alloc) Cpu_root(_env,
				      _md_alloc,
				      _ep,
				      pd_root,
				      bootstrap);
	
  cpu_service = new (_md_alloc) Genode::Local_service("CPU", cpu_root);


  // Preparing argument string
  char args_buf[160];
  Genode::snprintf(args_buf, sizeof(args_buf),
		   "priority=0x%x, ram_quota=%u, label=\"%s\"",
		   Genode::Cpu_session::DEFAULT_PRIORITY, 128*1024, label);

  // Issuing session method of Cpu_root
  Genode::Session_capability cpu_cap = cpu_root->session(args_buf, Genode::Affinity());

  // Find created RPC object in Cpu_root's list
  cpu_session = cpu_root->session_infos().first();
  if(cpu_session) cpu_session = cpu_session->find_by_badge(cpu_cap.local_name());
  if(!cpu_session)
    {
      Genode::error("Creating custom CPU session failed: Could not"
		    " find PD session in PD root");
      throw Genode::Exception();
    }

}  


Cpu_session_handler::~Cpu_session_handler()
{
  if(cpu_root)    Genode::destroy(_md_alloc, cpu_root);
  if(cpu_service) Genode::destroy(_md_alloc, cpu_service);  
}



void Cpu_session_handler::session_init()
{
  Genode::log("CPU_session_handler::session_init()");
}
void Cpu_session_handler::session_checkpoint()
{
  Genode::log("CPU_session_handler::session_checkpoint()");
}
void Cpu_session_handler::session_restore()
{
  Genode::log("CPU_session_handler::session_restore()");
}






