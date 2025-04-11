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

#ifndef FILTER_CONE_FILTER_IMPL_H
#define FILTER_CONE_FILTER_IMPL_H

template<class _Index, class _TEDAlgorithm>
ConeFilter<_Index, _TEDAlgorithm>::ConeFilter(
    LogType& log,
    _Index& index,
    _TEDAlgorithm& edit_distance)
    : AbstractConeFilter<_Index, _TEDAlgorithm>(
        log,
        index,
        edit_distance,
        "cone filter") {}

template<class _Index, class _TEDAlgorithm>
ConeFilter<_Index, _TEDAlgorithm>::~ConeFilter() {}

template<class _Index, class _TEDAlgorithm>
typename ConeFilter<_Index, _TEDAlgorithm>::Ranking ConeFilter<_Index, _TEDAlgorithm>::top_k(
    const TokenIdSet& query_token_id_set,
    POQueue<NodeData>& query_tree,
    const size_t k) {
  // type shortcuts/defs.
  using common::InAlgoTiming;

  // filter initialization
  InAlgoTiming::resetstart(interval_);
  initialize(query_token_id_set);
  InAlgoTiming::stopsetvalue(
    interval_,
    global_statistics_.timings.initialization.value);

  // sort relevant part of inverted lists by length ascending
  InAlgoTiming::resetstart(interval_);
  std::sort(
    shared_token_ids_.begin(),
    shared_token_ids_.end(),
    [this](const TokenIdType lhs, const TokenIdType rhs) -> bool {
      const SizeType lhs_size = index_.get_list_ref(lhs).size();
      const SizeType rhs_size = index_.get_list_ref(rhs).size();

      if (lhs_size == rhs_size) {
        return lhs < rhs;
      }

      return lhs_size < rhs_size;
    });
  InAlgoTiming::stopsetvalue(
    interval_,
    global_statistics_.timings.listsorting.value);

  log_.log(
    "[",
    name(),
    "] Sorting shared token ids wrt. list length ascending.\n");

  /*
  std::cerr << "sorted shared token ids: { ";
  for (const auto& t: shared_token_ids_) {
    std::cerr << t << " ";
  }
  std::cerr << "}" << std::endl;
  */

  // information for our traversal (the indexes of the inverted lists)
  std::vector<ListInfoTuple> list_infos;

  // top-k ranking
  Ranking ranking{k};

  // next potential lower bound
  previous_potential_lower_bound_ = 0;
  potential_lower_bound_ = 0;

  log_.log(
    "[",
    name(),
    "] Initial potential lower bound = ",
    potential_lower_bound_,
    ".\n");

  while (1) {
    InAlgoTiming::resetstart(interval_);
    bool terminate = round_prologue(query_tree, ranking);
    InAlgoTiming::stopaddvalue(
      interval_,
      global_statistics_.timings.roundprologue.value);

    if (terminate) {
      round_statistics_.set(potential_lower_bound_, current_round_statistics_);
      return ranking;
    }

    // expand each inverted list up to list at index/position
    // potential_lower_bound
    InAlgoTiming::resetstart(interval_);
    for (auto& list_info: list_infos) {
      bool isfinalranking{false}; // updated in nextround

      nextround(list_info, query_tree, ranking, isfinalranking);

      if (isfinalranking) {
        log_.log("[", name(), "] Terminating due to potential lower bound\n");
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

    // criteria to determine if we have to initialize the pointers for
    // the next inverted list
    InAlgoTiming::resetstart(interval_);
    const bool uninitializedlistinfos =
      potential_lower_bound_ >= list_infos.size();
    const bool tokenlistpresent =
      potential_lower_bound_ < shared_token_ids_.size();

    // check criteria
    if (uninitializedlistinfos && tokenlistpresent) {
      const TokenIdType token_id = shared_token_ids_.at(potential_lower_bound_);

      log_.log(
        "[",
        name(),
        "] Initializing list of token id ",
        token_id,
        " (list index ",
        potential_lower_bound_,
        ").\n");

      // initialize pointers for the potential_lower_bound's inverted list

      // get reference to potential_lower_bound's inverted list
      //const auto& next_list = token_lists.at(potential_lower_bound_);
      const auto& next_list = index_.get_list_ref(token_id);

      DownwardListPos down(0, next_list, potential_lower_bound_);
      UpwardListPos up(0, next_list, potential_lower_bound_);

      initialize_list_positions(down, up);

      // store compute starting indexes for potential_lower_bound's
      // inverted list
      list_infos.emplace_back(down, up);
    }
    InAlgoTiming::stopaddvalue(
      interval_,
      global_statistics_.timings.initializationpointers.value);

    // expand each inverted list up to list at index/position
    // potential_lower_bound
    InAlgoTiming::resetstart(interval_);
    auto& list_info = list_infos.back();
    bool isfinalranking{false}; // updated in nextround

    nextround(list_info, query_tree, ranking, isfinalranking);

    if (isfinalranking) {
      log_.log("[", name(), "] Terminating due to potential lower bound\n");
      InAlgoTiming::stopaddvalue(
        interval_,
        global_statistics_.timings.precandidateevaluation.value);
      round_statistics_.set(potential_lower_bound_, current_round_statistics_);
      return ranking;
    }
    InAlgoTiming::stopaddvalue(
      interval_,
      global_statistics_.timings.precandidateevaluation.value);

    InAlgoTiming::resetstart(interval_);
    previous_potential_lower_bound_ = potential_lower_bound_++;

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

template<class _Index, class _TEDAlgorithm>
template<std::size_t TupleIndex, class... T>
typename std::enable_if<TupleIndex == sizeof...(T), void>::type ConeFilter<_Index, _TEDAlgorithm>::nextround(
    std::tuple<T...>& t,
    POQueue<NodeData>& query_tree,
    Ranking& ranking,
    bool& terminate_return) {}

template<class _Index, class _TEDAlgorithm>
template<std::size_t TupleIndex, class... T>
typename std::enable_if<TupleIndex < sizeof...(T), void>::type ConeFilter<_Index, _TEDAlgorithm>::nextround(
    std::tuple<T...>& t,
    POQueue<NodeData>& query_tree,
    Ranking& ranking,
    bool& isfinalranking_return) {
  // extract tuple member
  auto& listinfo = std::get<TupleIndex>(t);

  log_.log("[", name(), "] Considering list at index ", listinfo.list_index, ".\n");

  // traverse next smaller (TupleIndex=0) / larger (TupleIndex=1) subtree sizes
  while (!isfinalranking_return && listinfo.has_next_position()) {
    const NodeIdType subtree_id = listinfo.subtree_id();
    const SizeType subtree_size = index_.get_size(subtree_id);

    log_.log(
      "[",
      name(),
      "] Considering subtree id ",
      subtree_id,
      " (size = ",
      subtree_size,
      ").\n");

    // termination criterion for subtree size
    const bool subtreesizeexceedslowerbound =
      listinfo.check_subtree_size(subtree_size, query_size_, potential_lower_bound_);
    if (subtreesizeexceedslowerbound) {
      log_.log(
        "[",
        name(),
        "] Stopping at subtree id ",
        subtree_id,
        " (size = ",
        subtree_size,
        ") since it exceeds the potential lower bound (",
        potential_lower_bound_,
        ").\n");
      break;
    }

    log_.log(
      "[",
      name(),
      "] Evaluating subtree id ",
      subtree_id,
      " (size = ",
      subtree_size,
      ").\n");

    // avoid duplicates
    if (already_processed(subtree_id)) {
      listinfo.advance();
      continue;
    }

    const unsigned int real_label_lower_bound = index_.lb_label(
      query_size_,
      query_token_id_map_,
      subtree_size,
      subtree_id);

    global_statistics_.lower_bounds.real.record(real_label_lower_bound);
    current_round_statistics_.precandidates.inc();
    global_statistics_.subtreesizes.record(subtree_size);

    const bool terminate =
      evaluate_subtree(real_label_lower_bound, subtree_id, query_tree, ranking);

    // termination criterion wrt. ranking entries;
    // NOTE: modifies isfinalranking_return
    if (terminate) {
      isfinalranking_return = true;
      return;
    }

    track_as_processed(subtree_id);

    // advance position in inverted list
    listinfo.advance();
  }

  // execute round algorithm for next tuple member
  nextround<TupleIndex + 1, T...>(
    t,
    query_tree,
    ranking,
    isfinalranking_return);
}

#endif // FILTER_CONE_FILTER_IMPL_H
