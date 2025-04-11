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

#ifndef INDEX_INVERTED_LIST_INDEX_H
#define INDEX_INVERTED_LIST_INDEX_H

#include "abstract_inverted_list_index.h"
#include "k_heap.h"
#include "node_distance_pair.h"
#include "label_hash_map.h"

#include <vector>
#include <iostream>
#include <iomanip>
#include <algorithm>

namespace tree_index {

template<class _NodeData, class _TEDAlgorithm>
class InvertedListIndex : public AbstractInvertedListIndex<_NodeData> {
public: // member types
  // make inherited type definitions visible
  using typename AbstractInvertedListIndex<_NodeData>::SizeType;
  using typename AbstractInvertedListIndex<_NodeData>::TokenIdType;
  using typename AbstractInvertedListIndex<_NodeData>::NodeIdType;
  using typename AbstractInvertedListIndex<_NodeData>::SizeSetType;
  using typename AbstractInvertedListIndex<_NodeData>::LogType;

  template<class _Type>
  using POQueue = typename _TEDAlgorithm::template POQueue<_Type>;

public: // member functions
  // make inherited member functions visible
  using AbstractIndex<_NodeData>::name;
  using AbstractIndex<_NodeData>::ready;
  using AbstractInvertedListIndex<_NodeData, typename AbstractIndex<_NodeData>::template SymmetricPair<unsigned int>>::token_map;
  using AbstractInvertedListIndex<_NodeData, typename AbstractIndex<_NodeData>::template SymmetricPair<unsigned int>>::get_token_id;
  using AbstractInvertedListIndex<_NodeData, typename AbstractIndex<_NodeData>::template SymmetricPair<unsigned int>>::get_size;

  InvertedListIndex(LogType& log);
  ~InvertedListIndex();

  inline void prepare();

  inline const SizeSetType& sizes() const; // immutable

  inline TokenIdType put(const _NodeData& token, const SizeType size);
  void rename(const NodeIdType node_id, const _NodeData& new_token) {}
  void deletenode(const NodeIdType node_id) {}

  inline POQueue<_NodeData> reconstruct_subtree(const NodeIdType subtree_id);
  unsigned int lb_label(
    const SizeType query_size,
    std::unordered_map<TokenIdType, std::pair<unsigned int, unsigned int>>& query_token_id_map,
    const SizeType subtree_size,
    const NodeIdType subtree_id);

  template<class _T1, class _T2>
  friend std::ostream& operator<<(
    std::ostream& os,
    const InvertedListIndex<_T1, _T2>& index);

public: // member variables
  // make inherited member variables visible
  using AbstractIndex<_NodeData>::statistics_;
  using AbstractIndex<_NodeData>::log_;

private: // member variables
  // make inherited member variables visible
  using AbstractIndex<_NodeData>::size_;
  using AbstractInvertedListIndex<_NodeData>::node_index_;
  using AbstractInvertedListIndex<_NodeData>::inverted_lists_;

  std::unordered_set<SizeType> sizes_{};

private: // member functions
  template<class _T1, class _T2>
  friend void append_size(
    std::ostream& os,
    const InvertedListIndex<_T1, _T2>& index);

  template<class _T1, class _T2>
  friend void append_node_index(
    std::ostream& os,
    const InvertedListIndex<_T1, _T2>& index);
};

#include "inverted_list_index_impl.h"

} // namespace tree_index

#endif // INDEX_INVERTED_LIST_INDEX_H
