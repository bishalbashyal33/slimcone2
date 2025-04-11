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

#ifndef TASM_TASM_DYNAMIC_H
#define TASM_TASM_DYNAMIC_H

#include "array_2d.h"

namespace tasm {

template<class _NodeData, class _Costs, template<class...> class _QueryTreeType, template<class...> class _DocumentTreeType>
class TASMDynamic {
public:
  TASMDynamic();
  ~TASMDynamic();

  int prefix_distance(
    const int q,
    const int d,
    const _QueryTreeType<_NodeData>& query,
    const _DocumentTreeType<_NodeData>& subtree,
    data_structures::Array2D<double>& pd,
    data_structures::Array2D<double>& td) const;
};

template<class _NodeData, class _Costs, template<class...> class _QueryTreeType, template<class...> class _DocumentTreeType>
TASMDynamic<_NodeData, _Costs, _QueryTreeType, _DocumentTreeType>::TASMDynamic() {}

template<class _NodeData, class _Costs, template<class...> class _QueryTreeType, template<class...> class _DocumentTreeType>
TASMDynamic<_NodeData, _Costs, _QueryTreeType, _DocumentTreeType>::~TASMDynamic()
{}

template<class _NodeData, class _Costs, template<class...> class _QueryTreeType, template<class...> class _DocumentTreeType>
int TASMDynamic<_NodeData, _Costs, _QueryTreeType, _DocumentTreeType>::prefix_distance(
    const int q,
    const int d,
    const _QueryTreeType<_NodeData>& query,
    const _DocumentTreeType<_NodeData>& subtree,
    data_structures::Array2D<double>& pd,
    data_structures::Array2D<double>& td) const {

  _Costs costs;
  pd[query.lmld(q) - 1][subtree.lmld(d) - 1] = 0;
  int document_prefix_size = -1;

  for (int dd = subtree.lmld(d); dd <= d; ++dd) {
    const _NodeData& document_data = subtree.data(dd);
    double insert_costs = costs.ins(&document_data);

    pd[query.lmld(q) - 1][dd] = pd[query.lmld(q) - 1][dd - 1] + insert_costs;

    for (int dq = query.lmld(q); dq <= q; ++dq) {
      const _NodeData& query_data = query.data(dq);
      double delete_costs = costs.del(&query_data);

      pd[dq][subtree.lmld(d) - 1] =
        pd[dq - 1][subtree.lmld(d) - 1] + delete_costs;

      if ((query.lmld(dq) == query.lmld(q)) &&
          (subtree.lmld(dd) == subtree.lmld(d))) {
        pd[dq][dd] = std::min({
          pd[dq - 1][dd] + delete_costs,
          pd[dq][dd - 1] + insert_costs,
          pd[dq - 1][dd - 1] + costs.ren(&query_data, &document_data)
        });

        td[dq][dd] = pd[dq][dd];
      } else {
        pd[dq][dd] = std::min({
          pd[dq - 1][dd] + delete_costs,
          pd[dq][dd - 1] + insert_costs,
          pd[query.lmld(dq) - 1][subtree.lmld(dd) - 1] + td[dq][dd]
        });
      }
    }
  }

  return document_prefix_size;
}

}

#endif // TASM_TASM_DYNAMIC_H
