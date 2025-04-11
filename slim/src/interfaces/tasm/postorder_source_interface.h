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
