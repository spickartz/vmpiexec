/**
 * vmpiexec
 *
 * Copyright 2017 by ACS RWTH Aachen University
 * Simon Pickartz     <spickartz@eonerc.rwth-aachen.de>
 *
 * Licensed under GNU General Public License 2.0 or later.
 * Some rights reserved. See LICENSE
 */

#ifndef vmpiexec_hpp
#define vmpiexec_hpp

#include <sstream>
#include <iterator>

#include <fast-lib/log.hpp>

using host_listT = std::vector<std::string>;
// parse the command-line options
void parse_cmd_options(int argc, char const *argv[]);

// start all domains and return the hostnames
host_listT start_virtual_cluster(host_listT host_list, size_t doms_per_host);

// stop all domains
void stop_virtual_cluster(host_listT host_list);

// the actual mpiexec call
void execute_command(host_listT virt_cluster, std::string mpiexec_args);


namespace std {
// The following operator<< are implemented in the std namespace to allow fastlib
// to use argument dependent lookup (ADL) to find these functions (i.e. use the namespace
// of the first argument passed to it to use the function FASTLIB_LOG() << std::vector looks into std.
// Alternative would be to ensure ordering on the includes and make sure theses functions are available
// before fast-lib includes spdlog, but this seems not to be feasible.

template <typename T, size_t N> std::ostream &operator<<(std::ostream &os, const std::array<T, N> &a) {
	os << "[";
	auto it = std::ostream_iterator<T>(os, ",");
	std::copy(std::begin(a), std::end(a), it);
	os << "]";
	return os;
}
template <template <typename, typename> class C, typename T, typename T2>
std::ostream &operator<<(std::ostream &os, const C<T, T2> &vec) {
	os << "[";
	std::stringstream vec_stream;
	for (const auto &vec_elem : vec) {
		vec_stream << vec_elem << ",";
	}

	std::string vec_str = vec_stream.str();
	if (!vec_str.empty()) vec_str.pop_back();
	os << vec_str;
	os << "]";

	return os;
}
}
#endif /* end of include guard: vmpiexec_hpp */
