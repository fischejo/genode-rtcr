/*
 * \brief  Intercepting Region map
 * \author Denis Huber
 * \author Johannes Fischer
 * \date   2019-08029
 */

#include <rtcr/rm/region_map.h>

#ifdef PROFILE
#include <util/profiler.h>
#define PROFILE_THIS_CALL PROFILE_FUNCTION("fuchsia");
#else
#define PROFILE_THIS_CALL
#endif

#if DEBUG 
#define DEBUG_THIS_CALL Genode::log("\e[38;5;206m", __PRETTY_FUNCTION__, "\033[0m");
#else
#define DEBUG_THIS_CALL
#endif

using namespace Rtcr;

Region_map::Region_map(Genode::Allocator &md_alloc,
					   Genode::Capability<Genode::Region_map> region_map_cap,
					   Genode::size_t size,
					   const char *label,
					   bool &bootstrap_phase)
	:
	Region_map_info(region_map_cap.local_name()),
	_md_alloc          (md_alloc),
	_bootstrap_phase   (bootstrap_phase),
	_label             (label),
	_parent_region_map (region_map_cap),
	_size(size),
	_ds_cap(_parent_region_map.dataspace())
{
	DEBUG_THIS_CALL
}


Region_map::~Region_map()
{
	while(Attached_region_info *ar = _attached_regions.first()) {
		_attached_regions.remove(ar);
		Genode::destroy(_md_alloc, ar);
	}
}


void Region_map::checkpoint()
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL;

//	info.badge = cap().local_name();
//	info.bootstrapped = _bootstrap_phase;
//  ck_upgrade_args = _upgrade_args.string();

  
	i_size = _size;
	i_ds_badge = _ds_cap.local_name();
	i_sigh_badge = _sigh.local_name();

	Attached_region_info *region = nullptr;
	while(region = _destroyed_attached_regions.dequeue()) {
		_attached_regions.remove(region);
		Genode::destroy(_md_alloc, region);
	}

	region = _attached_regions.first();
	while(region) {
		static_cast<Attached_region*>(region)->checkpoint();
		region = region->next();
	}  

	i_attached_regions = _attached_regions.first();
}



Attached_region *Region_map::find_attached_region_by_addr(Genode::addr_t addr)
{
	/* no lock needed, as we are looking for something, which will not be
	 * deleted */
	Attached_region_info *ar_info = _attached_regions.first();
	ar_info = ar_info->find_by_addr(addr);
	return static_cast<Attached_region*>(ar_info);
}




Genode::Region_map::Local_addr Region_map::attach(Genode::Dataspace_capability ds_cap,
												  Genode::size_t size,
												  Genode::off_t offset,
												  bool use_local_addr,
												  Region_map::Local_addr local_addr,
												  bool executable)
{
	DEBUG_THIS_CALL
#ifdef DEBUG
	if(use_local_addr) {
		Genode::log("Rmap<\033[35m", _label,"\033[0m>", "::",
				    "\033[33m", __func__, "\033[0m(",
				    "ds ",       ds_cap,
				    ", size=",       Genode::Hex(size),
				    ", offset=",     offset,
				    ", local_addr=", Genode::Hex(local_addr),
				    ", exe=",        executable,
				    ")");
	} else {
		Genode::log("Rmap<\033[35m", _label,"\033[0m>", "::",
				    "\033[33m", __func__, "\033[0m(",
				    "ds ",   ds_cap,
				    ", size=",   Genode::Hex(size),
				    ", offset=", offset,
				    ", exe=",    executable,
				    ")");
	}
#endif

	/* Attach dataspace to real Region map */
	Genode::addr_t addr = _parent_region_map.attach(
		ds_cap, size, offset, use_local_addr, local_addr, executable);

	/* Actual size of the attached region; page-aligned */
	Genode::size_t actual_size;
	if(size == 0) {
		Genode::size_t ds_size = Genode::Dataspace_client(ds_cap).size();
		actual_size = Genode::align_addr(ds_size - offset, 12);
	} else {
		actual_size = Genode::align_addr(size, 12);
	}
	//Genode::log("  actual_size=", Genode::Hex(actual_size));

	/* Store information about the attachment */
	Attached_region *new_obj = new (_md_alloc) Attached_region(ds_cap,
															   actual_size,
															   offset,
															   addr,
															   executable,
															   _bootstrap_phase);	

#ifdef DEBUG
	Genode::size_t num_pages = actual_size/4096;

	Genode::log("  Attached dataspace (", ds_cap, ")",
			    " into [", Genode::Hex(addr),
			    ", ", Genode::Hex(addr+actual_size), ") ",
			    num_pages, num_pages==1?" page":" pages");
#endif

	/* Store Attached_region in a list */
	Genode::Lock::Guard lock_guard(_attached_regions_lock);
	_attached_regions.insert(new_obj);

	return addr;
}


void Region_map::detach(Region_map::Local_addr local_addr)
{
	/* Detach from real region map */
	_parent_region_map.detach(local_addr);

	/* Find region */

	Attached_region_info *region = _attached_regions.first();
	if(region) region = region->find_by_addr((Genode::addr_t)local_addr);
	if(region) {
		/* Remove and destroy region from list and allocator */
		Genode::Lock::Guard lock_guard(_destroyed_attached_regions_lock);
		_destroyed_attached_regions.enqueue(region);
	} else {
			Genode::warning("Region not found in Rm::detach(). Local address",
							Genode::Hex(local_addr), " not in regions list.");
	}			
}


void Region_map::fault_handler(Genode::Signal_context_capability handler)
{
	_sigh = handler;
	_parent_region_map.fault_handler(handler);
}


Genode::Region_map::State Region_map::state()
{
	auto result = _parent_region_map.state();
	const char* type = result.type == Genode::Region_map::State::READ_FAULT ? "READ_FAULT" :
		result.type == Genode::Region_map::State::WRITE_FAULT ? "WRITE_FAULT" :
		result.type == Genode::Region_map::State::EXEC_FAULT ? "EXEC_FAULT" : "READY";
	return result;
}


Genode::Dataspace_capability Region_map::dataspace()
{
    return _parent_region_map.dataspace();
}
