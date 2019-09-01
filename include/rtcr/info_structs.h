/*
 * \brief  Structs for monitor's objects
 * \author Denis Huber
 * \date   2016-11-21
 */

#ifndef _RTCR_INFO_STRUCTS_H_
#define _RTCR_INFO_STRUCTS_H_

/* Genode includes */
#include <util/list.h>
#include <util/string.h>

namespace Rtcr {
	template<typename T> class Simple_counter;

	struct Normal_info;
	struct Session_info;
}

template<typename T>
class Rtcr::Simple_counter
{
private:
	Genode::size_t const current_id;
	static Genode::size_t get_id()
		{
			static Genode::size_t count = 0;
			return count++;
		}

public:
	Simple_counter() : current_id(get_id()) { }
	// Do not use copy ctor, because Genode::print(Output&, HEAD const&, TAIL ...) uses pass-by-value
	// which copies the object inherited from Simple_counter for the function call
	//Simple_counter(const Simple_counter&) : current_id(get_id()) { }
	//~Simple_counter() { }

	Genode::size_t id() const { return current_id; }
};

struct Rtcr::Normal_info
{
	bool bootstrapped;
	Genode::uint16_t badge;

	Normal_info() {}
	
	void print(Genode::Output &output) const {
		Genode::print(output,
					  "bootstrapped=", bootstrapped,
					  " badge=", badge);
	}	
};


struct Rtcr::Session_info : Rtcr::Normal_info
{
	Genode::String<160> creation_args;
	Genode::String<160> upgrade_args;

	Session_info(const char* creation_args)
		: creation_args (creation_args) { }

	void print(Genode::Output &output) const {
		Normal_info::print(output);		
		Genode::print(output,
					  ", cargs='", creation_args,
					  "', uargs='", upgrade_args, "'");

	}
};




#endif /* _RTCR_INFO_STRUCTS_H_ */
