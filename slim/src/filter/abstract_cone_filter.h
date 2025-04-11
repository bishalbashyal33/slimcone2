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

#ifndef FILTER_ABSTRACT_CONE_FILTER_H
#define FILTER_ABSTRACT_CONE_FILTER_H

#include "abstract_filter.h"

#include <unordered_set>
#include <vector>
#include <utility>
#include <cassert>

namespace filter {

// common Cone code
template<class _Index, class _TEDAlgorithm>
class AbstractConeFilter : public AbstractFilter<_Index, _TEDAlgorithm>  {
public: // types
  // make inherited type definitions visible
  using typename AbstractFilter<_Index, _TEDAlgorithm>::NodeData;
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

public: // member functions
  // make inherited member functions visible
  using AbstractFilter<_Index, _TEDAlgorithm>::name;

  AbstractConeFilter() = delete;
  AbstractConeFilter(
    LogType& log,
    _Index& index,
    _TEDAlgorithm& edit_distance,
    const std::string& name);
  ~AbstractConeFilter();

public: // member variables
  // make inherited member variables visible
  using AbstractFilter<_Index, _TEDAlgorithm>::log_;
  using AbstractFilter<_Index, _TEDAlgorithm>::global_statistics_;
  using AbstractFilter<_Index, _TEDAlgorithm>::round_statistics_;
  using AbstractFilter<_Index, _TEDAlgorithm>::current_round_statistics_;

protected: // member types
  struct Upward {
    using Position = unsigned int;

    static inline void advance(Position& position) { ++position; }
    static inline bool has_next_position(Position position, const List& list) {
      return position < list.size();
    }
  };

  struct Downward {
    using Position = int;

    static inline void advance(Position& position) { --position; }
    static inline bool has_next_position(Position position, const List& list) {
      return position > 0;
    }
  };

  template<class Direction = Upward>
  struct ListPosition {
    ListPosition(
        typename Direction::Position position,
        const List& list,
        const unsigned int list_index)
      : position(position), list(list), list_index(list_index) {}

    // common
    inline NodeIdType subtree_id() { return list.at(position); }

    // Direction-related
    inline void advance() { Direction::advance(position); }
    inline bool has_next_position() {
      return Direction::has_next_position(position, list);
    }

    typename Direction::Position position;
    const List& list;
    const unsigned int list_index;
  };

protected: // member functions
  inline void initialize(const TokenIdSet& query_token_id_set);
  template<class DownwardListPos, class UpwardListPos>
  void initialize_list_positions(DownwardListPos& down, UpwardListPos& up);
  inline void update_lower_bound_buckets(
    const unsigned int real_lower_bound,
    const NodeIdType subtree_id);
  bool evaluate_bucket(
    Bucket& bucket,
    const unsigned int real_lower_bound,
    POQueue<NodeData>& query_tree,
    Ranking& ranking);
  bool evaluate_subtree(
    const unsigned int lower_bound,
    const NodeIdType subtree_id,
    POQueue<NodeData>& query_tree,
    Ranking& ranking);
  bool round_prologue(POQueue<NodeData>& query_tree, Ranking& ranking);
  inline bool round_epilogue(const Ranking& ranking);

protected: // member variables
  // make inherited member variables visible
  using AbstractFilter<_Index, _TEDAlgorithm>::index_;
  using AbstractFilter<_Index, _TEDAlgorithm>::query_size_;

  std::vector<Bucket> lower_bound_buckets_{};
  unsigned int max_lower_bound_;
  unsigned int potential_lower_bound_{};
  unsigned int previous_potential_lower_bound_{};

private: // member functions
  // make inherited member functions visible
  using AbstractFilter<_Index, _TEDAlgorithm>::is_final_ranking;
  using AbstractFilter<_Index, _TEDAlgorithm>::update_ranking;
};

#include "abstract_cone_filter_impl.h"

} // namespace filter

#endif // FILTER_ABSTRACT_CONE_FILTER_H
