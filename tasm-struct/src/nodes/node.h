#ifndef NODE_TYPE_H
#define NODE_TYPE_H

#include <algorithm>
#include <iostream>
#include <random>
#include <vector>
#include <string>
#include <cstring>
#include <ctime>
#include <unordered_map>
#include <queue>

namespace nodes {

// Represents a node in a tree. A node label is a string and the children
// are stored in an array of pointers to nodes.
template<class _NodeData>
class Node {
private:
  int get_subtree_size_iterative() const;
  int get_subtree_size_recursive() const;

private:
  // A vector of pointers to child nodes
  std::vector<Node<_NodeData>*> children_;
  // Additional node data
  _NodeData* data_;

public:
  // Constructors
  Node(_NodeData* data = new _NodeData());

  // Copy Constructor
  Node(const Node<_NodeData>& other); // rule of three

  // Destructor
  ~Node(); // rule of three

  _NodeData* get_data() const;

  // Get children
  std::vector<Node<_NodeData>*> get_children() const;

  // Get children_number
  int get_children_number() const;

  // Add a child at last position.
  //
  // Params:  child A pointer to the node to be added
  //
  // Return:  None
  void add_child(Node<_NodeData>* child);

  // TODO
  Node<_NodeData>* get_child(int position) const;

  // Get subtree size rooted at this node.
  //
  // Return:  The size of the subtree rooted at this node (including this node)
  int get_subtree_size(const bool& iterative = false) const;

  // Operators
  Node<_NodeData>& operator=(const Node<_NodeData>& other); // rule of three
  bool operator<(const Node<_NodeData>& other) const;
  bool operator==(const Node<_NodeData>& other) const;
};

template<class _NodeData>
Node<_NodeData>::Node(_NodeData* data) : data_(data) { }


template<class _NodeData>
Node<_NodeData>::Node(const Node<_NodeData>& other)
  : data_(new _NodeData(*other.data_)) { }


template<class _NodeData>
Node<_NodeData>::~Node() {
  // delete all children nodes
  for ( typename std::vector<Node<_NodeData>*>::const_iterator node_it = children_.begin();
        node_it != children_.end(); ++node_it)
  {
    delete *node_it;
  }

  // clear vector to avoid dangling pointers
  children_.clear();

  //delete data_;
}

template<class _NodeData>
_NodeData* Node<_NodeData>::get_data() const {
  return data_;
}

template<class _NodeData>
std::vector<Node<_NodeData>*> Node<_NodeData>::get_children() const {
  return children_;
}

template<class _NodeData>
int Node<_NodeData>::get_children_number() const {
  return children_.size();
}

template<class _NodeData>
void Node<_NodeData>::add_child(Node<_NodeData>* child) {
  children_.push_back(child);
}

template<class _NodeData>
Node<_NodeData>* Node<_NodeData>::get_child(int position) const {
  return children_[position];
}

template<class _NodeData>
int Node<_NodeData>::get_subtree_size(const bool& iterative) const {
  if (iterative) {
    return get_subtree_size_iterative();
  }

  return get_subtree_size_recursive();
}

template<class _NodeData>
Node<_NodeData>& Node<_NodeData>::operator=(const Node<_NodeData>& other) {
  if (this != &other) {
    *data_ = *other.data_;
  }

  return *this;
}

template<class _NodeData>
bool Node<_NodeData>::operator<(const Node<_NodeData>& other) const {
  return (*data_ < *other.get_data());
}

template<class _NodeData>
bool Node<_NodeData>::operator==(const Node<_NodeData>& other) const {
  return (*data_ == *other.get_data());
}

template<class _NodeData>
int Node<_NodeData>::get_subtree_size_recursive() const {
  int descendants_sum = 1;
  
  // Sum up sizes of subtrees rooted at child nodes (number of descendants)
  for (Node<_NodeData>* child: children_) {
    descendants_sum = descendants_sum + child->get_subtree_size();
  }

  // Add this node to subtree size.
  return descendants_sum;
}

template<class _NodeData>
int Node<_NodeData>::get_subtree_size_iterative() const {
  std::queue<const Node<_NodeData>*> node_queue;
  int size = 0;

  node_queue.push(this);

  while (!node_queue.empty()) {
    ++size;

    const Node<_NodeData>* node = node_queue.front();
    node_queue.pop();
    for (auto child: node->children_) {
      node_queue.push(child);
    }
  }

  return size;
}

// Represents the cost functions to be used for the distance computation.
// Costs are generic for different node classes. A cost model has to provide
// three cost functions:
//  - ren(n1, n2) for rename costs between two nodes n1 and n2
//  - del(n) for deletion costs of a node n
//  - ins(n) for insertion costs of a node n
// All three cost functions must return an integer.
template<typename _NodeData>
struct Costs {
  // Basic rename cost function
  //
  // Params:  node1  The node to be renamed
  //          node2  The node having the desired name
  //
  // Return:  The cost of renaming node1 to node2
  virtual int ren(const _NodeData* node1, const _NodeData* node2);

  // Basic delete cost function
  //
  // Params:  node The node to be deleted
  //
  // Return:  The cost of deleting node
  virtual int del(const _NodeData* node);

  // Basic insert cost function
  //
  // Params:  node The node to be inserted
  //
  // Return:  The cost of inserting node
  virtual int ins(const _NodeData* node);

  // Dummy methods: TO BE REMOVED AT SOME POINT
  virtual int ren();
  virtual int del();
  virtual int ins();
};

//NodeType<> empty_node;

} // namespace nodes

#endif // NODE_TYPE_H

// TODO:
// NO - Should the fields of Node, especially children, be pointers?
// ?? - Should children be initialized only with the first child? Are they now?
// ?? - Fix the destructor which should delete all descendants.
// NO    - Should the nodes be wrapped with a Tree clas?
//       - How to recursively delete all nodes including the root node?
// NO       - Using 'delete this' in destructor is not entirely safe.
// ??       - I've found that one way of dealing with that is smart pointers like
//            'unique_ptr'.
// OK    - Is it enough to write a destructor that deletes all the descendants
//         without the root? Will the root gets deleted automatically when it goes
//         out of scope?
// OK       - Is what we have now enough?
// OK - Implement get_subtree_size() as member method.
//    - Implement get_postorder_traversal(Node *root) outside Node class.
