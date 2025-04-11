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

#ifndef FILTER_UPDATABLE_SHIN_CONE_FILTER_H
#define FILTER_UPDATABLE_SHIN_CONE_FILTER_H

#include "abstract_cone_filter.h"

namespace filter {

template<class _Index, class _TEDAlgorithm>
class UpdatableShinConeFilter : public AbstractConeFilter<_Index, _TEDAlgorithm> {
public: // types
  // make inherited type definitions visible
  using typename AbstractFilter<_Index, _TEDAlgorithm>::NodeData;
  using typename AbstractFilter<_Index, _TEDAlgorithm>::TokenIdType;
  using typename AbstractFilter<_Index, _TEDAlgorithm>::SizeType;
  using typename AbstractFilter<_Index, _TEDAlgorithm>::NodeIdType;
  using typename AbstractFilter<_Index, _TEDAlgorithm>::TokenIdSet;
  using typename AbstractFilter<_Index, _TEDAlgorithm>::List;
  template<class _Type>
  using POQueue =
    typename AbstractFilter<_Index, _TEDAlgorithm>::template POQueue<_Type>;
  using typename AbstractFilter<_Index, _TEDAlgorithm>::Ranking;
  using typename AbstractFilter<_Index, _TEDAlgorithm>::Bucket;
  using typename AbstractFilter<_Index, _TEDAlgorithm>::LogType;

public: // member variables
  // make inherited member variables visible
  using AbstractFilter<_Index, _TEDAlgorithm>::log_;
  using AbstractFilter<_Index, _TEDAlgorithm>::global_statistics_;
  using AbstractFilter<_Index, _TEDAlgorithm>::round_statistics_;
  using AbstractFilter<_Index, _TEDAlgorithm>::current_round_statistics_;
  using AbstractFilter<_Index, _TEDAlgorithm>::interval_;

public: // member functions
  // make inherited member functions visible
  using AbstractFilter<_Index, _TEDAlgorithm>::name;

  UpdatableShinConeFilter() = delete;
  UpdatableShinConeFilter(LogType& log, _Index& index, _TEDAlgorithm& edit_distance);
  ~UpdatableShinConeFilter();

  Ranking top_k(
    const TokenIdSet& query_token_id_set,
    POQueue<NodeData>& query_tree,
    const size_t k = 1);

private: // member variables
  // make inherited member variables visible
  using AbstractFilter<_Index, _TEDAlgorithm>::name_;
  using AbstractFilter<_Index, _TEDAlgorithm>::index_;
  using AbstractFilter<_Index, _TEDAlgorithm>::query_size_;
  using AbstractFilter<_Index, _TEDAlgorithm>::query_token_id_map_;
  using AbstractFilter<_Index, _TEDAlgorithm>::shared_token_ids_;
  using AbstractFilter<_Index, _TEDAlgorithm>::unshared_token_ids_;
  using AbstractConeFilter<_Index, _TEDAlgorithm>::potential_lower_bound_;
  using AbstractConeFilter<_Index, _TEDAlgorithm>::previous_potential_lower_bound_;

  common::Statistics<>::StatItem pathstatistics_helper_;

private: // member functions
  // make inherited member functions visible
  using AbstractFilter<_Index, _TEDAlgorithm>::already_processed;
  using AbstractFilter<_Index, _TEDAlgorithm>::track_as_processed;
  using AbstractConeFilter<_Index, _TEDAlgorithm>::initialize;
  using AbstractConeFilter<_Index, _TEDAlgorithm>::update_lower_bound_buckets;
  using AbstractConeFilter<_Index, _TEDAlgorithm>::evaluate_bucket;
  using AbstractConeFilter<_Index, _TEDAlgorithm>::evaluate_subtree;
  using AbstractConeFilter<_Index, _TEDAlgorithm>::round_prologue;
  using AbstractConeFilter<_Index, _TEDAlgorithm>::round_epilogue;

  bool process_smaller_subtrees(
    unsigned int list_index,
    std::vector<std::vector<Bucket>>& size_buckets,
    const SizeType min_subtree_size,
    POQueue<NodeData>& query_tree,
    Ranking& ranking);
  bool process_larger_subtrees(
    unsigned int list_index,
    std::vector<std::pair<typename List::const_iterator, Bucket>>& parent_ids,
    POQueue<NodeData>& query_tree,
    Ranking& ranking);
};

#include "updatable_shin_cone_filter_impl.h"

} // namespace filter

#endif // FILTER_UPDATABLE_SHIN_CONE_FILTER_H
