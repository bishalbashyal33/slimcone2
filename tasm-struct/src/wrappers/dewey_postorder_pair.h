#ifndef WRAPPERS_DEWEY_POSTORDER_PAIR_H
#define WRAPPERS_DEWEY_POSTORDER_PAIR_H

#include "dewey_identifier.h"

namespace wrappers {

class DeweyPostorderPair {
private:
  using DeweyId = data_structures::DeweyIdentifier;

private:
  DeweyId dewey_id_;
  int postorder_id_;

public:
  DeweyPostorderPair();
  DeweyPostorderPair(DeweyId dewey_id, const int postorder_id);
  ~DeweyPostorderPair();

  const DeweyId& dewey_id() const;
  const int postorder_id() const;
};

DeweyPostorderPair::DeweyPostorderPair() { }

DeweyPostorderPair::DeweyPostorderPair(DeweyId dewey_id, const int postorder_id)
    : dewey_id_(dewey_id), postorder_id_(postorder_id)
{ }

DeweyPostorderPair::~DeweyPostorderPair() { }

const data_structures::DeweyIdentifier& DeweyPostorderPair::dewey_id() const {
  return dewey_id_;
}

const int DeweyPostorderPair::postorder_id() const {
  return postorder_id_;
}

} // namespace wrappers

#endif // WRAPPERS_DEWEY_POSTORDER_PAIR_H