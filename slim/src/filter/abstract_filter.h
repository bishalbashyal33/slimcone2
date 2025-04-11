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

#ifndef FILTER_ABSTRACT_FILTER_H
#define FILTER_ABSTRACT_FILTER_H

#include "tasm_dynamic.h"
#include "tasm_tree.h"
#include "k_heap.h"

#include "statistics.h"
#include "log.h"
#include "timing.h"

#include <unordered_set>
#include <map>

namespace filter {

template<class _Index, class _TEDAlgorithm>
class AbstractFilter {
public: // member types
  // derived from index structure
  using NodeData = typename _Index::NodeData;
  using TokenIdType = typename _Index::TokenIdType;
  using SizeType = typename _Index::SizeType;
  using NodeIdType = typename _Index::NodeIdType;
  using TokenIdSet = typename _Index::TokenIdSet;
  using TokenFrequencyMap = typename _Index::TokenFrequencyMap;
  using List = typename _Index::List;
  template<class _Type>
  using POQueue = typename _TEDAlgorithm::template POQueue<_Type>;
  // shortcuts
  using RankingEntry = wrappers::NodeDistancePair<NodeIdType>;
  using Ranking = data_structures::KHeap<RankingEntry>;
  using Bucket = std::vector<NodeIdType>;
  using GlobalStatType = common::GlobalFilterStatistics<>;
  using RoundStatType = common::RoundFilterStatistics<>;
  using MultiRoundStatType = common::MultiRoundFilterStatistics<>;
  using LogType = common::Log<common::LogPrinter>;
  using IntervalType = common::Interval;

public: // member variables
  LogType& log_;
  GlobalStatType global_statistics_;
  MultiRoundStatType round_statistics_;
  RoundStatType current_round_statistics_;
  IntervalType interval_;

public: // member functions
  AbstractFilter() = delete;
  AbstractFilter(
    LogType& log,
    _Index& index,
    _TEDAlgorithm& edit_distance,
    const std::string& name);
  ~AbstractFilter();

  inline std::string name() const;

protected: // member variables
  _Index& index_;
  _TEDAlgorithm& edit_distance_;
  std::string name_{};
  std::map<POQueue<NodeData>, int> already_verified_{};
  SizeType query_size_{};
  std::unordered_map<TokenIdType, std::pair<unsigned int, unsigned int>> query_token_id_map_{};
  TokenIdSet shared_token_ids_{};
  TokenIdSet unshared_token_ids_{};
  std::unordered_set<NodeIdType> already_processed_{};

protected: // member functions
  inline void initialize(const TokenIdSet& query_token_id_set);
  inline int verify(POQueue<NodeData>& query, POQueue<NodeData>& subtree);
  inline bool is_final_ranking(
    const Ranking& ranking,
    const unsigned int lower_bound) const;
  void update_ranking(
    const unsigned int real_lower_bound,
    const NodeIdType subtree_id,
    POQueue<NodeData>& query_tree,
    Ranking& ranking);
  inline bool already_processed(const NodeIdType subtree_id) const;
  inline void track_as_processed(const NodeIdType subtree_id);

private: // member functions
  inline void extract_token_id_map(const TokenIdSet& token_id_set);
};

#include "abstract_filter_impl.h"

} // namespace filter

#endif // FILTER_ABSTRACT_FILTER_H
