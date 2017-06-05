/**
 * vmpiexec
 *
 * Copyright 2017 by ACS RWTH Aachen University
 * Simon Pickartz     <spickartz@eonerc.rwth-aachen.de>
 *
 * Licensed under GNU General Public License 2.0 or later.
 * Some rights reserved. See LICENSE
 */

#ifndef virt_cluster_hpp
#define virt_cluster_hpp

#include "vmpiexec/vmpiexec.hpp"

#include <fast-lib/mqtt_communicator.hpp>

class virt_clusterT  {
  public:
	virt_clusterT(const host_listT host_list, const size_t doms_per_host, const std::string mqtt_broker = "localhost", const int mqtt_port = 1883);
	~virt_clusterT();

	// start all domains and return the hostnames
	void start();

	// stop all domains
	void stop();

	// getters
	// list of nodes within the virtual cluster
	const host_listT &nodes;
  private:
	host_listT _nodes;
	const host_listT _hosts;
	const size_t _doms_per_host;
	std::shared_ptr<fast::MQTT_communicator> _comm;
};

#endif /* end of include guard: vmpiexec_hpp */
