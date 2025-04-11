#ifndef WRAPPERS_STRUCTURE_INDEX_VALUE_H
#define WRAPPERS_STRUCTURE_INDEX_VALUE_H

#include <list>

#include "dewey_identifier.h"

namespace wrappers {

class StructureIndexValue {
public:
  int labeled_nodes_{};
  int other_nodes_{};
  int depth_{};
  std::vector<int> preorder_ids_{};

public:
  StructureIndexValue();
  StructureIndexValue(
    const int labeled_nodes,
    const int other_nodes,
    const int depth,
    const std::vector<int>& preorder_ids);
  //StructureIndexValue(const StructureIndexValue& other);
  ~StructureIndexValue();

  const int labeled_nodes() const;
  const int other_nodes() const;
  const int depth() const;
  const std::vector<int>& preorder_ids() const;
};

StructureIndexValue::StructureIndexValue() { }

StructureIndexValue::StructureIndexValue(
    const int labeled_nodes,
    const int other_nodes,
    const int depth,
    const std::vector<int>& preorder_ids)
:   labeled_nodes_(labeled_nodes), other_nodes_(other_nodes), depth_(depth),
  preorder_ids_(preorder_ids)
{ }

StructureIndexValue::~StructureIndexValue() { }

const int StructureIndexValue::labeled_nodes() const {
  return labeled_nodes_;
}

const int StructureIndexValue::other_nodes() const {
  return other_nodes_;
}

const int StructureIndexValue::depth() const {
  return depth_;
}

const std::vector<int>& StructureIndexValue::preorder_ids() const
{
  return preorder_ids_;
}

}

#endif // WRAPPERS_STRUCTURE_INDEX_VALUE_H