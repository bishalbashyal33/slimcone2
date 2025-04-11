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

#ifndef INDEX_ABSTRACT_INDEX_IMPL_H
#define INDEX_ABSTRACT_INDEX_IMPL_H

template<class _NodeData>
AbstractIndex<_NodeData>::AbstractIndex(LogType& log, const std::string& name)
    : log_(log), name_(name) {
  unprepare();
}

template<class _NodeData>
AbstractIndex<_NodeData>::~AbstractIndex() {}

template<class _NodeData>
inline typename AbstractIndex<_NodeData>::SizeType AbstractIndex<_NodeData>::size() const {
  return size_;
}

template<class _NodeData>
inline std::string AbstractIndex<_NodeData>::name() const {
  return name_;
}

template<class _NodeData>
inline void AbstractIndex<_NodeData>::prepare() {
  ready_ = true;

  log_.log("[", name(), "] prepared.\n");
}

template<class _NodeData>
inline void AbstractIndex<_NodeData>::unprepare() {
  ready_ = false;
}

template<class _NodeData>
inline bool AbstractIndex<_NodeData>::ready() const {
  return ready_;
}

#endif // INDEX_ABSTRACT_INDEX_IMPL_H
