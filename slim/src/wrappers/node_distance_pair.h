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

#ifndef NODE_DISTANCE_PAIR_H
#define NODE_DISTANCE_PAIR_H

namespace wrappers {

template<class _Data = int>
class NodeDistancePair {
private:
  _Data data_;
  double distance_;

public:
  NodeDistancePair();
  NodeDistancePair(const _Data& data, const double distance);
  ~NodeDistancePair();

  inline double get_distance() const;
  inline const _Data get_data() const;

  inline bool operator<(const NodeDistancePair& other) const;
  inline bool operator>(const NodeDistancePair& other) const;
  inline bool operator==(const NodeDistancePair& other) const;
  inline bool operator!=(const NodeDistancePair& other) const;
  inline bool operator>=(const NodeDistancePair& other) const;
  inline bool operator<=(const NodeDistancePair& other) const;
};

template<class _Data>
NodeDistancePair<_Data>::NodeDistancePair() {}

template<class _Data>
NodeDistancePair<_Data>::NodeDistancePair(const _Data& data,
  const double distance) : data_(data), distance_(distance) {}

template<class _Data>
NodeDistancePair<_Data>::~NodeDistancePair() {}

template<class _Data>
inline double NodeDistancePair<_Data>::get_distance() const {
  return distance_;
}

template<class _Data>
inline const _Data NodeDistancePair<_Data>::get_data() const {
  return data_;
}

template<class _Data>
inline bool NodeDistancePair<_Data>::operator<(
    const NodeDistancePair& other) const {
  if (distance_ < other.get_distance()) {
    return true;
  } else if (distance_ == other.get_distance()) {
    return (data_ < other.get_data());
  }

  return false;
}

template<class _Data>
inline bool NodeDistancePair<_Data>::operator>(
    const NodeDistancePair& other) const {
  if (distance_ > other.get_distance()) {
    return true;
  } else if (distance_ == other.get_distance()) {
    return (data_ > other.get_data());
  }

  return false;
}

template<class _Data>
inline bool NodeDistancePair<_Data>::operator==(
    const NodeDistancePair& other) const {
  return ((distance_ == other.get_distance()) && (data_ == other.get_data()));
}

template<class _Data>
inline bool NodeDistancePair<_Data>::operator!=(
    const NodeDistancePair& other) const {
  return !(*this == other);
}

template<class _Data>
inline bool NodeDistancePair<_Data>::operator>=(
    const NodeDistancePair& other) const {
  return ((*this > other) || (*this == other));
}

template<class _Data>
inline bool NodeDistancePair<_Data>::operator<=(
    const NodeDistancePair& other) const {
  return ((*this < other) || (*this == other));
}

} // namespace wrappers

#endif // NODE_DISTANCE_PAIR_H
