/**
 * vmpiexec
 *
 * Copyright 2017 by ACS RWTH Aachen University
 * Simon Pickartz     <spickartz@eonerc.rwth-aachen.de>
 *
 * Licensed under GNU General Public License 2.0 or later.
 * Some rights reserved. See LICENSE
 */
#include <arrrgh.hpp>

#include "vmpiexec/vmpiexec.hpp"

// inititalize fast-lib log
FASTLIB_LOG_INIT(vmpiexec_log, "vmpiexec")
FASTLIB_LOG_SET_LEVEL_GLOBAL(vmpiexec_log, trace);

// default command-line arguments
static size_t doms_per_host = 1;
static std::string mpiexec_args = "";
static host_listT host_list = { "localhost" };

// parse the command-line options
void parse_cmd_options(int argc, char const *argv[]) {
	// configure the parser
	arrrgh::parser parser("vmpiexec", "An mpiexec for virtualized clusters.");

	const auto &host_list_arg = parser.add<std::string>("hosts", "A comma-seperated list of hosts.", 'H', arrrgh::Optional);
	const auto &doms_per_host_arg =
		parser.add<size_t>("doms-per-host", "The amount of domains created per node.", 'd', arrrgh::Optional);

	// parse the options
	try {
		parser.parse(argc, argv);
	} catch (const std::exception &e) {
		std::cerr << "Error parsing arguments: " << e.what() << std::endl;
		parser.show_usage(std::cerr);
		exit(-1);
	}

	// Get argument values.
	try {
		doms_per_host = doms_per_host_arg.value();
		// TODO: parse host list

		// create mpiexec call
		parser.each_unlabeled_argument([](const std::string &arg) { mpiexec_args += " " + arg; });

		FASTLIB_LOG(vmpiexec_log, trace) << "Calling: mpiexec" << mpiexec_args;
	} catch (const std::exception &e) {
		std::cerr << "Error reading argument values: " << e.what() << std::endl;
	}
}

int main(int argc, char const *argv[]) {
	parse_cmd_options(argc, argv);
	FASTLIB_LOG(vmpiexec_log, trace) << "Command-line options parsed.";

	host_listT virt_cluster = start_virtual_cluster(host_list, doms_per_host);
	execute_command(virt_cluster, mpiexec_args);
	stop_virtual_cluster(host_list);
	return 0;
}

// start all domains and wait until ready
host_listT start_virtual_cluster(host_listT host_list, size_t doms_per_host) {
	host_listT virt_cluster;

	return virt_cluster;
}

// stop all domains and wait until ready
void stop_virtual_cluster(host_listT host_list) {
}

// call mpiexec and run app on virt_cluster
void execute_command(host_listT virt_cluster, std::string mpiexec_args) {
}
