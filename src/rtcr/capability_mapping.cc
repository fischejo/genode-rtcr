/*
 * \brief  Checkpointing capabilities
 * \author Denis Huber
 * \author Johannes Fischer
 * \date   2019-08-29
 */


#include <base/internal/cap_map.h>
#include <base/internal/cap_alloc.h>

#include <rtcr/cap/capability_mapping.h>

#ifdef PROFILE
#include <util/profiler.h>
#define PROFILE_THIS_CALL PROFILE_FUNCTION("blue");
#else
#define PROFILE_THIS_CALL
#endif

#if DEBUG 
#define DEBUG_THIS_CALL Genode::log("\e[38;5;27m", __PRETTY_FUNCTION__, "\033[0m");
#else
#define DEBUG_THIS_CALL
#endif


using namespace Rtcr;


Capability_mapping::Capability_mapping(Genode::Env &env,
									   Genode::Allocator &alloc,
									   Pd_session &pd_session,			   				     Genode::Xml_node *config)
	:
	Checkpointable(env, config, "capability_mapping"),
	_env(env),
	_alloc (alloc),
	_pd_session(pd_session)
{
	DEBUG_THIS_CALL
}

Capability_mapping::~Capability_mapping()
{
	while(Kcap_badge_info *kcap = _kcap_mapping.first()) {
		Genode::destroy(_alloc, kcap);
	}
}


void Capability_mapping::checkpoint()
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL
		
	using Genode::log;
	using Genode::Hex;
	using Genode::addr_t;
	using Genode::size_t;
	using Genode::uint16_t;

	/* TODO FJO: the list is just on the stack!!! */
	Genode::List<Kcap_badge_info> result;

	/* Retrieve cap_idx_alloc_addr */
	Genode::Pd_session_client pd_client(_pd_session.parent_cap());
	addr_t const cap_idx_alloc_addr = Genode::Foc_native_pd_client(
		pd_client.native_pd()).cap_map_info();
	_cap_idx_alloc_addr = cap_idx_alloc_addr;

#ifdef DEBUG
	Genode::log("Address of cap_idx_alloc = ", Hex(cap_idx_alloc_addr));
#endif
	
	/* Find child's dataspace containing the capability map
	 * It is found via cap_idx_alloc_addr */
	Region_map &addr_space = _pd_session.address_space_component();

	/* This lock is necessary as the rm_session is also moving items from
	   new_attached_regions to ck_attached_regions during checkpointing */
	Attached_region_info *ar_info = nullptr;

	// TODO FJO: accesing attached_regions is not thread safe
	ar_info  =addr_space.attached_regions().first();
	if(ar_info) ar_info = ar_info->find_by_addr(cap_idx_alloc_addr);

	if(!ar_info) {
		Genode::error("No dataspace found for cap_idx_alloc's datastructure at ",
					  Hex(cap_idx_alloc_addr));
		throw Genode::Exception();
	}
	
	/* Create new badge_kcap list */
	size_t const struct_size    = sizeof(Genode::Cap_index_allocator_tpl<Genode::Cap_index,4096>);
	size_t const array_ele_size = sizeof(Genode::Cap_index);
	size_t const array_size     = array_ele_size*4096;

	addr_t const child_ds_start     = ar_info->rel_addr;
	addr_t const child_ds_end       = child_ds_start + ar_info->size;
	addr_t const child_struct_start = cap_idx_alloc_addr;
	addr_t const child_struct_end   = child_struct_start + struct_size;
	addr_t const child_array_start  = child_struct_start + 8;
	addr_t const child_array_end    = child_array_start + array_size;

	addr_t const local_ds_start     = _env.rm().attach(ar_info->attached_ds_cap, ar_info->size, ar_info->offset);
	addr_t const local_ds_end       = local_ds_start + ar_info->size;
	addr_t const local_struct_start = local_ds_start + (cap_idx_alloc_addr - child_ds_start);
	addr_t const local_struct_end   = local_struct_start + struct_size;
	addr_t const local_array_start  = local_struct_start + 8;
	addr_t const local_array_end    = local_array_start + array_size;

#ifdef DEBUG
	log("child_ds_start:     ", Hex(child_ds_start));
	log("child_struct_start: ", Hex(child_struct_start));
	log("child_array_start:  ", Hex(child_array_start));
	log("child_array_end:    ", Hex(child_array_end));
	log("child_struct_end:   ", Hex(child_struct_end));
	log("child_ds_end:       ", Hex(child_ds_end));

	log("local_ds_start:     ", Hex(local_ds_start));
	log("local_struct_start: ", Hex(local_struct_start));
	log("local_array_start:  ", Hex(local_array_start));
	log("local_array_end:    ", Hex(local_array_end));
	log("local_struct_end:   ", Hex(local_struct_end));
	log("local_ds_end:       ", Hex(local_ds_end));
#endif 

	//dump_mem((void*)local_array_start, 0x1200);

	enum { UNUSED = 0, INVALID_ID = 0xffff };
	for(addr_t curr = local_array_start; curr < local_array_end; curr += array_ele_size) {

		size_t const badge_offset = 6;

		/* Convert address to pointer and dereference it */
		uint16_t const badge = *(uint16_t*)(curr + badge_offset);
		/* Current capability map slot = Compute distance from start to current
		 * address of array and divide it by the element size; kcap = current
		 * capability map slot shifted by 12 bits to the left (last 12 bits are
		 * used by Fiasco.OC for parameters for IPC calls) */
		addr_t const kcap  = ((curr - local_array_start) / array_ele_size) << 12;

		if(badge != UNUSED && badge != INVALID_ID) {
			Kcap_badge_info *state_info = new (_alloc) Kcap_badge_info(kcap, badge);
			result.insert(state_info);

#ifdef DEBUG
			log("+ ", Hex(kcap), ": ", badge, " (", Hex(badge), ")");
#endif
		}
	}

	_env.rm().detach(local_ds_start);


#ifdef DEBUG
	Genode::log("Capability map:");
	Kcap_badge_info const *info = result.first();
	if(!info) Genode::log(" <empty>\n");
	while(info)
	{
		Genode::log(" ", *info);
		info = info->next();
	}
#endif

	_kcap_mapping = result;
}

Genode::addr_t Capability_mapping::find_kcap_by_badge(Genode::uint16_t badge)
{
	Genode::addr_t kcap = 0;

	Kcap_badge_info *info = _kcap_mapping.first();
	if(info) info = info->find_by_badge(badge);
	if(info) kcap = info->kcap;
	return kcap;
}
