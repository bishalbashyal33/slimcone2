#ifndef COMMON_HASH_FUNCTORS_H
#define COMMON_HASH_FUNCTORS_H

#include <vector>

#include "string_node_data.h"
#include "dewey_identifier.h"

namespace common {
  
  namespace hash_functors {

  using nodes::StringNodeData;
  using data_structures::DeweyIdentifier;

  // for TASM
  struct StringNodeDataHash {
    std::size_t operator()(const StringNodeData& key) const {
      return std::hash<std::string>()(key.get_label());
    }

    std::size_t operator()(const StringNodeData* key_ptr) const {
      return std::hash<std::string>()(key_ptr->get_label());
    }
  };

  // see boost::hash_combine template function (is equivalent)
  struct IntVectorHash {
    std::size_t operator()(const std::vector<int>& v) const {
      std::size_t seed = v.size();

      for(auto& value : v) {
        seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
      }

      return seed;
    }
  };

  // wrapper of IntVectorHash for Dewey Identifiers
  struct DeweyIdentifierHash {
    std::size_t operator()(const DeweyIdentifier& dewey_id) const {
      return IntVectorHash()(dewey_id.id_);
    }
  };

  } // namespace hash_functors

} // namespace common

#endif // COMMON_HASH_FUNCTORS_H