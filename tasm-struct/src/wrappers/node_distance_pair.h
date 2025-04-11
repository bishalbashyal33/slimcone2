#ifndef NODE_DISTANCE_PAIR_H
#define NODE_DISTANCE_PAIR_H

namespace wrappers {

// TODO: Make node_ of type _NodeData and adapt get_node/constr. accordingly

template<class _Data = int>
class NodeDistancePair {
private:
  _Data data_;
  double distance_;

public:
  NodeDistancePair();
  NodeDistancePair(const _Data& data, const double distance);
  ~NodeDistancePair();

  const double get_distance() const;
  const _Data get_data() const;

  bool operator<(const NodeDistancePair& other) const;
  bool operator>(const NodeDistancePair& other) const;
  bool operator==(const NodeDistancePair& other) const;
  bool operator>=(const NodeDistancePair& other) const;
  bool operator<=(const NodeDistancePair& other) const;
};

template<class _Data>
NodeDistancePair<_Data>::NodeDistancePair() { }

template<class _Data>
NodeDistancePair<_Data>::NodeDistancePair(const _Data& data,
  const double distance) : data_(data), distance_(distance) { }

template<class _Data>
NodeDistancePair<_Data>::~NodeDistancePair() { }

template<class _Data>
const double NodeDistancePair<_Data>::get_distance() const {
  return distance_;
}

template<class _Data>
const _Data NodeDistancePair<_Data>::get_data() const {
  return data_;
}

template<class _Data>
bool NodeDistancePair<_Data>::operator<(const NodeDistancePair& other) const
{
  if (distance_ < other.get_distance()) {
    return true;
  } else if (distance_ == other.get_distance()) {
    return (data_ < other.get_data());
  }

  return false;
}

template<class _Data>
bool NodeDistancePair<_Data>::operator>(const NodeDistancePair& other) const
{
  if (distance_ > other.get_distance()) {
    return true;
  } else if (distance_ == other.get_distance()) {
    return (data_ > other.get_data());
  }

  return false;
}

template<class _Data>
bool NodeDistancePair<_Data>::operator==(const NodeDistancePair& other) const
{
  return ((distance_ == other.get_distance()) && (data_ == other.get_data()));
}

template<class _Data>
bool NodeDistancePair<_Data>::operator>=(const NodeDistancePair& other) const
{
  return ((*this > other) || (*this == other));
}

template<class _Data>
bool NodeDistancePair<_Data>::operator<=(const NodeDistancePair& other) const
{
  return ((*this < other) || (*this == other));
}

} // namespace wrappers

#endif // NODE_DISTANCE_PAIR_H