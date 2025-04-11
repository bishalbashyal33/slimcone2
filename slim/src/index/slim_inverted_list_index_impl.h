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

#ifndef INDEX_SLIM_INVERTED_LIST_INDEX_IMPL_H
#define INDEX_SLIM_INVERTED_LIST_INDEX_IMPL_H

/// public constructors/destructors

template<class _NodeData, class _TEDAlgorithm>
SlimInvertedListIndex<_NodeData, _TEDAlgorithm>::SlimInvertedListIndex(LogType& log)
    : AbstractInvertedMapIndex<_NodeData, typename AbstractIndex<_NodeData>::template SymmetricTriple<unsigned int>>(log, "slim inverted list index") {
  // 1-based indexes:
  node_index_.resize(1);
  inverted_maps_.resize(1);
}

template<class _NodeData, class _TEDAlgorithm>
SlimInvertedListIndex<_NodeData, _TEDAlgorithm>::~SlimInvertedListIndex() {}

/// public member functions

template<class _NodeData, class _TEDAlgorithm>
inline void SlimInvertedListIndex<_NodeData, _TEDAlgorithm>::prepare() {
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
    const auto& subtreesize = std::get<1>(node_index_.at(rootid));
    std::unordered_set<unsigned int> seentokenids(subtreesize);

    unsigned int currentid = rootid;
    for (; currentid >= rootid - subtreesize + 1; --currentid) {
      const auto& tokenid = std::get<0>(node_index_.at(currentid));

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
  unsigned int subtree_id{1};
  // first entry in node index is dummy entry, thus skip it
  typename decltype(node_index_)::iterator nit = std::next(node_index_.begin());
  typename decltype(node_index_)::iterator nitend = node_index_.end();
  for (; nit != nitend; ++nit) {
    const auto& n = *nit;
    const unsigned int token_id = std::get<0>(n);
    const unsigned int subtree_size = std::get<1>(n);

    InAlgoTiming::resetstart(interval_);
    inverted_maps_.at(token_id).emplace(subtree_size, subtree_id);
    InAlgoTiming::stopaddvalue(
      interval_,
      statistics_.timingbuilddetails.preparelistappend.value);

    // loop is only executed if subtree_size > 1
    InAlgoTiming::resetstart(interval_);
    for (unsigned int i = 0; i < subtree_size - 1; ++i) {
      // do NOT use get_parent here since we need a reference
      unsigned int& parent_id_ref =
        std::get<2>(node_index_.at(subtree_id - i - 1));

      if (parent_id_ref == 0) {
	      // if no parent id is set yet, set it
        parent_id_ref = subtree_id;
      }
    }
    InAlgoTiming::stopaddvalue(
      interval_,
      statistics_.timingbuilddetails.preparenodeindexupdate.value);

    ++subtree_id;
  }

  // O(k * n * log(n)) for k lists, each of length n
  InAlgoTiming::resetstart(interval_);
  /*
  for (auto& list: inverted_lists_) {
    std::sort(
      list.begin(),
      list.end(),
      [this](const int lhs_subtree_id, const int rhs_subtree_id) -> bool {
        const unsigned int lhs_subtree_size = get_size(lhs_subtree_id);
        const unsigned int rhs_subtree_size = get_size(rhs_subtree_id);

        if (lhs_subtree_size == rhs_subtree_size) {
          return lhs_subtree_id < rhs_subtree_id;
        }

        return lhs_subtree_size < rhs_subtree_size;
      });
  }
  */
  InAlgoTiming::stopsetvalue(
    interval_,
    statistics_.timingbuilddetails.preparesortalllists.value);

  size_ = inverted_maps_.size() - 1;

  for (const auto& invertedlist: inverted_maps_) {
    statistics_.invertedlists.record(invertedlist.size());
  }

  AbstractInvertedMapIndex<_NodeData, typename AbstractIndex<_NodeData>::template SymmetricTriple<unsigned int>>::prepare();
}

template<class _NodeData, class _TEDAlgorithm>
inline typename SlimInvertedListIndex<_NodeData, _TEDAlgorithm>::TokenIdType SlimInvertedListIndex<_NodeData, _TEDAlgorithm>::put(
    const _NodeData& token,
    const SizeType size) {
  // type shortcuts/defs.
  using common::InAlgoTiming;

  InAlgoTiming::resetstart(interval_);
  const TokenIdType token_id =
    AbstractInvertedMapIndex<_NodeData, typename AbstractIndex<_NodeData>::template SymmetricTriple<unsigned int>>::put(token, size);

  // O(1) + O(time for allocation)
  node_index_.emplace_back(token_id, size, 0);
  InAlgoTiming::stopaddvalue(
    interval_,
    statistics_.timingbuilddetails.put.value);

  return token_id;
}

template<class _NodeData, class _TEDAlgorithm>
inline typename SlimInvertedListIndex<_NodeData, _TEDAlgorithm>::template POQueue<_NodeData> SlimInvertedListIndex<_NodeData, _TEDAlgorithm>::reconstruct_subtree(
    const NodeIdType subtree_id) {
  POQueue<_NodeData> subtree(token_map());

  const SizeType subtree_size = get_size(subtree_id);

  for (auto i = subtree_id - subtree_size + 1; i <= subtree_id; ++i) {
    subtree.append(get_token_id(i), get_size(i));
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
unsigned int SlimInvertedListIndex<_NodeData, _TEDAlgorithm>::lb_label(
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
  NodeIdType i = subtree_id;
  unsigned int overlap = 0;
  for (SizeType size = subtree_size; size > 0; --size, --i) {
    auto it = query_token_id_map.find(get_token_id(i));
    if (it != query_token_id_map.end() && it->second.first > it->second.second)
    {
      ++(it->second.second);

      // increase overlap and check in one statement
      if (++overlap >= maxoverlap) {
        break;
      }
    }
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

template<class _NodeData, class _TEDAlgorithm>
inline typename SlimInvertedListIndex<_NodeData, _TEDAlgorithm>::List SlimInvertedListIndex<_NodeData, _TEDAlgorithm>::get_path_to_first_larger_parent(
    const NodeIdType descendant_id,
    const SizeType min_size) const {
  List path;

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

template<class _NodeData, class _TEDAlgorithm>
inline typename SlimInvertedListIndex<_NodeData, _TEDAlgorithm>::NodeIdType SlimInvertedListIndex<_NodeData, _TEDAlgorithm>::get_parent(
    const NodeIdType subtree_id) const {
  try {
    return std::get<2>(get_entry(subtree_id));
  } catch(const std::out_of_range& oore) {
    throw;
  }
}

template<class _NodeData, class _TEDAlgorithm>
inline typename SlimInvertedListIndex<_NodeData, _TEDAlgorithm>::SizeType SlimInvertedListIndex<_NodeData, _TEDAlgorithm>::get_list_length(
    const TokenIdType token_id) const {
  try {
    return inverted_list_lengths_.at(token_id);
  } catch(const std::out_of_range& oore) {
    throw;
  }
}

/// private friend functions

template<class _T1, class _T2>
std::ostream& operator<<(
    std::ostream& os,
    const SlimInvertedListIndex<_T1, _T2>& index) {
  append_size(os, index);
  append_node_index(os, index);
  append_token_map(os, index);
  //append_inverted_lists(os, index);

  return os;
}

template<class _T1, class _T2>
void append_size(std::ostream& os, const SlimInvertedListIndex<_T1, _T2>& index)
{
  using std::setw;

  os << "Index size:\n"
    << setw(10) << index.size() << " inverted list entries\n";
}

template<class _T1, class _T2>
void append_node_index(
    std::ostream& os,
    const SlimInvertedListIndex<_T1, _T2>& index) {
  using std::setw;

  unsigned int counter{0};

  os << "Node Index entries:\n"
    << setw(10) << "<index>"
    << " -> (<token-id>, <subtree-size>, <parent-id>)\n";
  for (const auto& i: index.node_index_) {
    os << setw(10) << counter++ << " -> (";
    os << std::get<0>(i) << ", ";
    os << std::get<1>(i) << ", ";
    os << std::get<2>(i);
    os << ")\n";
  }
}

#endif // INDEX_SLIM_INVERTED_LIST_INDEX_IMPL_H
