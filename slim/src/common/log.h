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

#ifndef COMMON_LOG_H
#define COMMON_LOG_H

#include <iostream>
#include <sstream>
#include <string>

// Log

namespace common {

#ifdef NO_LOG

struct LogPrinter {
  template<class... ArgTypes>
  inline static void log(std::string&, const ArgTypes&...) {}

  template<class T> inline static void log(std::stringstream&, const T&) {}

  template<class T, class... ArgTypes>
  inline static void log(std::stringstream&, const T&, const ArgTypes&...) {}

  inline static void flush(std::ostream&, std::string&) {}
};

#else

struct LogPrinter {
  template<class... ArgTypes>
  inline static void log(std::string& s, const ArgTypes&... args) {
    std::stringstream ss;
    log(ss, args...);
    s += ss.str();
  }

  template<class T> inline static void log(
      std::stringstream& ss,
      const T& val) {
    ss << val;
  }

  template<class T, class... ArgTypes>
  inline static void log(
      std::stringstream& ss,
      const T& val,
      const ArgTypes&... args) {
    ss << val;
    log(ss, args...);
  }

  inline static void flush(std::ostream& os, std::string& s) {
    os << "Logging:\n" << s << std::endl;
    s.clear();
  }
};

#endif // NO_LOG

template<class Printer>
class Log {
public:
  std::string s;

  // NOT 'inline Log<Printer>& operator<<(...)' on purpose
  // operator<< would require us to return a reference to *this in order to use
  // it one after another (as usual, e.g., std::cout << "1" << "2" << std::endl)
  // we want the compiler to optimize the empty functions of the LogPrinter
  // class if NO_LOG is defined. returning *this from the operator<< function
  // would prevent the compiler from optimizing the "empty" functions
  template<class... ArgTypes>
  inline void log(const ArgTypes&... args) {
    Printer::log(s, args...);
  }

  inline void flush(std::ostream& os) {
    Printer::flush(os, s);
  }
};

} // namespace common

#endif // COMMON_LOG_H
