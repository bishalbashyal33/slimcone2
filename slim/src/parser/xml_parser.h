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

#ifndef PARSER_XML_PARSER_H
#define PARSER_XML_PARSER_H

#include <cstdio>

#include "abstract_event_handler.h"
#include "expat.h"

namespace parser {

  namespace xml {

  /**
   *  Wrapper class around the expat SAX XML parser.
   *
   *  This is a minimal wrapper without any error handling by now
   *  It may be extended in the future, see include/expat-2.2.0/lib/expat.h
   *  for all functions provided by expat.
   *
   *  See https://sourceforge.net/projects/expat/ for more information on expat.
   */
  class XMLParser {
  public:
    XML_Parser expat_parser_{ }; // XML_Parser is a pointer!
    AbstractEventHandler* event_handler_{ };

  public:
    XMLParser() = delete;
    XMLParser(AbstractEventHandler* event_handler);
    ~XMLParser();

    // Creates a new expat parser instance for the given encoding/separator.
    //
    // Params:  encoding  If set, it specifies a character encoding
    //          separator If set, namespace processing is enabled
    //
    // Return:  True if parser was successfully created, false otherwise.
    bool create(const XML_Char* encoding = nullptr,
      const XML_Char* separator = nullptr);

    // Frees the expat parser instance (if there exists one).
    void free();

    // Main parsing function based on a buffer and the expat parser instance.
    //
    // Params:  buffer    The buffer to use (has to be at least of size length)
    //          length    The length of the part to parse into the buffer
    //          is_final  If set to true, it tells the parser that it is the
    //                    last part of the text
    //
    // Return:  True if the part was parsed successfully, false otherwise.
    bool parse(const char* buffer, int& length, bool& is_final);

    static AbstractEventHandler* parse_file(const std::string& file_path,
      AbstractEventHandler* event_handler);
  };

  XMLParser::XMLParser(AbstractEventHandler* event_handler)
    : event_handler_(event_handler)
  {
    create();

    // set expat callbacks (see abstract_event_handler.h)
    XML_SetElementHandler(expat_parser_, start_element_handler,
      end_element_handler
    );
    XML_SetCharacterDataHandler(expat_parser_, textual_data_handler);
  }

  XMLParser::~XMLParser() {
    this->free();
  }

  bool XMLParser::create(const XML_Char* encoding, const XML_Char* separator) {
    this->free();

    if (!(expat_parser_ = XML_ParserCreate_MM(encoding, nullptr, separator)))
    {
      return false;
    }

    XML_SetUserData(expat_parser_, (void*)event_handler_);
    return true;
  }

  void XMLParser::free() {
    if (expat_parser_ != nullptr) {
      XML_ParserFree(expat_parser_);
    }

    expat_parser_ = nullptr;
  }

  bool XMLParser::parse(const char* buffer, int& length, bool& is_final) {
    return (XML_Parse(expat_parser_, buffer, length, is_final) != 0);
  }

  AbstractEventHandler* XMLParser::parse_file(const std::string& file_path,
      AbstractEventHandler* event_handler)
  {
    XMLParser parser(event_handler);

    char buffer[1024 * 4];
    std::FILE* file_handle = std::fopen(file_path.c_str(), "r");
    bool is_final = false;
    int read_bytes;

    while (!is_final) {
      read_bytes = fread(buffer, 1, 512, file_handle);
      is_final = feof(file_handle);
      parser.parse(buffer, read_bytes, is_final);
    }
    fclose(file_handle);
    event_handler->on_end_of_file();

    return event_handler;
  }

  } // namespace xml

} // namespace parser

#endif // PARSER_XML_PARSER_H
