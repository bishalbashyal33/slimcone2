#ifndef STRUCTURE_SEARCH_STRUCTURE_SEARCH_H
#define STRUCTURE_SEARCH_STRUCTURE_SEARCH_H

#include <vector>
#include <stack>
#include <list>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <cmath>

#include "hash_functors.h"
#include "btree.h"
#include "k_heap.h"
#include "posting_list_container.h"
#include "node_distance_pair.h"
#include "node_index_value.h"
#include "tasm_dynamic.h"

namespace structure_search {

////////////////////////////////////////////////////////////////////////////////
// Implements the StructureSearch algorithm described in
// 'Indexing for Subtree Similarity-Search using Edit Distance' by Sarah Cohen,
// published at ACM SIGMOD 2013.
//
// The StructureSearch algorithm solves the top-k subtree similarity-search
// (or top-k approximate subtree matching) problem by indexing the document trees
// in multiple indexes. For further information we refer to the original paper,
// i.e., https://doi.org/10.1145/2463676.2463716.
//
// Template params: _NodeData   Node data representing a label's data
//                  _Costs      Cost function used to compute tree edit distances
////////////////////////////////////////////////////////////////////////////////
template<
  class _NodeData = nodes::TASMStringNodeData,
  class _Costs = nodes::StringCosts<_NodeData>
>
class StructureSearch {
// public type aliases
public:
  // kheap type aliases
  using Ranking = data_structures::KHeap<wrappers::NodeDistancePair<int>>;

// private type aliases
private:
  // vector type aliases
  using IntVector = std::vector<int>;
  using IntVectorConstIter = IntVector::const_iterator;

  // hash table type aliases
  using IntHashTable = std::unordered_map<int, int>;

  // ordered map type aliases
  using IntOrderedMap = std::map<int, int>;
  using FrequencyMap = std::unordered_map<unsigned long, unsigned long>;

  // multiset type aliases
  using IntMultiSet = std::multiset<int>;

  // unordered set type aliases
  using IntUnorderedSet = std::unordered_set<int>;

  // Matrix type aliases
  using DoubleArray2D = data_structures::Array2D<double>;

  // LabelHashMap type aliases
  using DataHashMap = data_structures::LabelHashMap<_NodeData>;

  // BTree type aliases
  template<class _Key, class _Value>
  using BTree = data_structures::BTree<_Key, _Value>;

  // stack type aliases
  using StackEntry = std::pair<int, IntMultiSet>;
  using Stack = std::vector<StackEntry>;

  // tree type aliases
  template<class _Type> using QueryTemplate = data_structures::SSTree<_Type>;
  template<class _Type> using DocumentTemplate = data_structures::TASMTree<_Type>;
  using Query = QueryTemplate<_NodeData>;
  using Document = DocumentTemplate<_NodeData>;

  // posting list aliases
  template<class _ListType>
  using PostingLists = data_structures::PostingListContainer<_ListType>;

  // index type aliases
  using UncommonLabelIndex = BTree<int, std::vector<int>>;
  using CommonLabelIndex = BTree<int, IntOrderedMap>;
  using NodeIndex = std::vector<wrappers::NodeIndexValue>;
  using StructureIndex = std::vector<wrappers::StructureIndexValue>;
  using PureStructureIndex = std::multimap<int, wrappers::PureStructureIndexValue>;
  using NodeIndexConstIter = NodeIndex::const_iterator;

  // tree edit distance type/algorithm aliases
  using EditDistance =
    tasm::TASMDynamic<_NodeData, _Costs, QueryTemplate, DocumentTemplate>;

// public member functions
public:
  StructureSearch() = delete;

  StructureSearch(
    const UncommonLabelIndex& uncommon_label_index,
    const CommonLabelIndex& common_label_index,
    const NodeIndex& node_index,
    const StructureIndex& structure_index,
    const PureStructureIndex& pure_structure_index);

  ~StructureSearch();

  //****************************************************************************
  // Computes the top-k ranking for a given query wrt. the document(s) specified
  // in the constructor and wrt. a given threshold m. The ranking then consists
  // of the k most similar subtrees of the document(s) wrt. the given query and
  // whose edit distance to the query is at most m.
  //
  // Params:  query   Reference query
  //          m       Maximum edit distance to be considered in the ranking
  //          k       Number of answers/elements in the ranking
  //
  // Returns: A ranking which contains the k most similar subtrees of the document
  //          with an edit distance <= m
  //****************************************************************************
  Ranking compute_topk(Query& query, int m, const int k);

  inline unsigned int verified_candidates() const;
  inline unsigned int duplicate_verifications() const;
  inline unsigned int potential_candidates() const;
  inline unsigned int candidates_filtered_by_depth_bound() const;
  inline unsigned int candidates_filtered_by_label_bound() const;
  inline const FrequencyMap& verifications_per_distance_lower_bound() const;

  inline void increment_verifications(unsigned long lower_bound);

// private/inner data types/classes
private:
  // helper to categorize data ids (label ids) of the given query into three
  // (multi)sets: common, uncommon and others/unknown data (label) ids
  struct CategorizedDataIds {
    IntMultiSet common;
    IntMultiSet uncommon;
    IntMultiSet others;

    CategorizedDataIds() { }

    CategorizedDataIds(CategorizedDataIds&& other)
        : common(std::move(other.common)),
          uncommon(std::move(other.uncommon)),
          others(std::move(other.others))
    { }
  };

// private members
private:
  // the data (label) index for data (labels) considered uncommon
  // referred to as 'Lu-Label Index' in the original paper (see Sec. 4.1)
  const UncommonLabelIndex& uncommon_label_index_;

  // the data (label) index for data (labels) considered common
  // referred to as 'Lc-Label Index' in the original paper (see Sec. 4.1)
  const CommonLabelIndex& common_label_index_;

  // the node index
  // referred to as 'Node Index' in the original paper (see Sec. 4.1)
  const NodeIndex& node_index_;

  // the structure index
  // referred to as 'Structure Index' or 'Structural Index' in the original paper
  // (see Sec. 4.1)
  const StructureIndex& structure_index_;

  // the pure structure index
  // referred to as 'Pure Structural Index' in the original paper (see Sec. 4.4)
  const PureStructureIndex& pure_structure_index_;

  // used to compute the tree edit distance for two given (sub)trees
  EditDistance ed_{};

  // since we may compute the tree edit distance for multiple subtrees of the
  // document, but always wrt. the same query, we store the key root nodes once
  // we have computed them
  IntVector query_kr_{};

  // we need to keep track of the subtrees we have already reconstructed because
  // we do not want to evaluate them twice
  IntUnorderedSet reconstructed_{};

  unsigned int verified_candidates_{};
  unsigned int duplicate_verifications_{};
  unsigned int potential_candidates_{};
  unsigned int candidates_filtered_by_depth_bound_{};
  unsigned int candidates_filtered_by_label_bound_{};
  FrequencyMap verifications_per_distance_lower_bound_{};

  std::map<Document, int> already_verified_{};

  //****************************************************************************
  // Finds all structure ids s.t. the corresponding subtrees contain at least
  // one data (label) in the set of common data (labels) that is also in the
  // query. Therefore, for each structure id [s], the multiset intersection of
  // the common data (labels) of the query and the common data (labels) of the
  // subtree rooted in [s] is computed, denoted ms_is_size(T[s]).
  // See 'GatherInformation' Sec. 4.2.1 in the original paper.
  //
  // Params:  common_data_ids_query   A set of data (label) ids which are in the
  //                                  query tree AND in the set of common data
  //                                  labels
  //
  // Returns: A hash table containing entries of the form
  //          (structure id, ms_is_size(T[s]))
  //****************************************************************************
  IntHashTable gather_information(const IntMultiSet& common_data_ids_query) const;

  //****************************************************************************
  // Finds the top-k subtrees among all those that share a data (label) with the
  // uncommon data (labels) in the query.
  // See Sec. 4.2.2 and Sec. 3.2 in the original paper, respectively.
  //
  // Params:  query                   Reference query
  //          m                       Maximum edit distance to be considered in
  //                                  the ranking
  //          k                       Number of answers/elements in the ranking
  //          uncommon_data_ids_query Uncommon data (labels) in the query
  //          h                       Hash table returned by gather_information
  //          ranking                 Top-k ranking to update
  //
  // Returns: The (possibly) adapted value of m
  //****************************************************************************
  const int find_uncommon_data_answers(
    Query& query,
    int m,
    const int k,
    const IntMultiSet& uncommon_data_ids_query,
    const IntHashTable& h,
    Ranking& ranking);

  //****************************************************************************
  // Finds the top-k subtrees among all those that do not share a data (label)
  // with the uncommon data (labels) in the query.
  // See Sec. 4.2.3 in the original paper.
  //
  // Params:  query                   Reference query
  //          m                       Maximum edit distance to be considered in
  //                                  the ranking
  //          k                       Number of answers/elements in the ranking
  //          h                       Hash table returned by gather_information
  //          ranking                 Top-k ranking to update
  //****************************************************************************
  const int find_common_data_answers(
    Query& query,
    int m,
    const int k,
    const IntMultiSet& uncommon_data_ids_query,
    const IntHashTable& h,
    Ranking& ranking);

  //****************************************************************************
  // Finds the top-k subtrees among all those that do not share any data (label)
  // with the query.
  // See Sec. 4.4 in the original paper.
  //
  // Params:  query                   Reference query
  //          m                       Maximum edit distance to be considered in
  //                                  the ranking
  //          k                       Number of answers/elements in the ranking
  //          ranking                 Top-k ranking to update
  //****************************************************************************
  void find_unknown_data_answers(
    Query& query,
    int m,
    const int k,
    Ranking& ranking);

  //****************************************************************************
  // Categorizes the data (label) ids of the query into one of three multisets.
  //
  // Params:  query   Query tree whose data (label) ids are categorized
  //
  // Returns: A wrapper around the three multisets where the respective data
  //          (label) id is in the multiset it belongs to
  //****************************************************************************
  CategorizedDataIds categorize_data_ids(Query& query) const;

  //****************************************************************************
  // Helper member function which intersects two given multisets of integers and
  // inserts a pair (structure id, intersection size) into the hash table h.
  //
  // Params:  ms_query      First multiset of integers, i.e., data (label) ids
  //          ms_subtree    Second multiset of integers, i.e., data (label) ids
  //          structure_id  Key to be inserted (structure id)
  //          h             Hash table to update
  //****************************************************************************
  void intersect_and_insert(
    const IntMultiSet& ms_query,
    const IntMultiSet& ms_subtree,
    const int structure_id,
    IntHashTable& h) const;

  //****************************************************************************
  // Computes two bounds (one wrt. the data (labels), one wrt. the depth) which
  // are used to avoid tree edit distance computations for trees not satisfying
  // these bounds. If the bounds are satisfied, the actual tree edit distance is
  // computed between the query and the given subtree. Afterwards a pair of
  // (tree edit distance, root node identifier) is inserted into the ranking.
  // In the last step, the threshold (m) is adjusted to the distance of max.
  // element of the ranking, iff the ranking is full.
  //
  // Params:  query       Reference query
  //          data_ids    Multiset representing the overlap size wrt. the
  //                      uncommon data (labels)
  //          preorder_id Preorder ID of the root of the evaluated subtree
  //          m           Threshold wrt. the tree edit distance
  //          h           Hash table containing the overlap sizes wrt. the common
  //                      data (labels) of query and the respective structures
  //          ranking     Top-k ranking to update
  //
  // Returns: The (possibly) adapted value of m
  //****************************************************************************
  const int filter_and_add(
    Query& query,
    const IntMultiSet& data_ids,
    const int preorder_id,
    const int m,
    const IntHashTable& h,
    Ranking& ranking);

  //****************************************************************************
  // Pushes all preorder IDs to the given stack which are in between the two
  // given preorder IDs begin and end (excluding both of them).
  //
  // Params:  begin   Wrt. this preorder ID the following preorder IDs are
  //                  constructed and pushed to the stack s
  //          end     Wrt. this preorder ID the preceeding preorder ID are
  //                  constructed and pushed to the stack s
  //          s       Stack the constructed intermediate preorder ID are pushed
  //                  onto
  //****************************************************************************
  void push_intermediate_nodes(
    const int begin,
    const int end,
    Stack& s) const;

  //****************************************************************************
  // Reconstructs the subtree rooted at the node pointed to by the given node
  // index iterator. This tree can in turn be used in the tree edit distance
  // computation. Therefore, this function reconstructs also the data (label) ids
  // s.t. these can be utilized by the tree edit distance function.
  //
  // Params:  node_info_root_cit  const_iterator (of the node_index) to the root
  //                              of the subtree to reconstruct
  //          data_hash_map       The bidirectional hash table of data (labels)
  //
  // Returns: The reconstructed tree (i.e., TASMTree<_NodeData>)
  //****************************************************************************
  Document reconstruct_subtree(
    const NodeIndexConstIter node_info_root_cit,
    DataHashMap& data_hash_map);

  //****************************************************************************
  // Computes the tree edit distance for the two given trees (the query and the
  // current subtree) using the Zhang & Shasha algorithm.
  //
  // Params:  query   First tree (the reference query)
  //          subtree Second tree (the current subtree of the document)
  //          ranking Top-k ranking (which may or may not be updated)
  //          m       A tree edit distance threshold. The ranking is not modified,
  //                  if m < 0. Otherwise, the ranking is updated if a better
  //                  subtree (wrt. the tree edit distance) is found during the
  //                  computation of the tree edit distance
  //
  //  Returns: The tree edit distance between query and subtree
  //****************************************************************************
  const int ted(Query& query, Document& subtree, Ranking& ranking, const int m);

  //****************************************************************************
  // Checks whether a given prefix candidate is a prefix of a given reference.
  //
  // Params:  reference  The preorder id of the reference node
  //          candidate  The preorder id of the prefix candidate node
  //
  // Returns: True if candidate is a prefix of reference, false otherwise
  //****************************************************************************
  inline const bool is_prefix(const int reference, const int candidate) const;

  //****************************************************************************
  // Checks whether a the subtree rooted at a given preorder id was already
  // reconstructed. Searches the reconstructed_ member variable.
  //
  // Params:  preorder_id The preorder id to be checked
  //
  // Returns: True if the the subtree rooted at preorder_id was already
  //          reconstructed (i.e., is in reconstructed_), false otherwise.
  //****************************************************************************
  inline const bool is_reconstructed(const int preorder_id);
};

}

#include "structure_search_impl.h"

#endif // STRUCTURE_SEARCH_STRUCTURE_SEARCH_H
