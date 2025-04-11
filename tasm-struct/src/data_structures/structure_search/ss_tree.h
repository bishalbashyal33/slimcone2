#ifndef DATA_STRUCTURES_STRUCTURE_SEARCH_SS_TREE_H
#define DATA_STRUCTURES_STRUCTURE_SEARCH_SS_TREE_H

#include "tasm_tree.h"

namespace data_structures {

/**
 * Implements the Tree used in the Structure Search algorithm (for the query).
 * Extends the TASMTree by a depth member.
 */
template<class _NodeData = nodes::TASMStringNodeData>
class SSTree : public TASMTree<_NodeData> {
private:
  int depth_{};

public:
  SSTree() = delete;
  SSTree(LabelHashMap<_NodeData>& label_map);
  virtual ~SSTree();

  void set_depth(const int depth);
  const int depth() const;
};

template<class _NodeData>
SSTree<_NodeData>::SSTree(LabelHashMap<_NodeData>& label_map)
  : TASMTree<_NodeData>(label_map)
{ }

template<class _NodeData>
SSTree<_NodeData>::~SSTree()
{ }

template<class _NodeData>
void SSTree<_NodeData>::set_depth(const int depth) {
  depth_ = depth;
}

template<class _NodeData>
const int SSTree<_NodeData>::depth() const {
  return depth_;
}

}

#endif // DATA_STRUCTURES_STRUCTURE_SEARCH_SS_TREE_H