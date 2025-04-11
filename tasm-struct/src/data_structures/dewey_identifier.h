#ifndef DATA_STRUCTURES_DEWEY_IDENTIFIER_H
#define DATA_STRUCTURES_DEWEY_IDENTIFIER_H

#include <vector>

namespace data_structures {

class DeweyIdentifier {
public:
  std::vector<int> id_;

public:
  DeweyIdentifier();
  DeweyIdentifier(const int& value);
  DeweyIdentifier(const DeweyIdentifier& other);
  DeweyIdentifier(const DeweyIdentifier& other, const int& last_value);
  ~DeweyIdentifier();

  void clear();
  int& at(const int& level);
  const int& at(const int& level) const;
  void set(const int& value, const int& level);
  void push_back(const int& value);
  void pop_back();
  void pop_back_n(const int& number_of_levels);
  bool is_prefix(const DeweyIdentifier& other) const;
  bool is_predecessor(const DeweyIdentifier& other) const;
  int length() const;
  bool empty() const;
  const int& back() const;
  const int size() const;

  bool operator<(const DeweyIdentifier& other) const;
  bool operator==(const DeweyIdentifier& other) const;

  friend std::ostream& operator<<(std::ostream& os, const DeweyIdentifier& dewey);
};

DeweyIdentifier::DeweyIdentifier() { }

DeweyIdentifier::DeweyIdentifier(const int& value) {
  if (id_.size() > 0) {
    clear();
  }

  id_.push_back(value);
}

DeweyIdentifier::DeweyIdentifier(const DeweyIdentifier& other) {
  //id_.resize(other.id_.size());
  //std::copy(other.id_.begin(), other.id_.end(), id_.begin());
  id_.insert(id_.end(), other.id_.begin(), other.id_.end());
}

DeweyIdentifier::DeweyIdentifier(const DeweyIdentifier& other,
  const int& last_value) : DeweyIdentifier(other)
{
  id_.push_back(last_value);
}

DeweyIdentifier::~DeweyIdentifier() { }

void DeweyIdentifier::clear() {
  id_.clear();
}

int& DeweyIdentifier::at(const int& level) {
  try {
    return id_.at(level);
  } catch(std::exception& e) {
    throw;
  }
}

const int& DeweyIdentifier::at(const int& level) const {
  try {
    return id_.at(level);
  } catch(std::exception& e) {
    throw;
  }
}

void DeweyIdentifier::set(const int& value, const int& level) {
  try {
    id_.at(level) = value;
  } catch(std::exception& e) {
    throw;
  }
}

void DeweyIdentifier::push_back(const int& value) {
  id_.push_back(value);
}

void DeweyIdentifier::pop_back() {
  id_.pop_back();
}

void DeweyIdentifier::pop_back_n(const int& number_of_levels) {
  id_.resize(id_.size() - number_of_levels);
}

bool DeweyIdentifier::is_prefix(const DeweyIdentifier& other) const {
  if (id_.size() >= other.id_.size()) {
    return false;
  }

  // id_.size() < other.id_.size()
  std::vector<int>::const_iterator tcit, ocit;
  for ( tcit = id_.cbegin(), ocit = other.id_.cbegin(); tcit != id_.cend();
        ++tcit, ++ocit)
  {
    //std::cout << *tcit << ", " << *ocit << std::endl;
    if (*tcit != *ocit) {
      //std::cout << *tcit << " != " << *ocit << std::endl;
      return false;
    }
  }

  return true;
}

bool DeweyIdentifier::is_predecessor(const DeweyIdentifier& other) const {
  // other is a descendant of this iff did(this) is a prefix of did(other)
  // as a consequence, this is a predecessor of other if this holds
  return is_prefix(other);
}

int DeweyIdentifier::length() const {
  return id_.size();
}

bool DeweyIdentifier::empty() const {
  return id_.empty();
}

const int& DeweyIdentifier::back() const {
  return id_.back();
}

const int DeweyIdentifier::size() const {
  return std::move(id_.size());
}

bool DeweyIdentifier::operator<(const DeweyIdentifier& other) const {
  return (id_ < other.id_);
}

bool DeweyIdentifier::operator==(const DeweyIdentifier& other) const {
  return (id_ == other.id_);
}

/// friend functions
std::ostream& operator<<(std::ostream& os, const DeweyIdentifier& dewey) {
  auto it = dewey.id_.cbegin();
  for (; it != std::prev(dewey.id_.cend()); ++it) {
    os << *it << ".";
  }
  os << *it;

  return os;
}

} // namespace data_structures

#endif // DATA_STRUCTURES_DEWEY_IDENTIFIER_H