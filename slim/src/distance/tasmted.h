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

#ifndef DISTANCE_TASMTED_H
#define DISTANCE_TASMTED_H

#include "array_2d.h"
#include "tasm_dynamic.h"
#include "tasm_tree.h"

#include <vector>

namespace distance {

template<class _NodeData, class _Costs>
class TASMTED {
public:
  template<class _Type>
  using POQueue = data_structures::TASMTree<_Type>;

  using EditDistance = tasm::TASMDynamic<_NodeData, _Costs, POQueue, POQueue>;

public:
  TASMTED();
  ~TASMTED();

  int compute(POQueue<_NodeData>& query, POQueue<_NodeData>& subtree);

private:
  using DoubleArray2D = data_structures::Array2D<double>;
  using IntVector = std::vector<int>;

private:
  IntVector query_kr_{};
  EditDistance tasm_dynamic_{};
};

#include "tasmted_impl.h"

} // distance

#endif // DISTANCE_TASMTED_H
