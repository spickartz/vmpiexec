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

// command-line arguments
arrrgh::parser parser("vmpiexec", "An mpiexec for virtualized clusters.");

const auto &host_list = parser.add<std::string>("hosts", "A comma-seperated list of hosts.", 'H', arrrgh::Optional);
const auto &doms_per_host =
	parser.add<size_t>("doms-per-host", "The amount of domains created per node.", 'd', arrrgh::Optional);

static std::string mpiexec_args = "";

// parse the command-line options
void parse_cmd_options(int argc, char const *argv[]) {
	// parse the options
	try {
		parser.parse(argc, argv);
	} catch (const std::exception &e) {
		std::cerr << "Error parsing arguments: " << e.what() << std::endl;
		parser.show_usage(std::cerr);
		exit(-1);
	}

	// Get argument values.
	//
	try {
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

	return 0;
}
