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

#ifndef PARSER_DEFAULT_EVENT_HANDLER_H
#define PARSER_DEFAULT_EVENT_HANDLER_H

#include "abstract_event_handler.h"

#include "expat.h"

#include <vector>
#include <unordered_map>
#include <algorithm>

namespace parser {

  namespace xml {

  // StringEventHandler, in essence
  class DefaultEventHandler : public AbstractEventHandler {
  protected:
    // inherited, pure virtual
    // see abstract_event_handler.h
    virtual void process_data_buffer();

    bool spaces_only(const std::string& data) const;

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

    virtual void on_start_element(const std::string& name);
    virtual void on_end_element(const std::string& name);
    virtual void on_attribute(const std::pair<std::string, std::string>& pair);
    virtual void on_textual_data(std::string& data);

  public:
    DefaultEventHandler();
    virtual ~DefaultEventHandler();
  };

  DefaultEventHandler::DefaultEventHandler() {}

  DefaultEventHandler::~DefaultEventHandler() {}

  void DefaultEventHandler::on_start_element(const XML_Char* name,
      const XML_Char** attributes) {
    process_data_buffer();

    on_start_element(std::string(name));

    if (attributes != nullptr) {
      // attributes are assumed to not appear that often wrt. a single node
      // thus constructing a vector and sorting it afterwards should be faster
      // than creating a std::set<std::pair<std::string, std::string>>
      std::vector<std::pair<std::string, std::string>> sorted_attributes;

      for (int i = 0; attributes[i] != nullptr; i += 2) {
        sorted_attributes.emplace_back(
          std::string(attributes[i]),
          std::string(attributes[i + 1]));
      }

      std::sort(sorted_attributes.begin(), sorted_attributes.end());

      for (const auto& attribute_pair: sorted_attributes) {
        on_attribute(attribute_pair);
      }
      //sorted_attributes.clear();
    }
  }

  void DefaultEventHandler::on_end_element(const XML_Char* name) {
    process_data_buffer();
    on_end_element(std::string(name));
  }

  void DefaultEventHandler::on_textual_data(
      const XML_Char* data,
      const int length){
    // if one desires to skip empty textual data (i.e., only containing
    // whitespaces), this has to be done in the actual concrete handler
    // implementation. This class does NOT do this for you.
    //
    // However, this class provides a member function (i.e., spaces_only)
    // returning true if the given string contains only whitespaces, and false
    // otherwise.

    if (length <= 0) {
      return;
    }

    data_buffer_.append(data, data + length);
  }

  void DefaultEventHandler::process_data_buffer() {
    if (!data_buffer_.empty()) {
      on_textual_data(data_buffer_);
      data_buffer_.clear();
    }
  }

  bool DefaultEventHandler::spaces_only(const std::string& data) const {
    return std::all_of(data.begin(), data.end(), isspace);
  }

  void DefaultEventHandler::on_end_of_file() {}
  void DefaultEventHandler::on_start_element(const std::string& name) {}
  void DefaultEventHandler::on_end_element(const std::string& name) {}
  void DefaultEventHandler::on_attribute(
      const std::pair<std::string, std::string>& pair) {}
  void DefaultEventHandler::on_textual_data(std::string& data) {}

  } // namespace xml

} // namespace parser

#endif // PARSER_DEFAULT_EVENT_HANDLER_H
