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
#include <fast-lib/message/migfra/result.hpp>
#include <fast-lib/message/migfra/task.hpp>

extern std::vector<fast::msg::migfra::DHCP_info> glob_dhcp_pool;

class virt_clusterT  {
  public:
	virt_clusterT(const host_listT host_list, const size_t doms_per_host, const std::string mqtt_broker = "localhost", const int mqtt_port = 1883);
	~virt_clusterT();

	// start all domains and return the hostnames
	void start(const std::string job_name);

	// stop all domains
	void stop();

	// getters
	const host_listT &nodes;

  private:
	std::shared_ptr<fast::msg::migfra::Start_virt_cluster> generate_start_task(const std::string type) const;
	std::shared_ptr<fast::msg::migfra::Start_virt_cluster> generate_start_task(const std::string type, const std::string shmem_id) const;
	void acquire_dhcp_info(const size_t count);

  private:
	std::string _name; // a unique name of the virtual cluster instance
	host_listT _nodes; // list of nodes within the virtual cluster
	host_listT _hosts; // list of physical hosts
	const size_t _doms_per_host; // amount of domains per physical host
	std::shared_ptr<fast::MQTT_communicator> _comm; // MQTT communicator
	std::vector<fast::msg::migfra::DHCP_info> _dhcp_info; // DHCP info of the virtual cluster nodes
};

#endif /* end of include guard: vmpiexec_hpp */
