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

#ifndef INDEX_INVERTED_LIST_INDEX_IMPL_H
#define INDEX_INVERTED_LIST_INDEX_IMPL_H

/// public constructors/destructors

template<class _NodeData, class _TEDAlgorithm>
InvertedListIndex<_NodeData, _TEDAlgorithm>::InvertedListIndex(LogType& log)
    : AbstractInvertedListIndex<_NodeData>(log, "inverted list index") {
  // 1-based indexes:
  node_index_.resize(1);
  inverted_lists_.resize(1);
}

template<class _NodeData, class _TEDAlgorithm>
InvertedListIndex<_NodeData, _TEDAlgorithm>::~InvertedListIndex() {}

/// public member functions

template<class _NodeData, class _TEDAlgorithm>
inline const typename InvertedListIndex<_NodeData, _TEDAlgorithm>::SizeSetType& InvertedListIndex<_NodeData, _TEDAlgorithm>::sizes() const
{
  return sizes_;
}

template<class _NodeData, class _TEDAlgorithm>
inline void InvertedListIndex<_NodeData, _TEDAlgorithm>::prepare() {
  if (ready()) {
    return;
  }

  // O(n)
  inverted_lists_.resize(token_map().size() + 1);

  NodeIdType rootid{0};
  unsigned int end{static_cast<unsigned int>(node_index_.size())};
  for (rootid = 1; rootid < end; ++rootid) {
    const SizeType subtreesize = get_size(rootid);
    std::unordered_set<TokenIdType> seentokenids(subtreesize);

    NodeIdType currentid = rootid;
    for (; currentid >= rootid - subtreesize + 1; --currentid) {
      const TokenIdType tokenid = get_token_id(currentid);

      // skip duplicate token id in current subtree
      if (seentokenids.find(tokenid) != seentokenids.end()) {
        continue;
      }

      seentokenids.insert(tokenid);
      inverted_lists_.at(tokenid).push_back(rootid);
    }
  }

  for (auto& list: inverted_lists_) {
    std::sort(
      list.begin(),
      list.end(),
      [this](
          const NodeIdType lhs_subtree_id,
          const NodeIdType rhs_subtree_id) -> bool {
        const SizeType lhs_subtree_size = get_size(lhs_subtree_id);
        const SizeType rhs_subtree_size = get_size(rhs_subtree_id);

        if (lhs_subtree_size == rhs_subtree_size) {
          return lhs_subtree_id < rhs_subtree_id;
        }

        return lhs_subtree_size < rhs_subtree_size;
      });
  }

  size_ = inverted_lists_.size() - 1;

  for (const auto& invertedlist: inverted_lists_) {
    statistics_.invertedlists.record(invertedlist.size());
  }

  AbstractInvertedListIndex<_NodeData>::prepare();
}

template<class _NodeData, class _TEDAlgorithm>
inline typename InvertedListIndex<_NodeData, _TEDAlgorithm>::TokenIdType InvertedListIndex<_NodeData, _TEDAlgorithm>::put(
    const _NodeData& token,
    const SizeType size) {
  const TokenIdType token_id =
    AbstractInvertedListIndex<_NodeData>::put(token, size);

  // O(1) + O(time for allocation)
  node_index_.emplace_back(token_id, size);

  // O(1) amortized
  sizes_.insert(size);

  return token_id;
}

template<class _NodeData, class _TEDAlgorithm>
inline typename InvertedListIndex<_NodeData, _TEDAlgorithm>::template POQueue<_NodeData> InvertedListIndex<_NodeData, _TEDAlgorithm>::reconstruct_subtree(
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
unsigned int InvertedListIndex<_NodeData, _TEDAlgorithm>::lb_label(
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

/// private friend functions

template<class _T1, class _T2>
std::ostream& operator<<(
    std::ostream& os,
    const InvertedListIndex<_T1, _T2>& index) {
  append_size(os, index);
  append_node_index(os, index);
  append_token_map(os, index);
  append_inverted_lists(os, index);

  return os;
}

template<class _T1, class _T2>
void append_size(std::ostream& os, const InvertedListIndex<_T1, _T2>& index) {
  using std::setw;

  os << "Index size:\n"
    << setw(10) << index.node_index_.size() << " index entries\n"
    << setw(10) << index.size() << " inverted list entries\n";
}

template<class _T1, class _T2>
void append_node_index(
    std::ostream& os,
    const InvertedListIndex<_T1, _T2>& index) {
  using std::setw;

  unsigned int counter{0};

  os << "Node Index entries:\n"
    << setw(10) << "<index>"
    << " -> (<token-id>, <subtree-size>)\n";
  for (const auto& i: index.node_index_) {
    os << setw(10) << counter++ << " -> (";
    os << i.first << ", " << i.second << ")\n";
  }
}

#endif // INDEX_INVERTED_LIST_INDEX_IMPL_H
