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

#ifndef PARSER_LABEL_SET_EVENT_HANDLER_IMPL_H
#define PARSER_LABEL_SET_EVENT_HANDLER_IMPL_H

template<class _NodeData>
LabelSetEventHandler<_NodeData>::LabelSetEventHandler() {}

template<class _NodeData>
LabelSetEventHandler<_NodeData>::LabelSetEventHandler(
        POQueue<_NodeData>* postorder_queue,
        TokenMap& token_map)
        :   postorder_queue_(postorder_queue),
            token_map_(token_map),
            next_token_id_(token_map_.size()) {}

template<class _NodeData>
LabelSetEventHandler<_NodeData>::~LabelSetEventHandler() {}

template<class _NodeData>
void LabelSetEventHandler<_NodeData>::on_start_element(
        const XML_Char* name,
        const XML_Char** attributes) {
    DefaultEventHandler::on_start_element(name, attributes);
}

template<class _NodeData>
void LabelSetEventHandler<_NodeData>::on_end_element(const XML_Char* name) {
    DefaultEventHandler::on_end_element(name);
}

template<class _NodeData>
void LabelSetEventHandler<_NodeData>::on_textual_data(
        const XML_Char* data,
        const int length) {
    DefaultEventHandler::on_textual_data(data, length);
}

template<class _NodeData>
void LabelSetEventHandler<_NodeData>::on_end_of_file() {
    std::sort(token_id_set_.begin(), token_id_set_.end());
}

template<class _NodeData>
void LabelSetEventHandler<_NodeData>::on_start_element(const std::string& name) {
    sizes_.push_back(1);
}

template<class _NodeData>
void LabelSetEventHandler<_NodeData>::on_end_element(const std::string& name) {
    int size = sizes_.back();
    sizes_.pop_back();

    unsigned int token_id = convert_token(_NodeData(name));
    token_id_set_.push_back(token_id);

    postorder_queue_->append(token_id, size);

    if (!sizes_.empty()) {
      sizes_.back() += size; // top += size
    }
}

template<class _NodeData>
void LabelSetEventHandler<_NodeData>::on_attribute(
        const std::pair<std::string, std::string>& pair) {
    unsigned int name_token_id = convert_token(_NodeData(pair.first));
    unsigned int value_token_id = convert_token(_NodeData(pair.second));

    token_id_set_.push_back(name_token_id);
    token_id_set_.push_back(value_token_id);

    postorder_queue_->append(value_token_id, 1);
    postorder_queue_->append(name_token_id, 2);

    sizes_.back() += 2;
}

template<class _NodeData>
void LabelSetEventHandler<_NodeData>::on_textual_data(std::string& data) {
    data.erase(std::remove(data.begin(), data.end(), '\r'), data.end());
    data.erase(std::remove(data.begin(), data.end(), '\n'), data.end());

    if (spaces_only(data)) {
      return;
    }

    unsigned int token_id = convert_token(_NodeData(data));
    token_id_set_.push_back(token_id);

    postorder_queue_->append(token_id, 1);

    sizes_.back() += 1;
}

template<class _NodeData>
const std::vector<unsigned int> LabelSetEventHandler<_NodeData>::token_id_set() const
{
    return token_id_set_;
}

/// private member functions

template<class _NodeData>
void LabelSetEventHandler<_NodeData>::process_data_buffer() {
    DefaultEventHandler::process_data_buffer();
}

template<class _NodeData>
inline unsigned int LabelSetEventHandler<_NodeData>::convert_token(
        const _NodeData& token) {
    if (!token_map_.contains(token)) {
        return token_map_.insert(token);
    }

    return token_map_.at(token);
}

#endif // PARSER_LABEL_SET_EVENT_HANDLER_IMPL_H
