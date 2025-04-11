// MIT License
//
// Copyright (c) 2019 Daniel Kocher
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

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

StringNodeData::StringNodeData(std::string label)
  : label_(label) {}

const std::string& StringNodeData::get_label() const {
  return label_;
}

bool StringNodeData::operator<(const StringNodeData& other) const {
  return (label_.compare(other.get_label()) < 0);
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

  if (node_data1->get_label().compare(node_data2->get_label()) == 0) {
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

namespace std {

template<>
struct hash<nodes::StringNodeData> {
  typedef nodes::StringNodeData argument_type;
  typedef std::size_t result_type;

  result_type operator()(const argument_type& s) const {
    return std::hash<std::string>{}(s.get_label());
  }
};

} // namespace std

#endif // STRING_NODE_DATA_H
