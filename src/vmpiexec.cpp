/**
 * vmpiexec
 *
 * Copyright 2017 by ACS RWTH Aachen University
 * Simon Pickartz     <spickartz@eonerc.rwth-aachen.de>
 *
 * Licensed under GNU General Public License 2.0 or later.
 * Some rights reserved. See LICENSE
 */
#include "vmpiexec/vmpiexec.hpp"

// inititalize fast-lib log
FASTLIB_LOG_INIT(vmpiexec_log, "vmpiexec")
FASTLIB_LOG_SET_LEVEL_GLOBAL(vmpiexec_log, info);

int main(int argc, char const *argv[]) {
	FASTLIB_LOG(vmpiexec_log, trace) << "Command-line options parsed.";

	return 0;
}
