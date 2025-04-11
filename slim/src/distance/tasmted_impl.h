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

template<class _NodeData, class _Costs>
TASMTED<_NodeData, _Costs>::TASMTED() {}

template<class _NodeData, class _Costs>
TASMTED<_NodeData, _Costs>::~TASMTED() {}

template<class _NodeData, class _Costs>
int TASMTED<_NodeData, _Costs>::compute(
    POQueue<_NodeData>& query,
    POQueue<_NodeData>& subtree) {
  DoubleArray2D pd(query.node_count() + 1, subtree.node_count() + 1);
  DoubleArray2D td(query.node_count() + 1, subtree.node_count() + 1);

  IntVector subtree_kr = std::move(subtree.kr());

  // query key roots, since the query does not change, we only compute these once
  if (query_kr_.empty()) {
    query_kr_ = std::move(query.kr());
  }

  for (size_t d = 1; d < subtree_kr.size(); ++d) {
    for (size_t q = 1; q < query_kr_.size(); ++q) {
      tasm_dynamic_.prefix_distance(
        query_kr_.at(q),
        subtree_kr.at(d),
        query,
        subtree,
        pd,
        td
      );
    }
  }

  return static_cast<int>(td[query.root()][subtree.root()]);

}
