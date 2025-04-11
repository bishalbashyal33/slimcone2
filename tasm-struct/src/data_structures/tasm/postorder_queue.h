#ifndef DATA_STRUCTURES_POSTORDER_QUEUE_H
#define DATA_STRUCTURES_POSTORDER_QUEUE_H

#include <deque>

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
template<class _NodeData = nodes::TASMStringNodeData>
class PostorderQueue : public interfaces::tasm::PostorderQueueInterface<_NodeData>, public interfaces::tasm::PostorderSourceInterface<_NodeData> {
public:
  std::deque<std::pair<_NodeData, int>> data_{};
  size_t next_element_pop_{};

public:
  PostorderQueue();
  virtual ~PostorderQueue();

  // pure virtual, see src/interfaces/postorder_queue_interface.h
  virtual void append(_NodeData data, const int subtree_size = 1);

  // pure virtual, see src/interfaces/postorder_source_interface.h
  virtual void append_to(
    interfaces::tasm::PostorderQueueInterface<_NodeData>* postorder_queue);

  // additional
  const _NodeData& data(const int index) const;
  const int node_count() const;
  void set_data(const int index, const _NodeData& data);
  void reset();
};

template<class _NodeData>
PostorderQueue<_NodeData>::PostorderQueue() : next_element_pop_(0) { }

template<class _NodeData>
PostorderQueue<_NodeData>::~PostorderQueue() { }

template<class _NodeData>
void PostorderQueue<_NodeData>::append(_NodeData data, const int subtree_size) {
  data_.push_back(std::make_pair(std::move(data), subtree_size));
}

template<class _NodeData>
void PostorderQueue<_NodeData>::append_to(
    interfaces::tasm::PostorderQueueInterface<_NodeData>* postorder_queue) {
  while (next_element_pop_ < data_.size()) {
    const auto& next = data_.at(next_element_pop_++);
    postorder_queue->append(next.first, next.second);
  }
}

template<class _NodeData>
const int PostorderQueue<_NodeData>::node_count() const {
  return data_.size();
}

template<class _NodeData>
void PostorderQueue<_NodeData>::reset() {
  next_element_pop_ = 0;
}

} // namespace data_structures

#endif // DATA_STRUCTURES_TASM_TREE_H
