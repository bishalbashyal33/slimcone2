#ifndef PARSER_ABSTRACT_EVENT_HANDLER_H
#define PARSER_ABSTRACT_EVENT_HANDLER_H

#include <sstream>

#include "expat.h"

namespace parser {

  namespace xml {

  /**
   * Pure virtual base class for any EventHandler used in combination with the
   * XMLParser class.
   */
  class AbstractEventHandler {
  protected:
    std::string data_buffer_;

    // Define what to do when textual data (in data_buffer_) is processed.
    // Implement all actions to take when a textual data is found in here.
    // E.g. create a new child and put it in the correct position of the tree.
    virtual void process_data_buffer() = 0;

  public:
    AbstractEventHandler();
    virtual ~AbstractEventHandler();

    // Define what to do when a start element is reached.
    //
    // Params:  name        The name of the started element
    //          attributes  Array of all attributes found within the element
    virtual void on_start_element(const XML_Char* name,
      const XML_Char** attributes) = 0;

    // Define what to do when an end element is reached.
    //
    // Params:  name  The name of the ended element
    virtual void on_end_element(const XML_Char* name) = 0;

    // Define what to do when textual data is reached.
    //
    // Params:  data    The textual data found
    //          length  The length of the textual data found
    virtual void on_textual_data(const XML_Char* data, const int length) = 0;

    // Define what to do when the file was read entirely (i.e. end is reached).
    // In essence, define postprocessing steps.
    virtual void on_end_of_file() = 0;
  };

  AbstractEventHandler::AbstractEventHandler() {
    data_buffer_.reserve(1024 * 4);
  }

  AbstractEventHandler::~AbstractEventHandler() { }
  
  } // namespace xml

} // namespace parser

////////////////////////////////////////////////////////////////////////////////

// set AbstractEventHandler callback functions
extern "C" {

// set start_element callback function
static void start_element_handler(void* user_data, const XML_Char* name,
  const XML_Char** attributes)
{
  parser::xml::AbstractEventHandler* event_handler =
    static_cast<parser::xml::AbstractEventHandler*>(user_data);
  event_handler->on_start_element(name, attributes);
}

// set end_element callback function
static void end_element_handler(void* user_data, const XML_Char* name) {
  parser::xml::AbstractEventHandler* event_handler =
    static_cast<parser::xml::AbstractEventHandler*>(user_data);
  event_handler->on_end_element(name);
}

// set textual_data callback function
static void textual_data_handler(void* user_data, const XML_Char* data,
  int length)
{
  parser::xml::AbstractEventHandler* event_handler =
    static_cast<parser::xml::AbstractEventHandler*>(user_data);
  event_handler->on_textual_data(data, length);
}

};

#endif // PARSER_ABSTRACT_EVENT_HANDLER_H
