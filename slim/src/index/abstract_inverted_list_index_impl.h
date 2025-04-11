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

#ifndef INDEX_ABSTRACT_INVERTED_LIST_INDEX_IMPL_H
#define INDEX_ABSTRACT_INVERTED_LIST_INDEX_IMPL_H

template<class _NodeData, class _NodeIndexEntry>
AbstractInvertedListIndex<_NodeData, _NodeIndexEntry>::AbstractInvertedListIndex(
    LogType& log,
    const std::string& name)
    : AbstractIndex<_NodeData>(log, name) {}

template<class _NodeData, class _NodeIndexEntry>
AbstractInvertedListIndex<_NodeData, _NodeIndexEntry>::~AbstractInvertedListIndex()
{}

template<class _NodeData, class _NodeIndexEntry>
inline void AbstractInvertedListIndex<_NodeData, _NodeIndexEntry>::prepare() {
  max_token_id_ = token_map().max_id();
  AbstractIndex<_NodeData>::prepare();
}

template<class _NodeData, class _NodeIndexEntry>
inline typename AbstractInvertedListIndex<_NodeData, _NodeIndexEntry>::TokenMap& AbstractInvertedListIndex<_NodeData, _NodeIndexEntry>::token_map()
{
  return token_map_;
}

template<class _NodeData, class _NodeIndexEntry>
inline typename AbstractInvertedListIndex<_NodeData, _NodeIndexEntry>::TokenIdType AbstractInvertedListIndex<_NodeData, _NodeIndexEntry>::put(
    const _NodeData& token,
    const SizeType size) {
  const TokenIdType token_id = convert_token(token);

  statistics_.subtrees.record(size);
  AbstractIndex<_NodeData>::unprepare();

  return token_id;
}

template<class _NodeData, class _NodeIndexEntry>
inline typename AbstractInvertedListIndex<_NodeData, _NodeIndexEntry>::TokenIdType AbstractInvertedListIndex<_NodeData, _NodeIndexEntry>::convert_token(
    const _NodeData& token) {
  if (!token_map_.contains(token)) {
    return token_map_.insert(token);
  }

  return token_map_.at(token);
}

template<class _NodeData, class _NodeIndexEntry>
inline typename AbstractInvertedListIndex<_NodeData, _NodeIndexEntry>::TokenFrequencyMap AbstractInvertedListIndex<_NodeData, _NodeIndexEntry>::reconstruct_token_id_map(
    const NodeIdType subtree_id) const {
  std::unordered_map<TokenIdType, unsigned int> token_id_map{
    {get_token_id(subtree_id), 1}};

  NodeIdType i = subtree_id - 1;
  for (SizeType size = get_size(subtree_id) - 1; size > 0; --size, --i) {
    ++token_id_map[get_token_id(i)];
  }

  return token_id_map;
}

template<class _NodeData, class _NodeIndexEntry>
inline typename AbstractInvertedListIndex<_NodeData, _NodeIndexEntry>::SizeType AbstractInvertedListIndex<_NodeData, _NodeIndexEntry>::contains(
    const TokenIdType token_id) const {
  return token_id <= max_token_id_;
}

template<class _NodeData, class _NodeIndexEntry>
inline typename AbstractInvertedListIndex<_NodeData, _NodeIndexEntry>::SizeType AbstractInvertedListIndex<_NodeData, _NodeIndexEntry>::nodes() const {
  return node_index_.size() - 1;
}

template<class _NodeData, class _NodeIndexEntry>
inline const typename AbstractInvertedListIndex<_NodeData, _NodeIndexEntry>::List& AbstractInvertedListIndex<_NodeData, _NodeIndexEntry>::get_list_ref(
    const TokenIdType token_id) const {
  try {
    return inverted_lists_.at(token_id);
  } catch(const std::out_of_range& oore) {
    throw;
  }
}

template<class _NodeData, class _NodeIndexEntry>
inline const _NodeIndexEntry& AbstractInvertedListIndex<_NodeData, _NodeIndexEntry>::get_entry(
    const NodeIdType subtree_id) const {
  try {
    return node_index_.at(subtree_id);
  } catch(const std::out_of_range& oore) {
    throw;
  }
}

template<class _NodeData, class _NodeIndexEntry>
inline typename AbstractInvertedListIndex<_NodeData, _NodeIndexEntry>::TokenIdType AbstractInvertedListIndex<_NodeData, _NodeIndexEntry>::get_token_id(
    const _NodeIndexEntry& entry) const {
  try {
    return std::get<0>(entry);
  } catch(const std::out_of_range& oore) {
    throw;
  }
}

template<class _NodeData, class _NodeIndexEntry>
inline typename AbstractInvertedListIndex<_NodeData, _NodeIndexEntry>::SizeType AbstractInvertedListIndex<_NodeData, _NodeIndexEntry>::get_size(
    const _NodeIndexEntry& entry) const {
  try {
    return std::get<1>(entry);
  } catch(const std::out_of_range& oore) {
    throw;
  }
}

template<class _NodeData, class _NodeIndexEntry>
inline typename AbstractInvertedListIndex<_NodeData, _NodeIndexEntry>::TokenIdType AbstractInvertedListIndex<_NodeData, _NodeIndexEntry>::get_token_id(
    const NodeIdType subtree_id) const {
  try {
    return get_token_id(get_entry(subtree_id));
  } catch(const std::out_of_range& oore) {
    throw;
  }
}

template<class _NodeData, class _NodeIndexEntry>
inline typename AbstractInvertedListIndex<_NodeData, _NodeIndexEntry>::SizeType AbstractInvertedListIndex<_NodeData, _NodeIndexEntry>::get_size(
    const NodeIdType subtree_id) const {
  try {
    return get_size(get_entry(subtree_id));
  } catch(const std::out_of_range& oore) {
    throw;
  }
}

/// protected friend functions

template<class _T1, class _T2>
void append_token_map(
    std::ostream& os,
    const AbstractInvertedListIndex<_T1, _T2>& index) {
  using std::setw;

  os << "Token map:\n"
    << setw(10) << "<token>"
    << " -> <token-id>\n";
  for (const auto& p: index.token_map_.data_to_id()) {
    os << setw(10) << p.first.get_label() << " -> " << p.second << "\n";
  }
}

template<class _T1, class _T2>
void append_inverted_lists(
    std::ostream& os,
    const AbstractInvertedListIndex<_T1, _T2>& index) {
  using std::setw;

  unsigned int counter{0};

  os << "Inverted lists:\n"
    << setw(10) << "<token-id>"
    << " -> [<postorder ids in index containing the token>]\n";
  for (const auto& i: index.inverted_lists_) {
    if (counter == 0) {
      counter++;
      continue;
    }

    os << setw(10) << counter++ << " -> [ ";
    for (const auto& r: i) {
      os << r << " ";
    }
    os << "]\n";
  }
}

#endif // INDEX_ABSTRACT_INVERTED_LIST_INDEX_IMPL_H
