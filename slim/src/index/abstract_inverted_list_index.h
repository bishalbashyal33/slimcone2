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

#ifndef INDEX_ABSTRACT_INVERTED_LIST_INDEX_H
#define INDEX_ABSTRACT_INVERTED_LIST_INDEX_H

#include "abstract_index.h"

#include <unordered_set>

namespace tree_index {

template<
  class _NodeData,
  class NodeIndexEntry = typename AbstractIndex<_NodeData>::template SymmetricPair<unsigned int>>
class AbstractInvertedListIndex : public AbstractIndex<_NodeData> {
public: // member types
  // make inherited type definitions visible
  using typename AbstractIndex<_NodeData>::SizeType;
  using typename AbstractIndex<_NodeData>::TokenIdType;
  using typename AbstractIndex<_NodeData>::NodeIdType;
  using typename AbstractIndex<_NodeData>::TokenFrequencyMap;
  using typename AbstractIndex<_NodeData>::LogType;

  using List = std::vector<NodeIdType>;
  using TokenIdSet = List;
  using TokenMap = data_structures::LabelHashMap<_NodeData>;
  using SizeSetType = std::unordered_set<SizeType>;
  using NodeIndex = std::vector<NodeIndexEntry>;
  using InvertedLists = std::vector<List>;

public: // member functions
  AbstractInvertedListIndex(LogType& log, const std::string& name);
  ~AbstractInvertedListIndex();

  // Getter
  inline TokenMap& token_map(); // mutable

  inline void prepare();
  inline TokenIdType put(const _NodeData& token, const SizeType size);
  inline TokenIdType convert_token(const _NodeData& token);
  inline TokenFrequencyMap reconstruct_token_id_map(NodeIdType subtree_id) const;
  inline SizeType contains(const TokenIdType token_id) const;
  inline SizeType nodes() const;

  inline const List& get_list_ref(const TokenIdType token_id) const;

  inline const NodeIndexEntry& get_entry(const NodeIdType subtree_id) const;
  inline TokenIdType get_token_id(const NodeIndexEntry& entry) const;
  inline SizeType get_size(const NodeIndexEntry& entry) const;
  inline TokenIdType get_token_id(const NodeIdType subtree_id) const;
  inline SizeType get_size(const NodeIdType subtree_id) const;

public: // member variables
  // make inherited member variables visible
  using AbstractIndex<_NodeData>::statistics_;

protected: // member functions
  template<class _T1, class _T2>
  friend void append_token_map(
    std::ostream& os,
    const AbstractInvertedListIndex<_T1, _T2>& index);
  template<class _T1, class _T2>
  friend void append_inverted_lists(
      std::ostream& os,
      const AbstractInvertedListIndex<_T1, _T2>& index);

protected: // member variables
  InvertedLists inverted_lists_{};
  NodeIndex node_index_{};
  TokenMap token_map_{};
  SizeType max_token_id_{};
};

#include "abstract_inverted_list_index_impl.h"

} // namespace tree_index

#endif // INDEX_ABSTRACT_INVERTED_LIST_INDEX_H
