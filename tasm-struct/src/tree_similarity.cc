#include <iostream>
#include <random>
#include <unordered_map>
#include <unordered_set>
#include <fstream>
#include <exception>
#include <random>
#include <iomanip>
#include <sstream>
#include <vector>

#include "tree_similarity.h"

// TODO: make command-line parameter(s) - getopt
// robustness parameter
const int number_of_runs = 1;

void print_usage();
void print_tree(nodes::Node<nodes::StringNodeData>* node, int level);

void run_zhang_shasha(char* tree_string_1, char* tree_string_2);
void run_tasm_postorder(
  char* document_path,
  std::vector<char*> query_paths,
  const int k = 0);
void run_tasm_postorder_in_memory(
  char* document_path,
  std::vector<char*> query_paths,
  const int k = 0);
void run_tasm_dynamic(
  char* document_path,
  std::vector<char*> query_path,
  const int k = 0);
void run_structure_search(
  char* document_path,
  std::vector<char*> query_paths,
  const int k = 0,
  int m = -1,
  int size_lc = -1,
  bool indexonly = false);

int main (int argc, char* argv[]) {
  // Simple command-line parameter handling
  // TODO: handle command-line parameters using getopt (safer, easier, faster)

  //if(argc != 3 && argc != 4 && argc != 5 && argc != 6) {
  if (argc < 5) {
    print_usage();
    return 0;
  }

  int argoffset = 0;
  bool indexonly = false;
  if (std::string(argv[1]) == std::string("--indexonly")) {
    argoffset = 1;
    indexonly = true;
  }

  if ((argc >= 5) && (argv[1 + argoffset] == std::string("--tasm-postorder"))) {
    std::stringstream k_ss;
    k_ss << argv[2 + argoffset];
    int k = 0;
    k_ss >> k;

    std::vector<char*> query_paths;
    for (int i = 4  + argoffset; i < argc; ++i) {
      query_paths.push_back(argv[i]);
    }

    run_tasm_postorder(argv[3 + argoffset], query_paths, k);
    return 0;
  } else if ((argc >= 5) && (argv[1 + argoffset] == std::string("--tasm-postorder-im"))) {
    std::stringstream k_ss;
    k_ss << argv[2 + argoffset];
    int k = 0;
    k_ss >> k;

    std::vector<char*> query_paths;
    for (int i = 4 + argoffset; i < argc; ++i) {
      query_paths.push_back(argv[i]);
    }

    run_tasm_postorder_in_memory(argv[3 + argoffset], query_paths, k);
    return 0;
  } else if ((argc >= 5) && (argv[1 + argoffset] == std::string("--tasm-dynamic"))) {
    std::stringstream k_ss;
    k_ss << argv[2 + argoffset];
    int k = 0;
    k_ss >> k;

    std::vector<char*> query_paths;
    for (int i = 4 + argoffset; i < argc; ++i) {
      query_paths.push_back(argv[i]);
    }

    run_tasm_dynamic(argv[3 + argoffset], query_paths, k);
    return 0;
  } else if ((argc >= 5) && argv[1 + argoffset] == std::string("--structure-search")) {
    std::stringstream ss;
    int k = 0, m = -1, size_lc = -1;

    ss << argv[2 + argoffset];
    ss >> k;

    int start_index = 4;
    int offset = 0;

    // if edit bound is given, use it
    if (argc >= 7) {
      ss << argv[3 + argoffset];
      ss >> m;
      ++offset;
    }

    // if size of Lc is given, use it
    if (argc >= 8) {
      ss << argv[4 + argoffset];
      ss >> size_lc;
      ++offset;
    }

    std::vector<char*> query_paths;

    for (int i = start_index + argoffset + offset; i < argc; ++i) {
      query_paths.push_back(argv[i]);
    }

    run_structure_search(argv[start_index - 1  + argoffset], query_paths, k, m, size_lc, indexonly);
    return 0;
  }

  print_usage();
  return 0;
}

void print_usage() {
  std::cout << "Usage: ./tree_similarity [--indexonly] [document-tree | document-xml] "
    << "[[query-tree-n | query-xml-n]]\n\n";

  std::cout << "  --tasm-postorder k" << std::setw(9) << "";
  std::cout << "Execute the TASMPostorder (memory efficient) algorithm to "
    << "compute the k most similar subtrees in xml-file-2, with respect to "
    << "xml-file-1.\n";

    std::cout << "  --tasm-postorder-im k" << std::setw(6) << "";
  std::cout << "Execute the TASMPostorder (in memory) algorithm to "
    << "compute the k most similar subtrees in xml-file-2, with respect to "
    << "xml-file-1.\n";

  std::cout << "  --tasm-dynamic k" << std::setw(11) << "";
  std::cout << "Execute the TASMDynamic algorithm to compute the k most similar "
    << "subtrees in xml-file-2, with respect to xml-file-1.\n";

  std::cout << "  --structure-search k" << std::setw(3) << "" << " m n";
  std::cout << "Execute the StructureSearch algorithm to compute the k most "
    << "similar subtrees in xml-file-2, with respect to xml-file-1 and \n";
  std::cout << std::setw(29) << "" << "a tree edit distance threshold m. During "
    << "execution, the n most common textual values are included into the "
    << "common label set." << std::endl;
}

void run_zhang_shasha(char* tree_string_1, char* tree_string_2) {
  // TODO replace hashtable with a custom node class that sup. strings as labels
  parser::LabelIDMap hashtable_label_to_id;
  common::IDLabelMap hashtable_id_to_label;
  int node_id_counter = 1;
  nodes::Node<nodes::StringNodeData>* tree1 = parser::create_tree_from_string(
    tree_string_1, hashtable_label_to_id, node_id_counter
  );
  nodes::Node<nodes::StringNodeData>* tree2 = parser::create_tree_from_string(
    tree_string_2, hashtable_label_to_id, node_id_counter
  );

  for ( auto it = hashtable_label_to_id.begin();
        it != hashtable_label_to_id.end(); ++it )
  {
    hashtable_id_to_label.emplace(it->second, it->first);
  }

  // Zhang and Shasha cost = 1 (insert), 1 (delete), 1 (rename)
  // compute distance using basic nodes and basic cost model
  // no need to generate basic cost model since it is set as default template
  // parameter
  std::cout
    << "Distance (string-labeled tree, string-labeled cost model, Zhang Shasha):\t"
    << zhang_shasha::compute_zhang_shasha<nodes::StringNodeData, nodes::StringCosts<nodes::StringNodeData>>(tree1, tree2)
    << std::endl;

  delete tree1;
  delete tree2;
}

void run_tasm_postorder(
    char* document_path,
    std::vector<char*> query_paths,
    const int k) {
  using common::WrappingTiming;
  using common::Interval;

  Interval interval;

  std::cerr << "TASM Postorder (memory efficient)..." << std::endl;

  data_structures::LabelHashMap<nodes::TASMStringNodeData> label_map;

  std::cerr << "Loading document from '" << document_path << "'\n";

  for (const auto& query_path: query_paths) {
    std::cerr << "\nLoading query from '" << query_path << "'\n";

    data_structures::TASMTree<nodes::TASMStringNodeData> query(label_map);
    parser::xml::PostorderQueueEventHandler<nodes::TASMStringNodeData> postorder_queue_handler(&query);
    parser::xml::XMLParser::parse_file(std::string(query_path),
      &postorder_queue_handler
    );

    std::cerr << "Query with " << query.node_count() << " nodes loaded\n";

    interfaces::tasm::PostorderSourceInterface<nodes::TASMStringNodeData>* postorder_source
      = new parser::xml::XMLPostorderSource<nodes::TASMStringNodeData>(std::string(document_path));

    tasm::TASMPostorder<nodes::TASMStringNodeData, nodes::TASMStringCosts<nodes::TASMStringNodeData>> tasm_instance(
      query,
      postorder_source);

    double time_querying = 0;
    WrappingTiming::resetstart(interval);
    auto ranking = tasm_instance.compute_tasm(k);
    WrappingTiming::stopsetvalue(interval, time_querying);
    const unsigned int verificationstotal = tasm_instance.verificationstotal();
    //const unsigned int verificationsduplicates = tasm_instance.verificationsduplicates();

    std::cout << "{"
      << "\"paths\": {"
        << "\"document\": \"" << document_path << "\","
        << "\"query\": \"" << query_path << "\""
      << "},"
      << "\"ranking\": " << ranking << ","
      << "\"statistics\": {"
        << "\"filter\": {"
          << "\"global\": {"
            << "\"timingfiltering\": \"" << time_querying << "\","
            << "\"verificationstotal\": \"" << verificationstotal << "\""
            //<< "\"verificationsduplicates\":\"" << verificationsduplicates << "\""
          << "}"
        << "}"
      << "}"
    << "}" << std::endl;

    /*
    std::cout << "\nQuery load time (" << query_path << "): " << time_load_query
      << "ms\n";
    std::cout << "querying time (" << query_path << ", " << document_path
      << "): " << time_querying << "ms\n";
    std::cout << "total verified candidates (" << query_path << ", " << document_path
      << "): 0\n";
    std::cout << "ranking (" << query_path << ", " << document_path << "): "
      << ranking << std::endl;
    */

    delete postorder_source;
  }
}

void run_tasm_postorder_in_memory(
    char* document_path,
    std::vector<char*> query_paths,
    const int k) {
  using common::WrappingTiming;
  using common::Interval;

  Interval interval;

  std::cout << "TASM Postorder In Memory ..." << std::endl;

  data_structures::LabelHashMap<nodes::TASMStringNodeData> label_map;

  std::cout << "Loading document from '" << document_path << "'\n";

  double time_indexing = 0;
  WrappingTiming::resetstart(interval);
  data_structures::PostorderQueue<nodes::TASMStringNodeData> document;
  parser::xml::PostorderQueueEventHandler<nodes::TASMStringNodeData> postorder_queue_handler2(&document);
  parser::xml::XMLParser::parse_file(std::string(document_path),
    &postorder_queue_handler2
  );
  WrappingTiming::stopsetvalue(interval, time_indexing);

  std::cout << "Document with " << document.node_count() << " nodes loaded\n";

  for (const auto& query_path: query_paths) {
    document.reset(); // i.e., make postorder queue full again
    std::cout << "\nLoading query from '" << query_path << "'\n";

    data_structures::TASMTree<nodes::TASMStringNodeData> query(label_map);
    parser::xml::PostorderQueueEventHandler<nodes::TASMStringNodeData> postorder_queue_handler(&query);
    parser::xml::XMLParser::parse_file(std::string(query_path),
      &postorder_queue_handler
    );

    std::cout << "Query with " << query.node_count() << " nodes loaded\n";

    tasm::TASMPostorder<nodes::TASMStringNodeData, nodes::TASMStringCosts<nodes::TASMStringNodeData>> tasm_instance(
      query,
      &document);

    double time_querying = 0;
    WrappingTiming::resetstart(interval);
    auto ranking = tasm_instance.compute_tasm(k);
    WrappingTiming::stopsetvalue(interval, time_querying);

    std::cout << "\nIndexing time: " << time_indexing << "ms\n";
    std::cout << "querying time (" << query_path << ", " << document_path
        << "): " << time_querying << "ms\n";
    std::cout << "ranking (" << query_path << ", " << document_path << "): "
      << ranking << std::endl;
  }
}

void run_tasm_dynamic(
    char* document_path,
    std::vector<char*> query_paths,
    const int k) {
  using common::WrappingTiming;
  using common::Interval;

  Interval interval;

  std::cout << "TASM Dynamic\n";

  data_structures::LabelHashMap<nodes::TASMStringNodeData> label_map;

  std::cout << "Loading document from '" << document_path << "'\n";

  double time_indexing = 0;
  WrappingTiming::resetstart(interval);
  data_structures::TASMTree<nodes::TASMStringNodeData> document(label_map);
  parser::xml::PostorderQueueEventHandler<nodes::TASMStringNodeData> postorder_queue_handler2(&document);
  parser::xml::XMLParser::parse_file(std::string(document_path),
    &postorder_queue_handler2
  );
  WrappingTiming::stopsetvalue(interval, time_indexing);

  std::cout << "Document with " << document.node_count() << " nodes loaded\n";

  for (const auto& query_path: query_paths) {
    std::cout << "\nLoading query from '" << query_path << "'\n";

    data_structures::TASMTree<nodes::TASMStringNodeData> query(label_map);
    parser::xml::PostorderQueueEventHandler<nodes::TASMStringNodeData> postorder_queue_handler(&query);
    parser::xml::XMLParser::parse_file(std::string(query_path),
      &postorder_queue_handler
    );

    std::cout << "Query with " << query.node_count() << " nodes loaded\n";

    tasm::TASMDynamic<nodes::TASMStringNodeData, nodes::TASMStringCosts<nodes::TASMStringNodeData>, data_structures::TASMTree, data_structures::TASMTree> tasm_instance;

    double time_querying = 0;
    WrappingTiming::resetstart(interval);
    auto ranking = tasm_instance.compute_tasm(query, document, k);
    WrappingTiming::stopsetvalue(interval, time_querying);

    std::cout << "\nIndexing time: " << time_indexing << "ms\n";
    std::cout << "querying time (" << query_path << ", " << document_path
      << "): " << time_querying << "ms\n";
    std::cout << "ranking (" << query_path << ", " << document_path << "): "
      << ranking << std::endl;
  }
}

void run_structure_search(
    char* document_path,
    std::vector<char*> query_paths,
    const int k,
    int m,
    int size_lc,
    bool indexonly) {
  using common::WrappingTiming;
  using common::Interval;

  Interval interval;

  if (size_lc == -1) {
    size_lc = 1000;
  }

  std::cerr << "k = " << k << ", m = " << (m == -1 ? "|Q|" : std::to_string(m)) << ", |Lc| = " << size_lc << "\n";
  std::cerr << "Loading document from '" << document_path << "'\n";

  std::unordered_set<int> common_data_ids;
  data_structures::LabelHashMap<nodes::TASMStringNodeData> label_map;

  double time_indexing = 0;
  WrappingTiming::resetstart(interval);
  parser::xml::CommonLabelEventHandler<nodes::TASMStringNodeData> common_label_event_handler(
    label_map,
    common_data_ids,
    size_lc);
  parser::xml::XMLParser::parse_file(
    std::string(document_path),
    &common_label_event_handler);

  parser::xml::IndexEventHandler<nodes::TASMStringNodeData> index_event_handler(
    label_map,
    common_data_ids);
  parser::xml::XMLParser::parse_file(std::string(document_path),
    &index_event_handler);
  WrappingTiming::stopsetvalue(interval, time_indexing);

  auto& uncommon_label_index = index_event_handler.uncommon_label_index();
  auto& node_index = index_event_handler.node_index();
  auto& common_label_index = index_event_handler.common_label_index();
  auto& structure_index = index_event_handler.structure_index();
  auto& pure_structure_index = index_event_handler.pure_structure_index();

  std::cerr << "Document with " << node_index.size() << " nodes indexed\n";

  std::cerr << "Lu-Label-Index: size() = "
    << uncommon_label_index.size() << "\n";
  std::cerr << "Node-Index: size() = "
    << node_index.size() << "\n";
  std::cerr << "Lc-Label-Index: size() = "
    << common_label_index.size() << "\n";
  std::cerr << "Structure-Index: size() = "
    << structure_index.size() << "\n";
  std::cerr << "Pure-Structure-Index: size() = "
    << pure_structure_index.size() << "\n";

  structure_search::StructureSearch<> ss(
    uncommon_label_index,
    common_label_index,
    node_index,
    structure_index,
    pure_structure_index);

  for (const auto& query_path: query_paths) {
    std::cerr << "\nLoading query from '" << query_path << "'\n";

    data_structures::SSTree<nodes::TASMStringNodeData> query(label_map);
    parser::xml::PostorderQueueEventHandler<nodes::TASMStringNodeData> postorder_queue_handler(&query);
    parser::xml::XMLParser::parse_file(std::string(query_path),
      &postorder_queue_handler
    );

    //**********************************************
    // IMPORTANT for StructureSearch to work correctly
    query.set_depth(postorder_queue_handler.depth());

    //std::cout << "Query of depth " << query.depth() << " and size " << query.node_count() << "\n";

    // if no edit bound given, use the query size as edit bound (cf. experiments
    // section of original paper, https://doi.org/10.1145/2463676.2463716)
    if (m == -1) {
      m = query.node_count();
    }

    double time_querying = 0;
    typename decltype(ss)::Ranking ranking(k);
    if (!indexonly) {
      WrappingTiming::resetstart(interval);
      ranking = ss.compute_topk(query, m, k);
      WrappingTiming::stopsetvalue(interval, time_querying);
    }

    std::cerr << "Query with " << query.node_count() << " nodes loaded (m = "
      << m << ")\n";

    const unsigned int verificationstotal = ss.verified_candidates();
    const unsigned int verificationsduplicates = ss.duplicate_verifications();

    std::cout << "{";
    std::cout << "\"paths\": {";
    std::cout << "\"document\": \"" << document_path << "\",";
    std::cout << "\"query\": \"" << query_path << "\"";
    std::cout << "},";
    std::cout << "\"ranking\": " << ranking << ",";
    std::cout << "\"statistics\": {";
    std::cout << "\"filter\": {";
    std::cout << "\"global\": {";
    std::cout << "\"timingfiltering\": \"" << time_querying << "\",";
    std::cout << "\"timingtotal\": \"" << (time_querying + time_indexing) << "\",";
    std::cout << "\"verificationstotal\": \"" << verificationstotal << "\",";
    std::cout << "\"verificationsduplicates\": \"" << verificationsduplicates << "\"";
    std::cout << "}";
    std::cout << "},";
    std::cout <<  "\"index\": {";
    std::cout << "\"timingindexing\": \"" << time_indexing << "\"";
    std::cout << "}";
    std::cout << "}";
    std::cout << "}" << std::endl;

    /*
    std::cout << "indexing time (" << query_path << ", " << document_path
        << "): " << time_indexing << "ms" << std::endl;
    std::cout << "Query load time (" << query_path << "): " << time_load_query
      << "ms\n";
    std::cout << "querying time (" << query_path << ", " << document_path
      << "): " << time_querying << "ms\n";
    std::cout << "ranking (" << query_path << ", " << document_path << "): "
      << ranking << std::endl;
    std::cout << "total verified candidates (" << query_path << ", "
      << document_path << "): " << ss.verified_candidates()  << std::endl;
    std::cout << "Candidates before bounds: " << ss.potential_candidates()
      << std::endl;
    std::cout << "Filtered by bounds: " << ss.candidates_filtered_by_depth_bound()
      << " (depth) and " << ss.candidates_filtered_by_label_bound()
      << " (label overlap) " << std::endl;
    std::cout << "verifications per distance lower bound (" << query_path << ", "
          << document_path << "): {";
      const auto& verifications_per_distance_lower_bound =
          ss.verifications_per_distance_lower_bound();
      if (!verifications_per_distance_lower_bound.empty()) {
          auto it = verifications_per_distance_lower_bound.begin();
          std::cout << it->first << ":" << it->second;

          ++it;
          for (; it != verifications_per_distance_lower_bound.end(); ++it) {
              std::cout << ", " << it->first << ":" << it->second;
          }
      }
      std::cout << "}" << std::endl;
      */
  }
}

void print_tree(nodes::Node<nodes::StringNodeData>* node, int level) {
  std::cout << std::setw(level * 2) << "";
  std::cout << "\"" << node->get_data()->get_label() << "\"" << std::endl;

  for (nodes::Node<nodes::StringNodeData>*& child: node->get_children()) {
    print_tree(child, level + 1);
  }
}
