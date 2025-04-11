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

#ifndef FILTER_LABEL_OVERLAP_FILTER_H
#define FILTER_LABEL_OVERLAP_FILTER_H

#include "abstract_filter.h"

#include <set>

namespace filter {

template<class _Index, class _TEDAlgorithm>
class LabelOverlapFilter : public AbstractFilter<_Index, _TEDAlgorithm> {
public: // member types
  // make inherited type definitions visible
  using typename AbstractFilter<_Index, _TEDAlgorithm>::TokenIdType;
  using typename AbstractFilter<_Index, _TEDAlgorithm>::SizeType;
  using typename AbstractFilter<_Index, _TEDAlgorithm>::NodeIdType;

  using typename AbstractFilter<_Index, _TEDAlgorithm>::NodeData;
  using typename AbstractFilter<_Index, _TEDAlgorithm>::Ranking;
  template<class _Type>
  using POQueue = typename AbstractFilter<_Index, _TEDAlgorithm>::template POQueue<_Type>;
  using typename AbstractFilter<_Index, _TEDAlgorithm>::LogType;

  using TokenIdSet = typename _Index::TokenIdSet;
  using List = typename _Index::List;

  // new type definitions
  using SubtreeLbPair = std::pair<NodeIdType, unsigned int>;
  using SubtreeLbPairs = std::vector<SubtreeLbPair>;

public: // member functions
  // make inherited member functions visible
  using AbstractFilter<_Index, _TEDAlgorithm>::name;

  LabelOverlapFilter() = delete;
  LabelOverlapFilter(
    LogType& log,
    _Index& index,
    _TEDAlgorithm& edit_distance);
  ~LabelOverlapFilter();

  Ranking top_k(
    const TokenIdSet& query_token_id_set,
    POQueue<NodeData>& query_tree,
    const size_t k = 1);

public: // member variables
  // make inherited member variables visible
  using AbstractFilter<_Index, _TEDAlgorithm>::global_statistics_;
  using AbstractFilter<_Index, _TEDAlgorithm>::round_statistics_;
  using AbstractFilter<_Index, _TEDAlgorithm>::current_round_statistics_;
  using AbstractFilter<_Index, _TEDAlgorithm>::log_;
  using AbstractFilter<_Index, _TEDAlgorithm>::interval_;

private: // member functions
  // make inherited member functions visible
  using AbstractFilter<_Index, _TEDAlgorithm>::initialize;
  using AbstractFilter<_Index, _TEDAlgorithm>::is_final_ranking;
  using AbstractFilter<_Index, _TEDAlgorithm>::update_ranking;
  using AbstractFilter<_Index, _TEDAlgorithm>::already_processed;
  using AbstractFilter<_Index, _TEDAlgorithm>::track_as_processed;

  SubtreeLbPairs gather_candidates();

private: // member variables
  // make inherited member variables visible
  using AbstractFilter<_Index, _TEDAlgorithm>::name_;
  using AbstractFilter<_Index, _TEDAlgorithm>::index_;
  using AbstractFilter<_Index, _TEDAlgorithm>::query_size_;
  using AbstractFilter<_Index, _TEDAlgorithm>::query_token_id_map_;
  using AbstractFilter<_Index, _TEDAlgorithm>::shared_token_ids_;
  using AbstractFilter<_Index, _TEDAlgorithm>::unshared_token_ids_;
};

#include "label_overlap_filter_impl.h"

} // namespace filter

#endif // FILTER_LABEL_OVERLAP_FILTER_H
