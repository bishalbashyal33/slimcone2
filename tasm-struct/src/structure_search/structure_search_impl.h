#ifndef STRUCTURE_SEARCH_STRUCTURE_SEARCH_IMPL_H
#define STRUCTURE_SEARCH_STRUCTURE_SEARCH_IMPL_H

namespace structure_search {

template<class _NodeData, class _Costs>
StructureSearch<_NodeData, _Costs>::StructureSearch(
    const UncommonLabelIndex& uncommon_label_index,
    const CommonLabelIndex& common_label_index,
    const NodeIndex& node_index,
    const StructureIndex& structure_index,
    const PureStructureIndex& pure_structure_index)
    : uncommon_label_index_(uncommon_label_index),
      common_label_index_(common_label_index),
      node_index_(node_index),
      structure_index_(structure_index),
      pure_structure_index_(pure_structure_index),
      verified_candidates_(0)
{ }

template<class _NodeData, class _Costs>
StructureSearch<_NodeData, _Costs>::~StructureSearch()
{ }

template<class _NodeData, class _Costs>
typename StructureSearch<_NodeData, _Costs>::Ranking
StructureSearch<_NodeData, _Costs>::compute_topk(
    Query& query,
    int m,
    const int k) {

  // clear query key roots before computation is started if old key roots are
  // present
  if (!query_kr_.empty()) {
    query_kr_.clear();
  }

  if (!reconstructed_.empty()) {
    reconstructed_.clear();
  }

  auto query_data_ids = std::move(categorize_data_ids(query));

  auto h = std::move(gather_information(query_data_ids.common));

  /*std::cout << "H: {\n";
  for (const auto& p: h) {
    std::cout << "\t" << p.first << " -> " << p.second << std::endl;
  }
  std::cout << "}" << std::endl;*/

  Ranking ranking(k);

  const int uncommon_size_query = query_data_ids.uncommon.size();

  m = find_uncommon_data_answers(query, m, k, query_data_ids.uncommon, h, ranking);

  //std::cout << "Top-k after find_uncommon_data_answers: " << ranking << std::endl;
  //std::cout << "m after find_uncommon_data_answers: " << m << std::endl;

  const int max_distance = ranking.front().get_distance();

  // in these two cases, we have to find answers among the common data/labels
  if ((m >= uncommon_size_query) || (uncommon_size_query < max_distance)) {
    m = find_common_data_answers(query, m, k, query_data_ids.uncommon, h, ranking);
  }

  //std::cout << "Top-k after find_common_data_answers: " << ranking << std::endl;

  if (m >= pure_structure_index_.begin()->first) {
    find_unknown_data_answers(query, m, k, ranking);
  }

  return ranking;
}

template<class _NodeData, class _Costs>
typename StructureSearch<_NodeData, _Costs>::CategorizedDataIds
StructureSearch<_NodeData, _Costs>::categorize_data_ids(
    Query& query) const {
  CategorizedDataIds categorized;

  for (const int data_id: query.data_ids_) {
    if (common_label_index_.search(data_id) != nullptr) {
      categorized.common.insert(data_id);
    } else if (uncommon_label_index_.search(data_id) != nullptr) {
      categorized.uncommon.insert(data_id);
    }
  }

  return std::move(categorized);
}

template<class _NodeData, class _Costs>
typename StructureSearch<_NodeData, _Costs>::IntHashTable
StructureSearch<_NodeData, _Costs>::gather_information(
    const IntMultiSet& common_data_ids_query) const {
  using PostingListsEntry = PostingLists<IntOrderedMap>::EntryType;

  PostingLists<IntOrderedMap> pl;

  // initialize PL
  auto data_id_it = common_data_ids_query.cbegin();
  for ( ; data_id_it != common_data_ids_query.cend(); ) {
    // insert into posting list; key data_id has to exist -> no check necessary
    //std::cout << data_id << std::endl;
    pl.push_back(*data_id_it, *common_label_index_.search(*data_id_it));

    // move it to next data_id (skip duplicates); O(log(n))
    data_id_it = common_data_ids_query.upper_bound(*data_id_it);
  }

  IntHashTable h; // H
  int previous_structure_id = -1; // s_prev

  // L, labels (if a label occurs n times, it's inserted n times here)
  IntMultiSet data_ids_subtree{};

  while (!pl.empty()) {
    // get next entry in list of posting lists (wrt. first list element)
    const PostingListsEntry current = std::move(pl.next());

    // extract fields
    const int data_id = current.first;
    const int structure_id = current.second.first;
    const int occurrence_count = current.second.second;

    if ((structure_id != previous_structure_id) && (previous_structure_id != -1))
    {
      intersect_and_insert(
        common_data_ids_query,
        data_ids_subtree,
        previous_structure_id,
        h
      );

      data_ids_subtree.clear();
    }

    std::fill_n(
      std::inserter(data_ids_subtree, data_ids_subtree.end()),
      occurrence_count,
      data_id
    );

    previous_structure_id = structure_id;
  }

  if (previous_structure_id != -1) {
    intersect_and_insert(
      common_data_ids_query,
      data_ids_subtree,
      previous_structure_id,
      h
    );
  }

  return std::move(h);
}

template<class _NodeData, class _Costs>
void StructureSearch<_NodeData, _Costs>::intersect_and_insert(
    const IntMultiSet& ms_query,
    const IntMultiSet& ms_subtree,
    const int structure_id,
    IntHashTable& h) const {
  IntMultiSet intersection{};

  std::set_intersection(
    ms_query.begin(),
    ms_query.end(),
    ms_subtree.begin(),
    ms_subtree.end(),
    std::inserter(intersection, intersection.begin())
  );

  h[structure_id] = intersection.size();
}

template<class _NodeData, class _Costs>
const int StructureSearch<_NodeData, _Costs>::find_uncommon_data_answers(
    Query& query,
    int m,
    const int k,
    const IntMultiSet& uncommon_data_ids_query,
    const IntHashTable& h,
    Ranking& ranking) {
  using std::move;
  using std::make_pair;

  using PostingListsEntry = PostingLists<IntVector>::EntryType;

  //std::cout << "Lu-Find-Answers:\n";

  PostingLists<IntVector> pl;

  // initialize pl
  auto data_id_it = uncommon_data_ids_query.cbegin();
  for (; data_id_it != uncommon_data_ids_query.cend(); ) {
    pl.push_back(*data_id_it, *uncommon_label_index_.search(*data_id_it));

    // move it to next data_id (skip duplicates); O(log(n))
    data_id_it = uncommon_data_ids_query.upper_bound(*data_id_it);
  }

  Stack s{};

  // ranking given as parameter

  IntMultiSet data{};

  PostingListsEntry current_entry;
  int current_preorder_id;
  int current_data_id;

  StackEntry previous;

  while (!pl.empty()) {
    current_entry = move(pl.next());

    // no move since only int
    current_preorder_id = current_entry.second;
    current_data_id = current_entry.first;

    data.clear();

    while (!s.empty() && !is_prefix(current_preorder_id, s.back().first)) {
      // get top element of stack (and pop it)
      previous = move(s.back());
      s.pop_back();

      // (multi)set union of multiset of current data (labels) with set of
      // previous data (labels), denoted L u L_prev in the original paper
      // (multi)set union is stored in multiset of current data
      data.insert(previous.second.begin(), previous.second.end());

      m = filter_and_add(query, data, previous.first, m, h, ranking);
    }

    int top_preorder_id{};

    if (!s.empty()) {
      // pop top and push again with modified L, i.e. directly modify top element
      // of the stack (since it possible)
      s.back().second.insert(data.begin(), data.end());
      top_preorder_id = s.back().first;
    }

    push_intermediate_nodes(top_preorder_id, current_preorder_id, s);

    s.push_back(move(make_pair(
      current_preorder_id,
      move(IntMultiSet{current_data_id})
    )));
  }

  data.clear();

  while (!s.empty()) {
    previous = move(s.back());
    s.pop_back();

    data.insert(previous.second.begin(), previous.second.end()); // union

    m = filter_and_add(query, data, previous.first, m, h, ranking);
  }

  return m;
}

template<class _NodeData, class _Costs>
const int StructureSearch<_NodeData, _Costs>::filter_and_add(
    Query& query,
    const IntMultiSet& data,
    const int preorder_id,
    const int m,
    const IntHashTable& h,
    Ranking& ranking) {

  //std::cout << "\tConsidering preorder_id " << preorder_id << "\n";

  // lookup preorder_id in node_index; has to exist
  const NodeIndexConstIter node_info_cit = node_index_.cbegin() + preorder_id;

  // get value (mapped to preorder_id)
  const wrappers::NodeIndexValue& node_info = *node_info_cit;

  // store some information in variables to use them later on
  const int structure_id = node_info.structure_id();
  const int subtree_depth = node_info.depth();
  const int postorder_id = node_info.postorder_id();

  // lookup structure-id in hashtable (computed in gather_information)
  // if there's no entry in h, it means that the intersection/overlap size is 0
  // otherwise, store the value read from h
  int overlap_size_common = 0;

  auto h_it = h.find(structure_id);
  if (h_it != h.cend()) {
    overlap_size_common = h_it->second;
  }

  reconstructed_.insert(preorder_id);

  //std::cout << "\tNot yet reconstructed\n";

  // lookup structure-id in structure index
  const auto& structure_info = structure_index_.at(structure_id);

  // L_c(T_d)
  const int subtree_common = structure_info.labeled_nodes();

  // L_u(T_d)
  const int subtree_uncommon = structure_info.other_nodes();

  // size of T_d
  const int subtree_size = subtree_common + subtree_uncommon;

  // compute max size of both trees (T_d and query)
  const int max_tree_size = std::max(query.node_count(), subtree_size);

  // compute overlap/intersection size of wrt. uncommon labels
  const int overlap_size_uncommon = static_cast<int>(data.size());

  // sum up the overlaps of common and uncommon labels
  const int overlap_size_total = overlap_size_common + overlap_size_uncommon;

  // combined diff or distance (wrt. to both, common and uncommon)
  const int data_diff = max_tree_size - overlap_size_total;

  //std::cout << "\tdepth(Q) = " << query.depth() << ", depth(T_s) = " << subtree_depth << "\n";

  const int depth_diff = std::abs(query.depth() - subtree_depth);

  //std::cout << "\tdata_diff = " << data_diff << ", depth_diff = " << depth_diff << ", m = " << m << "\n";

  ++potential_candidates_;
  candidates_filtered_by_label_bound_ += (data_diff > m);
  candidates_filtered_by_depth_bound_ += (depth_diff > m);

  if ((data_diff <= m) && (depth_diff <= m)) {
    //std::cout << "\t\tSatisfies bounds\n";

    Document subtree =
      std::move(reconstruct_subtree(node_info_cit, query.label_map()));

    const int distance = ted(query, subtree, ranking, m);

    if ((distance <= m)) {
      //std::cout << "\t\tDistance short enough (" << postorder_id << ", " << distance << ")\n";

      wrappers::NodeDistancePair<int> candidate(postorder_id, distance);

      if (!ranking.full()) {
        ranking.insert(candidate);
      } else {
        if ((candidate < ranking.front()) && !ranking.insert(candidate)) {
          ranking.replace_front(candidate);
        }
      }
    }
  }

  if (ranking.full()) {
    //std::cout << "\tAdapting m to " << ranking.front().get_distance() << "\n";
    return ranking.front().get_distance();
  }

  return m;
}

template<class _NodeData, class _Costs>
void StructureSearch<_NodeData, _Costs>::push_intermediate_nodes(
    const int begin,
    const int end,
    Stack& s) const {

  using std::move;
  using std::make_pair;

  int parent_id = node_index_.at(end).parent_id();

  if ((!is_prefix(end, begin) && (begin != 0)) || (begin == parent_id)) {
    // in one of these cases, there's no need to push anything, hence return
    // cases: (1) begin is not a prefix of end and begin is not the 0 (i.e., an
    // existing node), (2) begin is the parent of end
    return;
  }

  Stack intermediate{};

  while (parent_id != begin) {
    intermediate.push_back(move(make_pair(parent_id, move(IntMultiSet{}))));
    parent_id = node_index_.at(parent_id).parent_id();
  }

  s.insert(s.end(), intermediate.rbegin(), intermediate.rend());
}

template<class _NodeData, class _Costs>
typename StructureSearch<_NodeData, _Costs>::Document
StructureSearch<_NodeData, _Costs>::reconstruct_subtree(
    const NodeIndexConstIter node_info_root_cit,
    DataHashMap& data_hash_map) {

  using std::move;

  // NOTE: possible optimization: reuse already reconstructed subtrees

  // postorder queue; insert in postorder required
  Document subtree(data_hash_map);

  // get size of subtree (i.e., the number of nodes to iterate over)
  const int size = node_info_root_cit->size() - 1;

  // beginning of the iteration
  NodeIndexConstIter it = std::next(node_info_root_cit);

  // used as stack in order to append nodes in postorder to 'subtree'
  std::vector<NodeIndexConstIter> s{node_info_root_cit};

  for (int i = 0; it != node_index_.cend() && i < size; ++i) {
    while ((s.back() - node_index_.cbegin()) != it->parent_id()) {
      subtree.append(s.back()->data_id(), s.back()->size());
      s.pop_back();
    }

    s.push_back(it);
    ++it;
  }

  // process remaining items of the stack
  while (!s.empty()) {
    subtree.append(s.back()->data_id(), s.back()->size());
    s.pop_back();
  }

  return move(subtree);
}

template<class _NodeData, class _Costs>
const int StructureSearch<_NodeData, _Costs>::ted(
    Query& query,
    Document& subtree,
    Ranking& ranking,
    const int m) {
  auto it = already_verified_.find(subtree);
  if (it != already_verified_.end()) {
    ++duplicate_verifications_;
    return it->second;
  }

  DoubleArray2D pd(query.node_count() + 1, subtree.node_count() + 1);
  DoubleArray2D td(query.node_count() + 1, subtree.node_count() + 1);

  unsigned long worst_distance = ranking.front().get_distance();

  // subtree key roots, have to be computed on every call
  IntVector subtree_kr = std::move(subtree.kr());

  // query key roots, since the query does not change, we only compute these once
  if (query_kr_.empty()) {
    query_kr_ = std::move(query.kr());
  }

  for (size_t d = 1; d < subtree_kr.size(); ++d) {
    for (size_t q = 1; q < query_kr_.size(); ++q) {
      ed_.prefix_distance(
        query_kr_.at(q),
        subtree_kr.at(d),
        query,
        subtree,
        pd,
        td,
        ranking,
        m
      );
    }
  }

  ++verified_candidates_;
  if (ranking.full()) {
    //std::cerr << "\t- " << ranking.size() << ", " << worst_distance << std::endl;
    increment_verifications(worst_distance);
  }

  const int distance = static_cast<int>(td[query.root()][subtree.root()]);

  already_verified_.emplace(subtree, distance);

  return distance;
}

template<class _NodeData, class _Costs>
const int StructureSearch<_NodeData, _Costs>::find_common_data_answers(
    Query& query,
    int m,
    const int k,
    const IntMultiSet& uncommon_data_ids_query,
    const IntHashTable& h,
    Ranking& ranking) {

  auto pair_it = h.begin();
  for (; pair_it != h.end(); ++pair_it) {
    // get fields
    const int structure_id = pair_it->first;
    const int common_overlap_size = pair_it->second;

    // look up structure id in structure index
    auto structure_info = structure_index_.at(structure_id);

    const IntVector& preorder_ids = structure_info.preorder_ids();

    if (preorder_ids.empty()) {
      continue; // continue evaluation, i.e., with next entry in H
    }

    // preorder id of the root of the representative subtree of this structure
    IntVectorConstIter preorder_id_it = preorder_ids.cbegin();
    for (; preorder_id_it != preorder_ids.cend(); ++preorder_id_it) {
      if (!is_reconstructed(*preorder_id_it)) {
        break;
      }
    }

    if (preorder_id_it == preorder_ids.cend()) {
      continue;
    }

    // number of uncommon data (labels) in the current subtree has to be 0 by
    // definition, since the respective subtree does not contain any data (labels)
    // from uncommon_data set
    const int subtree_uncommon = structure_info.other_nodes();

    // in this case equivalent to the node count of this structure's subtree
    // since none of the labels is in the uncommon_data set AND every node of
    // the document is either in the uncommon_data or the common_data set
    const int subtree_common = structure_info.labeled_nodes();

    const int subtree_size = subtree_common + subtree_uncommon;

    const int max_subtree_size = std::max(query.node_count(), subtree_size);

    const int min_diff_uncommon_data =
      std::min(uncommon_data_ids_query.size(), uncommon_label_index_.size());

    const int data_diff = max_subtree_size - (common_overlap_size);// + min_diff_uncommon_data);

    ++potential_candidates_;
    candidates_filtered_by_label_bound_ += (data_diff > m);

    if (data_diff <= m) {
      NodeIndexConstIter node_info_cit = node_index_.cbegin() + *preorder_id_it;

      Document representative_subtree =
        std::move(reconstruct_subtree(node_info_cit, query.label_map()));

      // the edit distance between query and the representative subtree is
      // computed once. afterwards, we know the distance for all subtrees which
      // have this structure id
      const int distance = ted(query, representative_subtree, ranking, m);

      if ((distance <= m)) {
        // we add trees of the current structure id
        for (; preorder_id_it != preorder_ids.cend(); ++preorder_id_it) {
          if (is_reconstructed(*preorder_id_it)) {
            continue;
          }

          reconstructed_.insert(*preorder_id_it);

          // to return the postorder id, we have to look the info up in the node
          // index (could be omitted if we add (dewey id, distance)-pairs)
          // could be omitted for index 0 anyways
          node_info_cit = node_index_.cbegin() + *preorder_id_it;

          const int postorder_id = node_info_cit->postorder_id();

          wrappers::NodeDistancePair<int> candidate(postorder_id, distance);

          if (!ranking.full()) {
            ranking.insert(candidate);
          } else {
            ranking.replace_front_if_smaller(candidate);
          }

          if (ranking.full()) {
            m = ranking.front().get_distance();
          }
        }
      }
    }
  }

  return m;
}

template<class _NodeData, class _Costs>
void StructureSearch<_NodeData, _Costs>::find_unknown_data_answers(
    Query& query,
    int m,
    const int k,
    Ranking& ranking) {

  // iterate pure structure index until pure structures of size > m
  auto it = pure_structure_index_.cbegin();
  for (; ((it != pure_structure_index_.cend()) && (it->first <= m)); ++it) {
    NodeIndexConstIter node_info_cit;
    int postorder_id = 0;

    // find an (unevaluated) representative of the current pure structure id
    auto structure_id_it = it->second.cbegin();
    for (; structure_id_it != it->second.cend(); ++structure_id_it) {
      // get info to currently evaluated structure
      const auto& structure_info = structure_index_.at(*structure_id_it);

      // get all preorder ids associated with the currently evaluate structure
      const auto& candidate_preorder_ids = structure_info.preorder_ids();

      // we have no candidates
      if (candidate_preorder_ids.empty()) {
        continue;
      }

      // get representative, if exists
      IntVectorConstIter candidate_it = candidate_preorder_ids.cbegin();
      for (; candidate_it != candidate_preorder_ids.cend(); ++candidate_it) {
        if (!is_reconstructed(*candidate_it)) {
          break;
        }
      }

      if (candidate_it == candidate_preorder_ids.cend()) {
        continue;
      }

      ++potential_candidates_;



      // get node info of representative
      node_info_cit = node_index_.cbegin() + *candidate_it;

      // reconstruct corresponding subtree
      Document representative_subtree =
        std::move(reconstruct_subtree(node_info_cit, query.label_map()));

      // the edit distance between query and the representative subtree is
      // computed once. afterwards, we know the distance for all subtrees which
      // have this structure id
      const int distance = ted(query, representative_subtree, ranking, m);

      if (distance <= m) {
        for (const int preorder_id: candidate_preorder_ids) {
          if (is_reconstructed(preorder_id)) {
            continue;
          }

          // get postorder id to dewey identifier (has to exist)
          node_info_cit = node_index_.cbegin() + preorder_id;

          postorder_id = node_info_cit->postorder_id();

          wrappers::NodeDistancePair<int> candidate(
            postorder_id,
            distance
          );

          if (!ranking.full()) {
            ranking.insert(candidate);
          } else {
            ranking.replace_front_if_smaller(candidate);
          }

          if (ranking.full()) {
            m = ranking.front().get_distance();
          }
        }
      }
    }
  }
}

template<class _NodeData, class _Costs>
inline const bool StructureSearch<_NodeData, _Costs>::is_prefix(
    const int reference,
    const int candidate) const {

  const int reference_postorder_id = node_index_.at(reference).postorder_id();
  const int candidate_postorder_id = node_index_.at(candidate).postorder_id();

  return ((reference > candidate) &&
          (reference_postorder_id < candidate_postorder_id));
}

template<class _NodeData, class _Costs>
inline const bool StructureSearch<_NodeData, _Costs>::is_reconstructed(
    const int preorder_id) {
  return (reconstructed_.find(preorder_id) != reconstructed_.cend());
}

template<class _NodeData, class _Costs>
inline unsigned int StructureSearch<_NodeData, _Costs>::verified_candidates() const {
  return verified_candidates_;
}

template<class _NodeData, class _Costs>
inline unsigned int StructureSearch<_NodeData, _Costs>::duplicate_verifications() const {
  return duplicate_verifications_;
}

template<class _NodeData, class _Costs>
inline unsigned int StructureSearch<_NodeData, _Costs>::potential_candidates() const {
  return potential_candidates_;
}

template<class _NodeData, class _Costs>
inline unsigned int StructureSearch<_NodeData, _Costs>::candidates_filtered_by_depth_bound() const {
  return candidates_filtered_by_depth_bound_;
}

template<class _NodeData, class _Costs>
inline unsigned int StructureSearch<_NodeData, _Costs>::candidates_filtered_by_label_bound() const {
  return candidates_filtered_by_label_bound_;
}

template<class _NodeData, class _Costs>
inline const typename StructureSearch<_NodeData, _Costs>::FrequencyMap& StructureSearch<_NodeData, _Costs>::verifications_per_distance_lower_bound() const {
  return verifications_per_distance_lower_bound_;
}

template<class _NodeData, class _Costs>
inline void StructureSearch<_NodeData, _Costs>::increment_verifications(
    unsigned long lower_bound) {
  ++verifications_per_distance_lower_bound_[lower_bound];
}

} // namespace structure_search

#endif // STRUCTURE_SEARCH_STRUCTURE_SEARCH_IMPL_H
