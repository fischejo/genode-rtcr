/*
 * \brief  State of core module
 * \author Johannes Fischer
 * \date   2019-04-23
 */

#include <rtcr_core/core_state.h>

using namespace Rtcr;


Core_state::Core_state()
    :
    _cap_idx_alloc_addr(0)
{ }


Core_state::~Core_state()
{
    // TODO delete all list elements
}

void Core_state::print(Genode::Output &output) const
{
    _print_pd(output);
    _print_cpu(output);
    _print_ram(output);
    _print_rom(output);
    _print_rm(output);
}



void Core_state::_print_pd(Genode::Output &output) const
{
    using Genode::Hex;
    using Genode::print;
  
    Genode::print(output, "PD sessions:\n");
    Stored_pd_session_info const *pd_info = _stored_pd_sessions.first();
    if(!pd_info) Genode::print(output, " <empty>\n");
    while(pd_info) {
	print(output, " ", *pd_info, "\n");

	// Signal contexts
	print(output, "  Signal contexts:\n");
	Stored_signal_context_info const *context_info = pd_info->stored_context_infos.first();
	if(!context_info) print(output, "   <empty>\n");
	while(context_info) {
	    print(output, "   ", *context_info, "\n");
	    context_info = context_info->next();
	}

	// Signal sources
	print(output, "  Signal sources:\n");
	Stored_signal_source_info const *source_info = pd_info->stored_source_infos.first();
	if(!source_info) print(output, "   <empty>\n");
	while(source_info) {
	    print(output, "   ", *source_info, "\n");
	    source_info = source_info->next();
	}

	// Address space
	Stored_region_map_info const &address_space_info = pd_info->stored_address_space;
	print(output, "  Address space: ", address_space_info,"\n");
	Stored_attached_region_info const *attached_info = address_space_info.stored_attached_region_infos.first();
	if(!attached_info) print(output, "   <empty>\n");
	while(attached_info) {
	    print(output, "   ", *attached_info, "\n");
	    attached_info = attached_info->next();
	}

	// Stack area
	Stored_region_map_info const &stack_area_info = pd_info->stored_stack_area;
	print(output, "  Stack area: ", stack_area_info,"\n");
	attached_info = stack_area_info.stored_attached_region_infos.first();
	if(!attached_info) print(output, "   <empty>\n");
	while(attached_info) {
	    print(output, "   ", *attached_info, "\n");
	    attached_info = attached_info->next();
	}

	// Linker area
	Stored_region_map_info const &linker_area_info = pd_info->stored_linker_area;
	print(output, "  Linker area: ", linker_area_info,"\n");
	attached_info = linker_area_info.stored_attached_region_infos.first();
	if(!attached_info) print(output, "   <empty>\n");
	while(attached_info) {
	    print(output, "   ", *attached_info, "\n");
	    attached_info = attached_info->next();
	}

	pd_info = pd_info->next();
    }
  
}

void Core_state::_print_cpu(Genode::Output &output) const
{
    using Genode::Hex;
    using Genode::print;
  
    print(output, "CPU sessions:\n");
    Stored_cpu_session_info const *cpu_info = _stored_cpu_sessions.first();
    if(!cpu_info) print(output, " <empty>\n");
    while(cpu_info) {
	print(output, " ", *cpu_info, "\n");

	Stored_cpu_thread_info const *cpu_thread_info = cpu_info->stored_cpu_thread_infos.first();
	if(!cpu_thread_info) print(output, "  <empty>\n");
	while(cpu_thread_info) {
	    print(output, "  ", *cpu_thread_info, "\n");
	    cpu_thread_info = cpu_thread_info->next();
	}

	cpu_info = cpu_info->next();
    }
  
}

void Core_state::_print_ram(Genode::Output &output) const
{
    using Genode::Hex;
    using Genode::print;

    Genode::print(output, "RAM sessions:\n");
    Stored_ram_session_info const *ram_info = _stored_ram_sessions.first();
    if(!ram_info) print(output, " <empty>\n");
    while(ram_info) {
	print(output, " ", *ram_info, "\n");

	Stored_ram_dataspace_info const *ramds_info = ram_info->stored_ramds_infos.first();
	if(!ramds_info) print(output, "  <empty>\n");
	while(ramds_info) {
	    print(output, "  ", *ramds_info, "\n");
	    ramds_info = ramds_info->next();
	}

	ram_info = ram_info->next();
    }
  
}

void Core_state::_print_rom(Genode::Output &output) const
{
    using Genode::Hex;
    using Genode::print;

    Genode::print(output, "ROM sessions:\n");
    Stored_rom_session_info const *rom_info = _stored_rom_sessions.first();
    if(!rom_info) Genode::print(output, " <empty>\n");
    while(rom_info) {
	Genode::print(output, " ", *rom_info, "\n");
	rom_info = rom_info->next();
    }
  
}

void Core_state::_print_rm(Genode::Output &output) const
{
    using Genode::Hex;
    using Genode::print;

    Genode::print(output, "RM sessions:\n");
    Stored_rm_session_info const *rm_info = _stored_rm_sessions.first();
    if(!rm_info) Genode::print(output, " <empty>\n");
    while(rm_info) {
	Genode::print(output, " ", *rm_info, "\n");
	Stored_region_map_info const *region_map_info = rm_info->stored_region_map_infos.first();
	if(!region_map_info) Genode::print(output, "  <empty>\n");
	while(region_map_info) {
	    Genode::print(output, "  ", *region_map_info, "\n");
	    Stored_attached_region_info const *attached_info =
		region_map_info->stored_attached_region_infos.first();
	    if(!attached_info) Genode::print(output, "   <empty>\n");
	    while(attached_info)
		{
		    Genode::print(output, "   ", *attached_info, "\n");
		    attached_info = attached_info->next();
		}
	    region_map_info = region_map_info->next();
	}
	rm_info = rm_info->next();
    }
  
}


Genode::Dataspace_capability Core_state::find_stored_dataspace(Genode::uint16_t badge)
{
    Genode::Dataspace_capability result;

    // RAM dataspace
    result = _find_stored_dataspace(badge, _stored_ram_sessions);
    if(result.valid()) return result;

    // Attached region in PD
    result = _find_stored_dataspace(badge, _stored_pd_sessions);
    if(result.valid()) return result;

    // Attached region in RM
    result = _find_stored_dataspace(badge, _stored_rm_sessions);
    if(result.valid()) return result;

    return result;
}


Genode::Dataspace_capability Core_state::_find_stored_dataspace(Genode::uint16_t badge,
								Genode::List<Stored_ram_session_info> &state_infos)
{
    Genode::Dataspace_capability result;

    Stored_ram_session_info *session_info = state_infos.first();
    while(session_info) {
	Stored_ram_dataspace_info *ramds_info = session_info->stored_ramds_infos.first();
	if(ramds_info)
	    ramds_info = ramds_info->find_by_badge(badge);

	if(ramds_info)
	    return ramds_info->memory_content;

	session_info = session_info->next();
    }

    return result;
}


Genode::Dataspace_capability Core_state::_find_stored_dataspace(Genode::uint16_t badge,
								Genode::List<Stored_pd_session_info> &state_infos)
{
    Genode::Dataspace_capability result;

    Stored_pd_session_info *session_info = state_infos.first();
    while(session_info) {
	result = _find_stored_dataspace(badge, session_info->stored_address_space.stored_attached_region_infos);
	if(result.valid())
	    return result;

	result = _find_stored_dataspace(badge, session_info->stored_stack_area.stored_attached_region_infos);
	if(result.valid())
	    return result;

	result = _find_stored_dataspace(badge, session_info->stored_linker_area.stored_attached_region_infos);
	if(result.valid())
	    return result;

	session_info = session_info->next();
    }

    return result;
}


Genode::Dataspace_capability Core_state::_find_stored_dataspace(Genode::uint16_t badge,
								Genode::List<Stored_rm_session_info> &state_infos)
{
    Genode::Dataspace_capability result;

    Stored_rm_session_info *session_info = state_infos.first();
    while(session_info) {
	Stored_region_map_info *rm_info = session_info->stored_region_map_infos.first();
	while(rm_info) {
	    result = _find_stored_dataspace(badge, rm_info->stored_attached_region_infos);
	    if(result.valid())
		return result;

	    rm_info = rm_info->next();
	}

	session_info = session_info->next();
    }
    return result;
}


Genode::Dataspace_capability Core_state::_find_stored_dataspace(Genode::uint16_t badge,
								Genode::List<Stored_attached_region_info> &state_infos)
{
    Genode::Dataspace_capability result;

    Stored_attached_region_info *info = state_infos.first();
    if(info)
	info = info->find_by_badge(badge);

    if(info)
	return info->memory_content;

    return result;
}

