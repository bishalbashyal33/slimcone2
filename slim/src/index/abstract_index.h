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

#ifndef INDEX_ABSTRACT_INDEX_H
#define INDEX_ABSTRACT_INDEX_H

#include "statistics.h"
#include "log.h"
#include "timing.h"

#include <vector>
#include <unordered_map>

namespace tree_index {

template<class _NodeData>
class AbstractIndex {
public: // member types
  using SizeType = unsigned int;
  using TokenIdType = unsigned int;
  using NodeIdType = unsigned int;

  template<class _Type> using SymmetricPair = std::pair<_Type, _Type>;
  template<class _Type> using SymmetricTriple = std::tuple<_Type, _Type, _Type>;
  template<class _Type> using SymmetricQuintuple = std::tuple<_Type, _Type, _Type, _Type, _Type>;

  using TokenFrequencyMap = std::unordered_map<TokenIdType, unsigned int>;
  using NodeData = _NodeData;

  using StatisticsType = common::IndexStatistics<>;
  using LogType = common::Log<common::LogPrinter>;
  using IntervalType = common::Interval;

public: // member functions
  AbstractIndex(LogType& log, const std::string& name);
  ~AbstractIndex();

  // Getter
  inline SizeType size() const;
  inline std::string name() const;

  inline void prepare();
  inline void unprepare();
  inline bool ready() const; // immutable

public: // member variables
  StatisticsType statistics_;
  LogType& log_;
  IntervalType interval_;

protected: // member variables
  SizeType size_{}; // number of inverted lists
  std::string name_{};
  bool ready_{};
};

#include "abstract_index_impl.h"

} // namespace tree_index

#endif // INDEX_ABSTRACT_INDEX_H
