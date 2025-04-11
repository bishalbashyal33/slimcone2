#ifndef DATA_STRUCTURES_TASM_RING_BUFFER_TREE_H
#define DATA_STRUCTURES_TASM_RING_BUFFER_TREE_H

#include "label_hash_map.h"
#include "string_node_data.h"
#include "postorder_queue_interface.h"
#include "postorder_source_interface.h"

// TODO: own namespace (or second level) because it's only used in the TASM impl.

namespace data_structures {

/**
 * Implements the RingBuffer used in the TASM algorithm. Some method
 * implementations are equivalent to the implements in the TASMTree class.
 * Of course, an abstract base class would reduce the code complexity/duplication
 * but from a performance point of view the current version is benefitial, i.e.,
 * faster.
 */
template<class _NodeData = nodes::StringNodeData>
class RingBufferTree : public interfaces::tasm::PostorderQueueInterface<_NodeData>, interfaces::tasm::PostorderSourceInterface<_NodeData> {
public:
  LabelHashMap<_NodeData>& label_map_;
  std::vector<int>& lmlds_;
  std::vector<_NodeData>& data_;
  std::vector<int>& data_ids_;
  int start_{};
  int node_count_{};
  int size_{};
  double max_costs_{};
  bool track_labels_{};
  int next_data_id_{};

public:
  void resize(const int size);

public:
  RingBufferTree() = delete;
  RingBufferTree(const int size, LabelHashMap<_NodeData>& label_map,
    std::vector<int>& lmlds, std::vector<_NodeData>& data,
    std::vector<int>& data_ids, const bool track_labels = false);
  RingBufferTree(const int start, const int node_count,
    std::vector<int>& lmlds, std::vector<_NodeData>& data,
    std::vector<int>& data_ids, LabelHashMap<_NodeData>& label_map,
    const int size, const bool track_labels = false);
  virtual ~RingBufferTree();

  // pure virtual, see src/interfaces/postorder_queue_interface.h
  virtual void append(_NodeData label, const int subtree_size = 1) override;

  // pure virtual, see src/interfaces/postorder_source_interface.h
  virtual void append_to(
    interfaces::tasm::PostorderQueueInterface<_NodeData>* postorder_queue) override;

  // additional
  const int lmld(const int index) const;
  const int node_count() const;
  const _NodeData& data(const int index) const;
  const bool leaf(const int index) const;
  void set_lmld(const int index, const int lmld);
  void set_data(const int index, _NodeData data);
  const int data_id(const int index) const;
  const bool equal_data(
    const int index_this,
    const RingBufferTree<_NodeData>& other,
    const int index_other) const;
  const int size() const;
  const int global_id(const int index) const;
  const int smallest_valid_id() const;
  const int leaf_kr(const int index) const;
  RingBufferTree<_NodeData> subtree(const int index) const;
  const int subtree_root() const;
  const std::vector<int> kr();
  const int leaf_count();
  const double max_costs() const;

  // operators
  inline bool operator==(const RingBufferTree<_NodeData>& other) const;
  inline bool operator<(const RingBufferTree<_NodeData>& other) const;
};

template<class _NodeData>
RingBufferTree<_NodeData>::RingBufferTree(const int size,
  LabelHashMap<_NodeData>& label_map, std::vector<int>& lmlds,
  std::vector<_NodeData>& data, std::vector<int>& data_ids,
  const bool track_labels)
: label_map_(label_map), lmlds_(lmlds), data_(data), data_ids_(data_ids),
  size_(size), track_labels_(track_labels)
{ }

template<class _NodeData>
RingBufferTree<_NodeData>::RingBufferTree(const int start,
  const int node_count, std::vector<int>& lmlds,
  std::vector<_NodeData>& data, std::vector<int>& data_ids,
  LabelHashMap<_NodeData>& label_map, const int size, const bool track_labels)
: RingBufferTree(size, label_map, lmlds, data, data_ids)
{
  start_ = start;
  node_count_ = node_count;
  track_labels_ = track_labels;
}

template<class _NodeData>
RingBufferTree<_NodeData>::~RingBufferTree() { }

template<class _NodeData>
void RingBufferTree<_NodeData>::append(_NodeData data, const int subtree_size)
{
  const int new_id = node_count_ + 1;
  set_data(new_id, std::move(data));
  set_lmld(new_id, new_id - subtree_size + 1);
}

template<class _NodeData>
void RingBufferTree<_NodeData>::append_to(
  interfaces::tasm::PostorderQueueInterface<_NodeData>* postorder_queue)
{
  for (int i = 1; i <= node_count_; ++i) {
    postorder_queue->append(data(i), i - lmld(i) + 1);
  }
}

template<class _NodeData>
const int RingBufferTree<_NodeData>::lmld(const int index) const {
  return std::min(index,
    lmlds_.at((index + start_ - 1) % size_) - start_ + 1
  );
}

template<class _NodeData>
const int RingBufferTree<_NodeData>::node_count() const {
  return node_count_;
}

template<class _NodeData>
const _NodeData& RingBufferTree<_NodeData>::data(const int index) const {
  return data_.at((index + start_ - 1) % size_);
}

template<class _NodeData>
const bool RingBufferTree<_NodeData>::leaf(const int index) const {
  return (lmld(index) == index);
}

template<class _NodeData>
void RingBufferTree<_NodeData>::set_lmld(const int index, const int lmld) {
  if (node_count_ < index) {
    node_count_ = index;
  }

  lmlds_.at((index + start_ - 1) % size_) = lmld + start_ - 1;

  if ((lmld >= smallest_valid_id()) && (leaf_kr(lmld) < index)) {
    if (node_count_ < lmld) {
      node_count_ = lmld;
    }

    lmlds_.at((lmld + start_ - 1) % size_) = index + start_ - 1;
  }
}

template<class _NodeData>
void RingBufferTree<_NodeData>::set_data(const int index, _NodeData data) {
  if (node_count_ < index) {
    node_count_ = index;
  }

  int id = (track_labels_ ? label_map_.insert(data) : ++next_data_id_);
  data_.at((index + start_ - 1) % size_) = std::move(data);

  // adjust max_costs
  max_costs_ = std::max(max_costs_, data.get_cost());

  data_ids_.at((index + start_ - 1) % size_) = id;
}

template<class _NodeData>
const int RingBufferTree<_NodeData>::data_id(const int index) const {
  return data_ids_.at((index + start_ - 1) % size_);
}

template<class _NodeData>
const bool RingBufferTree<_NodeData>::equal_data(
    const int index_this,
    const RingBufferTree<_NodeData>& other,
    const int index_other) const {
  try {
    return (data_id(index_this) == other.data_id(index_other));
  } catch(const std::exception& e) {
    return (data(index_this) == other.data(index_other));
  }
}

template<class _NodeData>
const int RingBufferTree<_NodeData>::size() const {
  return size_;
}

template<class _NodeData>
const int RingBufferTree<_NodeData>::global_id(const int index) const {
  return (start_ + index);
}

template<class _NodeData>
const int RingBufferTree<_NodeData>::smallest_valid_id() const {
  int smallest_valid_id = ((node_count_ > 0) ? 1 : 0);
  return std::max<int>(smallest_valid_id, (node_count_ - size_ + 1));
}

template<class _NodeData>
const int RingBufferTree<_NodeData>::leaf_kr(const int index) const {
  if (leaf(index)) {
    return lmlds_.at((index + start_ - 1) % size_) - start_ + 1;
  }

  return 0;
}

template<class _NodeData>
RingBufferTree<_NodeData> RingBufferTree<_NodeData>::subtree(const int index) const
{
  const int subtree_start = (lmld(index) + start_ - 1);
  const int subtree_node_count = (index - lmld(index) + 1);

  return std::move(RingBufferTree<_NodeData>(
    subtree_start, subtree_node_count, lmlds_, data_, data_ids_, label_map_,
    size_
  ));
}

template<class _NodeData>
inline bool RingBufferTree<_NodeData>::operator==(
    const RingBufferTree<_NodeData>& other) const {
  if (node_count_ != other.node_count_) {
    return false;
  }

  if (size_ != other.lmlds_.size()) {
    return false;
  }

  if (size_ != other.data_.size()) {
    return false;
  }

  int start = std::max(1, (node_count_ - size_ + 1));
  for (int i = start; i <= node_count_; ++i) {
    if ((lmld(i) != other.lmld(i)) || !equal_data(i, other, i)) {
      return false;
    }
  }

  return true;
}

template<class _NodeData>
inline bool RingBufferTree<_NodeData>::operator<(
    const RingBufferTree<_NodeData>& other) const {
  if (node_count() == other.node_count()) {
    if (data_ids_ == other.data_ids_) {
      return lmlds_ < other.lmlds_;
    }

    return data_ids_ < other.data_ids_;
  }

  return node_count() < other.node_count();
}

template<class _NodeData>
const int RingBufferTree<_NodeData>::subtree_root() const {
  return node_count();
}

template<class _NodeData>
const std::vector<int> RingBufferTree<_NodeData>::kr() {
  int leaf_count = this->leaf_count();
  int node_count = this->node_count();
  std::vector<int> kr(leaf_count + 1);
  std::vector<bool> visited(node_count + 1); // already initialized to false
  int lmld = 0;

  for (int i = node_count; i >= 1; --i) {
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
const int RingBufferTree<_NodeData>::leaf_count() {
  int leaf_count = 0;
  for (int i = 1; i <= node_count_; ++i) {
    if (leaf(i)) {
      ++leaf_count;
    }
  }

  return leaf_count;
}

template<class _NodeData>
void RingBufferTree<_NodeData>::resize(const int size) {
  lmlds_.resize(size);
  data_.resize(size);
  data_ids_.resize(size);
  size_ = size;
}

template<class _NodeData>
const double RingBufferTree<_NodeData>::max_costs() const {
  return max_costs_;
}

} // namespace data_structures

#endif // DATA_STRUCTURES_TASM_RING_BUFFER_TREE_H
