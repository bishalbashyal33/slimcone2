#ifndef PARSER_XML_POSTORDER_SOURCE_H
#define PARSER_XML_POSTORDER_SOURCE_H

#include <string>

#include "string_node_data.h"
#include "postorder_source_interface.h"
#include "postorder_queue_interface.h"
#include "postorder_queue_event_handler.h"

namespace parser {

  namespace xml {

  template<class _NodeData = nodes::TASMStringNodeData>
  class XMLPostorderSource : public interfaces::tasm::PostorderSourceInterface<_NodeData>
  {
  private:
    std::string file_path_{};

  public:
    XMLPostorderSource(const std::string& file_path);
    ~XMLPostorderSource();

    void append_to(interfaces::tasm::PostorderQueueInterface<_NodeData>* postorder_queue) override;
  };

  template<class _NodeData>
  XMLPostorderSource<_NodeData>::XMLPostorderSource(const std::string& file_path)
  : file_path_(file_path)
  { }

  template<class _NodeData>
  XMLPostorderSource<_NodeData>::~XMLPostorderSource() { }

  template<class _NodeData>
  void XMLPostorderSource<_NodeData>::append_to(
    interfaces::tasm::PostorderQueueInterface<_NodeData>* postorder_queue)
  {
    xml::PostorderQueueEventHandler<_NodeData>::parse_file(file_path_, postorder_queue);
  }

  } // namespace xml

} // namespace parser

#endif // PARSER_XML_POSTORDER_SOURCE_H