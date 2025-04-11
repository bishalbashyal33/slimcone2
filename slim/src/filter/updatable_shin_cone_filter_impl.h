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

#ifndef FILTER_UPDATABLE_SHIN_CONE_FILTER_IMPL_H
#define FILTER_UPDATABLE_SHIN_CONE_FILTER_IMPL_H

template<class _Index, class _TEDAlgorithm>
UpdatableShinConeFilter<_Index, _TEDAlgorithm>::UpdatableShinConeFilter(
    LogType& log,
    _Index& index,
    _TEDAlgorithm& edit_distance)
    : AbstractConeFilter<_Index, _TEDAlgorithm>(
        log,
        index,
        edit_distance,
        "shin cone filter") {}

template<class _Index, class _TEDAlgorithm>
UpdatableShinConeFilter<_Index, _TEDAlgorithm>::~UpdatableShinConeFilter() {}

template<class _Index, class _TEDAlgorithm>
typename UpdatableShinConeFilter<_Index, _TEDAlgorithm>::Ranking UpdatableShinConeFilter<_Index, _TEDAlgorithm>::top_k(
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

  SizeType max_subtree_size = query_size_ * (1 + 1) + k * 1;
  log_.log(
    "[",
    name(),
    "] Max. subtree size (TASM bound for unit cost model) = ",
    max_subtree_size,
    ".\n");

  // sort relevant part of inverted lists by length ascending
  InAlgoTiming::resetstart(interval_);
  std::sort(
    shared_token_ids_.begin(),
    shared_token_ids_.end(),
    [this](const TokenIdType lhs, const TokenIdType rhs) -> bool {
      //const SizeType lhs_size = index_.get_list_ref(lhs).size();
      //const SizeType rhs_size = index_.get_list_ref(rhs).size();

      const SizeType lhs_size = index_.get_list_length(lhs);
      const SizeType rhs_size = index_.get_list_length(rhs);

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

  // <proof-of-concept>
  /*
  for (auto& tokenlist: token_lists) {
    tokenlist.pop_back();
  }
  */
  // </proof-of-concept>

  // buckets for specific lower bounds, lower_bound_buckets.at(i) retrieves a
  // bucket containing element that have a lower bound of i
  std::vector<Bucket> lower_bound_buckets;

  // for each token list (consecutive integers, thus we can use a vector
  // instead of an unordered_map), we maintain a hashmap that stores
  // pairs (subtree size, sequence of subtree ids).
  // size_buckets.at(i) retrieves all size buckets whose entries originate from
  // token list i. Consequently, size_buckets.at(i).at(j) retrieves all
  // subtrees Ti for which |Ti| = j holds.
  // The first entry in a bucket represents the index at which we can stop
  // with during the bucket evaluation (to avoid multiple evaluations of the
  // same subtree since the buckets may grow in the process and we may
  // have already evaluated a suffix of the bucket).
  std::vector<std::vector<Bucket>> size_buckets;

  // information for our traversal (the next/parent ids of the list entries we
  // started with)
  // one such parent list for each inverted list (since there may be multiple
  // paths we need to traverse upwards) as well as one index per list (to
  // continue upwards traversal if necessary)
  std::vector<std::pair<typename List::const_iterator, Bucket>> parent_ids;

  // the top-k ranking
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

    const SizeType current_min_subtree_size =
      query_size_ - potential_lower_bound_;

    unsigned int list_index = 0;
    for (; list_index < potential_lower_bound_ && list_index < parent_ids.size(); ++list_index) {
      const TokenIdType token_id = shared_token_ids_.at(list_index);
      log_.log("[", name(), "] Processing list of token id ", token_id, ".\n");

      // process subtrees having a size < |Q|
      InAlgoTiming::resetstart(interval_);
      bool terminate = process_smaller_subtrees(
        list_index,
        size_buckets,
        current_min_subtree_size,
        query_tree,
        ranking);
      InAlgoTiming::stopaddvalue(
        interval_,
        global_statistics_.timings.sizebucketevaluation.value);

      if (terminate) {
        round_statistics_.set(potential_lower_bound_, current_round_statistics_);
        return ranking;
      }

      // process subtrees having a size >= |Q|
      InAlgoTiming::resetstart(interval_);
      terminate = process_larger_subtrees(
        list_index,
        parent_ids,
        query_tree,
        ranking);
      InAlgoTiming::stopaddvalue(
        interval_,
        global_statistics_.timings.parentsevaluation.value);

      if (terminate) {
        round_statistics_.set(potential_lower_bound_, current_round_statistics_);
        return ranking;
      }
    }

    InAlgoTiming::resetstart(interval_);
    const bool uninitializedparentids =
      potential_lower_bound_ >= parent_ids.size();
    const bool tokenlistpresent =
      potential_lower_bound_ < shared_token_ids_.size();

    if (uninitializedparentids && tokenlistpresent) {
      const TokenIdType token_id = shared_token_ids_.at(potential_lower_bound_);

      log_.log(
        "[",
        name(),
        "] Initializing list of token id ",
        token_id,
        " (list index ",
        potential_lower_bound_,
        ").\n");

      // process next inverted list

      // construct size bucket and give it its max number of entries
      size_buckets.emplace_back(max_subtree_size);
      log_.log(
        "[",
        name(),
        "] Constructing size cache for list of token id ",
        token_id,
        ".\n");

      // construct parents bucket
      parent_ids.emplace_back();

      log_.log(
        "[",
        name(),
        "] Constructing path cache for list of token id ",
        token_id,
        ".\n");

      // retrieve reference to inverted list to process
      const auto& next_token_list = index_.get_list_ref(token_id);

      // .first is an index representing the next
      // index to be processed in the inverted list for upwards traversals
      // since the initial traversal stops as soon as the first subtree having a
      // size >= |Q| is encountered in the respective inverted list. this entry
      // is incremented in the process
      parent_ids.back().first = next_token_list.cbegin();

      // look at each subtree in the inverted list to process and check if we
      // have to traverse it upwards

      auto partition_it = next_token_list.cbegin();
      const SizeType maxsize = query_size_;
      for (; partition_it != next_token_list.cend() && partition_it->first < maxsize; ++partition_it) {
        const auto& partition = partition_it->second;
        for (const NodeIdType subtreeid: partition) {
          NodeIdType currentsubtreeid = subtreeid;

          log_.log(
            "[",
            name(),
            "] Considering subtree id ",
            currentsubtreeid,
            " (size = ",
            index_.get_size(currentsubtreeid),
            ").\n");

          // if we encounter an already processed subtree id, we know that we
          // already processed the remainder of the path from this point on
          pathstatistics_helper_.reset();
          SizeType currentsubtreesize = 0;
          while (!already_processed(currentsubtreeid)) {
            const NodeIdType parentsubtreeid = index_.get_parent(currentsubtreeid);
            currentsubtreesize = index_.get_size(currentsubtreeid);

            // terminate if current subtree id is root or current subtree size
            // is too large (i.e., exceeds |Q|)
            if ((parentsubtreeid == 0) || (currentsubtreesize >= maxsize)) {
              log_.log(
                "[",
                name(),
                "] Stopping upwards traversal at subtree id ",
                currentsubtreeid,
                " (size = ",
                currentsubtreesize,
                ", parent subtree id ",
                parentsubtreeid,
                ").\n");

              break;
            }

            // retrieve size bucket for the size of currently considered subtree
            // NOTE: this needs to be changed to
            // size_buckets.at(potential_lower_bound_).at(currentsubtreesize) if
            // the incremental of the potential lower bound is changed, i.e., more
            // than just += 1
            Bucket& bucket = size_buckets.back().at(currentsubtreesize);

            // set end index of the bucket entries when bucket is constructed
            if (bucket.empty()) {
              log_.log(
                "[",
                name(),
                "] Initializing bucket ",
                currentsubtreesize,
                " of size cache for list of token id ",
                token_id,
                ".\n");

              bucket.push_back(1);
            }

            // two cases:
            // (1) replace existing element (at)
            // (2) insert new element at the end (emplace_back)
            // and set end index of the bucket

            const int endindex = bucket.front();
            const int bucketsize = bucket.size();

            if (endindex < bucketsize) {
              // case (1)
              bucket.at(endindex) = currentsubtreeid;
            } else {
              // case (2)
              bucket.push_back(currentsubtreeid);
            }

            ++bucket.front();

            log_.log(
              "[",
              name(),
              "] Adding subtree id ",
              currentsubtreeid,
              " to bucket ",
              currentsubtreesize,
              " of size cache for list of token id ",
              token_id,
              ".\n");

            // update duplicate set
            track_as_processed(currentsubtreeid);

            current_round_statistics_.sizecache.record(currentsubtreesize);
            pathstatistics_helper_.inc();

            log_.log(
              "[",
              name(),
              "] Traversing path upwards from subtree id ",
              currentsubtreeid,
              " to parent subtree id ",
              parentsubtreeid,
              ".\n");

            // goto parent subtree id
            currentsubtreeid = parentsubtreeid;
          }

          current_round_statistics_.paths.record(pathstatistics_helper_.value);
          log_.log(
            "[",
            name(),
            "] Adding subtree id ",
            currentsubtreeid,
            " (size = ",
            index_.get_size(currentsubtreeid),
            ") to path cache for list of token id ",
            token_id,
            ".\n");
          log_.log("[",
            name(),
            "] Path length: ",
            pathstatistics_helper_.value,
            ".\n");

          // construct parent id entry and retrieve reference to it (to traverse it
          // upwards)
          // the first subtree root on a path that has a size >= |Q| is the entry
          // point (see ConeFilter) and is stored in the corresponding parent list
          // NOTE: this needs to be changed to
          // parent_ids.at(potential_lower_bound_).second.push_back(currentsubtreeid)
          // if the incremental of the potential lower bound is changed, i.e., more
          // than just += 1
          if (currentsubtreesize == maxsize && !already_processed(currentsubtreeid)) {
      	    const unsigned int real_label_lower_bound = index_.lb_label(
              query_size_,
              query_token_id_map_,
              currentsubtreesize,
              currentsubtreeid);

            global_statistics_.lower_bounds.real.record(real_label_lower_bound);
            current_round_statistics_.precandidates.inc();
            global_statistics_.subtreesizes.record(currentsubtreesize);

            const bool terminate = evaluate_subtree(
              real_label_lower_bound,
              currentsubtreeid,
              query_tree,
              ranking);

            if (terminate) {
     	      InAlgoTiming::stopaddvalue(
                interval_,
                global_statistics_.timings.upwardtraversal.value);
              round_statistics_.set(
                potential_lower_bound_,
                current_round_statistics_);
              return ranking;
            }

            track_as_processed(currentsubtreeid);
            currentsubtreeid = index_.get_parent(currentsubtreeid);
          }

          parent_ids.back().second.push_back(currentsubtreeid);
        }
      }

      // store next index to process for current list
      parent_ids.back().first = partition_it;
    }

    InAlgoTiming::stopaddvalue(
      interval_,
      global_statistics_.timings.upwardtraversal.value);

    if (potential_lower_bound_ < size_buckets.size()) {
      list_index = potential_lower_bound_;

      // process subtrees having a size < |Q|
      InAlgoTiming::resetstart(interval_);
      terminate = process_smaller_subtrees(
        list_index,
        size_buckets,
        current_min_subtree_size,
        query_tree,
        ranking);
      InAlgoTiming::stopaddvalue(
        interval_,
        global_statistics_.timings.sizebucketevaluation.value);

      if (terminate) {
        round_statistics_.set(potential_lower_bound_, current_round_statistics_);
        return ranking;
      }

      // process subtrees having a size >= |Q|
      InAlgoTiming::resetstart(interval_);
      terminate = process_larger_subtrees(
        list_index,
        parent_ids,
        query_tree,
        ranking);
      InAlgoTiming::stopaddvalue(
        interval_,
        global_statistics_.timings.parentsevaluation.value);

      if (terminate) {
        round_statistics_.set(potential_lower_bound_, current_round_statistics_);
        return ranking;
      }
    }

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
bool UpdatableShinConeFilter<_Index, _TEDAlgorithm>::process_smaller_subtrees(
    unsigned int list_index,
    std::vector<std::vector<Bucket>>& size_buckets,
    const SizeType min_subtree_size,
    POQueue<NodeData>& query_tree,
    Ranking& ranking) {
  log_.log(
    "[",
    name(),
    "] Considering subtrees having size < |Q| = ",
    query_size_,
    ", current minimum subtree size = ",
    min_subtree_size,
    ", i.e., |Q| = ",
    query_size_,
    " - potential lower bound = ",
    potential_lower_bound_,
    ", from corresponding size cache.\n");

  // retrieve all size buckets for current list
  std::vector<Bucket>& list_size_buckets = size_buckets.at(list_index);

  // for all but the last list, we only need to evaluate the size buckets for
  // the current min. subtree size. for the last list, we need to evaluate
  // all size buckets in the range [|Q| - 1; current min. subtree size]
  SizeType subtreesize = (list_index == potential_lower_bound_ ? query_size_ - 1 : min_subtree_size);
  for (; subtreesize >= min_subtree_size; --subtreesize) {
    log_.log(
      "[",
      name(),
      "] Checking size cache for subtrees of size ",
      subtreesize,
      ".\n");

    // try to find bucket for current size
    Bucket& size_bucket = list_size_buckets.at(subtreesize);

    // only if the bucket exists, process elements
    if (!size_bucket.empty()) {
      for (typename Bucket::value_type i = size_bucket.front() - 1; i >= 1 ; --i) {
        const NodeIdType subtree_id = size_bucket.at(i);

        log_.log(
          "[",
          name(),
          "] Considering subtree id ",
          subtree_id,
          " (size = ",
          subtreesize,
          ") from size cache.\n");

        const unsigned int real_label_lower_bound = index_.lb_label(
          query_size_,
          query_token_id_map_,
          subtreesize,
          subtree_id);

        global_statistics_.lower_bounds.real.record(real_label_lower_bound);
        current_round_statistics_.precandidates.inc();
        global_statistics_.subtreesizes.record(subtreesize);

        const bool terminate = evaluate_subtree(
          real_label_lower_bound,
          subtree_id,
          query_tree,
          ranking);

        if (terminate) {
          return true;
        }
      }

      // reset end index of sub bucket
      size_bucket.at(0) = 1;
    }
  }

  return false;
}

template<class _Index, class _TEDAlgorithm>
bool UpdatableShinConeFilter<_Index, _TEDAlgorithm>::process_larger_subtrees(
    unsigned int list_index,
    std::vector<std::pair<typename List::const_iterator, Bucket>>& parent_ids,
    POQueue<NodeData>& query_tree,
    Ranking& ranking) {
  auto& parent_list = parent_ids.at(list_index);

  // for subtrees having a size >= |Q|, the max subtree size differs for
  // each list since it depends on the index of the inverted list
  const SizeType current_max_subtree_size =
    query_size_ + potential_lower_bound_ - list_index;

  log_.log(
    "[",
    name(),
    "] Considering subtrees having size < |Q| = ",
    query_size_,
    ", current maximum subtree size = ",
    current_max_subtree_size,
    ", i.e., |Q| = ",
    query_size_,
    " + potential lower bound = ",
    potential_lower_bound_,
    " - list index = ",
    list_index,
    ", from corresponding path cache.\n");

  // before we process the parent buckets, we check the list for entries that
  // we need to consider in this round. Each such list entry, we put into the
  // parent bucket in order to process it in this round
  auto& next_entry_it = parent_list.first;
  auto& parent_bucket = parent_list.second;

  const TokenIdType token_id = shared_token_ids_.at(list_index);
  const auto& list = index_.get_list_ref(token_id);

  while (next_entry_it != list.cend() && next_entry_it->first <= current_max_subtree_size) {
    for (const NodeIdType subtree_id: next_entry_it->second) {
      parent_bucket.push_back(subtree_id);
    }
    ++next_entry_it;

    /*
    const NodeIdType subtree_id = next_entry_it->second;

    if (index_.get_size(subtree_id) > current_max_subtree_size) {
      break;
    }

    parent_bucket.push_back(subtree_id);
    ++next_entry_it;
    */
  }

  // traverse parent list by reference to update parent in the process if
  // necessary
  for (NodeIdType& subtree_id: parent_bucket) {
    log_.log(
      "[",
      name(),
      "] Considering subtree id ",
      subtree_id,
      ") from path cache.\n");

    // deal with root case, i.e., there is not parent anymore
    if (subtree_id == 0) {
      log_.log("[", name(), "] Subtree id ", subtree_id, " is root node.\n");
      continue;
    }

    // deal with duplicates, i.e., with parents that are already processed
    // through another path
    if (already_processed(subtree_id)) {
      continue;
    }

    // process parent id only if it does not exceed the current maximum
    // subtree size
    const SizeType subtree_size = index_.get_size(subtree_id);

    log_.log(
      "[",
      name(),
      "] Considered subtree id ",
      subtree_id,
      " has size ",
      subtree_size,
      ".\n");

    if (subtree_size <= current_max_subtree_size) {
      // process parent id
      log_.log(
        "[",
        name(),
        "] Evaluating subtree id ",
        subtree_id,
        " (size = ",
        subtree_size,
        ").\n");

      const unsigned int real_label_lower_bound = index_.lb_label(
        query_size_,
        query_token_id_map_,
        subtree_size,
        subtree_id);

      global_statistics_.lower_bounds.real.record(real_label_lower_bound);
      current_round_statistics_.precandidates.inc();
      global_statistics_.subtreesizes.record(subtree_size);

      const bool terminate = evaluate_subtree(
        real_label_lower_bound,
        subtree_id,
        query_tree,
        ranking);

      if (terminate) {
        return true;
      }

      track_as_processed(subtree_id);

      log_.log("[",
        name(),
        "] Updating path cache entry from subtree id ",
        subtree_id,
        " to parent subtree id ",
        index_.get_parent(subtree_id),
        ".\n");

      subtree_id = index_.get_parent(subtree_id);
    }
  }

  return false;
}

#endif // FILTER_UPDATABLE_SHIN_CONE_FILTER_IMPL_H
