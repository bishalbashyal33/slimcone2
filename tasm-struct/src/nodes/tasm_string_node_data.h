#ifndef NODES_TASM_STRING_NODE_DATA_H
#define NODES_TASM_STRING_NODE_DATA_H

#include <string>

#include "string_node_data.h"

namespace nodes {

class TASMStringNodeData : public StringNodeData {
private:
  // costs
  double cost_;

public:
  // Basic constructor
  TASMStringNodeData(std::string label = "", const double& cost = 1);
  // implicit default copy constructor
  // implicit default move constructor

  // implicit default destructor

  const double get_cost() const;

  // implicit default copy assign operator
  // implicit default move assign operator
};

TASMStringNodeData::TASMStringNodeData(std::string label, const double& cost)
  : StringNodeData(std::move(label)), cost_(cost)
{ }

const double TASMStringNodeData::get_cost() const {
  return cost_;
}


// Example of a struct representing custom cost functions to be used for the
// distance computation. This shows how to use the framework with custom cost
// functions. Also these custom costs need to be generic for different node
// classes.
template<class _NodeData = TASMStringNodeData>
struct TASMStringCosts {
  // Basic rename cost function
  //
  // Params:  node_data1  Data of the node to be renamed
  //          node_data2  Data of the node having the desired name
  //
  // Return:  The cost of renaming node1 to node2
  int ren(const _NodeData* node_data1, const _NodeData* node_data2);

  // Basic delete cost function
  //
  // Params:  node_data Data of the node to be deleted
  //
  // Return:  The cost of deleting node
  int del(const _NodeData* node_data);

  // Basic insert cost function
  //
  // Params:  node_data Data of the node to be inserted
  //
  // Return:  The cost of inserting node
  int ins(const _NodeData* node_data);
};

template<class _NodeData>
int TASMStringCosts<_NodeData>::ren(const _NodeData* node_data1, const _NodeData* node_data2) {
  if (*node_data1 == *node_data2) {
    return 0;
  }

  return ((node_data1->get_cost() + node_data2->get_cost()) / 2);
}

template<class _NodeData>
int TASMStringCosts<_NodeData>::del(const _NodeData* node_data) {
  return node_data->get_cost();
}

template<class _NodeData>
int TASMStringCosts<_NodeData>::ins(const _NodeData* node_data) {
  return node_data->get_cost();
}

} // namespace nodes

#endif // STRING_NODE_DATA_H