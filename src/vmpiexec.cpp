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
#include "vmpiexec/virt_cluster.hpp"
#include "vmpiexec/sigint_handler.hpp"

#include <arrrgh.hpp>
#include <libgen.h>

#include <algorithm>
#include <cstdio>

// inititalize fast-lib log
FASTLIB_LOG_INIT(vmpiexec_log, "vmpiexec")
FASTLIB_LOG_SET_LEVEL_GLOBAL(vmpiexec_log, trace);

// default command-line arguments
static size_t num_procs = 1;
static size_t doms_per_host = 1;
static std::string mpiexec_args = "";
static std::string mqtt_broker = "localhost";
static host_listT host_list { "localhost" };

// obtain basename from given path
static
std::string generate_basename(std::string path) {
	char *path_cstr = new char[path.size() + 1];
	std::copy(path.begin(), path.end(), path_cstr);
	path_cstr[path.size()] = '\0';
	std::string base_name = std::string(basename(path_cstr));
	delete [] path_cstr;

	return base_name;
}

// parse the command-line options
void parse_cmd_options(int argc, char const *argv[]) {
	// configure the parser
	arrrgh::parser parser("vmpiexec", "An mpiexec for virtualized clusters.");

	const auto &num_procs_arg = parser.add<size_t>("np", "Number of processes.", 'n', arrrgh::Optional);
	const auto &host_list_arg = parser.add<std::string>("hosts", "A comma-seperated list of hosts.", 'H', arrrgh::Optional);
	const auto &mqtt_broker_arg = parser.add<std::string>("broker", "Name of the MQTT broker to connect to.", 'b', arrrgh::Optional);
	const auto &doms_per_host_arg =
		parser.add<size_t>("doms-per-host", "The amount of domains created per node.", 'd', arrrgh::Optional);
	const auto &help =
		parser.add<bool>("help", "PRint this help message.", 'h', arrrgh::Optional);

	// parse the options
	try {
		parser.parse(argc, argv);
	} catch (const std::exception &e) {
		std::cerr << "ERROR: could not parse the arguments: " << e.what() << std::endl;
		parser.show_usage(std::cerr);
		exit(-1);
	}

	// Get argument values.
	try {
		// exit on help
		if (help.value()) {
			exit(-1);
		}

		if (doms_per_host_arg.value()) {
			doms_per_host = doms_per_host_arg.value();
		}
		if (num_procs_arg.value()) {
			num_procs = num_procs_arg.value();
		}
		if (mqtt_broker_arg.value().length() > 0) {
			mqtt_broker = mqtt_broker_arg.value();
		}

		// parse host list
		std::string host_list_str = host_list_arg.value();
		if (!host_list_str.empty()) {
			size_t host_cnt = std::count(host_list_str.begin(), host_list_str.end(), ',');

			host_list.clear();
			host_list.reserve(host_cnt);
			size_t start = 0, end = 0;
			while ((end = host_list_str.find(',', start)) != std::string::npos) {
				if (end != start) {
					host_list.emplace_back(host_list_str.substr(start, end - start));
				}
				start = end + 1;
			}
			if (end != start && !host_list_str.substr(start).empty()) {
				host_list.emplace_back(host_list_str.substr(start));
			};
		}
		FASTLIB_LOG(vmpiexec_log, trace) << "Hostlist (" << host_list.size() << "): " << host_list;

		// create mpiexec call
		parser.each_unlabeled_argument([](const std::string &arg) { mpiexec_args += arg + " "; });
		if (mpiexec_args.size() == 0)
			throw std::runtime_error("No command to execute was passed.");
		mpiexec_args.pop_back();

		FASTLIB_LOG(vmpiexec_log, trace) << "Calling: mpiexec " << mpiexec_args;
	} catch (const std::exception &e) {
		std::cerr << "Error reading argument values: " << e.what() << std::endl;
		parser.show_usage(std::cerr);
		exit(-1);
	}
}

int main(int argc, char const *argv[]) {
	// Parse command-line options
	FASTLIB_LOG(vmpiexec_log, debug) << "Parsing command-line options ...";
	parse_cmd_options(argc, argv);

	// Generate job name (job_name = executable = ivshmem device name)
	std::string job_name = generate_basename(mpiexec_args.substr(0, mpiexec_args.find(" ")));

	// Start virtual cluster
	FASTLIB_LOG(vmpiexec_log, trace) << "Executable: " + job_name;
	FASTLIB_LOG(vmpiexec_log, debug) << "Starting virtual cluster ...";
	virt_clusterT virt_cluster(job_name, host_list, doms_per_host, mqtt_broker);

	// Handle sigint (graceful shutdown on ctrl+c)
	Sigint_handler sigint_handler(std::bind(std::mem_fn(&virt_clusterT::stop), &virt_cluster));

	// Execute command
	FASTLIB_LOG(vmpiexec_log, debug) << "Executing command ...";
	execute_command(virt_cluster.nodes, mpiexec_args);

	FASTLIB_LOG(vmpiexec_log, debug) << "Done!";

	return 0;
}

// call mpiexec and run app on virt_cluster
void execute_command(host_listT virt_cluster, std::string mpiexec_args) {
	FILE *in;
	char buff[512];

	// generate virtual node list
	std::string node_list;
	for (const auto &node : virt_cluster) {
		node_list += node + ",";
	}
	node_list.pop_back();

	std::stringstream cmd_stream;
	cmd_stream << "mpiexec -np ";
	cmd_stream << num_procs;
	cmd_stream << " -hosts " + node_list;
	cmd_stream << " " + mpiexec_args;

	FASTLIB_LOG(vmpiexec_log, trace) << "Calling '" << cmd_stream.str() << "'";
	if (!(in = popen(cmd_stream.str().c_str(), "r"))) {
		std::cerr << "ERROR: could not execute '" << cmd_stream.str() << "'" <<	std::endl;
		return;
	}

	while (fgets(buff, sizeof(buff), in) != NULL){
		std::cout << buff;
	}
	std::cout << std::endl;
	pclose(in);
}
