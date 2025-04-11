#ifndef INTERFACES_TASM_POSTORDER_QUEUE_H
#define INTERFACES_TASM_POSTORDER_QUEUE_H

#include "string_node_data.h"

namespace interfaces {

  namespace tasm {

  // PostorderQueue interface
  template<class _NodeData = nodes::StringNodeData>
  struct PostorderQueueInterface {
    virtual ~PostorderQueueInterface() { }
    virtual void append(_NodeData data, const int subtree_size = 1) = 0;
  };

  } // namespace tasm

} // namespace interfaces

#endif // INTERFACES_TASM_POSTORDER_QUEUE_H