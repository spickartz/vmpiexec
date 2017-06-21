/**
 * vmpiexec
 *
 * Copyright 2017 by ACS RWTH Aachen University
 * Simon Pickartz     <spickartz@eonerc.rwth-aachen.de>
 *
 * Licensed under GNU General Public License 2.0 or later.
 * Some rights reserved. See LICENSE
 */

#ifndef virt_cluster
#define virt_cluster

#include "vmpiexec/virt_cluster.hpp"

// inititalize fast-lib log
FASTLIB_LOG_INIT(virt_cluster_log, "virt-cluster")
FASTLIB_LOG_SET_LEVEL_GLOBAL(virt_cluster_log, trace);

// TODO: retrieve DHCP list from other service
std::vector<fast::msg::migfra::DHCP_info> virt_clusterT::get_dhcp_info_list(const size_t count) const {
	FASTLIB_LOG(virt_cluster_log, trace) << "Virtual node count: " << count;
	return std::vector<fast::msg::migfra::DHCP_info>(glob_dhcp_pool.begin(), glob_dhcp_pool.begin() + count);
}

// generates the Start_virt_cluster task
std::shared_ptr<fast::msg::migfra::Start_virt_cluster> virt_clusterT::generate_start_task(const std::string type) const {
	// prepare IB device
	std::vector<fast::msg::migfra::PCI_id> pci_ids;
	pci_ids.emplace_back(0x15b3, 0x1004);

	auto start_task = std::make_shared<fast::msg::migfra::Start_virt_cluster>();
	start_task->base_name = type;
	start_task->pci_ids = std::move(pci_ids);
	start_task->dhcp_info = get_dhcp_info_list(_doms_per_host*_hosts.size());

	// add ivshmem device if count > 1
	if (_doms_per_host > 1) {
		fast::msg::migfra::Device_ivshmem ivshmem_device;
	        ivshmem_device.id = "shmem0";
		ivshmem_device.size = "512";
		start_task->ivshmem = std::move(ivshmem_device);
	}

	return start_task;
}

// constructor
virt_clusterT::virt_clusterT(const host_listT host_list, const size_t doms_per_host, const std::string mqtt_broker,
							 const int mqtt_port)
	: nodes(_nodes), _hosts(host_list), _doms_per_host(doms_per_host) {
	_comm = std::make_shared<fast::MQTT_communicator>("vmpiexec", "", mqtt_broker, mqtt_port, 60, fast::MQTT_communicator::timeout_duration_t(2));
}

// destructor
virt_clusterT::~virt_clusterT() {}

// start all domains and wait until ready
void virt_clusterT::start() {
	host_listT virt_cluster();

	// request start of all domains on all nodes
	for (const auto &host : _hosts) {
		// create task container and add tasks per slot
		fast::msg::migfra::Task_container m;
		m.tasks.push_back(generate_start_task("centos"));

		// send start request
		std::string topic = "fast/migfra/" + host + "/task";
		FASTLIB_LOG(virt_cluster_log, debug) << "sending message \n topic: " << topic << "\n message:\n"
											 << m.to_string();
		_comm->send_message(m.to_string(), topic);
	}

	// wait for response and retrieve virt-cluster
	fast::msg::migfra::Result_container response;
	for (const auto &host : _hosts) {
		// wait for VMs to be started
		std::string topic = "fast/migfra/" + host + "/result";
		response.from_string(_comm->get_message(topic));

		// check success for each result
		for (auto result : response.results) {
			assert(result.status == "success");
		}
	}


	return virt_cluster;
}

// stop all domains and wait until ready
void virt_clusterT::stop() {
	// request stop of all domains on all hosts
	for (const auto &host : _hosts) {
		// generate stop tasks
		fast::msg::migfra::Task_container m;
		auto task = std::make_shared<fast::msg::migfra::Stop>();
		task->regex = ".*";
		task->force = true;
		task->concurrent_execution = true;
		m.tasks.push_back(task);

		// send stop request
		std::string topic = "fast/migfra/" + host + "/task";
		FASTLIB_LOG(virt_cluster_log, debug) << "sending message \n topic: " << topic << "\n message:\n"
											 << m.to_string();
		_comm->send_message(m.to_string(), topic);
	}

	// wait for completion
	fast::msg::migfra::Result_container response;
	for (const auto &host : _hosts) {
		std::string topic = "fast/migfra/" + host + "/result";
		response.from_string(_comm->get_message(topic));
		for (auto result : response.results) {
			assert(result.status == "success");
		}
	}
}


#endif /* end of include guard: virt_cluster */
