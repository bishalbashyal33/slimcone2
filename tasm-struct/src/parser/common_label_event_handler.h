#ifndef PARSER_COMMON_LABEL_EVENT_HANDLER_H
#define PARSER_COMMON_LABEL_EVENT_HANDLER_H

#include <unordered_map>
#include <unordered_set>

#include "hash_functors.h"
#include "default_event_handler.h"
#include "label_hash_map.h"

namespace parser {

  namespace xml {

  /**
   * Generates a common label set from an XML file.
   * Uses the unit cost model, i.e., delete/insert/rename costs 1.
   */
  template<
    class _NodeData = nodes::StringNodeData,
    class _NodeDataHash = common::hash_functors::StringNodeDataHash,
    template<class...> class _MapContainer = std::unordered_map>
  class CommonLabelEventHandler : public DefaultEventHandler {
  public:
    // the label map to be constructed in the process (contains all labels)
    data_structures::LabelHashMap<_NodeData, _NodeDataHash, _MapContainer>& label_map_;
    // the set of data ids (label ids) which are considered common
    std::unordered_set<int>& common_data_ids_;
    //the number of (textual) data values considered in the common set
    const size_t number_of_data_values_;
    // maps data id to the number of occurences in the parsed XML file
    std::unordered_map<int, int> common_data_values_{};

  private:
    // inherited, pure virtual
    // see abstract_event_handler.h
    virtual void process_data_buffer() override;

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
    CommonLabelEventHandler() = delete;
    CommonLabelEventHandler(
      data_structures::LabelHashMap<_NodeData, _NodeDataHash, _MapContainer>& label_map,
      std::unordered_set<int>& common_data_ids,
      const size_t number_of_data_values = 0);
    ~CommonLabelEventHandler();
  };

  template<class _NodeData, class _NodeDataHash, template<class...> class _MapContainer>
  CommonLabelEventHandler<_NodeData, _NodeDataHash, _MapContainer>::CommonLabelEventHandler(
    data_structures::LabelHashMap<_NodeData,
    _NodeDataHash,
    _MapContainer>& label_map,
    std::unordered_set<int>& common_data_ids,
    const size_t number_of_data_values)
    : label_map_(label_map),
      common_data_ids_(common_data_ids),
      number_of_data_values_(number_of_data_values) {}

  template<class _NodeData, class _NodeDataHash, template<class...> class _MapContainer>
  CommonLabelEventHandler<_NodeData, _NodeDataHash, _MapContainer>::~CommonLabelEventHandler()
  {}

  template<class _NodeData, class _NodeDataHash, template<class...> class _MapContainer>
  void CommonLabelEventHandler<_NodeData, _NodeDataHash, _MapContainer>::process_data_buffer()
  {
    DefaultEventHandler::process_data_buffer();
  }

  template<class _NodeData, class _NodeDataHash, template<class...> class _MapContainer>
  void CommonLabelEventHandler<_NodeData, _NodeDataHash, _MapContainer>::on_start_element(
      const XML_Char* name,
      const XML_Char** attributes) {
    DefaultEventHandler::on_start_element(name, attributes);
  }

  template<class _NodeData, class _NodeDataHash, template<class...> class _MapContainer>
  void CommonLabelEventHandler<_NodeData, _NodeDataHash, _MapContainer>::on_end_element(
      const XML_Char* name) {
    DefaultEventHandler::on_end_element(name);
  }

  template<class _NodeData, class _NodeDataHash, template<class...> class _MapContainer>
  void CommonLabelEventHandler<_NodeData, _NodeDataHash, _MapContainer>::on_textual_data(
      const XML_Char* data,
      const int length) {
    DefaultEventHandler::on_textual_data(data, length);
  }

  template<class _NodeData, class _NodeDataHash, template<class...> class _MapContainer>
  void CommonLabelEventHandler<_NodeData, _NodeDataHash, _MapContainer>::on_end_of_file()
  {
    if (number_of_data_values_ > 0) {
      std::set<std::pair<int, int>, std::greater<std::pair<int, int>>> best;
      auto it = common_data_values_.cbegin();
      for (; it != common_data_values_.cend(); ++it) {
        if (best.size() < number_of_data_values_) {
          best.insert(std::make_pair(it->second, it->first));
          continue;
        } else {
          if (it->second <= best.rbegin()->first) {
            continue;
          } else {
            best.erase(std::prev(best.cend()));
            best.insert(std::make_pair(it->second, it->first));
          }
        }
      }

      for (auto it = best.cbegin(); it != best.cend(); ++it) {
        common_data_ids_.insert(it->second);
      }
    }
  }

  template<class _NodeData, class _NodeDataHash, template<class...> class _MapContainer>
  void CommonLabelEventHandler<_NodeData, _NodeDataHash, _MapContainer>::on_start_element(
      const std::string& name) {}

  template<class _NodeData, class _NodeDataHash, template<class...> class _MapContainer>
  void CommonLabelEventHandler<_NodeData, _NodeDataHash, _MapContainer>::on_end_element(
      const std::string& name) {
    common_data_ids_.insert(label_map_.insert(_NodeData(name)));
  }

  template<class _NodeData, class _NodeDataHash, template<class...> class _MapContainer>
  void CommonLabelEventHandler<_NodeData, _NodeDataHash, _MapContainer>::on_attribute(
      const std::pair<std::string, std::string>& pair) {
    common_data_ids_.insert(label_map_.insert(_NodeData(pair.first)));
    common_data_values_[label_map_.insert(_NodeData(pair.second))]++;
  }

  template<class _NodeData, class _NodeDataHash, template<class...> class _MapContainer>
  void CommonLabelEventHandler<_NodeData, _NodeDataHash, _MapContainer>::on_textual_data(
      std::string& data) {
    data.erase(std::remove(data.begin(), data.end(), '\r'), data.end());
    data.erase(std::remove(data.begin(), data.end(), '\n'), data.end());

    if (spaces_only(data)) {
      return;
    }

    common_data_values_[label_map_.insert(_NodeData(data))]++;
  }

  } // namespace xml

} // namespace parser

#endif // PARSER_COMMON_LABEL_EVENT_HANDLER_H
