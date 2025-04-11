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

#ifndef COMMON_HASH_FUNCTORS_H
#define COMMON_HASH_FUNCTORS_H

#include <vector>

#include "string_node_data.h"

namespace common {

  namespace hash_functors {

  using nodes::StringNodeData;

  // for TASM
  struct StringNodeDataHash {
    std::size_t operator()(const StringNodeData& key) const {
      return std::hash<std::string>()(key.get_label());
    }

    std::size_t operator()(const StringNodeData* key_ptr) const {
      return std::hash<std::string>()(key_ptr->get_label());
    }
  };

  // see boost::hash_combine template function (is equivalent)
  struct IntVectorHash {
    std::size_t operator()(const std::vector<int>& v) const {
      std::size_t seed = v.size();

      for(auto& value : v) {
        seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
      }

      return seed;
    }
  };

  } // namespace hash_functors

} // namespace common

#endif // COMMON_HASH_FUNCTORS_H
