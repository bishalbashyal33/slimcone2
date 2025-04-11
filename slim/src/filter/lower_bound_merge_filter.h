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

#ifndef FILTER_LOWER_BOUND_MERGE_FILTER_H
#define FILTER_LOWER_BOUND_MERGE_FILTER_H

#include "abstract_filter.h"

namespace filter {

template<class _Index, class _TEDAlgorithm>
class LowerBoundMergeFilter : public AbstractConeFilter<_Index, _TEDAlgorithm> {
public:
  // make inherited type definitions visible
  using typename AbstractFilter<_Index, _TEDAlgorithm>::NodeData;
  using typename AbstractFilter<_Index, _TEDAlgorithm>::SizeType;
  using typename AbstractFilter<_Index, _TEDAlgorithm>::NodeIdType;
  using typename AbstractFilter<_Index, _TEDAlgorithm>::TokenIdSet;
  template<class _Type> using POQueue =
    typename AbstractFilter<_Index, _TEDAlgorithm>::template POQueue<_Type>;
  using typename AbstractFilter<_Index, _TEDAlgorithm>::Ranking;
  using typename AbstractFilter<_Index, _TEDAlgorithm>::LogType;

public: // member functions
  // make inherited member functions visible
  using AbstractFilter<_Index, _TEDAlgorithm>::name;

  LowerBoundMergeFilter() = delete;
  LowerBoundMergeFilter(
    LogType& log,
    _Index& index,
    _TEDAlgorithm& edit_distance);
  ~LowerBoundMergeFilter();

  Ranking top_k(
    const TokenIdSet& query_token_id_set,
    POQueue<NodeData>& query_tree,
    const unsigned int k = 1);

public: // member variables
  // make inherited member variables visible
  using AbstractFilter<_Index, _TEDAlgorithm>::log_;
  using AbstractFilter<_Index, _TEDAlgorithm>::global_statistics_;
  using AbstractFilter<_Index, _TEDAlgorithm>::round_statistics_;
  using AbstractFilter<_Index, _TEDAlgorithm>::current_round_statistics_;
  using AbstractFilter<_Index, _TEDAlgorithm>::interval_;

private: // member types
  // make inherited member types visible
  using typename AbstractConeFilter<_Index, _TEDAlgorithm>::Downward;
  using typename AbstractConeFilter<_Index, _TEDAlgorithm>::Upward;
  template<class Direction>
  using ListPosition =
    typename AbstractConeFilter<_Index, _TEDAlgorithm>::template ListPosition<Direction>;

  template<class Direction>
  struct LBMFListPosition : ListPosition<Direction> {
    using ListPosition<Direction>::position;
    using ListPosition<Direction>::list;
    using ListPosition<Direction>::list_index;

    LBMFListPosition(
        typename Direction::Position position,
        const std::vector<unsigned int>& list,
        unsigned int list_index)
      : ListPosition<Direction>(position, list, list_index) {}

    inline int prev_position() { return Direction::prev_position(position); }
  };

  struct LBMFUpward : Upward {
    static inline int prev_position(int position) { return position - 1; }
  };

  struct LBMFDownward : Downward {
    static inline int prev_position(int position) { return position + 1; }
  };

  using DownwardListPos = LBMFListPosition<LBMFDownward>;
  using UpwardListPos = LBMFListPosition<LBMFUpward>;
  using ListInfoPair = std::pair<DownwardListPos, UpwardListPos>;

private: // member functions
  // make inherited member functions visible
  using AbstractConeFilter<_Index, _TEDAlgorithm>::initialize;
  using AbstractConeFilter<_Index, _TEDAlgorithm>::initialize_list_positions;
  using AbstractConeFilter<_Index, _TEDAlgorithm>::update_lower_bound_buckets;
  using AbstractConeFilter<_Index, _TEDAlgorithm>::evaluate_bucket;
  using AbstractConeFilter<_Index, _TEDAlgorithm>::evaluate_subtree;
  using AbstractConeFilter<_Index, _TEDAlgorithm>::round_prologue;
  using AbstractConeFilter<_Index, _TEDAlgorithm>::round_epilogue;

private: // member variables
  // make inherited member variables visible
  using AbstractFilter<_Index, _TEDAlgorithm>::name_;
  using AbstractFilter<_Index, _TEDAlgorithm>::index_;
  using AbstractFilter<_Index, _TEDAlgorithm>::query_size_;
  using AbstractFilter<_Index, _TEDAlgorithm>::query_token_id_map_;
  using AbstractConeFilter<_Index, _TEDAlgorithm>::potential_lower_bound_;
  using AbstractConeFilter<_Index, _TEDAlgorithm>::previous_potential_lower_bound_;
};

#include "lower_bound_merge_filter_impl.h"

} // namespace filter

#endif // FILTER_LOWER_BOUND_MERGE_FILTER_H
