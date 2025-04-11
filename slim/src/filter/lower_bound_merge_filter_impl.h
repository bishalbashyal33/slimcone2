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

#ifndef FILTER_LOWER_BOUND_MERGE_FILTER_IMPL_H
#define FILTER_LOWER_BOUND_MERGE_FILTER_IMPL_H

template<class _Index, class _TEDAlgorithm>
LowerBoundMergeFilter<_Index, _TEDAlgorithm>::LowerBoundMergeFilter(
    LogType& log,
    _Index& index,
    _TEDAlgorithm& edit_distance)
    : AbstractConeFilter<_Index, _TEDAlgorithm>(
        log,
        index,
        edit_distance,
        "lower bound merge filter") {}

template<class _Index, class _TEDAlgorithm>
LowerBoundMergeFilter<_Index, _TEDAlgorithm>::~LowerBoundMergeFilter() {}

template<class _Index, class _TEDAlgorithm>
typename LowerBoundMergeFilter<_Index, _TEDAlgorithm>::Ranking LowerBoundMergeFilter<_Index, _TEDAlgorithm>::top_k(
    const TokenIdSet& query_token_id_set,
    POQueue<NodeData>& query_tree,
    const unsigned int k) {
  // type shortcuts/defs.
  using common::InAlgoTiming;

  // filter initialization
  InAlgoTiming::resetstart(interval_);
  initialize(query_token_id_set);
  InAlgoTiming::stopsetvalue(
    interval_,
    global_statistics_.timings.initialization.value);

  // get different sizes of subtrees from index
  InAlgoTiming::resetstart(interval_);
  std::vector<SizeType> sizes{index_.sizes().begin(), index_.sizes().end()};
  InAlgoTiming::stopsetvalue(
    interval_,
    global_statistics_.timings.sizesretrieval.value);

  log_.log("[", name(), "] Retrieving distinct subtree sizes from index.\n");

  // sort sizes wrt. abs. distance to query size
  InAlgoTiming::resetstart(interval_);
  std::sort(
    sizes.begin(),
    sizes.end(),
    [this](const SizeType lhs, const SizeType rhs) -> bool {
      const auto lhs_abs = std::max(lhs, query_size_) - std::min(lhs, query_size_);
      const auto rhs_abs = std::max(rhs, query_size_) - std::min(rhs, query_size_);

      if (lhs_abs == rhs_abs) {
        // if absolute size difference is equal, evaluate smaller subtrees first
        return lhs < rhs;
      }

      return lhs_abs < rhs_abs;
    });
  InAlgoTiming::stopsetvalue(
    interval_,
    global_statistics_.timings.sizessorting.value);

  log_.log(
    "[",
    name(),
    "] Sorting distinct subtree sizes wrt. absolute size difference to Q",
    " ascending, i.e., ||Q| - |Ti||.\n");

  /*
  std::cerr << "sorted sizes: { ";
  for (const auto& size: sizes) { std::cerr << size << " "; }
  std::cerr << "}" << std::endl;
  */

  // initialize ranking
  Ranking ranking{k};

  // initialize merge positions

  // element index represents the index of the inverted list
  // element value (pair) represents the indexes in the list to start/continue
  // merging
  // first ... list index that moves towards smaller subtree sizes
  // second ... list index that moves towards larger subtree sizes
  InAlgoTiming::resetstart(interval_);
  std::vector<ListInfoPair> mergepositions;

  unsigned int listindex{0};
  for (const auto token_id: query_token_id_set) {
    // avoid out_of_range exception for token ids that are not in the document
    if (!index_.contains(token_id)) {
      continue;
    }

    log_.log(
      "[",
      name(),
      "] Initializing list of token id ",
      token_id,
      " (list index ",
      listindex,
      ").\n");

    const auto& list = index_.get_list_ref(token_id);

    DownwardListPos down(0, list, listindex);
    UpwardListPos up(0, list, listindex);

    initialize_list_positions(down, up);

    // .first moves towards subtrees having |Ti| < |Q| and starts at first
    // subtree having |Ti| < |Q|. .second moves towards subtrees having
    // |Ti| >= |Q|, i.e., .second traverses subtrees having |Ti| = |Q|
    mergepositions.emplace_back(down, up);

    ++listindex;
  }
  InAlgoTiming::stopsetvalue(
    interval_,
    global_statistics_.timings.inititalizationmergepositions.value);

  if (!sizes.empty()) {
    potential_lower_bound_ =
      std::max(query_size_, sizes.front()) - std::min(query_size_, sizes.front());

    log_.log(
      "[",
      name(),
      "] Initial potential lower bound = ",
      potential_lower_bound_,
      ".\n");
  }

  // pair of iterators, first pointing to current and second pointing to next
  // size entry
  auto nextsizeit = std::next(sizes.cbegin());
  for (; nextsizeit != sizes.cend(); ++nextsizeit) {
    InAlgoTiming::resetstart(interval_);
    bool terminate = round_prologue(query_tree, ranking);
    InAlgoTiming::stopaddvalue(
      interval_,
      global_statistics_.timings.roundprologue.value);

    if (terminate) {
      round_statistics_.set(potential_lower_bound_, current_round_statistics_);
      return ranking;
    }

    const SizeType size = *std::prev(nextsizeit);

    log_.log("[", name(), "] Evaluating stripe for subtree size ", size, ".\n");

    InAlgoTiming::resetstart(interval_);
    std::unordered_map<NodeIdType, unsigned int> subtreeoverlaps;
    for (ListInfoPair& mergeindexes: mergepositions) {
      DownwardListPos& down = mergeindexes.first;
      UpwardListPos& up = mergeindexes.second;

      while (index_.get_size(down.subtree_id()) == size) {
	      ++subtreeoverlaps[down.subtree_id()];

        if (!down.has_next_position()) {
          break;
        }

        down.advance();
      }

      while (index_.get_size(up.subtree_id()) == size) {
	      ++subtreeoverlaps[up.subtree_id()];

        if (!up.has_next_position()) {
          break;
        }

        up.advance();
      }
    }
    InAlgoTiming::stopaddvalue(
      interval_,
      global_statistics_.timings.overlapcomputation.value);

    InAlgoTiming::resetstart(interval_);
    for (const auto& p: subtreeoverlaps) {
      const NodeIdType subtreeid = p.first;
      unsigned int overlap = p.second;

      const SizeType subtreesize = index_.get_size(subtreeid);

      unsigned int larger = std::max(subtreesize, query_size_);
      unsigned int smaller = std::min(subtreesize, query_size_);

      // compute label lower bound
      auto lowerbound = std::max(larger - overlap, larger - smaller);

      global_statistics_.lower_bounds.real.record(lowerbound);
      current_round_statistics_.precandidates.inc();
      global_statistics_.subtreesizes.record(subtreesize);

      log_.log(
        "[",
        name(),
        "] Evaluating subtree id ",
        subtreeid,
        " (size = ",
        size, "), max. possible overlap = ",
        overlap,
        ", min. possible lower bound = ",
        lowerbound,
        ".\n");

      const bool terminate =
        evaluate_subtree(lowerbound, subtreeid, query_tree, ranking);

      if (terminate) {
        InAlgoTiming::stopaddvalue(
          interval_,
          global_statistics_.timings.precandidateevaluation.value);
        round_statistics_.set(potential_lower_bound_, current_round_statistics_);
        return ranking;
      }
    }
    InAlgoTiming::stopaddvalue(
      interval_,
      global_statistics_.timings.precandidateevaluation.value);

    // choose next subtree size

    InAlgoTiming::resetstart(interval_);
    // since stripes are symmetric for +/- (except for subtrees of size |Q|)
    previous_potential_lower_bound_ = potential_lower_bound_;
    potential_lower_bound_ =
      std::max(query_size_, *nextsizeit) - std::min(query_size_, *nextsizeit);

    log_.log(
      "[",
      name(),
      "] Potential lower bound for next round: ",
      potential_lower_bound_,
      " (previously ",
      previous_potential_lower_bound_,
      ").\n");

    terminate = round_epilogue(ranking);
    InAlgoTiming::stopaddvalue(
      interval_,
      global_statistics_.timings.roundepilogue.value);

    if (terminate) {
      round_statistics_.set(potential_lower_bound_, current_round_statistics_);
      return ranking;
    }
  }

  round_statistics_.set(potential_lower_bound_, current_round_statistics_);
  return ranking;
}

#endif // FILTER_LOWER_BOUND_MERGE_FILTER_IMPL_H
