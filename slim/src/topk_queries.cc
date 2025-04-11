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

#include "label_set_event_handler.h"
#include "label_set_index_event_handler.h"
#include "xml_parser.h"

#include "inverted_list_index.h"
#include "slim_inverted_list_index.h"
#include "slim_inverted_map_index.h"

#include "tasmted.h"

#include "label_overlap_filter.h"
#include "cone_filter.h"
#include "shin_cone_filter.h"
#include "updatable_shin_cone_filter.h"
#include "lower_bound_merge_filter.h"

#include "timing.h"

#include <sstream>
#include <iostream>
#include <random>

void usage();

template<
  class _NodeData,
  class _TEDAlgorithm,
  template<class...> class _IndexType>
std::vector<std::pair<data_structures::TASMTree<_NodeData>, std::vector<unsigned int>>> readqueries(
    const std::vector<std::string>& queries,
    const _IndexType<_NodeData, _TEDAlgorithm>& ili);

template<
  class _NodeData,
  class _TEDAlgorithm,
  template<class...> class _IndexType>
_IndexType<_NodeData, _TEDAlgorithm> buildindex(const std::string& document);

template<
  class _NodeData,
  class _TEDAlgorithm,
  template<class...> class _IndexType>
void updateindex(
  _IndexType<_NodeData, _TEDAlgorithm>& index,
  const int updates,
  const std::string& updatemode,
  typename _IndexType<_NodeData, _TEDAlgorithm>::LogType& logger);

template<
  class _NodeData,
  template<class...> class _Costs,
  template<class...> class _TEDAlgorithm,
  template<class...> class _IndexType,
  template<class...> class _FilterType>
typename _IndexType<_NodeData, _TEDAlgorithm<_NodeData, _Costs<_NodeData>>>::LogType runalgorithm(
    const std::string& document,
    const std::vector<std::string>& queries,
    const int k,
    const int updates,
    const std::string& updatemode,
    const bool indexonly = false);

int main(int argc, char** argv) {
  // Simple command-line parameter handling
  // TODO: handle command-line parameters properly (safer, easier, faster)

  if (argc < 7) {
    usage();
    return -1;
  }

  int argoffset = 0;
  bool opt_indexonly = false;
  if (std::string(argv[1]) == std::string("--indexonly")) {
    argoffset = 1;
    opt_indexonly = true;
  }

  // parse number of updates
  std::stringstream ss;
  int opt_updates = 0;
  ss << argv[2 + argoffset];
  ss >> opt_updates;

  // parse update mode
  std::string opt_updatemode(argv[3 + argoffset]);

  if (opt_updatemode != std::string("--rename")
      && opt_updatemode != std::string("--delete")
      && opt_updatemode != std::string("--insert")
      && opt_updatemode != std::string("--mixedstructural")
      && opt_updatemode != std::string("--mixedall")) {
    usage();
    return -1;
  }

  // parse k
  ss.clear();
  int opt_k = 3;
  ss << argv[4 + argoffset];
  ss >> opt_k;

  // parse document
  std::string opt_document(argv[5 + argoffset]);

  // parse queries
  std::vector<std::string> opt_queries{};
  for (int i = 6 + argoffset; i < argc; ++i) {
      opt_queries.push_back(argv[i]);
  }

  using nodes::StringNodeData;
  using nodes::StringCosts;
  using distance::TASMTED;
  using tree_index::InvertedListIndex;
  using tree_index::SlimInvertedListIndex;
  using tree_index::SlimInvertedMapIndex;
  using filter::LabelOverlapFilter;
  using filter::LowerBoundMergeFilter;
  using filter::ConeFilter;
  using filter::ShinConeFilter;
  using filter::UpdatableShinConeFilter;

  // parse algorithm
  std::string opt_algorithm = argv[1 + argoffset];

  // call algorithm
  if (opt_algorithm != std::string("--overlap")
      && opt_algorithm != std::string("--merge")
      && opt_algorithm != std::string("--cone")
      && opt_algorithm != std::string("--shincone")
      && opt_algorithm != std::string("--shincone-updatable")) {
    usage();
    return -1;
  }

  if (opt_algorithm == std::string("--overlap")) {
    runalgorithm<
        StringNodeData,
        StringCosts,
        TASMTED,
        InvertedListIndex,
        LabelOverlapFilter>(
      opt_document,
      opt_queries,
      opt_k,
      opt_updates,
      opt_updatemode,
      opt_indexonly).flush(std::cout);
  } else if (opt_algorithm == std::string("--merge")) {
    runalgorithm<
        StringNodeData,
        StringCosts,
        TASMTED,
        InvertedListIndex,
        LowerBoundMergeFilter>(
      opt_document,
      opt_queries,
      opt_k,
      opt_updates,
      opt_updatemode,
      opt_indexonly).flush(std::cout);
  } else if (opt_algorithm == std::string("--cone")) {
    runalgorithm<
        StringNodeData,
        StringCosts,
        TASMTED,
        InvertedListIndex,
        ConeFilter>(
      opt_document,
      opt_queries,
      opt_k,
      opt_updates,
      opt_updatemode,
      opt_indexonly).flush(std::cout);
  } else if (opt_algorithm == std::string("--shincone")) {
    runalgorithm<
        StringNodeData,
        StringCosts,
        TASMTED,
        SlimInvertedListIndex,
        UpdatableShinConeFilter>(
      opt_document,
      opt_queries,
      opt_k,
      opt_updates,
      opt_updatemode,
      opt_indexonly).flush(std::cout);
  } else if (opt_algorithm == std::string("--shincone-updatable")) {
    runalgorithm<
        StringNodeData,
        StringCosts,
        TASMTED,
        SlimInvertedMapIndex,
        UpdatableShinConeFilter>(
      opt_document,
      opt_queries,
      opt_k,
      opt_updates,
      opt_updatemode,
      opt_indexonly).flush(std::cout);
  }

  return 0;
}

void usage() {
  std::cerr << "USAGE: ./topk_queries [--indexonly] --ALGORITHM <UPDATECOUNT> --UPDATEOPS <K> <DOCUMENT> <QUERIES>\n\n";
  std::cerr << "ALGORITHM" << std::setw(10) << "";
  std::cerr << "Specify the algorithm to execute (--overlap, --merge, --cone, ";
  std::cerr << "--shincone, --shincone-updatable).\n";
  std::cerr << "UPDATEOPS" << std::setw(10) << "";
  std::cerr << "Specify the update operations to execute randomly (--rename, ";
  std::cerr << "--delete)." << std::endl;
}

template<
  class _NodeData,
  class _TEDAlgorithm,
  template<class...> class _IndexType>
_IndexType<_NodeData, _TEDAlgorithm> buildindex(
      const std::string& document,
      typename _IndexType<_NodeData, _TEDAlgorithm>::LogType& logger) {
    using parser::xml::LabelSetIndexEventHandler;
    using parser::xml::XMLParser;

    _IndexType<_NodeData, _TEDAlgorithm> index(logger);
    LabelSetIndexEventHandler<decltype(index)> label_set_index_handler{index};

    XMLParser::parse_file(document, &label_set_index_handler);

    logger.log(index, "\n");

    return index;
}

template<
  class _NodeData,
  class _TEDAlgorithm,
  template<class...> class _IndexType>
std::vector<std::pair<data_structures::TASMTree<_NodeData>, std::vector<unsigned int>>> readqueries(
        const std::vector<std::string>& queries,
        _IndexType<_NodeData, _TEDAlgorithm>& ili) {
    using data_structures::TASMTree;
    using parser::xml::XMLParser;
    using parser::xml::LabelSetEventHandler;

    std::vector<std::pair<TASMTree<_NodeData>, std::vector<unsigned int>>> qs;
    qs.reserve(queries.size());

    for (const std::string& query: queries) {
        TASMTree<_NodeData> query_tree{ili.token_map()};
        LabelSetEventHandler<_NodeData> label_set_handler(
          &query_tree,
          ili.token_map());

        XMLParser::parse_file(query, &label_set_handler);

        qs.push_back(std::make_pair(
          query_tree,
          label_set_handler.token_id_set()));
    }

    return qs;
}

template<
  class _NodeData,
  class _TEDAlgorithm,
  template<class...> class _IndexType>
void updateindex(
    _IndexType<_NodeData, _TEDAlgorithm>& index,
    const int updates,
    const std::string& updatemode,
    typename _IndexType<_NodeData, _TEDAlgorithm>::LogType& logger) {
  using common::WrappingTiming;
  using common::Interval;

  Interval interval;

  const auto nodeid_max = index.nodes();
  const auto nodeid_min = 1;

  const int fixedseed = 42;
  std::mt19937 mt;
  mt.seed(fixedseed);
  std::uniform_int_distribution<> distr(nodeid_min, nodeid_max);

  const auto& tokenmap = index.token_map();

  for (auto i = 0; i < updates; ++i) {
    if (updatemode == std::string("--rename")) {
      const auto subtreeid_toupdate = distr(mt);
      const auto subtreeid2 = distr(mt);

      //const auto& e1 = index.get_entry(subtreeid_toupdate);
      const auto& e2 = index.get_entry(subtreeid2);

      // const auto tokenid_old = index.get_token_id(e1);
      const auto tokenid_new = index.get_token_id(e2);

      //const _NodeData& token_old = tokenmap.at(tokenid_old);
      const _NodeData& token_new = tokenmap.at(tokenid_new);

      WrappingTiming::resetstart(interval);
      index.rename(subtreeid_toupdate, token_new);
      WrappingTiming::stopaddvalue(
        interval,
        index.statistics_.timingrename.value);
    } else {
      // if (updatemode == std::string("--delete"))
      auto subtreeid_todelete = distr(mt);

      // only inner or leaf node
      // while (index.get_size(index.get_entry(subtreeid_todelete)) == 1) {
      //   subtreeid_todelete = distr(mt);
      // }

      WrappingTiming::resetstart(interval);
      index.deletenode(subtreeid_todelete);

      WrappingTiming::stopaddvalue(
        interval,
        index.statistics_.timingdelete.value);
    }
  }

  logger.log(index, "\n");
}

template<
  class _NodeData,
  template<class...> class _Costs,
  template<class...> class _TEDAlgorithm,
  template<class...> class _IndexType,
  template<class...> class _FilterType>
typename _IndexType<_NodeData, _TEDAlgorithm<_NodeData, _Costs<_NodeData>>>::LogType runalgorithm(
    const std::string& document,
    const std::vector<std::string>& queries,
    const int k,
    const int updates,
    const std::string& updatemode,
    const bool indexonly) {
  using data_structures::TASMTree;
  using common::WrappingTiming;
  using common::Interval;

  typename _IndexType<_NodeData, _TEDAlgorithm<_NodeData, _Costs<_NodeData>>>::LogType logger;
  Interval interval;

  _TEDAlgorithm<_NodeData, _Costs<_NodeData>> edit_distance;

  WrappingTiming::resetstart(interval);
  auto ili =
    buildindex<_NodeData, decltype(edit_distance), _IndexType>(document, logger);
  WrappingTiming::stopsetvalue(interval, ili.statistics_.timing.value);

  // prepareindexupdates<_NodeData, decltype(edit_distance), _IndexType>(
  //   updates,
  //   updatemode,
  //   logger);


  updateindex(ili, updates, updatemode, logger);

  auto query_pairs = readqueries<_NodeData, decltype(edit_distance), _IndexType>(queries, ili);

  _FilterType<decltype(ili), decltype(edit_distance)> filter{
    logger,
    ili,
    edit_distance};

  for (unsigned int i = 0; i < query_pairs.size(); ++i) {
    auto& query_tree = query_pairs.at(i).first;
    auto& query_token_id_set = query_pairs.at(i).second;

    typename decltype(filter)::Ranking ranking(k);
    if (!indexonly) {
      WrappingTiming::resetstart(interval);
      ranking = filter.top_k(query_token_id_set, query_tree, k);
      WrappingTiming::stopsetvalue(
        interval,
        filter.global_statistics_.timings.filtering.value);
      filter.global_statistics_.rounds.set(
        filter.round_statistics_.rounds.size());
    }

    filter.global_statistics_.timings.total.set(
      filter.global_statistics_.timings.filtering.value + ili.statistics_.timing.value);

    std::cout << "{"
      << "\"paths\": {"
        << "\"query\": \"" << queries.at(i) << "\","
        << "\"document\": \"" << document << "\""
      << "},"
      << "\"ranking\": " << ranking << ","
      << "\"statistics\": {"
        << "\"index\": {" << ili.statistics_ << "},"
        << "\"filter\": {"
          << "\"rounds\": {" << filter.round_statistics_ << "},"
          << "\"global\": {" << filter.global_statistics_ << "}"
        << "}"
      << "}"
    << "}"
    << std::endl;
  }

  return logger;
}
