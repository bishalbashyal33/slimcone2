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

#ifndef ARRAY_2D_H
#define ARRAY_2D_H

#include <memory>
#include <algorithm>

namespace data_structures {

/**
 * Encapsulates a two dimensional array of a given type (generic).
 * Main reason: it gives better performance than the standard _type** approach
 * due to its allocation pattern. In a _type** array each row in the matrix is
 * a separately allocated dynamic array on the heap - O(n) in general. Same goes
 * for the deallocation because one has to iterate through all elements again.
 * Moreover, separate allocations tend to live in discontinuous chunks of memory.
 * As a result, lots of cache misses happen, in general. This slows down the
 * actual access.
 */
template<typename _Type>
class Array2D {
private:
  // consecutive allocated memory: rows * columns
  size_t rows_;
  size_t columns_;
  // consecutive allocated row containing the array elements
  _Type* data_;

public:
  // Constructor(s)
  Array2D();
  Array2D(size_t rows, size_t columns);
  Array2D(const Array2D& other);

  // Destructor
  ~Array2D();

  // Get number of rows
  size_t get_rows() const;

  // Get number of columns
  size_t get_columns() const;

  // Set dimensions
  void set_dimensions(size_t rows, size_t columns);

  // Enable access as usual using the [][] notation.
  //
  // Params:  row the row to be accessed
  //
  // Return:  A pointer to the beginning of the row to be accessed
  _Type* operator[](size_t row);
};

template<typename _Type>
Array2D<_Type>::Array2D() : rows_(0), columns_(0), data_(nullptr) { }

template<typename _Type>
Array2D<_Type>::Array2D(size_t rows, size_t columns)
  : rows_(rows), columns_(columns), data_(nullptr)
{
  // allocate array and initialize to zero
  data_ = new _Type[rows_ * columns_] { };
}

template<typename _Type>
Array2D<_Type>::Array2D(const Array2D& other)
  : Array2D(other.get_rows(), other.get_columns())
{
  std::copy(other.data, other.data + (rows_ * columns_), data_);
}

template<typename _Type>
Array2D<_Type>::~Array2D() {
  delete[] data_;
}

template<typename _Type>
size_t Array2D<_Type>::get_rows() const {
  return rows_;
}

template<typename _Type>
size_t Array2D<_Type>::get_columns() const {
  return columns_;
}

template<typename _Type>
void Array2D<_Type>::set_dimensions(size_t rows, size_t columns) {
  // no reallocation, only if nothing is allocated by now
  if ((data_ == nullptr) && (rows > 0) && (columns > 0)) {
    rows_ = rows;
    columns_ = columns;
    data_ = new _Type[rows_ * columns_] { };
  }
}

template<typename _Type>
_Type* Array2D<_Type>::operator[](size_t row) {
  return data_ + row * columns_;
}

} // namespace data_structures

#endif // ARRAY_2D_H
