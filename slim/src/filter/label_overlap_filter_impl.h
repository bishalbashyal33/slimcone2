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

#ifndef FILTER_LABEL_OVERLAP_FILTER_IMPL_H
#define FILTER_LABEL_OVERLAP_FILTER_IMPL_H

template<class _Index, class _TEDAlgorithm>
LabelOverlapFilter<_Index, _TEDAlgorithm>::LabelOverlapFilter(
    LogType& log,
    _Index& index,
    _TEDAlgorithm& edit_distance)
    : AbstractFilter<_Index, _TEDAlgorithm>(
        log,
        index,
        edit_distance,
        "label overlap filter") {}

template<class _Index, class _TEDAlgorithm>
LabelOverlapFilter<_Index, _TEDAlgorithm>::~LabelOverlapFilter() {}

template<class _Index, class _TEDAlgorithm>
typename LabelOverlapFilter<_Index, _TEDAlgorithm>::Ranking LabelOverlapFilter<_Index, _TEDAlgorithm>::top_k(
    const TokenIdSet& query_token_id_set,
    POQueue<NodeData>& query_tree,
    const size_t k) {
  using wrappers::NodeDistancePair;
  using common::InAlgoTiming;

  // filter initialization
  InAlgoTiming::resetstart(interval_);
  initialize(query_token_id_set);
  InAlgoTiming::stopsetvalue(
    interval_,
    global_statistics_.timings.initialization.value);

  SubtreeLbPairs candidate_subtrees = gather_candidates();

  log_.log("[", name(), "] Initial number of candidates: ", candidate_subtrees.size(), ".\n");

  Ranking ranking{k};

  while (!candidate_subtrees.empty()) {
    const auto& p = candidate_subtrees.back();

    const NodeIdType subtree_id = p.first;
    const unsigned int lower_bound = p.second;

    log_.log(
      "[",
      name(),
      "] Considering candidate having subtree id ",
      subtree_id,
      ".\n");

    if (is_final_ranking(ranking, lower_bound)) {
      log_.log("[", name(), "] Terminating at subtree id ", subtree_id, ".\n");
      return ranking;
    }

    update_ranking(lower_bound, subtree_id, query_tree, ranking);

    candidate_subtrees.pop_back();
  }

  return ranking;
}

/// private member functions

template<class _Index, class _TEDAlgorithm>
inline typename LabelOverlapFilter<_Index, _TEDAlgorithm>::SubtreeLbPairs LabelOverlapFilter<_Index, _TEDAlgorithm>::gather_candidates()
{
  SubtreeLbPairs subtrees{};

  log_.log("[", name(), "] Gathering candidate subtrees.\n");

  for (const TokenIdType p: shared_token_ids_) {
    log_.log("[", name(), "] Considering (shared) token id: ", p, ".\n");

    for (const NodeIdType subtree_id: index_.get_list_ref(p)) {
      if (already_processed(subtree_id)) {
        continue;
      }

      const SizeType subtree_size = index_.get_size(subtree_id);

      const auto real_label_lower_bound = index_.lb_label(
        query_size_,
        query_token_id_map_,
        subtree_size,
        subtree_id);
      subtrees.emplace_back(subtree_id, real_label_lower_bound);

      global_statistics_.lower_bounds.real.record(real_label_lower_bound);
      current_round_statistics_.precandidates.inc();
      global_statistics_.subtreesizes.record(subtree_size);

      log_.log(
        "[",
        name(),
        "] Gathered subtree id ",
        subtree_id,
        " as candidate subtree (label lower bound = ",
        real_label_lower_bound,
        ").\n");

      track_as_processed(subtree_id);
    }
  }

  std::sort(
    subtrees.begin(),
    subtrees.end(),
    [this](const SubtreeLbPair& lhs, const SubtreeLbPair& rhs) -> bool {
      const NodeIdType lhs_subtree_id = lhs.first;
      const NodeIdType rhs_subtree_id = rhs.first;

      const unsigned int lhs_lower_bound = lhs.second;
      const unsigned int rhs_lower_bound = rhs.second;

      if (lhs_lower_bound == rhs_lower_bound) {
        return lhs_subtree_id > rhs_subtree_id;
      }

      // reverse order s.t. we can use pop_back
      return lhs_lower_bound > rhs_lower_bound;
    });

  log_.log("[", name(), "] Sorting candidates wrt. label lower bound.\n");

  return subtrees;
}

#endif // FILTER_LABEL_OVERLAP_FILTER_IMPL_H
