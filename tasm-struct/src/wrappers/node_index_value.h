#ifndef WRAPPERS_NODE_INDEX_VALUE_H
#define WRAPPERS_NODE_INDEX_VALUE_H

namespace wrappers {

class NodeIndexValue {
public:  
  int size_{};
  int depth_{};
  int data_id_{};
  int structure_id_{};
  int postorder_id_{};
  int parent_id_{};

public:
  NodeIndexValue();
  NodeIndexValue(
    const int size,
    const int depth,
    const int data_id,
    const int structure_id,
    const int postorder_id,
    const int parent_id_);
  ~NodeIndexValue();

  const int size() const;
  const int depth() const;
  const int data_id() const;
  const int structure_id() const;
  const int postorder_id() const;
  const int parent_id() const;
};

NodeIndexValue::NodeIndexValue()
  : NodeIndexValue(0, 0, 0, 0, 0, 0)
{ }

NodeIndexValue::NodeIndexValue(
    const int size,
    const int depth,
    const int data_id,
    const int structure_id,
    const int postorder_id,
    const int parent_id)
  : size_(size),
    depth_(depth),
    data_id_(data_id),
    structure_id_(structure_id),
    postorder_id_(postorder_id),
    parent_id_(parent_id)
{ }

NodeIndexValue::~NodeIndexValue() { }

const int NodeIndexValue::size() const {
  return size_;
}

const int NodeIndexValue::depth() const {
  return depth_;
}

const int NodeIndexValue::data_id() const {
  return data_id_;
}

const int NodeIndexValue::structure_id() const {
  return structure_id_;
}

const int NodeIndexValue::postorder_id() const {
  return postorder_id_;
}

const int NodeIndexValue::parent_id() const {
  return parent_id_;
}
  
} // namespace wrappers

#endif // WRAPPERS_NODE_INDEX_VALUE_H
