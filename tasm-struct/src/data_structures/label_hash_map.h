#ifndef DATA_STRUCTURES_LABEL_HASH_MAP_H
#define DATA_STRUCTURES_LABEL_HASH_MAP_H

#include <unordered_map>
#include <functional>

#include "hash_functors.h"

namespace data_structures {

template<
  class _NodeData = nodes::StringNodeData,
  class _NodeDataHash = common::hash_functors::StringNodeDataHash,
  template<class...> class _MapContainer = std::unordered_map
>
class LabelHashMap {
private:
  _MapContainer<_NodeData, int, _NodeDataHash> data_to_id_{};
  _MapContainer<int, typename decltype(data_to_id_)::const_iterator> id_to_data_{};
  int next_value_{};

public:
  LabelHashMap();
  ~LabelHashMap();

  int insert(const _NodeData& data);
  const _NodeData& at(const int id) const;
  int at(const _NodeData& data) const;
  int size() const;
  bool contains(const _NodeData& data) const;
  bool contains(const int id) const;
  const _MapContainer<_NodeData, int, _NodeDataHash>& data_to_id() const;
  int size_bytes() const;
  void clear();
  int max_id() const;
};

template<class _NodeData, class _NodeDataHash, template<class...> class _MapContainer>
LabelHashMap<_NodeData, _NodeDataHash, _MapContainer>::LabelHashMap()
  : next_value_(0) {}

template<class _NodeData, class _NodeDataHash, template<class...> class _MapContainer>
LabelHashMap<_NodeData, _NodeDataHash, _MapContainer>::~LabelHashMap() {}

template<class _NodeData, class _NodeDataHash, template<class...> class _MapContainer>
int LabelHashMap<_NodeData, _NodeDataHash, _MapContainer>::insert(
    const _NodeData& data) {
  /*typename _MapContainer<_NodeData, int, _NodeDataHash>::const_iterator it;
  if ((it = data_to_id_.find(data)) == data_to_id_.cend()) {
    // not found
    data_to_id_.insert(std::move(std::make_pair(data, ++next_value_)));
    id_to_data_.insert(std::move(std::make_pair(next_value_, std::move(data))));

    return next_value_;
  }

  return it->second;*/
  int& id_ref = data_to_id_[data];

  if (id_ref) {
    return id_ref;
  }

  id_ref = ++next_value_;
  id_to_data_[next_value_] = data_to_id_.find(data);

  return next_value_;
}

template<class _NodeData, class _NodeDataHash, template<class...> class _MapContainer>
const _NodeData& LabelHashMap<_NodeData, _NodeDataHash, _MapContainer>::at(
    const int id) const {
  try {
    return id_to_data_.at(id)->first;
  } catch (const std::exception& e) {
    throw;
  }
}

template<class _NodeData, class _NodeDataHash, template<class...> class _MapContainer>
int LabelHashMap<_NodeData, _NodeDataHash, _MapContainer>::at(
    const _NodeData& data) const {
  try {
    return data_to_id_.at(data);
  } catch (const std::exception& e) {
    throw;
  }
}

template<class _NodeData, class _NodeDataHash, template<class...> class _MapContainer>
int LabelHashMap<_NodeData, _NodeDataHash, _MapContainer>::size() const {
  return data_to_id_.size();
}

template<class _NodeData, class _NodeDataHash, template<class...> class _MapContainer>
bool LabelHashMap<_NodeData, _NodeDataHash, _MapContainer>::contains(
    const _NodeData& data) const {
  if (data_to_id_.find(data) != data_to_id_.cend()) {
    return true;
  }

  return false;
}

template<class _NodeData, class _NodeDataHash, template<class...> class _MapContainer>
bool LabelHashMap<_NodeData, _NodeDataHash, _MapContainer>::contains(
    const int id) const {
  if (id_to_data_.find(id) != id_to_data_.cend()) {
    return true;
  }

  return false;
}

template<class _NodeData, class _NodeDataHash, template<class...> class _MapContainer>
const _MapContainer<_NodeData, int, _NodeDataHash>& LabelHashMap<_NodeData, _NodeDataHash, _MapContainer>::data_to_id() const
{
  return data_to_id_;
}

template<class _NodeData, class _NodeDataHash, template<class...> class _MapContainer>
int LabelHashMap<_NodeData, _NodeDataHash, _MapContainer>::size_bytes() const {
  return (sizeof(next_value_) +
    + sizeof(data_to_id_) + data_to_id_.size() * (sizeof(_NodeData) + sizeof(int))
    + sizeof(id_to_data_) + id_to_data_.size() * (sizeof(int) + sizeof(_NodeData))
  );
}

template<class _NodeData, class _NodeDataHash, template<class...> class _MapContainer>
void LabelHashMap<_NodeData, _NodeDataHash, _MapContainer>::clear() {
  data_to_id_.clear();
  id_to_data_.clear();
  next_value_ = 0;
}

template<class _NodeData, class _NodeDataHash, template<class...> class _MapContainer>
int LabelHashMap<_NodeData, _NodeDataHash, _MapContainer>::max_id() const {
  return next_value_ - 1;
}

} // namespace data_structures

#endif // DATA_STRUCTURES_LABEL_HASH_MAP_H
