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

#ifndef INDEX_SLIM_INVERTED_MAP_INDEX_IMPL_H
#define INDEX_SLIM_INVERTED_MAP_INDEX_IMPL_H

/// public constructors/destructors

template<class _NodeData, class _TEDAlgorithm>
SlimInvertedMapIndex<_NodeData, _TEDAlgorithm>::SlimInvertedMapIndex(LogType& log)
    : AbstractInvertedMapIndex<_NodeData, typename AbstractIndex<_NodeData>::template SymmetricQuintuple<unsigned int>>(log, "slim inverted map index (updatable)") {
  // 1-based indexes:
  node_index_.resize(1);
  inverted_maps_.resize(1);
}

template<class _NodeData, class _TEDAlgorithm>
SlimInvertedMapIndex<_NodeData, _TEDAlgorithm>::~SlimInvertedMapIndex() {}

/// public member functions

template<class _NodeData, class _TEDAlgorithm>
inline void SlimInvertedMapIndex<_NodeData, _TEDAlgorithm>::prepare() {
  // type shortcuts/defs.
  using common::InAlgoTiming;

  if (ready()) {
    return;
  }

  // O(n)
  InAlgoTiming::resetstart(interval_);
  inverted_maps_.resize(token_map().size() + 1);
  InAlgoTiming::stopsetvalue(
    interval_,
    statistics_.timingbuilddetails.prepareallocatelists.value);

  // <proof-of-concept>
  InAlgoTiming::resetstart(interval_);
  inverted_list_lengths_.resize(token_map().size() + 1, 0);

  unsigned int rootid{0};
  unsigned int end{static_cast<unsigned int>(node_index_.size())};
  for (rootid = 1; rootid < end; ++rootid) {
    const auto& subtreesize = get_size(node_index_.at(rootid));
    std::unordered_set<unsigned int> seentokenids(subtreesize);

    unsigned int currentid = rootid;
    for (; currentid >= rootid - subtreesize + 1; --currentid) {
      const auto& tokenid = get_token_id(node_index_.at(currentid));

      // skip duplicate token id in current subtree
      if (seentokenids.find(tokenid) != seentokenids.end()) {
        continue;
      }

      seentokenids.insert(tokenid);
      ++inverted_list_lengths_.at(tokenid);
    }
  }
  InAlgoTiming::stopsetvalue(
    interval_,
    statistics_.timingbuilddetails.preparecomputelistslengths.value);
  // </proof-of-concept>

  // O(n)
  // set parent ids in node index and update inverted lists
  // NOTE: by no means optimal but works for now
  // std::vector<std::vector<std::pair<SizeType, NodeIdType>>> lists(token_map().size() + 1);
  unsigned int subtree_id{1};
  // first entry in node index is dummy entry, thus skip it
  typename decltype(node_index_)::iterator nit = std::next(node_index_.begin());
  typename decltype(node_index_)::iterator nitend = node_index_.end();
  for (; nit != nitend; ++nit) {
    const auto& n = *nit;
    const unsigned int token_id = get_token_id(n);
    const unsigned int subtree_size = get_size(n);

    InAlgoTiming::resetstart(interval_);
    inverted_maps_.at(token_id).emplace(subtree_size, subtree_id);
    // lists.at(token_id).emplace_back(subtree_size, subtree_id);
    InAlgoTiming::stopaddvalue(
      interval_,
      statistics_.timingbuilddetails.preparelistappend.value);

    // loop is only executed if subtree_size > 1
    InAlgoTiming::resetstart(interval_);
    unsigned int first_child_id = 0;
    unsigned int previous_sibling_id = 0;
    for (unsigned int i = 0; i < subtree_size - 1; ++i) {
      unsigned int descendant_id = subtree_id - i - 1;

      // do NOT use get_parent here since we need a reference
      unsigned int& parent_id_ref =
        std::get<2>(node_index_.at(descendant_id));

      if (parent_id_ref == 0) {
	      // if no parent id is set yet, set it
        parent_id_ref = subtree_id;

        // set next sibling id for all but the last child
        if (previous_sibling_id != 0) {
          set_next_sibling(node_index_.at(descendant_id), previous_sibling_id);
        }
        previous_sibling_id = descendant_id;

        // track child in reverse order => last is first child
        first_child_id = descendant_id;
      }
    }

    // set first child id (last child_id in reverse order
    set_first_child(node_index_.at(subtree_id), first_child_id);
    InAlgoTiming::stopaddvalue(
      interval_,
      statistics_.timingbuilddetails.preparenodeindexupdate.value);

    ++subtree_id;
  }

  // O(k * n * log(n)) for k lists, each of length n
  // for (TokenIdType token_id = 0; token_id < lists.size(); ++token_id) {
  //   auto& list = lists.at(token_id);
  //   inverted_maps_.at(token_id) =
  //     std::multimap<SizeType, NodeIdType>(list.begin(), list.end());
  // }

  size_ = inverted_maps_.size() - 1;

  for (const auto& invertedlist: inverted_maps_) {
    statistics_.invertedlists.record(invertedlist.size());
  }

  AbstractInvertedMapIndex<_NodeData, typename AbstractIndex<_NodeData>::template SymmetricQuintuple<unsigned int>>::prepare();
}

template<class _NodeData, class _TEDAlgorithm>
inline typename SlimInvertedMapIndex<_NodeData, _TEDAlgorithm>::TokenIdType SlimInvertedMapIndex<_NodeData, _TEDAlgorithm>::put(
    const _NodeData& token,
    const SizeType size) {
  // type shortcuts/defs.
  using common::InAlgoTiming;

  InAlgoTiming::resetstart(interval_);
  const TokenIdType token_id =
    AbstractInvertedMapIndex<_NodeData, typename AbstractIndex<_NodeData>::template SymmetricQuintuple<unsigned int>>::put(token, size);

  // O(1) + O(time for allocation)
  node_index_.emplace_back(token_id, size, 0, 0, 0);
  InAlgoTiming::stopaddvalue(
    interval_,
    statistics_.timingbuilddetails.put.value);

  return token_id;
}

template<class _NodeData, class _TEDAlgorithm>
void SlimInvertedMapIndex<_NodeData, _TEDAlgorithm>::rename(
    const NodeIdType node_id,
    const _NodeData& new_token) {
  using common::InAlgoTiming;

  // invalid node id
  if (node_id > node_index_.size()) {
    return;
  }

  // get token id for new token
  const TokenIdType new_token_id = convert_token(new_token);

  // get entry from node index
  auto& entry = get_entry(node_id);
  const TokenIdType old_token_id = get_token_id(entry);

  log_.log(
    "[",
    name(),
    "] Renaming node id ",
    node_id,
    " to ",
    new_token.get_label(),
    ".\n");

  if (old_token_id != new_token_id) {
    InAlgoTiming::resetstart(interval_);
    max_token_id_ = token_map().max_id();

    // update node index
    set_token_id(entry, new_token_id);

    InAlgoTiming::stopaddvalue(
      interval_,
      statistics_.timingdetails.nodeindex.value);

    // update inverted map index

    // get all elements within partition, iterate over them, and delete given
    // node id
    const SizeType size = get_size(entry);
    delete_node_lists(old_token_id, size, node_id);
    insert_node_lists(new_token_id, size, node_id);
  }
}

template<class _NodeData, class _TEDAlgorithm>
void SlimInvertedMapIndex<_NodeData, _TEDAlgorithm>::deletenode(const NodeIdType node_id) {
  // NOTE
  // Currently, there is no (explicit) support for deletion of the root node.
  // However, the SlimCone algorithm may still report correct results if the root node is
  // deleted, since the document itself will most probably never be in the top-k ranking.

  using common::InAlgoTiming;

  // invalid node id
  if (node_id > node_index_.size()) {
    return;
  }

  // get entry and its fields from node index
  auto& entry_todelete = get_entry(node_id);
  const TokenIdType token_id = get_token_id(entry_todelete);
  const SizeType size = get_size(entry_todelete);
  const NodeIdType parent_id = get_parent(entry_todelete);
  const NodeIdType first_child_id = get_first_child(entry_todelete);
  const NodeIdType next_sibling_id = get_next_sibling(entry_todelete);

  // node id already deleted; do nothing
  if (size == 0) {
    return;
  }

  log_.log(
    "[",
    name(),
    "] Deleting node id ",
    node_id,
    " (size: ",
    size,
    ").\n");

  InAlgoTiming::resetstart(interval_);

  auto& entry_parent = get_entry(parent_id);

  const bool is_inner_node = (size > 1);

  NodeIdType new_sibling_id_of_previous_sibling = next_sibling_id;
  if (is_inner_node) {
    set_parents_of_all_children(entry_todelete, parent_id);
    new_sibling_id_of_previous_sibling = first_child_id;
  }

  auto& entry_previous_sibling = get_previous_sibling(node_id);
  set_next_sibling(entry_previous_sibling, new_sibling_id_of_previous_sibling);

  if (is_inner_node) {
    const auto& start_sibling_id = get_next_sibling(entry_previous_sibling);
    auto& entry_last_sibling = get_last_sibling(start_sibling_id);
    set_next_sibling(entry_last_sibling, next_sibling_id);
  }

  if (get_first_child(entry_parent) == node_id) {
    set_first_child(entry_parent, new_sibling_id_of_previous_sibling);
  }

  InAlgoTiming::stopaddvalue(
    interval_,
    statistics_.timingdetails.nodeindex.value);

  // update sizes of all parents
  update_parents_lists(parent_id);
  // delete node id from inverted list
  delete_node_lists(token_id, size, node_id);
  // in any case, tombstone deleted node index entry
  tombstone_node(entry_todelete);

  return;
}

template<class _NodeData, class _TEDAlgorithm>
inline typename SlimInvertedMapIndex<_NodeData, _TEDAlgorithm>::template POQueue<_NodeData> SlimInvertedMapIndex<_NodeData, _TEDAlgorithm>::reconstruct_subtree(
    const NodeIdType subtree_id) {
  POQueue<_NodeData> subtree(token_map());

  const SizeType subtree_size = get_size(subtree_id);

  // std::cout << "Reconstructing " << subtree_id << "." << std::endl;

  auto current_entry = get_lld(get_entry(subtree_id));
  bool coming_from_child = false;
  for (SizeType size = subtree_size; size > 0; --size) {
    subtree.append(get_token_id(current_entry), get_size(current_entry));

    auto next = get_next_entry(current_entry, coming_from_child);
    current_entry = next.first;
    coming_from_child = next.second;
  }

  log_.log(
    "[",
    name(),
    "] Reconstructing and verifying subtree id ",
    subtree_id,
    " (size = ",
    subtree_size,
    ").\n");

  return subtree;
}

template<class _NodeData, class _TEDAlgorithm>
unsigned int SlimInvertedMapIndex<_NodeData, _TEDAlgorithm>::lb_label(
    const SizeType query_size,
    std::unordered_map<TokenIdType, std::pair<unsigned int, unsigned int>>& query_token_id_map,
    const SizeType subtree_size,
    const NodeIdType subtree_id) {
  // reset overlap counts
  for (auto& e: query_token_id_map) {
    e.second.second = 0;
  }

  // compute overlap
  SizeType maxoverlap = std::min(subtree_size, query_size);
  unsigned int overlap = 0;

  typename NodeIndex::value_type current_entry = get_entry(subtree_id);
  bool coming_from_child = false;

  for (SizeType size = subtree_size; size > 0; --size) {
    const TokenIdType token_id = get_token_id(current_entry);

    auto it = query_token_id_map.find(token_id);
    if (it != query_token_id_map.end() && it->second.first > it->second.second)
    {
      ++(it->second.second);

      // increase overlap and check in one statement
      if (++overlap >= maxoverlap) {
        break;
      }
    }

    auto next = get_next_entry(current_entry, coming_from_child);
    current_entry = next.first;
    coming_from_child = next.second;
  }

  const unsigned long label_lower_bound =
    std::max(query_size, subtree_size) - overlap;

  log_.log(
    "[",
    name(),
    "] Computing label lower bound for subtree id ",
    subtree_id,
    " (size = ",
    subtree_size,
    "). Label lower bound = ",
    label_lower_bound,
    " (overlap = ",
    overlap,
    ").\n");

  return label_lower_bound;
}

/*
template<class _NodeData, class _TEDAlgorithm>
inline std::vector<typename AbstractInvertedMapIndex<_NodeData, typename AbstractIndex<_NodeData>::template SymmetricQuintuple<unsigned int>>::NodeIdType> SlimInvertedMapIndex<_NodeData, _TEDAlgorithm>::get_path_to_first_larger_parent(
    const NodeIdType descendant_id,
    const SizeType min_size) const {
  std::vector<NodeIdType> path;

  NodeIdType current_subtree_id = descendant_id;

  while (1) {
    const NodeIdType parent_subtree_id = get_parent(current_subtree_id);
    const SizeType current_subtree_size = get_size(current_subtree_id);

    path.push_back(current_subtree_id);

    if ((parent_subtree_id == 0) || (current_subtree_size >= min_size)) {
      break;
    }

    current_subtree_id = parent_subtree_id;
  }

  return path;
}
*/

template<class _NodeData, class _TEDAlgorithm>
inline typename SlimInvertedMapIndex<_NodeData, _TEDAlgorithm>::NodeIdType SlimInvertedMapIndex<_NodeData, _TEDAlgorithm>::get_parent(
    const NodeIndexEntry& entry) const {
  try {
    return std::get<2>(entry);
  } catch(const std::out_of_range& oore) {
    throw;
  }
}

template<class _NodeData, class _TEDAlgorithm>
inline typename SlimInvertedMapIndex<_NodeData, _TEDAlgorithm>::NodeIdType SlimInvertedMapIndex<_NodeData, _TEDAlgorithm>::get_first_child(
    const NodeIndexEntry& entry) const {
  try {
    return std::get<3>(entry);
  } catch(const std::out_of_range& oore) {
    throw;
  }
}

template<class _NodeData, class _TEDAlgorithm>
inline typename SlimInvertedMapIndex<_NodeData, _TEDAlgorithm>::NodeIdType SlimInvertedMapIndex<_NodeData, _TEDAlgorithm>::get_next_sibling(
    const NodeIndexEntry& entry) const {
  try {
    return std::get<4>(entry);
  } catch(const std::out_of_range& oore) {
    throw;
  }
}

template<class _NodeData, class _TEDAlgorithm>
inline typename SlimInvertedMapIndex<_NodeData, _TEDAlgorithm>::NodeIdType SlimInvertedMapIndex<_NodeData, _TEDAlgorithm>::get_parent(
    const NodeIdType subtree_id) const {
  try {
    return get_parent(get_entry(subtree_id));
  } catch(const std::out_of_range& oore) {
    throw;
  }
}

template<class _NodeData, class _TEDAlgorithm>
inline typename SlimInvertedMapIndex<_NodeData, _TEDAlgorithm>::NodeIdType SlimInvertedMapIndex<_NodeData, _TEDAlgorithm>::get_first_child(
    const NodeIdType subtree_id) const {
  try {
    return get_first_child(get_entry(subtree_id));
  } catch(const std::out_of_range& oore) {
    throw;
  }
}

template<class _NodeData, class _TEDAlgorithm>
inline typename SlimInvertedMapIndex<_NodeData, _TEDAlgorithm>::NodeIdType SlimInvertedMapIndex<_NodeData, _TEDAlgorithm>::get_next_sibling(
    const NodeIdType subtree_id) const {
  try {
    return get_next_sibling(get_entry(subtree_id));
  } catch(const std::out_of_range& oore) {
    throw;
  }
}

template<class _NodeData, class _TEDAlgorithm>
inline typename SlimInvertedMapIndex<_NodeData, _TEDAlgorithm>::SizeType SlimInvertedMapIndex<_NodeData, _TEDAlgorithm>::get_list_length(
    const TokenIdType token_id) const {
  try {
    return inverted_list_lengths_.at(token_id);
  } catch(const std::out_of_range& oore) {
    throw;
  }
}

template<class _NodeData, class _NodeIndexEntry>
inline typename SlimInvertedMapIndex<_NodeData, _NodeIndexEntry>::NodeIdType SlimInvertedMapIndex<_NodeData, _NodeIndexEntry>::set_parent(
    NodeIndexEntry& entry,
    const NodeIdType new_parent_id) {
  std::get<2>(entry) = new_parent_id;
  return get_parent(entry);
}

template<class _NodeData, class _NodeIndexEntry>
inline typename SlimInvertedMapIndex<_NodeData, _NodeIndexEntry>::NodeIdType SlimInvertedMapIndex<_NodeData, _NodeIndexEntry>::set_first_child(
    NodeIndexEntry& entry,
    const NodeIdType new_first_child_id) {
  std::get<3>(entry) = new_first_child_id;
  return get_first_child(entry);
}

template<class _NodeData, class _NodeIndexEntry>
inline typename SlimInvertedMapIndex<_NodeData, _NodeIndexEntry>::NodeIdType SlimInvertedMapIndex<_NodeData, _NodeIndexEntry>::set_next_sibling(
    NodeIndexEntry& entry,
    const NodeIdType new_next_sibling_id) {
  std::get<4>(entry) = new_next_sibling_id;
  return get_next_sibling(entry);
}

/// private member functions

template<class _NodeData, class _TEDAlgorithm>
inline typename SlimInvertedMapIndex<_NodeData, _TEDAlgorithm>::NodeIndexEntry SlimInvertedMapIndex<_NodeData, _TEDAlgorithm>:: get_lld(
    const NodeIndexEntry& root_entry) const {
  // mutable
  NodeIdType first_child_id = get_first_child(root_entry);

  if (first_child_id == 0) {
    return root_entry;
  }

  NodeIdType lld_id = first_child_id;
  while (first_child_id > 0) {
    lld_id = first_child_id;
    first_child_id = get_first_child(get_entry(first_child_id));
  }

  return get_entry(lld_id);
}

template<class _NodeData, class _TEDAlgorithm>
inline std::pair<typename SlimInvertedMapIndex<_NodeData, _TEDAlgorithm>::NodeIndexEntry, bool> SlimInvertedMapIndex<_NodeData, _TEDAlgorithm>::get_next_entry(
    NodeIndexEntry& current_entry,
    bool coming_from_child) const {
  // immutable
  const NodeIdType parent_id = get_parent(current_entry);
  const NodeIdType next_sibling_id = get_next_sibling(current_entry);

  // mutable
  NodeIdType first_child_id = get_first_child(current_entry);

  if (!coming_from_child && first_child_id > 0) {
    // std::cout << "1" << std::endl;
    return std::make_pair(get_lld(current_entry), false);
  }

  if (next_sibling_id > 0) {
    // std::cout << "2" << std::endl;
    return std::make_pair(get_lld(get_entry(next_sibling_id)), false);
  }

  // std::cout << "3" << std::endl;
  return std::make_pair(get_entry(parent_id), true);
}

template<class _NodeData, class _TEDAlgorithm>
void SlimInvertedMapIndex<_NodeData, _TEDAlgorithm>::delete_node_lists(
    const TokenIdType token_id,
    const SizeType size,
    const NodeIdType node_id) {
  using common::InAlgoTiming;

  InAlgoTiming::resetstart(interval_);

  auto& inverted_map = inverted_maps_.at(token_id);
  /*
  auto partition_iters = inverted_map.equal_range(size);

  auto node_it = partition_iters.first;
  for (; node_it != partition_iters.second; ++node_it) {
    if (node_it->second == node_id) {
      inverted_map.erase(node_it);
      break;
    }
  }
  */

  inverted_map.erase(size, node_id);
}

template<class _NodeData, class _TEDAlgorithm>
inline void SlimInvertedMapIndex<_NodeData, _TEDAlgorithm>::insert_node_lists(
    const TokenIdType token_id,
    const SizeType size,
    const NodeIdType node_id) {
  using common::InAlgoTiming;

  InAlgoTiming::resetstart(interval_);

  // insert node id into new list (allocate list if it does not exist yet)
  if (token_id > max_token_id_) {
    inverted_maps_.push_back(data_structures::VectorMultimap<SizeType, NodeIdType>());
  }

  inverted_maps_.at(token_id).emplace(size, node_id);

  InAlgoTiming::stopaddvalue(
    interval_,
    statistics_.timingdetails.listinsertions.value);
}

template<class _NodeData, class _TEDAlgorithm>
inline void SlimInvertedMapIndex<_NodeData, _TEDAlgorithm>::set_parents_of_all_children(
    const NodeIndexEntry& entry,
    const NodeIdType parent_id) {
  NodeIdType current_child_id = get_first_child(entry);
  do {
    set_parent(get_entry(current_child_id), parent_id);
    current_child_id = get_next_sibling(get_entry(current_child_id));
  } while (current_child_id != 0);
}

template<class _NodeData, class _TEDAlgorithm>
void SlimInvertedMapIndex<_NodeData, _TEDAlgorithm>::update_parents_lists(
    const NodeIdType parent_id) {
  NodeIdType next_parent_id = parent_id;
  do {
    const TokenIdType next_token_id = get_token_id(next_parent_id);
    SizeType next_size = get_size(next_parent_id);

    // update node index entry
    set_size(get_entry(next_parent_id), next_size - 1);

    // update inverted list entry
    delete_node_lists(next_token_id, next_size, next_parent_id);
    insert_node_lists(next_token_id, next_size - 1, next_parent_id);

    next_parent_id = get_parent(next_parent_id);
  } while (next_parent_id != 0);
}

template<class _NodeData, class _TEDAlgorithm>
inline typename SlimInvertedMapIndex<_NodeData, _TEDAlgorithm>::NodeIndexEntry& SlimInvertedMapIndex<_NodeData, _TEDAlgorithm>::get_previous_sibling(
    const NodeIdType node_id) {
  const auto& entry_parent = get_entry(get_parent(get_entry(node_id)));
  const NodeIdType first_sibling_id = get_first_child(entry_parent);

  NodeIdType current_sibling_id = first_sibling_id;
  NodeIdType previous_sibling_id = first_sibling_id;
  while (current_sibling_id != node_id && get_next_sibling(current_sibling_id) != 0) {
    const auto& entry_current_sibling = get_entry(current_sibling_id);
    previous_sibling_id = current_sibling_id;
    current_sibling_id = get_next_sibling(entry_current_sibling);
  }

  return get_entry(previous_sibling_id);
}

template<class _NodeData, class _TEDAlgorithm>
inline typename SlimInvertedMapIndex<_NodeData, _TEDAlgorithm>::NodeIndexEntry& SlimInvertedMapIndex<_NodeData, _TEDAlgorithm>::get_last_sibling(
    const NodeIdType start_sibling_id) {
  NodeIdType current_sibling_id = start_sibling_id;
  while (get_next_sibling(current_sibling_id) != 0) {
    const auto& entry_current_sibling = get_entry(current_sibling_id);
    current_sibling_id = get_next_sibling(entry_current_sibling);
  }

  return get_entry(current_sibling_id);
}

template<class _NodeData, class _TEDAlgorithm>
inline void SlimInvertedMapIndex<_NodeData, _TEDAlgorithm>::tombstone_node(
    NodeIndexEntry& entry) {
  set_size(entry, 0);
  set_parent(entry, 0);
  set_first_child(entry, 0);
  set_next_sibling(entry, 0);
}

/// private friend functions

template<class _T1, class _T2>
std::ostream& operator<<(
    std::ostream& os,
    const SlimInvertedMapIndex<_T1, _T2>& index) {
  append_size(os, index);
  append_node_index(os, index);
  append_token_map(os, index);
  //append_inverted_maps(os, index);

  return os;
}

template<class _T1, class _T2>
void append_size(std::ostream& os, const SlimInvertedMapIndex<_T1, _T2>& index) {
  using std::setw;

  os << "Index size:\n"
    << setw(10) << index.size() << " inverted map entries\n";
}

template<class _T1, class _T2>
void append_node_index(
    std::ostream& os,
    const SlimInvertedMapIndex<_T1, _T2>& index) {
  using std::setw;

  unsigned int counter{0};

  os << "Node Index entries:\n"
    << setw(10) << "<index>"
    << " -> (<token-id>, <subtree-size>, <parent-id>, <first-child>, <next-sibling>)\n";
  for (const auto& i: index.node_index_) {
    os << setw(10) << counter++ << " -> (";
    os << index.get_token_id(i) << ", ";
    os << index.get_size(i) << ", ";
    os << index.get_parent(i) << ", ";
    os << index.get_first_child(i) << ", ";
    os << index.get_next_sibling(i);
    os << ")\n";
  }
}

#endif // INDEX_SLIM_INVERTED_MAP_INDEX_IMPL_H
