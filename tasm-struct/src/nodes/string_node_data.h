#ifndef STRING_NODE_DATA_H
#define STRING_NODE_DATA_H

#include <vector>
#include <string>

namespace nodes {

class StringNodeData {
private:
  // custom label
  std::string label_;

public:
  // Basic constructor
  StringNodeData(std::string label = "");
  // implicit default copy constructor
  // implicit default move constructor

  // implicit default destructor

  const std::string& get_label() const;

  bool operator<(const StringNodeData& other) const;
  bool operator==(const StringNodeData& other) const;
  // implicit default copy assign operator
  // implicit default move assign operator
};

StringNodeData::StringNodeData(std::string label) : label_(std::move(label)) { }

const std::string& StringNodeData::get_label() const {
  return label_;
}

bool StringNodeData::operator<(const StringNodeData& other) const {
  return (label_.compare(other.get_label()) < 0); // TODO: modify to have distance comparison
}

bool StringNodeData::operator==(const StringNodeData& other) const {
  return (label_.compare(other.get_label()) == 0);
}

// Example of a struct representing custom cost functions to be used for the
// distance computation. This shows how to use the framework with custom cost
// functions. Also these custom costs need to be generic for different node
// classes.
template<class _NodeData = StringNodeData>
struct StringCosts : public Costs<_NodeData> {
  // Basic rename cost function
  //
  // Params:  node_data1  Data of the node to be renamed
  //          node_data2  Data of the node having the desired name
  //
  // Return:  The cost of renaming node1 to node2
  int ren(const _NodeData* node_data1, const _NodeData* node_data2) override;

  // Basic delete cost function
  //
  // Params:  node_data Data of the node to be deleted
  //
  // Return:  The cost of deleting node
  int del(const _NodeData* node_data) override;

  // Basic insert cost function
  //
  // Params:  node_data Data of the node to be inserted
  //
  // Return:  The cost of inserting node
  int ins(const _NodeData* node_data) override;

  // Dummy methods
  int ren() override;
  int del() override;
  int ins() override;
};

template<class _NodeData>
int StringCosts<_NodeData>::ren(
    const _NodeData* node_data1,
    const _NodeData* node_data2) {

  if (node_data1->get_label() == node_data2->get_label()) {
    return 0;
  }

  return ((del(node_data1) + ins(node_data2)) / 2);
}

template<class _NodeData>
int StringCosts<_NodeData>::del(const _NodeData* node_data) {
  return 1;
}

template<class _NodeData>
int StringCosts<_NodeData>::ins(const _NodeData* node_data) {
  return 1;
}

template<class _NodeData>
int StringCosts<_NodeData>::ren() {
  return 1; 
}

template<class _NodeData>
int StringCosts<_NodeData>::del() {
  return 1;
}

template<class _NodeData>
int StringCosts<_NodeData>::ins() {
  return 1;
}

} // namespace nodes

#endif // STRING_NODE_DATA_H