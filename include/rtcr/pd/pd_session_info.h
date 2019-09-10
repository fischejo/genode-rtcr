/*
 * \brief  Intercepting Pd session
 * \author Denis Huber
 * \author Johannes Fischer
 * \date   2019-08-29
 */

#ifndef _RTCR_PD_SESSION_INFO_H_
#define _RTCR_PD_SESSION_INFO_H_


/* Rtcr includes */
#include <rtcr/pd/native_capability_info.h>
#include <rtcr/pd/signal_context_info.h>
#include <rtcr/pd/signal_source_info.h>
#include <rtcr/rm/region_map_info.h>


namespace Rtcr {
	class Pd_session_info;
}

class Rtcr::Pd_session_info : public Session_info {
public:
	Signal_source_info* i_signal_sources;
	Signal_context_info* i_signal_contexts;
	Native_capability_info* i_native_caps;

	
	Region_map_info *i_address_space;
	Region_map_info *i_stack_area;
	Region_map_info *i_linker_area;

	
	Pd_session_info(const char* creation_args, Genode::uint16_t badge)
	 : Session_info(creation_args, badge) {}

	Pd_session_info() {}
	
	void print(Genode::Output &output) const {
		Genode::print(output, " PD session:\n  ");
		Session_info::print(output);
		Genode::print(output, "\n");
		
		/* Signal contexts */
		Genode::print(output, "  Signal contexts:\n");
		Signal_context_info *context = i_signal_contexts;
		if(!context) Genode::print(output, "   <empty>\n");
		while(context) {
			Genode::print(output, "   ", *context, "\n");
			context = context->next();
		}

		/* Signal sources */
		Genode::print(output, "  Signal sources:\n");
		Signal_source_info *source = i_signal_sources;
		if(!source) Genode::print(output, "   <empty>\n");
		while(source) {
			Genode::print(output, "   ", *source, "\n");
			source = source->next();
		}

		/* Native capabilities */
		Genode::print(output, "  Native Capabilities:\n");
		Native_capability_info *nc = i_native_caps;
		if(!nc) Genode::print(output, "   <empty>\n");
		while(nc) {
			Genode::print(output, "   ", *nc, "\n");
			nc = nc->next();
		}

		/* Address space */
		Genode::print(output, "  Address space: \n");
		Genode::print(output, "   ", *i_address_space);
		Genode::print(output, "  Stack area: \n");		
		Genode::print(output, "   ", *i_stack_area);		
		Genode::print(output, "  Linker area: \n");
		Genode::print(output, "   ", *i_linker_area);				
	}
};


#endif /* _RTCR_PD_SESSION_INFO_H_ */
