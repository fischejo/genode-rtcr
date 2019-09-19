/*
 * \brief Base module
 * \author Johannes Fischer
 * \date   2019-08-29
 */
#include <rtcr/child_info.h>

using namespace Rtcr;

	
Child_info *Child_info::find_by_name(const char *_name)
{
	if(!Genode::strcmp(name.string(), _name))
		return this;
	Child_info *obj = next();
	return obj ? obj->find_by_name(_name) : 0;
};

bool Child_info::child_destroyed()
{
	return (!cpu_session || !pd_session);
}


void Child_info::print(Genode::Output &output) const
{
	Genode::print(output, "Child: ",name,"\n");
	Genode::print(output, *pd_session);
	Genode::print(output, *cpu_session);

	/* (optional) RM session */
	if(rm_session) Genode::print(output, *rm_session);
	else Genode::print(output, " RM session: <empty>\n");

	/* (optional) LOG session */
	if(log_session) Genode::print(output, *log_session);
	else Genode::print(output, " LOG session: <empty>\n");

	/* (optional) Timer session */
	if(timer_session) Genode::print(output, *timer_session);
	else Genode::print(output, " Timer session: <empty>\n");

	/* (optional) Rom session */	
	if(rom_session) Genode::print(output, *rom_session);
	else Genode::print(output, " ROM session: <empty>\n");

	/* Capabilities */
	Genode::print(output, *capability_mapping);
}	

