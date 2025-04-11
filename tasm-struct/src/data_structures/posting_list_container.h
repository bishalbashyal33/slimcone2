#ifndef DATA_STRUCTURES_POSTING_LIST_CONTAINER_H
#define DATA_STRUCTURES_POSTING_LIST_CONTAINER_H

#include <vector>
#include <utility>

namespace data_structures {

// TODO: replace std::map<int, int> with std::set<std::pair<int, int>>
//        but entries of std::set are const -> modifying them is impossible :(
template<class _ListType = std::map<int, int>>
class PostingListContainer {
public:
  using ListTypeRefWrapper = std::reference_wrapper<const _ListType>;
  using EntryType = typename std::pair<int, typename _ListType::value_type>;
  using ListTypeConstIter = typename _ListType::const_iterator;
  using ListTypeConstIterPair = std::pair<ListTypeConstIter, ListTypeConstIter>;

private:
  std::vector<std::pair<int, ListTypeRefWrapper>> posting_lists_;
  size_t number_of_list_entries_;
  std::vector<ListTypeConstIterPair> bounds_{};

public:
  PostingListContainer();
  PostingListContainer(const int size);
  ~PostingListContainer();

  void push_back(const int data_id, const _ListType& posting_list);
  EntryType next();
  void update_bounds();

  bool empty() const;
  size_t size() const;
  void reserve(const int size);

  inline const bool is_empty(const ListTypeConstIterPair& iter_pair) const;
};

template<class _ListType>
PostingListContainer<_ListType>::PostingListContainer()
    : number_of_list_entries_(0)
{ }

template<class _ListType>
PostingListContainer<_ListType>::PostingListContainer(const int size)
    : PostingListContainer() {
  posting_lists_.reserve(size);
}

template<class _ListType>
PostingListContainer<_ListType>::~PostingListContainer() { }

template<class _ListType>
void PostingListContainer<_ListType>::push_back(
    const int data_id,
    const _ListType& posting_list) {

  try {
    posting_lists_.push_back(std::move(std::make_pair(data_id, std::ref(posting_list))));
    number_of_list_entries_ += posting_list.size();

    // one could think bounds can be computed here whenever an insertion takes
    // place, but no! push_back invalidates iterators if a reallocation happens
    // (and this may be the case whenever we do not know the sizes upfront).
  } catch(std::exception& e) {
    throw;
  }
}

template<class _ListType>
typename PostingListContainer<_ListType>::EntryType PostingListContainer<_ListType>::next()
{
  if (empty()) {
    // TODO: throw exception
    return std::move(EntryType{});
  }

  if (bounds_.empty()) {
    update_bounds();
  }

  auto bounds_it = bounds_.cbegin();
  while (is_empty(*bounds_it) && (bounds_it != bounds_.cend())) {
    ++bounds_it;
  }

  auto list_min_it = bounds_it;
  for (; bounds_it != bounds_.cend(); ++bounds_it) {
    if (!is_empty(*bounds_it) && (*(bounds_it->first) < *(list_min_it->first))) {
      list_min_it = bounds_it;
    }
  }

  const int min_index = list_min_it - bounds_.cbegin();

  auto list_min = std::move(*(list_min_it->first));
  std::advance(bounds_.at(min_index).first, 1);
  --number_of_list_entries_;

  return std::move(std::make_pair(posting_lists_.at(min_index).first, list_min));
}

template<class _ListType>
void PostingListContainer<_ListType>::update_bounds() {
  for (const auto& pair: posting_lists_) {
    bounds_.push_back(std::move(std::make_pair(
      pair.second.get().cbegin(),
      pair.second.get().cend()
    )));
  }
}

template<class _ListType>
bool PostingListContainer<_ListType>::empty() const {
  return (number_of_list_entries_ <= 0);
}

template<class _ListType>
size_t PostingListContainer<_ListType>::size() const {
  return posting_lists_.size();
}

template<class _ListType>
void PostingListContainer<_ListType>::reserve(const int size) {
  posting_lists_.reserve(size);
}

template<class _ListType>
inline const bool PostingListContainer<_ListType>::is_empty(
    const ListTypeConstIterPair& iter_pair) const {
  return (iter_pair.first == iter_pair.second);
}

} // namespace data_structures

#endif // DATA_STRUCTURES_POSTING_LIST_CONTAINER_H