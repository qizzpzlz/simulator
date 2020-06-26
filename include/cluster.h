#pragma once
#include <vector>
#include "host.h"
#include <unordered_map>

namespace cs
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
			nodes_.push_back(std::forward<Host>(node));
			total_slot_number += node.max_slot;
		}
		//auto find_node(const std::string& name) const noexcept { return nodes_.find(name); }
		//auto find_node(const std::string& name) noexcept { return nodes_.find(name); }
		std::size_t count() const { return nodes_.size(); }
		Host& operator[](unsigned index) { return nodes_[index]; }
		const Host& operator[](unsigned index) const { return nodes_[index]; }

		void update() noexcept { ++version_; }
		auto get_version() const noexcept { return version_; }

		void set_simulation(ClusterSimulation* simulation)
		{
			this->simulation = simulation;
			//for (auto& [name, host] : nodes_)
			//	host.simulation = simulation;
			for (auto& node : nodes_)
				node.simulation = simulation;
		}

	private:
		std::vector<Host> nodes_;
		std::size_t version_{ 1 };
		//std::unordered_map<std::string, Host> nodes_;
	};
}

