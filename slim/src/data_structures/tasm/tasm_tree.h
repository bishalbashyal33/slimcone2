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

#ifndef DATA_STRUCTURES_TASM_TREE_H
#define DATA_STRUCTURES_TASM_TREE_H

#include <vector>

#include "label_hash_map.h"
#include "string_node_data.h"
#include "postorder_queue_interface.h"
#include "postorder_source_interface.h"

namespace data_structures {

/**
 * Implements the Tree used in the TASM algorithm (for the query). Some method
 * implementations are equivalent to the implements in the RingBufferTree class.
 * Of course, an abstract base class would reduce the code complexity/duplication
 * but from a performance point of view the current version is benefitial, i.e.,
 * faster.
 */
template<class _NodeData = nodes::StringNodeData>
class TASMTree : public interfaces::tasm::PostorderQueueInterface<_NodeData>, public interfaces::tasm::PostorderSourceInterface<_NodeData> {
public:
  std::vector<size_t> lmlds_{};
  std::vector<int> data_ids_{};
  LabelHashMap<_NodeData>& label_map_;
  size_t node_count_{};
  int leaf_count_{};
  size_t start_{};

public:
  TASMTree() = delete;
  TASMTree(LabelHashMap<_NodeData>& label_map);
  TASMTree(
    const size_t start,
    const size_t node_count,
    LabelHashMap<_NodeData>& label_map,
    const std::vector<size_t>& lmlds,
    const std::vector<int>& data_ids,
    const int leaf_count);
  TASMTree(TASMTree&& other);
  TASMTree(const TASMTree& other);
  virtual ~TASMTree();

  inline bool operator<(const TASMTree<_NodeData>& other) const;

  // pure virtual, see src/interfaces/postorder_queue_interface.h
  virtual void append(_NodeData data, const int subtree_size = 1);

  // overloaded version if data_id is already known (faster; skips lookup)
  void append(const int data_id, const int subtree_size = 1);

  // pure virtual, see src/interfaces/postorder_source_interface.h
  virtual void append_to(
    interfaces::tasm::PostorderQueueInterface<_NodeData>* postorder_queue);

  // additional
  const _NodeData& data(const size_t index) const;
  int data_id(const size_t index) const;
  size_t node_count() const;
  size_t lmld(const size_t index) const;
  void set_data(const size_t index, const _NodeData& data);
  void set_data(const size_t index, const int data_id);
  void set_lmld(const size_t index, const size_t lmld);
  LabelHashMap<_NodeData>& label_map() const;
  bool leaf(const size_t index) const;
  int leaf_count();
  const std::vector<int> kr();
  int root() const;
  int global_id(const size_t index) const;
};

template<class _NodeData>
TASMTree<_NodeData>::TASMTree(LabelHashMap<_NodeData>& label_map)
    : label_map_(label_map) {}

template<class _NodeData>
TASMTree<_NodeData>::TASMTree(
    const size_t start,
    const size_t node_count,
    LabelHashMap<_NodeData>& label_map,
    const std::vector<size_t>& lmlds,
    const std::vector<int>& data_ids,
    const int leaf_count)
    : start_(start),
      node_count_(node_count),
      label_map_(label_map),
      lmlds_(lmlds),
      data_ids_(data_ids),
      leaf_count_(leaf_count) {}

template<class _NodeData>
TASMTree<_NodeData>::TASMTree(TASMTree&& other)
    : lmlds_(std::move(other.lmlds_)),
      data_ids_(std::move(other.data_ids_)),
      label_map_(other.label_map_),
      node_count_(other.node_count_),
      leaf_count_(other.leaf_count_),
      start_(other.start_) {}

template<class _NodeData>
TASMTree<_NodeData>::TASMTree(const TASMTree& other)
    : lmlds_(other.lmlds_),
      data_ids_(other.data_ids_),
      label_map_(other.label_map_),
      node_count_(other.node_count_),
      leaf_count_(other.leaf_count_),
      start_(other.start_) {}

template<class _NodeData>
TASMTree<_NodeData>::~TASMTree() {}

template<class _NodeData>
inline bool TASMTree<_NodeData>::operator<(
    const TASMTree<_NodeData>& other) const {
  if (node_count() == other.node_count()) {
    if (data_ids_ == other.data_ids_) {
      return lmlds_ < other.lmlds_;
    }

    return data_ids_ < other.data_ids_;
  }

  return node_count() < other.node_count();
}

template<class _NodeData>
void TASMTree<_NodeData>::append(_NodeData data, const int subtree_size) {
  const size_t new_id = node_count_ + 1;

  set_data(new_id, std::move(data));
  set_lmld(new_id, new_id - subtree_size + 1);
}

template<class _NodeData>
void TASMTree<_NodeData>::append(const int data_id, const int subtree_size) {
  const size_t new_id = node_count_ + 1;

  set_data(new_id, data_id);
  set_lmld(new_id, new_id - subtree_size + 1);
}

template<class _NodeData>
void TASMTree<_NodeData>::append_to(
    interfaces::tasm::PostorderQueueInterface<_NodeData>* postorder_queue) {
  for (size_t i = 1; i <= node_count_; ++i) {
    postorder_queue->append(data(i), i - lmld(i) + 1);
  }
}

template<class _NodeData>
const _NodeData& TASMTree<_NodeData>::data(const size_t index) const {
  return label_map_.at(data_id(index));
}

template<class _NodeData>
int TASMTree<_NodeData>::data_id(const size_t index) const {
  return data_ids_.at(index + start_ - 1);
}

template<class _NodeData>
size_t TASMTree<_NodeData>::node_count() const {
  return node_count_;
}

template<class _NodeData>
size_t TASMTree<_NodeData>::lmld(const size_t index) const {
  return (lmlds_.at(index + start_ - 1) - start_ + 1);
}

template<class _NodeData>
void TASMTree<_NodeData>::set_data(const size_t index, const _NodeData& data) {
  const int data_id = label_map_.insert(std::move(data));

  set_data(index, data_id);
}

template<class _NodeData>
void TASMTree<_NodeData>::set_data(const size_t index, const int data_id) {
  if (data_ids_.size() <= (index + start_ - 1)) {
    data_ids_.resize(index + start_);
  }

  data_ids_.at(index + start_ - 1) = data_id;

  if (index >= node_count_) {
    node_count_ = index;
  }
}

template<class _NodeData>
void TASMTree<_NodeData>::set_lmld(const size_t index, const size_t lmld) {
  if (leaf_count_ > 0) {
    leaf_count_ = 0;
  }

  if (lmlds_.size() <= (index + start_ - 1)) {
    lmlds_.resize(index + start_);
  }


  lmlds_.at(index + start_ - 1) = lmld + start_ - 1;

  if (index >= node_count_) {
    node_count_ = index;
  }
}

template<class _NodeData>
LabelHashMap<_NodeData>& TASMTree<_NodeData>::label_map() const {
  return label_map_;
}

template<class _NodeData>
bool TASMTree<_NodeData>::leaf(const size_t index) const {
  return (lmld(index) == index);
}

template<class _NodeData>
int TASMTree<_NodeData>::leaf_count() {
  if (leaf_count_ <= 0) {
    leaf_count_ = 0;
    for (size_t i = 1; i <= node_count_; ++i) {
      if (leaf(i)) {
        ++leaf_count_;
      }
    }
  }

  return leaf_count_;
}

template<class _NodeData>
const std::vector<int> TASMTree<_NodeData>::kr() {
  int leaf_count = this->leaf_count();
  size_t node_count = this->node_count();
  std::vector<int> kr(leaf_count + 1);
  std::vector<bool> visited(node_count + 1); // already initialized to false
  size_t lmld = 0;

  for (size_t i = node_count; i >= 1; --i) {
    lmld = this->lmld(i);
    if (!visited.at(lmld)) {
      kr.at(leaf_count) = i;
      visited.at(lmld) = true;
      --leaf_count;
    }
  }

  return kr;
}

template<class _NodeData>
int TASMTree<_NodeData>::root() const {
  return node_count();
}

template<class _NodeData>
int TASMTree<_NodeData>::global_id(const size_t index) const {
  return (start_ + index);
}

} // namespace data_structures

#endif // DATA_STRUCTURES_TASM_TREE_H
