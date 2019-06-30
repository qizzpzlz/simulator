#pragma once
#include <vector>
#include "Host.h"

namespace ClusterSimulator
{
	class Cluster
	{
	public:
		auto begin() { return nodes_.begin(); }
		auto end() { return nodes_.end(); }
		auto begin() const { return nodes_.cbegin(); }
		auto end() const { return nodes_.cend(); }
		void add_node(const Host& node) { nodes_.push_back(node); }
		int count() const { return nodes_.size(); }
	private:
		std::vector<Host> nodes_;
	};
}

