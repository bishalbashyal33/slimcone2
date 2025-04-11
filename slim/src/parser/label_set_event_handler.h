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

#ifndef PARSER_LABEL_SET_EVENT_HANDLER_H
#define PARSER_LABEL_SET_EVENT_HANDLER_H

#include "default_event_handler.h"
#include "node.h"
#include "tasm_tree.h"
#include "label_hash_map.h"

#include <vector>
#include <map>
#include <iostream>

namespace parser {

    namespace xml {

    using nodes::Node;
    using nodes::StringNodeData;

    template<class _NodeData>
    class LabelSetEventHandler : public DefaultEventHandler {
    public:
        template<class _T> using Stack = std::vector<_T>;
        using TokenMap = data_structures::LabelHashMap<_NodeData>;

        template<class _Type>
        using POQueue = data_structures::TASMTree<_Type>;

    public:
        LabelSetEventHandler();
        LabelSetEventHandler(
            POQueue<_NodeData>* postorder_queue,
            TokenMap& token_map);
        virtual ~LabelSetEventHandler();

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

        const std::vector<unsigned int> token_id_set() const;
        static void parse_file(
            const std::string& file_path,
            POQueue<_NodeData>* postorder_queue);

    private:
        virtual void process_data_buffer() override;
        inline unsigned int convert_token(const _NodeData& token);

    private:
        std::vector<unsigned int> token_id_set_{};
        Stack<int> sizes_{};
        POQueue<_NodeData>* postorder_queue_{};
        TokenMap& token_map_;
        unsigned int next_token_id_{};
    };

    #include "label_set_event_handler_impl.h"

    }

}

#endif // PARSER_LABEL_SET_EVENT_HANDLER_H
