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

#ifndef FILTER_ABSTRACT_FILTER_IMPL_H
#define FILTER_ABSTRACT_FILTER_IMPL_H

/// public

template<class _Index, class _TEDAlgorithm>
AbstractFilter<_Index, _TEDAlgorithm>::AbstractFilter(
    LogType& log,
    _Index& index,
    _TEDAlgorithm& edit_distance,
    const std::string& name)
    : log_(log),
      index_(index),
      edit_distance_(edit_distance),
      name_(name) {}

template<class _Index, class _TEDAlgorithm>
AbstractFilter<_Index, _TEDAlgorithm>::~AbstractFilter() {}

template<class _Index, class _TEDAlgorithm>
inline std::string AbstractFilter<_Index, _TEDAlgorithm>::name() const {
  return name_;
}

/// protected

template<class _Index, class _TEDAlgorithm>
inline void AbstractFilter<_Index, _TEDAlgorithm>::initialize(
    const TokenIdSet& query_token_id_set) {
  query_size_ = query_token_id_set.size();

  // retrieve token-id table for query once
  extract_token_id_map(query_token_id_set);

  log_.log(
    "[",
    name(),
    "] Initializing |Q| to ",
    query_size_,
    " and extracting token id set.\n");
}

template<class _Index, class _TEDAlgorithm>
inline int AbstractFilter<_Index, _TEDAlgorithm>::verify(
    POQueue<NodeData>& query,
    POQueue<NodeData>& subtree) {
  global_statistics_.verifications.total.inc();

  // check if (query, subtree) pairs was already verified
  // if so, return distance instantaneously
  auto it = already_verified_.find(subtree);
  if (it != already_verified_.end()) {
    global_statistics_.verifications.duplicates.inc();
    return it->second;
  }

  const int distance = edit_distance_.compute(query, subtree);
  already_verified_.emplace(subtree, distance);

  global_statistics_.verifications.fresh.inc();

  return distance;
}

template<class _Index, class _TEDAlgorithm>
inline bool AbstractFilter<_Index, _TEDAlgorithm>::is_final_ranking(
    const Ranking& ranking,
    unsigned int lower_bound) const {
  const bool is_final =
    ranking.full() && lower_bound >= ranking.front().get_distance();

  log_.log(
    "[",
    name(),
    "] Checking if we found the final ranking: ",
    is_final,
    ".\n");

  return is_final;
}

template<class _Index, class _TEDAlgorithm>
void AbstractFilter<_Index, _TEDAlgorithm>::update_ranking(
    const unsigned int real_lower_bound,
    const NodeIdType subtree_id,
    POQueue<NodeData>& query_tree,
    Ranking& ranking) {
  using wrappers::NodeDistancePair;

  log_.log("[", name(), "] Updating the ranking.\n");

  POQueue<NodeData> subtree = index_.reconstruct_subtree(subtree_id);

  const auto distance = verify(query_tree, subtree);
  log_.log(
    "[",
    name(),
    "] Edit distance for subtree id ",
    subtree_id,
    " to Q: ",
    distance,
    ".\n");

  if (!ranking.full()) {
    ranking.insert(NodeDistancePair<NodeIdType>(subtree_id, distance));

    current_round_statistics_.rankingextended.inc();
    log_.log(
      "[",
      name(),
      "] Extending the ranking with pair (subtree id ",
      subtree_id,
      ", ",
      distance,
      ") as it is not full yet.\n");
  } else {
    //if (real_lower_bound < ranking.front().get_distance()) {
      //const auto distance = verify(query_tree, subtree);
      ranking.replace_front_if_smaller(
        NodeDistancePair<NodeIdType>(subtree_id, distance));

      current_round_statistics_.rankingupdated.inc();
      log_.log(
        "[",
        name(),
        "] Modifying the ranking to contain pair (subtree id ",
        subtree_id,
        ", distance ",
        distance,
        ") as it is full already.\n");
    //}
  }

  current_round_statistics_.rankingsize.set(ranking.size());
  log_.log("[", name(), "] Updated ranking: ", ranking, ".\n");
}

template<class _Index, class _TEDAlgorithm>
inline bool AbstractFilter<_Index, _TEDAlgorithm>::already_processed(
    const NodeIdType subtree_id) const {
  log_.log(
    "[",
    name(),
    "] Checking if subtree id ",
    subtree_id,
    " was already seen in some list earlier on.\n");
  return already_processed_.find(subtree_id) != already_processed_.end();
}

template<class _Index, class _TEDAlgorithm>
inline void AbstractFilter<_Index, _TEDAlgorithm>::track_as_processed(
    const NodeIdType subtree_id) {
  already_processed_.insert(subtree_id);

  log_.log("[", name(), "] Marking subtree id ", subtree_id, " as \"seen\".\n");
}

/// private member functions

template<class _Index, class _TEDAlgorithm>
inline void AbstractFilter<_Index, _TEDAlgorithm>::extract_token_id_map(
    const TokenIdSet& token_id_set) {
  for (const TokenIdType token_id: token_id_set) {
    if (++query_token_id_map_[token_id].first == 1) {
      if (index_.contains(token_id)) {
        shared_token_ids_.push_back(token_id);
      } else {
        unshared_token_ids_.push_back(token_id);
      }
    }
  }
}

#endif // FILTER_ABSTRACT_FILTER_IMPL_H
