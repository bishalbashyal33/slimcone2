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

#ifndef PARSER_LABEL_SET_INDEX_EVENT_HANDLER_IMPL_H
#define PARSER_LABEL_SET_INDEX_EVENT_HANDLER_IMPL_H

template<class _IndexType>
LabelSetIndexEventHandler<_IndexType>::LabelSetIndexEventHandler(
        _IndexType& index) : index_(index), postorder_id_(1) {}

template<class _IndexType>
LabelSetIndexEventHandler<_IndexType>::~LabelSetIndexEventHandler() {}

template<class _IndexType>
void LabelSetIndexEventHandler<_IndexType>::on_end_of_file() {
    index_.prepare();
}

template<class _IndexType>
void LabelSetIndexEventHandler<_IndexType>::on_start_element(
        const std::string& name) {
    sizes_.push_back(1);
}

template<class _IndexType>
void LabelSetIndexEventHandler<_IndexType>::on_end_element(
        const std::string& name) {
    int size = sizes_.back();
    sizes_.pop_back();

    if (!sizes_.empty()) {
      sizes_.back() += size;
    }

    index_.put(NodeData(name), size);
}

template<class _IndexType>
void LabelSetIndexEventHandler<_IndexType>::on_attribute(
        const std::pair<std::string, std::string>& pair) {
    sizes_.back() += 2;

    index_.put(NodeData(pair.second), 1);
    index_.put(NodeData(pair.first), 2);
}

template<class _IndexType>
void LabelSetIndexEventHandler<_IndexType>::on_textual_data(
        std::string& data) {
    data.erase(std::remove(data.begin(), data.end(), '\r'), data.end());
    data.erase(std::remove(data.begin(), data.end(), '\n'), data.end());

    if (spaces_only(data)) {
      return;
    }

    sizes_.back() += 1;

    index_.put(NodeData(data), 1);
}

template<class _IndexType>
void LabelSetIndexEventHandler<_IndexType>::process_data_buffer() {
    DefaultEventHandler::process_data_buffer();
}


#endif // PARSER_LABEL_SET_INDEX_EVENT_HANDLER_IMPL_H
