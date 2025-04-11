#ifndef PARSER_INDEX_EVENT_HANDLER_H
#define PARSER_INDEX_EVENT_HANDLER_H

#include <stack>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <map>

#include "hash_functors.h"
#include "abstract_event_handler.h"
#include "btree.h"
#include "node_index_value.h"
#include "pure_structure_index_value.h"
#include "structure_index_value.h"

namespace parser {

  namespace xml {

  /**
   * Generates the indices to be used in structure_search from an XML file.
   */
  template<class _NodeData = nodes::StringNodeData>
  class IndexEventHandler : public DefaultEventHandler {
  private:
    // inherited, pure virtual
    // see abstract_event_handler.h
    void process_data_buffer() override;

    // additional
    int adjust_top(std::vector<int>& s, const int offset);
    inline void insert_label_indices(const int data_id, const int preorder_id);
    void insert_uncommon_label_index(const int data_id, const int preorder_id);
    std::pair<int, std::vector<int>> get_structure_index(
      const _NodeData& data,
      const int subtree_size = 1);
    void insert_structure_index(
      const _NodeData& data,
      std::pair<int, std::vector<int>>& structure_id_pair,
      const int depth,
      const int preorder_id);
    void construct_common_label_index();
    void enqueue_descendants(
      std::queue<int>& q,
      const std::vector<int>& structure);
    void update_common_label_index(const int label_id, const int structure_id);
    const bool is_zero_equivalent(
        const std::vector<int>& structure1,
        const std::vector<int>& structure2);

  private:
    // label map
    const data_structures::LabelHashMap<_NodeData>& label_map_;

    // set of indices (within the label map) pointing to the common labels
    const std::unordered_set<int>& common_data_ids_;

    // uncommon label index
    data_structures::BTree<int, std::vector<int>> uncommon_label_index_;

    // common label index
    data_structures::BTree<int, std::map<int, int>> common_label_index_;

    // node index
    std::vector<wrappers::NodeIndexValue> node_index_;

    // structure index
    std::vector<wrappers::StructureIndexValue> structure_index_;

    // pure structure index
    std::multimap<int, wrappers::PureStructureIndexValue> pure_structure_index_;

    // to compute necessary values during scan (to avoid repetitive scans)
    std::vector<int> sizes_{};
    std::vector<int> depths_{};
    std::vector<std::pair<int, int>> structure_ids_{};
    std::vector<int> preorder_ids_{};
    std::vector<int> parent_ids_{};
    int preorder_id_{};
    int postorder_id_{};
    data_structures::LabelHashMap<std::vector<int>, common::hash_functors::IntVectorHash> structure_id_map_{};

  public:
    // inherited, pure virtual
    // see abstract_event_handler.h for documentation
    virtual void on_start_element(
      const XML_Char* name,
      const XML_Char** attributes) override;
    virtual void on_end_element(const XML_Char* name) override;
    virtual void on_textual_data(
      const XML_Char* data,
      const int length) override;
    virtual void on_end_of_file() override;

    virtual void on_start_element(const std::string& name) override;
    virtual void on_end_element(const std::string& name) override;
    virtual void on_attribute(
      const std::pair<std::string, std::string>& pair) override;
    virtual void on_textual_data(std::string& data) override;

  public:
    IndexEventHandler() = delete;
    IndexEventHandler(
      const data_structures::LabelHashMap<_NodeData>& label_map,
      const std::unordered_set<int>& common_data_ids);
    ~IndexEventHandler();

    const data_structures::BTree<int, std::vector<int>>& uncommon_label_index() const;
    const data_structures::BTree<int, std::map<int, int>>& common_label_index() const;
    const std::vector<wrappers::NodeIndexValue>& node_index() const;
    const std::vector<wrappers::StructureIndexValue>& structure_index() const;
    const std::multimap<int, wrappers::PureStructureIndexValue>& pure_structure_index() const;
  };

  template<class _NodeData>
  IndexEventHandler<_NodeData>::IndexEventHandler(
      const data_structures::LabelHashMap<_NodeData>& label_map,
      const std::unordered_set<int>& common_data_ids)
      : label_map_(label_map),
        common_data_ids_(common_data_ids),
        preorder_id_(1),
        postorder_id_(1) {

    // node index index is 1-based, thus first element is a dummy element
    node_index_.push_back(wrappers::NodeIndexValue{});

    // structure index is 1-based, thus first element is a dummy element
    structure_index_.push_back(wrappers::StructureIndexValue{});

    // if possible, reserve space for vectors (to hopefully improve performance)
    try {
      node_index_.reserve(label_map.size());
      structure_index_.reserve(label_map_.size() / 2);
    } catch (const std::bad_alloc& ba) {
      // nothing to do, it is just an optimization if reserve succeeds
      std::cerr << "Reserve failed" << std::endl;
    }
  }

  template<class _NodeData>
  IndexEventHandler<_NodeData>::~IndexEventHandler() {}

  template<class _NodeData>
  void IndexEventHandler<_NodeData>::process_data_buffer() {
    DefaultEventHandler::process_data_buffer();
  }

  template<class _NodeData>
  void IndexEventHandler<_NodeData>::on_start_element(
      const XML_Char* name,
      const XML_Char** attributes) {
    DefaultEventHandler::on_start_element(name, attributes);
  }

  template<class _NodeData>
  void IndexEventHandler<_NodeData>::on_end_element(
      const XML_Char* name) {
    DefaultEventHandler::on_end_element(name);
  }

  template<class _NodeData>
  void IndexEventHandler<_NodeData>::on_textual_data(
      const XML_Char* data,
      const int length) {
    DefaultEventHandler::on_textual_data(data, length);
  }

  template<class _NodeData>
  void IndexEventHandler<_NodeData>::on_end_of_file() {
    // set parent of root node to 0
    if (node_index_.size() > 1) {
      node_index_.at(1).parent_id_ = 0;
    }

    // construct common_label_index once all structures/structure_ids are known
    construct_common_label_index();
  }

  template<class _NodeData>
  void IndexEventHandler<_NodeData>::on_start_element(const std::string& name) {
    sizes_.push_back(1);

    depths_.push_back(0);

    parent_ids_.push_back(preorder_id_);
    preorder_ids_.push_back(preorder_id_++);

    node_index_.push_back(wrappers::NodeIndexValue{});
  }

  template<class _NodeData>
  void IndexEventHandler<_NodeData>::on_attribute(
      const std::pair<std::string, std::string>& pair) {
    adjust_top(sizes_, 2);
    //adjust_top(depths_, std::move(2));
    if (!depths_.empty()) {
      int top = depths_.back();
      depths_.pop_back();
      depths_.push_back(std::max(top, 2));
    }

    // fill indices ()
    _NodeData node_data_name(pair.first);
    _NodeData node_data_value(pair.second);

    const int data_id_name = label_map_.at(node_data_name);
    const int data_id_value = label_map_.at(node_data_value);

    // get structure id (value)
    std::pair<int, std::vector<int>> structure_id_pair_value
      = std::move(get_structure_index(node_data_value, 1));

    // push structure id (value); has to be done in between
    structure_ids_.push_back(std::move(
      std::make_pair(std::move(structure_id_pair_value.first), 1)
    ));

    // get structure id (name)
    std::pair<int, std::vector<int>> structure_id_pair_name
      = std::move(get_structure_index(node_data_name, 2));

    // node index (name)
    node_index_.push_back(std::move(wrappers::NodeIndexValue(
      2, 2, data_id_name, structure_id_pair_name.first, postorder_id_ + 1,
      parent_ids_.back()
    )));

    // label indices
    insert_label_indices(data_id_name, node_index_.size() - 1);

    // node index (value)
    node_index_.push_back(std::move(wrappers::NodeIndexValue(
      1, 1, data_id_value, structure_id_pair_value.first, postorder_id_,
      node_index_.size() - 1
    )));

    // label indices
    insert_label_indices(data_id_value, node_index_.size() - 1);

    // structure index (value)
    insert_structure_index(
      node_data_value, structure_id_pair_value, 1, preorder_id_ + 1);

    // structure index (name)
    insert_structure_index(
      node_data_name, structure_id_pair_name, 2, preorder_id_);

    // push structure id (name)
    structure_ids_.push_back(std::move(
      std::make_pair(structure_id_pair_name.first, 2)));

    // update postorder id
    postorder_id_ += 2;
    preorder_id_ += 2;
  }

  template<class _NodeData>
  void IndexEventHandler<_NodeData>::on_end_element(const std::string& name) {
    // subtree size
    int size = sizes_.back();
    sizes_.pop_back();

    if (!sizes_.empty()) {
      int top = sizes_.back();
      sizes_.pop_back();
      sizes_.push_back(top + size);
    }

    // subtree depth
    int depth = depths_.back() + 1;
    depths_.pop_back();

    if (!depths_.empty()) {
      int top = depths_.back();
      depths_.pop_back();
      depths_.push_back(std::max(top, depth));
    }

    // parent id
    parent_ids_.pop_back();

    // preorder id
    int preorder_id = preorder_ids_.back();
    preorder_ids_.pop_back();

    // fill indices
    _NodeData node_data(name);
    const int data_id = label_map_.at(node_data);

    // get structure id
    std::pair<int, std::vector<int>> structure_id_pair
      = std::move(get_structure_index(node_data, size));

    // label indices
    insert_label_indices(data_id, preorder_id);

    // structure index
    insert_structure_index(node_data, structure_id_pair, depth, preorder_id);

    // node index
    auto& node_index_entry = node_index_.at(preorder_id);
    node_index_entry.size_ = size;
    node_index_entry.depth_ = depth;
    node_index_entry.data_id_ = data_id;
    node_index_entry.structure_id_ = structure_id_pair.first;
    node_index_entry.postorder_id_ = postorder_id_;
    node_index_entry.parent_id_ = parent_ids_.back();

    // push structure id
    structure_ids_.push_back(std::move(
      std::make_pair(structure_id_pair.first, size)
    ));

    // update postorder id
    ++postorder_id_;
  }

  template<class _NodeData>
  void IndexEventHandler<_NodeData>::on_textual_data(std::string& data) {
    data.erase(std::remove(data.begin(), data.end(), '\r'), data.end());
    data.erase(std::remove(data.begin(), data.end(), '\n'), data.end());

    if (spaces_only(data)) {
      return;
    }

    adjust_top(sizes_, 1);
    //adjust_top(depths_, std::move(1));
    if (!depths_.empty()) {
      int top = depths_.back();
      depths_.pop_back();
      depths_.push_back(std::max(top, 1));
    }


    // fill indices
    _NodeData node_data(data);
    const int data_id = label_map_.at(node_data);

    // get structure id
    std::pair<int, std::vector<int>> structure_id_pair
      = std::move(get_structure_index(node_data));

    // structure index
    insert_structure_index(node_data, structure_id_pair, 1, preorder_id_);

    // node index
    node_index_.push_back(std::move(wrappers::NodeIndexValue(
      1, 1, data_id, structure_id_pair.first, postorder_id_, parent_ids_.back()
    )));

    // label indices
    insert_label_indices(data_id, node_index_.size() - 1);

    // push structure id
    structure_ids_.push_back(std::move(std::make_pair(structure_id_pair.first, 1)));

    // update postorder/preorder ids
    ++postorder_id_;
    ++preorder_id_;
  }

  template<class _NodeData>
  const data_structures::BTree<int, std::vector<int>>& IndexEventHandler<_NodeData>::uncommon_label_index() const
  {
    return uncommon_label_index_;
  }

  template<class _NodeData>
  const data_structures::BTree<int, std::map<int, int>>& IndexEventHandler<_NodeData>::common_label_index() const
  {
    return common_label_index_;
  }

  template<class _NodeData>
  const std::vector<wrappers::NodeIndexValue>& IndexEventHandler<_NodeData>::node_index() const
  {
    return node_index_;
  }

  template<class _NodeData>
  const std::vector<wrappers::StructureIndexValue>& IndexEventHandler<_NodeData>::structure_index() const {
    return structure_index_;
  }

  template<class _NodeData>
  const std::multimap<int, wrappers::PureStructureIndexValue>& IndexEventHandler<_NodeData>::pure_structure_index() const {
    return pure_structure_index_;
  }

  template<class _NodeData>
  int IndexEventHandler<_NodeData>::adjust_top(
      std::vector<int>& s,
      const int offset) {

    int top = s.back();
    s.pop_back();
    s.push_back(top + offset);
    return top;
  }

  template<class _NodeData>
  inline void IndexEventHandler<_NodeData>::insert_label_indices(
      const int data_id,
      const int preorder_id) {

    if (common_data_ids_.find(data_id) == common_data_ids_.cend()) {
      // insert into uncommon_label_index
      insert_uncommon_label_index(data_id, preorder_id);
    }
  }

  template<class _NodeData>
  void IndexEventHandler<_NodeData>::insert_uncommon_label_index(
      const int data_id,
      const int preorder_id) {

    auto search_result = uncommon_label_index_.search(data_id);

    // already present: append to list
    if (search_result != nullptr) {
      search_result->push_back(preorder_id);
      //uncommon_label_index_.insert(std::move(data), search_result.second.second);
      return;
    }

    // not found: create new entry
    uncommon_label_index_.insert(
      data_id,
      std::move(std::vector<int>{preorder_id})
    );
  }

  // TODO: rename to insert_structure_indices
  template<class _NodeData>
  void IndexEventHandler<_NodeData>::insert_structure_index(
      const _NodeData& data,
      std::pair<int, std::vector<int>>& structure_id_pair,
      const int depth,
      const int preorder_id) {

    const int structure_id = structure_id_pair.first;
    const auto& structure = structure_id_pair.second;
    const int structure_index_size = structure_index_.size() - 1;

    // already present: append preorder id to list at corresponding index entry
    if (structure_id <= structure_index_size) {
      // get corresponding index entry
      auto& entry = structure_index_.at(structure_id);

      // add preorder id to list
      entry.preorder_ids_.push_back(preorder_id);

      // pure structure index
      const int size = entry.labeled_nodes() + entry.other_nodes();

      auto size_cluster = pure_structure_index_.equal_range(size);
      for (auto it = size_cluster.first; it != size_cluster.second; ++it) {
        const auto& candidate_structure_id = *(it->second.begin());
        auto& candidate_structure = structure_id_map_.at(candidate_structure_id);

        if (is_zero_equivalent(candidate_structure, structure)) {
          it->second.insert(structure_id);
          break;
        }
      }

      return;
    }

    // otherwise: compute fields for new entry and create it
    int labeled_nodes = 0;
    int other_nodes = 0;
    for (auto it = std::next(structure.cbegin()); it != structure.cend(); ++it)
    {
      // has to present, thus no check
      const auto& child = structure_index_.at(*it);

      labeled_nodes += child.labeled_nodes();
      other_nodes += child.other_nodes();
    }

    if (common_data_ids_.find(label_map_.at(data)) != common_data_ids_.cend()) {
      ++labeled_nodes;
    } else {
      ++other_nodes;
    }

    structure_index_.push_back(
      std::move(wrappers::StructureIndexValue(
        labeled_nodes,
        other_nodes,
        depth,
        std::move(std::vector<int>{preorder_id})
      ))
    );

    // pure structure index
    const int size = labeled_nodes + other_nodes;

    auto size_cluster = pure_structure_index_.equal_range(size);
    for (auto it = size_cluster.first; it != size_cluster.second; ++it) {
      auto& candidate_structure_id = *(it->second.begin());
      auto& candidate_structure = structure_id_map_.at(candidate_structure_id);

      if (is_zero_equivalent(candidate_structure, structure)) {
        it->second.insert(structure_id);
        return;
      }
    }

    // only reached, if pure structure index does not have a tree of this size
    // thus, insert
    if (pure_structure_index_.empty()) {
      pure_structure_index_.insert(std::move(std::make_pair(
        size,
        std::move(std::unordered_set<int>{structure_id})
      )));
    } else {
      pure_structure_index_.insert(std::move(std::make_pair(
        size,
        std::move(std::unordered_set<int>{structure_id})
      )));
    }
  }

  template<class _NodeData>
  std::pair<int, std::vector<int>> IndexEventHandler<_NodeData>::get_structure_index(
      const _NodeData& data,
      const int subtree_size) {
    int data_id = label_map_.at(data);

    if (common_data_ids_.find(data_id) == common_data_ids_.cend()) {
      data_id = 0;
    }

    std::vector<int> structure{data_id};
    std::pair<int, int> popped_pair;
    int remaining_subtree_size = subtree_size;
    while (remaining_subtree_size > 1) {
      // only for non-leaf nodes

      popped_pair = structure_ids_.back();
      structure_ids_.pop_back();

      structure.push_back(popped_pair.first);
      remaining_subtree_size -= popped_pair.second;
    }

    int structure_id = structure_id_map_.insert(structure);

    return std::make_pair(structure_id, std::move(structure));
  }

  template<class _NodeData>
  void IndexEventHandler<_NodeData>::construct_common_label_index() {
    // create keys upfront (s.t. the searches afterwards do not result in nullptr
    // (and we have to do the insertion anyways at some point)
    for (const auto& common_data_id: common_data_ids_) {
      common_label_index_.insert(
        common_data_id,
        std::move(std::map<int, int>{}));
    }

    // traverse tree in preorder, hence start with max structure id
    int structure_id = 0;
    const int max_structure_id = structure_index_.size() - 1;
    for (structure_id = max_structure_id; structure_id > 0; --structure_id) {
      // get structure to structure_id; has to be present
      const std::vector<int>& structure = structure_id_map_.at(structure_id);
      const int data_id = structure.at(0);

      // if label id is not in the set of common data ids -> terminate iteration
      if (common_data_ids_.find(data_id) == common_data_ids_.cend()) {
        continue;
      }

      // otherwise search for entry of data (label) id
      // -> is present due to preceeding for loop filling the index with keys
      // and empty values
      auto entry = common_label_index_.search(data_id); // pointer, never nullptr

      // search value entries for structure id and add it if necessary
      (*entry)[structure_id] = 1;

      // create entries in the common label index for the current
      // label id and all its descendants

      // queue of structure ids to update
      std::queue<int> propagation_queue{};
      enqueue_descendants(propagation_queue, structure);

      while (!propagation_queue.empty()) {
        const int child_structure_id = propagation_queue.front();
        propagation_queue.pop();

        auto& child_structure = structure_id_map_.at(child_structure_id);
        const int child_label_id = child_structure.at(0); // present by definition

        update_common_label_index(child_label_id, structure_id);

        // enqueue children
        enqueue_descendants(propagation_queue, child_structure);
      }
    }
  }

  template<class _NodeData>
  void IndexEventHandler<_NodeData>::enqueue_descendants(
      std::queue<int>& q,
      const std::vector<int>& structure) {
    for ( auto child_it = std::next(structure.cbegin());
          child_it != structure.cend(); ++child_it) {
      q.push(*child_it);
    }
  }

  template<class _NodeData>
  void IndexEventHandler<_NodeData>::update_common_label_index(
      const int label_id,
      const int structure_id) {
    auto entry_ptr = common_label_index_.search(label_id); // pointer

    // only common label ids are updated, thus terminate if not found because
    // all common label ids are present in the common label index by definition
    if (entry_ptr == nullptr) {
      return;
    }

    // to avoid multiple dereferences (find/cend/insert)
    auto& entry = *entry_ptr;
    auto map_entry = entry.find(structure_id);

    if (map_entry == entry.cend()) {
      // not found -> insert (structure_id, 1) into map
      entry.insert(std::move(std::make_pair(structure_id, 1)));
      return;
    }

    ++map_entry->second;
  }

  template<class _NodeData>
  const bool IndexEventHandler<_NodeData>::is_zero_equivalent(
      const std::vector<int>& structure1,
      const std::vector<int>& structure2) {
    if (structure1.size() != structure2.size()) {
      return false;
    }

    for (size_t i = 1; i < structure1.size(); ++i) {
      // structure ids of i-th child are equal
      if (structure1.at(i) == structure2.at(i)) {
        continue;
      }

      const std::vector<int>& child1 = structure_id_map_.at(structure1.at(i));
      const std::vector<int>& child2 = structure_id_map_.at(structure2.at(i));

      if (!is_zero_equivalent(child1, child2)) {
        return false;
      }
    }

    return true;
  }

  } // namespace xml

} // namespace parser

#endif // PARSER_INDEX_EVENT_HANDLER_H
