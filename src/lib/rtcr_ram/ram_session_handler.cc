/*
 * \brief  RAM Session Handler
 * \author Johannes Fischer
 * \date   2019-03-21
 */

#include <rtcr_ram/ram_session_handler.h>


using namespace Rtcr;

// This line registers the Cpu_session_handler during application start.
Rtcr::Ram_session_handler_factory _ram_session_handler_factory;


Ram_session_handler::Ram_session_handler(Genode::Env &env,
					 Genode::Allocator &md_alloc,
					 Genode::Entrypoint &ep,
					 const char* label,
					 bool &bootstrap)
  :
  _env(env),
  _md_alloc(md_alloc),
  _ep(ep)
{
	ram_root = new (_md_alloc) Ram_root(_env,
					    _md_alloc,
					    _ep,
					    0, // granularity
					    bootstrap);
	
	ram_service = new (_md_alloc) Genode::Local_service("RAM", ram_root);


	// Preparing argument string
	char args_buf[160];
	Genode::snprintf(args_buf, sizeof(args_buf),
			 "ram_quota=%u, phys_start=0x%lx, phys_size=0x%lx, label=\"%s\"",
			 4*1024*sizeof(long), 0UL, 0UL, label);

	// Issuing session method of Ram_root
	Genode::Session_capability ram_cap = ram_root->session(args_buf, Genode::Affinity());

	// Find created RPC object in Ram_root's list
	ram_session = ram_root->session_infos().first();
	if(ram_session) ram_session = ram_session->find_by_badge(ram_cap.local_name());
	if(!ram_session)
	  {
	    Genode::error("Creating custom PD session failed: "
			  "Could not find PD session in PD root");
	    throw Genode::Exception();
	  }
}  

Ram_session_handler::~Ram_session_handler()
{
  if(ram_root)    Genode::destroy(_md_alloc, ram_root);
  if(ram_service) Genode::destroy(_md_alloc, ram_service);
}




void Ram_session_handler::session_init()
{
  Genode::log("Ram_session_handler::session_init()");
}
void Ram_session_handler::session_checkpoint()
{
  Genode::log("Ram_session_handler::session_checkpoint()");
}
void Ram_session_handler::session_restore()
{
  Genode::log("Ram_session_handler::session_restore()");
}


