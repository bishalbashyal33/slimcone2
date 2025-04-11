#ifndef INTERFACES_TASM_POSTORDER_SOURCE_H
#define INTERFACES_TASM_POSTORDER_SOURCE_H

#include "string_node_data.h"
#include "postorder_queue_interface.h"

namespace interfaces {

  namespace tasm {

  // PostorderSource interface
  template<class _NodeData = nodes::StringNodeData>
  struct PostorderSourceInterface {
    virtual ~PostorderSourceInterface() { }
    virtual void append_to(PostorderQueueInterface<_NodeData>* postorder_queue) = 0;
  };

  } // namespace tasm

} // namespace interfaces

#endif // INTERFACES_TASM_POSTORDER_SOURCE_H