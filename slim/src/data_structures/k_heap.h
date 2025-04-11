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

#ifndef K_HEAP_H
#define K_HEAP_H

namespace data_structures {

/**
 * Encapsulates a maximum heap of a given type (generic) which only keeps the k
 * smallest elements (wrt. the operator< of the specified type). This class is
 * useful for a top-k ranking, e.g. in the TASM implementation.
 */
template<class _Type>
class KHeap {
public:
  // maximum and current size of the heap
  size_t max_size_;
  size_t size_;
  // array to store the elements of the heap
  _Type* data_;

  // Shifts the element at the given position up to fulfill the heap condition.
  //
  // Params:  start_position  The position to start from
  void shift_up(const size_t& start_position);

  // Shifts the element at the given position down to fulfill the heap condition.
  //
  // Params:  start_position  The position to start from
  void shift_down(const size_t& start_position);

public:
  // Constructor(s)
  KHeap(const size_t max_size = 10);
  KHeap(const KHeap<_Type>& other);
  // Destructor
  ~KHeap();

  // Operators
  KHeap<_Type>& operator=(const KHeap<_Type>& other);

  template<class _HeapEntryType>
  friend std::ostream& operator<<(std::ostream& os,
    const KHeap<_HeapEntryType>& heap);

  // Resize the heap (retaining the elements it holds before the resize)
  //
  // Params:  max_size  The desired maximum size of the heap
  void resize(const size_t max_size);

  // Check if the heap is currently empty.
  //
  // Return:  True if the heap is empty, false otherwise
  bool empty() const;

  // Check if the heap is currently full.
  //
  // Return:  True if the heap is full, false otherwise
  bool full() const;

  // Get the current number of elements in the heap.
  //
  // Return:  The number of elements in the heap
  size_t size() const;

  // Get the maximum number of elements the heap can hold.
  //
  // Return:  The maximum number of elements the heap can store
  size_t max_size() const;

  // Get the top element of the heap (the root).
  //
  // Return:  A constant reference to the top element of the heap.
  _Type const& front() const;

  // Get the "last" element of the heap (the last element of the array).
  //
  // Return:  A constant referenceto the last element of the heap array.
  _Type const& back() const;

  // Insert a new element into the heap.
  //
  // Params:  element The element to be inserted
  //
  // Return:  True if the element was successfully inserted, false otherwise
  bool insert(_Type element);

  // Remove the top element of the heap while maintaining the heap state.
  //
  // Return:  The former top element of the heap
  const _Type erase_front();

  // Replace the top element of the heap by another element while maintaining the
  // heap state.
  //
  // Params:  replacement The element to replace the top element of the heap
  //
  // Return:  A copy of the former top element of the heap
  const _Type replace_front(_Type replacement);

  // Replace the top element of the heap by another element only if the
  // replacement element is acutally smaller than the current top element.
  //
  // Params:  replacement The element to replace the top element of the heap
  //
  // Return:  A copy of the former top element of the heap if it was replaced,
  //          the current top element otherwise
  const _Type replace_front_if_smaller(_Type replacement);

  // STL-typical iterators for range-based loops
  _Type* begin() const;
  _Type* end() const;
  // please notice: by now, the reverse iterator has to be DECREMENTED in the
  // loop (not incremeneted like in the STL rbegin()/rend())
  _Type* rbegin() const;
  _Type* rend() const;

  // STL-typical find member function.
  // Uses the operator== for comparison of the objects.
  //
  // Params:  element A reference to the element to search for
  //
  // Return:  An iterator pointing to the position, if found. Otherwise: end()
  _Type* find(_Type& element) const;
};

template<class _Type>
KHeap<_Type>::KHeap(const size_t max_size) : max_size_(max_size) {
  data_ = new _Type[max_size_]{};
  size_ = 0;
}

template<class _Type>
KHeap<_Type>::KHeap(const KHeap<_Type>& other)
  : max_size_(other.max_size_),
    size_(other.size_),
    data_(new _Type[other.max_size_]) {
  std::copy(other.data_, other.data_ + other.size_, data_);
}

template<class _Type>
KHeap<_Type>::~KHeap() {
  delete[] data_;
}

template<class _Type>
KHeap<_Type>& KHeap<_Type>::operator=(const KHeap<_Type>& other) {
  if (this != &other) {
    size_ = other.size_;
    max_size_ = other.max_size_;

    _Type* old_data = data_;
    data_ = new _Type[max_size_];
    std::copy(other.data_, other.data_ + other.size_, data_);

    if (old_data != nullptr) {
      delete[] old_data;
    }
  }

  return *this;
}

template<class _Type>
void KHeap<_Type>::resize(const size_t max_size) {
  _Type* old_data = data_;

  max_size_ = max_size;
  data_ = new _Type[max_size_];
  std::move(old_data, old_data + size_, data_);

  if (old_data != nullptr) {
    delete[] old_data;
  }
}

template<class _Type>
bool KHeap<_Type>::empty() const {
  return (size_ == 0);
}

template<class _Type>
bool KHeap<_Type>::full() const {
  return (size_ == max_size_);
}

template<class _Type>
size_t KHeap<_Type>::size() const {
  return size_;
}

template<class _Type>
size_t KHeap<_Type>::max_size() const {
  return max_size_;
}

template<class _Type>
_Type const& KHeap<_Type>::front() const {
  return data_[0];
}

template<class _Type>
_Type const& KHeap<_Type>::back() const {
  return data_[size_];
}

template<class _Type>
bool KHeap<_Type>::insert(_Type element) {
  if (size_ == max_size_) {
    return false;
  }

  data_[size_] = element;
  shift_up (size_);
  ++size_;
  return true;
}

template<class _Type>
const _Type KHeap<_Type>::erase_front() {
  _Type root = data_[0];

  data_[0] = data_[--size_];
  shift_down(0);

  return root;
}

template<class _Type>
const _Type KHeap<_Type>::replace_front(_Type replacement) {
  _Type root = data_[0];
  data_[0] = replacement;

  if (replacement < root) {
    shift_down(0);
  }

  return root;
}

template<class _Type>
const _Type KHeap<_Type>::replace_front_if_smaller(_Type replacement) {
  _Type& root = data_[0];
  if (root < replacement) {
    return root;
  }

  return replace_front(replacement);
}

template<class _Type>
_Type* KHeap<_Type>::begin() const {
  return data_;
}

template<class _Type>
_Type* KHeap<_Type>::end() const {
  return &(data_[size_]); // data_[max_size_] for the complete array
}

template<class _Type>
_Type* KHeap<_Type>::rbegin() const {
  return &(data_[size_ - 1]); // data[max_size_ - 1] for the complete array
}

template<class _Type>
_Type* KHeap<_Type>::rend() const {
  return (data_ - 1);
}

template<class _Type>
_Type* KHeap<_Type>::find(_Type& element) const {
  _Type* it = begin();

  for (; it != end(); ++it) {
    if (*it == element) {
      return it;
    }
  }

  return end();
}

template<class _Type>
void KHeap<_Type>::shift_up(const size_t& start_position) {
  const _Type back = this->back();
  size_t parent_position = ((start_position - 1) / 2);

  size_t current_position = start_position;
  while (current_position > 0 && data_[parent_position] < back) {
    data_[current_position] = data_[parent_position];
    current_position = parent_position;
    parent_position = ((parent_position - 1) / 2);
  }

  data_[current_position] = back;
}

template<class _Type>
void KHeap<_Type>::shift_down(const size_t& start_position) {
  const _Type front = this->front();
  size_t left_position = 0, right_position = 0, swap_position = 0;

  size_t current_position = start_position;
  while (current_position < (size_ / 2)) {
    left_position = 2 * current_position + 1;
    right_position = left_position + 1;

    if (right_position < size_ && data_[left_position] < data_[right_position]) {
      swap_position = right_position;
    } else {
      swap_position = left_position;
    }

    if (front >= data_[swap_position]) {
      break;
    }

    data_[current_position] = data_[swap_position];
    current_position = swap_position;
  }
  data_[current_position] = front;
}

/// friend functions
template<class _Type>
std::ostream& operator<<(std::ostream& os, const KHeap<_Type>& heap) {
  // copy KHeap content and sort it
  std::vector<_Type> sorted_heap(heap.size());
  std::copy(heap.data_, heap.data_ + heap.size(),
    sorted_heap.begin()
  );
  std::sort(sorted_heap.begin(), sorted_heap.end());

  os << "[";
  for (auto it = sorted_heap.cbegin(); it != sorted_heap.cend(); ++it) {
    os << "[\"" << it->get_data() << "\", \""
      << it->get_distance() << ".0\"" << "]";

    if ((it + 1) != sorted_heap.cend()) {
      os << ",";
    }
  }
  os << "]";

  return os;
}

} // namespace data_structures

#endif // K_HEAP_H
