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

#ifndef PARSER_LABEL_SET_INDEX_EVENT_HANDLER_H
#define PARSER_LABEL_SET_INDEX_EVENT_HANDLER_H

#include "default_event_handler.h"
#include "node.h"
#include "tasm_tree.h"
#include "postorder_queue_interface.h"
#include "inverted_list_index.h"

#include <vector>
#include <map>
#include <iostream>

namespace parser {

    namespace xml {

    template<class _T> using Stack = std::vector<_T>;

    template<class _IndexType>
    class LabelSetIndexEventHandler : public DefaultEventHandler {
    public:
        using NodeData = typename _IndexType::NodeData;
        using TokenIdSet = std::vector<unsigned int>;

    public:
        LabelSetIndexEventHandler(_IndexType& index);
        virtual ~LabelSetIndexEventHandler();

        virtual void on_end_of_file() override;

        virtual void on_start_element(const std::string& name) override;
        virtual void on_end_element(const std::string& name) override;
        virtual void on_attribute(
          const std::pair<std::string, std::string>& pair) override;
        virtual void on_textual_data(std::string& data) override;

    private:
        virtual void process_data_buffer() override;

    private:
        _IndexType& index_;
        Stack<unsigned int> sizes_{};
        unsigned int postorder_id_{};
    };

    #include "label_set_index_event_handler_impl.h"

    }

}

#endif // PARSER_LABEL_SET_INDEX_EVENT_HANDLER_H
