#pragma once
#include <vector>
#include "host.h"
#include <map>
#include <unordered_map>

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
		void add_node(Host&& node) 
		{ 
			//nodes_.insert({node.get_name(), node}); 
			nodes_.push_back(node);
			nodes_map_.insert({ node.get_name(), &nodes_.back() });
			total_slot_number += node.max_slot;
		}
		auto find_node(const std::string& name) const noexcept { return nodes_map_.find(name); }
		auto find_node(const std::string& name) noexcept { return nodes_map_.find(name); }
		int count() const { return nodes_.size(); }

		void update() noexcept { ++version_; }
		auto get_version() const noexcept { return version_; }

		std::vector<Host>& vector() { return nodes_; }

	private:
		std::size_t version_{1};
		std::unordered_map<std::string, Host*> nodes_map_;
		std::vector<Host> nodes_;
	};
}

