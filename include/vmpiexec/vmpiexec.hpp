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

#endif /* end of include guard: vmpiexec_hpp */
