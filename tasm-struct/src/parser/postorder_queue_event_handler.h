#ifndef PARSER_POSTORDER_QUEUE_EVENT_HANDLER_H
#define PARSER_POSTORDER_QUEUE_EVENT_HANDLER_H

#include <vector>

#include "default_event_handler.h"
#include "postorder_queue_interface.h"
#include "xml_parser.h"

namespace parser {

  namespace xml {

  /**
   * Generates a postorder queue from an XML file.
   * Uses the unit cost model, i.e., delete/insert/rename costs 1.
   */
  template<class _NodeData = nodes::TASMStringNodeData>
  class PostorderQueueEventHandler : public DefaultEventHandler {
  private:
    std::vector<int> sizes_{}; // used as stack
    std::vector<int> depths_{}; // used as stack
    int depth_{}; // depth at the end
    int postorder_id_{};
    interfaces::tasm::PostorderQueueInterface<_NodeData>* postorder_queue_{};

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

    virtual void on_start_element(const std::string& name) override;
    virtual void on_end_element(const std::string& name) override;
    virtual void on_attribute(
      const std::pair<std::string, std::string>& pair) override;
    virtual void on_textual_data(std::string& data) override;

  public:
    PostorderQueueEventHandler();
    PostorderQueueEventHandler(
      interfaces::tasm::PostorderQueueInterface<_NodeData>* postorder_queue);
    virtual ~PostorderQueueEventHandler();

    void set_postorder_queue(
      interfaces::tasm::PostorderQueueInterface<_NodeData>* postorder_queue);

    // to override if one wants a different costs model than unit cost
    // e.g., subtree weighting
    virtual const double costs() const;

    const int depth() const;

    static void parse_file(const std::string& file_path,
      interfaces::tasm::PostorderQueueInterface<_NodeData>* postorder_queue);
  };

  template<class _NodeData>
  PostorderQueueEventHandler<_NodeData>::PostorderQueueEventHandler()
    : postorder_id_(1) {}

  template<class _NodeData>
  PostorderQueueEventHandler<_NodeData>::PostorderQueueEventHandler(
      interfaces::tasm::PostorderQueueInterface<_NodeData>* postorder_queue)
    : postorder_id_(1),
      postorder_queue_(postorder_queue) {}

  template<class _NodeData>
  PostorderQueueEventHandler<_NodeData>::~PostorderQueueEventHandler() {}

  template<class _NodeData>
  void PostorderQueueEventHandler<_NodeData>::process_data_buffer() {
    DefaultEventHandler::process_data_buffer();
  }

  template<class _NodeData>
  void PostorderQueueEventHandler<_NodeData>::on_start_element(
      const XML_Char* name,
      const XML_Char** attributes) {
    DefaultEventHandler::on_start_element(name, attributes);
  }

  template<class _NodeData>
  void PostorderQueueEventHandler<_NodeData>::on_end_element(
      const XML_Char* name) {
    DefaultEventHandler::on_end_element(name);
  }

  template<class _NodeData>
  void PostorderQueueEventHandler<_NodeData>::on_textual_data(
      const XML_Char* data,
      const int length) {
    DefaultEventHandler::on_textual_data(data, length);
  }

  template<class _NodeData>
  void PostorderQueueEventHandler<_NodeData>::on_start_element(
      const std::string& name) {
    sizes_.push_back(1); // push
    depths_.push_back(0); // push
  }

  template<class _NodeData>
  void PostorderQueueEventHandler<_NodeData>::on_end_element(
      const std::string& name) {
    int size = sizes_.back(); // top
    sizes_.pop_back(); // pop

    int depth = depths_.back() + 1; // top
    depths_.pop_back();

    // debug
    //std::cout << "(" << name << ", " << postorder_id_ << ", "
    //  << (postorder_id_ - size + 1) << ", " << costs() << ")" << std::endl;

    postorder_queue_->append(std::move(_NodeData(name, costs())), size);
    ++postorder_id_;

    if (!sizes_.empty()) {
      sizes_.back() += size; // top += size
    }

    if (!depths_.empty()) {
      int top = depths_.back();
      depths_.pop_back();
      depths_.push_back(std::max(top, depth));
    } else {
      depth_ = depth;
    }
  }

  template<class _NodeData>
  void PostorderQueueEventHandler<_NodeData>::on_attribute(
      const std::pair<std::string, std::string>& pair) {
    postorder_queue_->append(std::move(_NodeData(pair.second, costs())), 1); // value
    ++postorder_id_;

    postorder_queue_->append(std::move(_NodeData(pair.first, costs())), 2); // name
    ++postorder_id_;

    sizes_.back() += 2; // top += 2
    depths_.back() = std::max(2, depths_.back()); // top = max(2, top)
  }

  template<class _NodeData>
  void PostorderQueueEventHandler<_NodeData>::on_textual_data(std::string& data)
  {
    data.erase(std::remove(data.begin(), data.end(), '\r'), data.end());
    data.erase(std::remove(data.begin(), data.end(), '\n'), data.end());

    if (spaces_only(data)) {
      return;
    }

    postorder_queue_->append(std::move(_NodeData(data, costs())), 1);
    ++postorder_id_;

    sizes_.back() += 1; // top += 1
    depths_.back() = std::max(1, depths_.back()); // top = max(1, top)
  }

  template<class _NodeData>
  void PostorderQueueEventHandler<_NodeData>::set_postorder_queue(
      interfaces::tasm::PostorderQueueInterface<_NodeData>* postorder_queue) {

    postorder_queue_ = postorder_queue;
  }

  template<class _NodeData>
  const double PostorderQueueEventHandler<_NodeData>::costs() const {
    return 1; // Unit cost
  }

  template<class _NodeData>
  const int PostorderQueueEventHandler<_NodeData>::depth() const {
    return depth_;
  }

  template<class _NodeData>
  void PostorderQueueEventHandler<_NodeData>::parse_file(
      const std::string& file_path,
      interfaces::tasm::PostorderQueueInterface<_NodeData>* postorder_queue) {

    PostorderQueueEventHandler<_NodeData> event_handler(postorder_queue);
    parser::xml::XMLParser::parse_file(file_path, &event_handler);
  }

  } // namespace xml

} // namespace parser

#endif // PARSER_POSTORDER_QUEUE_EVENT_HANDLER_H
