#pragma once
#include <vector>
#include "Node.h"

// Contains all Nodes
class Cluster
{
public:
	Cluster();
	~Cluster();
	vector<Node> get_all_nodes() const { return nodes_; }
	void add_node(Node& node) { nodes_.push_back(node); }
private:
	std::vector<Node> nodes_;
};

