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
                                       Pd_session *pd_session)
	:
	Checkpointable(env, "capability_mapping"),
	_env(env),
	_alloc (alloc),
	_pd_session(pd_session)
{
	DEBUG_THIS_CALL
		}

Capability_mapping::~Capability_mapping(){}


void Capability_mapping::checkpoint()
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL;

	using Genode::log;
	using Genode::Hex;
	using Genode::addr_t;
	using Genode::size_t;
	using Genode::uint16_t;


	index = 0;

	/* Retrieve cap_idx_alloc_addr */
	// commented due to porting to Genode 18.02
	//	Genode::Pd_session_client pd_client(_pd_session->parent_cap());
	//	addr_t const cap_idx_alloc_addr = Genode::Foc_native_pd_client(
	//		pd_client.native_pd()).cap_map_info();
	addr_t const cap_idx_alloc_addr = 0xc0198;

	_cap_idx_alloc_addr = cap_idx_alloc_addr;


#ifdef DEBUG
	Genode::log("Address of cap_idx_alloc = ", Hex(cap_idx_alloc_addr));
#endif

	/* Find child's dataspace containing the capability map
	 * It is found via cap_idx_alloc_addr */
	Attached_region *ar = _pd_session->address_space_component()
		.find_attached_region_by_addr(cap_idx_alloc_addr);

	if(!ar) {
		Genode::error("No dataspace found for cap_idx_alloc's datastructure at ",
		              Hex(cap_idx_alloc_addr));
		throw Genode::Exception();
	}

	/* Create new badge_kcap list */
	size_t const struct_size    = sizeof(Genode::Cap_index_allocator_tpl<Genode::Cap_index,4096>);
	size_t const array_ele_size = sizeof(Genode::Cap_index);
	size_t const array_size     = array_ele_size*4096;

	addr_t const child_ds_start     = ar->i_rel_addr;
	addr_t const child_ds_end       = child_ds_start + ar->i_size;
	addr_t const child_struct_start = cap_idx_alloc_addr;
	addr_t const child_struct_end   = child_struct_start + struct_size;
	addr_t const child_array_start  = child_struct_start + 8;
	addr_t const child_array_end    = child_array_start + array_size;

	addr_t const local_ds_start     = _env.rm().attach(ar->attached_ds_cap, ar->i_size, ar->i_offset);
	addr_t const local_ds_end       = local_ds_start + ar->i_size;
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
			_kcap_mapping[index].kcap = kcap;
			_kcap_mapping[index].badge = badge;
			index++;
		}
	}

	_env.rm().detach(local_ds_start);
}


void Capability_mapping::print(Genode::Output &output) const {
	Genode::print(output, " Capability map:\n");
	for(int i = 0; i < index; i++) {
		Genode::print(output,
		              " kcap=", _kcap_mapping[i].kcap,
		              " badge=", _kcap_mapping[i].badge, "\n");
	}
}


Genode::addr_t Capability_mapping::find_kcap_by_badge(Genode::uint16_t badge)
{
	Genode::addr_t kcap = 0;

	for(int i = 0; i < index; i++) {
		if(_kcap_mapping[i].badge == badge)
			return _kcap_mapping[i].kcap;
	}
	return 0;
}
