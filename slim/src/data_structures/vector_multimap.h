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

#ifndef DATA_STRUCTURES_VECTOR_MULTIMAP_H
#define DATA_STRUCTURES_VECTOR_MULTIMAP_H

#include <map>
#include <vector>

namespace data_structures {

template<class KeyType, class ValueType>
class VectorMultimap {
public:
  using const_iterator = typename std::map<KeyType, std::vector<ValueType>>::const_iterator;

public:
  void emplace(const KeyType key, const ValueType value) {
    auto vectorit = kvstore_.lower_bound(key);

    if ((vectorit != kvstore_.end()) && !(kvstore_.key_comp()(key, vectorit->first))) {
      // add value to corresponding vector
      vectorit->second.emplace_back(value);

      // optional: sort vectorit->second afterwards
    } else {
      kvstore_.insert(vectorit, typename decltype(kvstore_)::value_type(key, {value}));
    }

    ++size_;
  }

  ValueType erase(const KeyType key, const ValueType value) {
    auto it = kvstore_.find(key);

    if (it == kvstore_.end()) {
      return 0;
    }

    auto& vector = it->second;

    auto valueit = std::find(vector.begin(), vector.end(), value);
    auto ret = *valueit;

    vector.erase(valueit);

    --size_;

    return ret;
  }

  unsigned long size() const {
    return size_;
  }

  typename std::map<KeyType, std::vector<ValueType>>::const_iterator cbegin() const {
    return kvstore_.cbegin();
  }

  typename std::map<KeyType, std::vector<ValueType>>::const_iterator cend() const {
    return kvstore_.end();
  }

private:
  std::map<KeyType, std::vector<ValueType>> kvstore_;
  unsigned long size_;
};

} // namespace data_structures

#endif // DATA_STRUCTURES_VECTOR_MULTIMAP_H
