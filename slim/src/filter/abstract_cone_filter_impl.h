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

#ifndef FILTER_ABSTRACT_CONE_FILTER_IMPL_H
#define FILTER_ABSTRACT_CONE_FILTER_IMPL_H

template<class _Index, class _TEDAlgorithm>
AbstractConeFilter<_Index, _TEDAlgorithm>::AbstractConeFilter(
    LogType& log,
    _Index& index,
    _TEDAlgorithm& edit_distance,
    const std::string& name)
    : AbstractFilter<_Index, _TEDAlgorithm>(log, index, edit_distance, name) {}

template<class _Index, class _TEDAlgorithm>
AbstractConeFilter<_Index, _TEDAlgorithm>::~AbstractConeFilter() {}

template<class _Index, class _TEDAlgorithm>
inline void AbstractConeFilter<_Index, _TEDAlgorithm>::initialize(
    const TokenIdSet& query_token_id_set) {
  AbstractFilter<_Index, _TEDAlgorithm>::initialize(query_token_id_set);

  // one bucket for each possible lower bound
  max_lower_bound_ = 2 * query_size_;
  lower_bound_buckets_.resize(max_lower_bound_);

  log_.log(
    "[",
    name(),
    "] Initializing max lower bound to 2 * |Q| = ",
    max_lower_bound_,
    " and allocating ",
    max_lower_bound_,
    " buckets in lower bound cache.\n");
}

template<class _Index, class _TEDAlgorithm>
template<class DownwardListPos, class UpwardListPos>
void AbstractConeFilter<_Index, _TEDAlgorithm>::initialize_list_positions(
    DownwardListPos& down,
    UpwardListPos& up) {
  const auto& list = down.list; // equivalent to up.list

  // find index of first subtree Ti that has |Ti| >= |Q|
  typename List::const_iterator lbit = std::lower_bound(
    list.cbegin(),
    list.cend(),
    query_size_,
    [this](
        const NodeIdType subtree_id,
        const SizeType query_size) -> bool {
      const SizeType subtree_size = index_.get_size(subtree_id);
      return subtree_size < query_size;
    });

  if (lbit == list.cend()) {
    lbit = std::prev(lbit);
  }

  int index = lbit - list.cbegin();
  int position_first_smaller_subtree = std::max(index - 1, 0);
  int listsize = list.size();
  int position_first_larger_subtree = std::min(index, listsize - 1);

  down.position = position_first_smaller_subtree;
  up.position = position_first_larger_subtree;

  log_.log(
    "[",
    name(),
    "] Initializing list positions for list ",
    down.list_index,
    " and |Q| = ",
    query_size_,
    ". Pointer positions: [(down position = ",
    down.position,
    ", down subtree size = ",
    index_.get_size(down.subtree_id()),
    "), (up position = ",
    up.position,
    ", up subtree size = ",
    index_.get_size(up.subtree_id()),
    ")].\n");
}

template<class _Index, class _TEDAlgorithm>
inline void AbstractConeFilter<_Index, _TEDAlgorithm>::update_lower_bound_buckets(
    const unsigned int lower_bound,
    const NodeIdType subtree_id) {
  lower_bound_buckets_.at(lower_bound).push_back(subtree_id);
  current_round_statistics_.lowerboundcache.record(lower_bound);

  log_.log(
    "[",
    name(),
    "] Caching subtree id ",
    subtree_id,
    " in lower bound cache bucket ",
    lower_bound,
    ".\n");
}

template<class _Index, class _TEDAlgorithm>
bool AbstractConeFilter<_Index, _TEDAlgorithm>::evaluate_bucket(
    Bucket& bucket,
    unsigned int real_lower_bound,
    POQueue<NodeData>& query_tree,
    Ranking& ranking) {
  assert(real_lower_bound <= potential_lower_bound_);

  // sort bucket content (optional)
  std::sort(
    bucket.begin(),
    bucket.end(),
    [](const NodeIdType lhs, const NodeIdType rhs) -> bool {
      return lhs < rhs;
    });

  log_.log(
    "[",
    name(),
    "] Evaluating bucket ",
    real_lower_bound,
    " from the lower bound cache.\n");

  while (!bucket.empty()) {
    NodeIdType subtree_id = bucket.back();

    log_.log("[", name(), "] Evaluating subtree id ", subtree_id, ".\n");

    update_ranking(real_lower_bound, subtree_id, query_tree, ranking);

    current_round_statistics_.lowerboundcacheverifications.inc();

    if (is_final_ranking(ranking, real_lower_bound)) {
      log_.log(
        "[",
        name(),
        "] Terminating in bucket evaluation (",
        "real lower bound = ",
        real_lower_bound,
        " >= R[k].distance = ",
        ranking.front().get_distance(),
        ").\n");

      return true;
    }

    // should have negligible performance impact since it stores PODs
    bucket.pop_back();
  }

  return false;
}

template<class _Index, class _TEDAlgorithm>
bool AbstractConeFilter<_Index, _TEDAlgorithm>::evaluate_subtree(
    const unsigned int lower_bound,
    const NodeIdType subtree_id,
    POQueue<NodeData>& query_tree,
    Ranking& ranking) {
  log_.log("[", name(), "] Evaluating subtree id ", subtree_id, ".\n");

  if (lower_bound > potential_lower_bound_) {
    update_lower_bound_buckets(lower_bound, subtree_id);
  } else {
    update_ranking(lower_bound, subtree_id, query_tree, ranking);

    current_round_statistics_.candidates.inc();

    if (is_final_ranking(ranking, potential_lower_bound_)) {
      log_.log(
        "[",
        name(),
        "] Terminating in subtree evaluation (",
        "potential lower bound = ",
        potential_lower_bound_,
        " >= R[k].distance = ",
        ranking.front().get_distance(), ").\n");
      return true;
    }
  }

  return false;
}

template<class _Index, class _TEDAlgorithm>
bool AbstractConeFilter<_Index, _TEDAlgorithm>::round_prologue(
    POQueue<NodeData>& query_tree,
    Ranking& ranking) {
  log_.log(
    "[",
    name(),
    "] Round prologue for potential lower bound ",
    potential_lower_bound_,
    " (previously ",
    previous_potential_lower_bound_,
    ").\n");

  global_statistics_.lower_bounds.potential.record(potential_lower_bound_);

  // for the sake of a less complex codebase.
  // could be set to (previous_potential_lower_bound_ + 1) for cone and shincone
  // filters but not for the lower bound merge filter (since it may happen that
  // there are two stripes for the same absolute size diff.).
  unsigned int lower_bound = previous_potential_lower_bound_;

  const unsigned int end = std::min(potential_lower_bound_, max_lower_bound_);
  // important here: <=, since we are at the start of the next round and may
  // already have candidates cached in buckets
  for (; lower_bound <= end; ++lower_bound) {
    const bool terminate = evaluate_bucket(
      lower_bound_buckets_.at(lower_bound),
      lower_bound,
      query_tree,
      ranking);

    if (terminate) {
      log_.log("[", name(), "] Terminating after bucket evaluation.\n");
      return true;
    }
  }

  return false;
}

template<class _Index, class _TEDAlgorithm>
inline bool AbstractConeFilter<_Index, _TEDAlgorithm>::round_epilogue(
    const Ranking& ranking)
{
  log_.log(
    "[",
    name(),
    "] Round epilogue for potential lower bound ",
    potential_lower_bound_,
    " (previously ",
    previous_potential_lower_bound_,
    ").\n");

  if (previous_potential_lower_bound_ != potential_lower_bound_) {
    round_statistics_.set(
      previous_potential_lower_bound_, // potential lower bound already incr.
      current_round_statistics_);
    current_round_statistics_.reset();
  }

  log_.log(
    "[",
    name(),
    "] Ranking after potential lower bound ",
    potential_lower_bound_,
    ": ",
    ranking,
    ".\n");

  if (is_final_ranking(ranking, potential_lower_bound_)) {
    log_.log(
      "[",
      name(),
      "] Terminating in round epilogue after adjusting",
      " potential lower bound.\n");
    return true;
  }

  return false;
}

#endif // FILTER_ABSTRACT_CONE_FILTER_IMPL_H
