#pragma once
#include <vector>
#include "host.h"
#include <map>

namespace ClusterSimulator
{
	class Cluster
	{
	public:
		const ClusterSimulation* simulation;
		int total_slot_number;

		auto begin() { return nodes_.begin(); }
		auto end() { return nodes_.end(); }
		auto begin() const { return nodes_.cbegin(); }
		auto end() const { return nodes_.cend(); }
		//void add_node(const Host& node) { nodes_.push_back(node); }
		void add_node(const Host& node) 
		{ 
			nodes_.insert({node.name, node}); 
			total_slot_number += node.max_slot;
		}
		auto find_node(const std::string& name) const noexcept { return nodes_.find(name); }
		auto find_node(const std::string& name) noexcept { return nodes_.find(name); }
		int count() const { return nodes_.size(); }
	private:
		//std::vector<Host> nodes_;
		std::map<std::string, Host> nodes_;
	};
}

