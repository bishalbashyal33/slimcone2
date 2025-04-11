#ifndef TASM_TASM_POSTORDER_H
#define TASM_TASM_POSTORDER_H

#include <vector>
#include <queue>
#include <utility>
#include <map>

#include "k_heap.h"
#include "node_distance_pair.h"

namespace tasm {

template<class _NodeData = nodes::StringNodeData, class _Costs = nodes::StringCosts<_NodeData>>
class TASMPostorder : public interfaces::tasm::PostorderQueueInterface<_NodeData>
{
private:
  interfaces::tasm::PostorderSourceInterface<_NodeData>* postorder_source_{};
  data_structures::RingBufferTree<_NodeData>* document_stream_{};
  data_structures::TASMTree<_NodeData>& query_;
  int k_{};
  data_structures::KHeap<wrappers::NodeDistancePair<int>> ranking_{};
  std::vector<int> query_kr_{};
  int compute_next_{};
  int appended_{};
  data_structures::Array2D<double> pd_{}; // forest distances
  data_structures::Array2D<double> td_{}; // tree distances
  TASMDynamic<_NodeData, _Costs, data_structures::TASMTree, data_structures::RingBufferTree> tasm_dynamic_;
  //std::vector<int> query_data_ids_{}; // TASMPostorder+
  unsigned int verificationstotal_{};
  unsigned int verificationsduplicates_{};
  std::map<data_structures::RingBufferTree<_NodeData>, int> already_verified_{};

public:
  TASMPostorder() = delete;
  TASMPostorder(
    data_structures::TASMTree<_NodeData>& query_,
    interfaces::tasm::PostorderSourceInterface<_NodeData>* postorder_source);
  virtual ~TASMPostorder();

  virtual void append(_NodeData data, const int subtree_size = 1) override;

  data_structures::KHeap<wrappers::NodeDistancePair<int>> compute_tasm(
    const int& k);
  void compute_subtree_tasm(int subtree_root);
  void resize_ring_buffer();
  inline unsigned int verificationstotal() const;
  inline unsigned int verificationsduplicates() const;
};

template<class _NodeData, class _Costs>
TASMPostorder<_NodeData, _Costs>::TASMPostorder(
    data_structures::TASMTree<_NodeData>& query,
    interfaces::tasm::PostorderSourceInterface<_NodeData>* postorder_source)
    : postorder_source_(postorder_source),
      query_(query) {
  verificationstotal_ = 0;
  /*
  // TASMPostorder+
  std::vector<int> query_data_ids_temp = query_.data_ids_; // copy
  std::sort(query_data_ids_temp.begin(), query_data_ids_temp.end());
  query_data_ids_.resize(query_data_ids_temp.back() + 1);
  for (const int data_id: query_data_ids_temp) {
    ++query_data_ids_.at(data_id);
  }*/
}

template<class _NodeData, class _Costs>
TASMPostorder<_NodeData, _Costs>::~TASMPostorder() {}

template<class _NodeData, class _Costs>
data_structures::KHeap<wrappers::NodeDistancePair<int>> TASMPostorder<_NodeData, _Costs>::compute_tasm(
    const int& k) {

  if (k == 0) {
    return std::move(ranking_);
  }

  appended_ = 0;
  k_ = k;

  int temp_max_subtree_size = round(ceil(2 * query_.node_count() + k));

  std::vector<int> lmlds(temp_max_subtree_size);
  std::vector<int> data_ids(temp_max_subtree_size);
  std::vector<_NodeData> data(temp_max_subtree_size);

  document_stream_ = new data_structures::RingBufferTree<_NodeData>(
    temp_max_subtree_size, query_.label_map(), lmlds, data, data_ids
  );
  ranking_.resize(k);

  query_kr_ = query_.kr();

  pd_.set_dimensions(query_.node_count() + 1, document_stream_->size() + 1);
  td_.set_dimensions(query_.node_count() + 1, document_stream_->size() + 1);
  compute_next_ = 1;

  postorder_source_->append_to(this);
  compute_subtree_tasm(document_stream_->subtree_root());

  delete document_stream_;

  return std::move(ranking_);
}

template<class _NodeData, class _Costs>
void TASMPostorder<_NodeData, _Costs>::append(
    _NodeData data,
    const int subtree_size) {

  if ((document_stream_->node_count() >= document_stream_->size()) &&
      (compute_next_ == document_stream_->smallest_valid_id())) {

    // node at compute_next_ is leaf
    if (document_stream_->lmld(compute_next_) == compute_next_) {
      const int next_subtree_root = document_stream_->leaf_kr(compute_next_);

      compute_subtree_tasm(next_subtree_root);

      compute_next_ += next_subtree_root - compute_next_ + 1;
    } else {
      ++compute_next_;
    }
  }

  document_stream_->append(std::move(data), subtree_size);
  resize_ring_buffer();
}

template<class _NodeData, class _Costs>
void TASMPostorder<_NodeData, _Costs>::resize_ring_buffer() {
  ++appended_;
  if (appended_ == k_) {
    int max_subtree_size = round(ceil(query_.node_count()
      + query_.node_count() * query_.max_costs()
      + k_ * document_stream_->max_costs()
    ));

    if (max_subtree_size != document_stream_->size()) {
      document_stream_->resize(max_subtree_size);
    }
  }
}

template<class _NodeData, class _Costs>
inline unsigned int TASMPostorder<_NodeData, _Costs>::verificationstotal() const
{
  return verificationstotal_;
}

template<class _NodeData, class _Costs>
inline unsigned int TASMPostorder<_NodeData, _Costs>::verificationsduplicates() const
{
  return verificationsduplicates_;
}

template<class _NodeData, class _Costs>
void TASMPostorder<_NodeData, _Costs>::compute_subtree_tasm(int subtree_root) {
  int u = document_stream_->size() - query_.node_count();
  while (subtree_root >= compute_next_) {
    const int subtree_size = subtree_root - document_stream_->lmld(subtree_root) + 1;
    const int l = std::abs(subtree_size - query_.node_count());

    if ((l <= u) &&
        (!ranking_.full() || (l < ranking_.front().get_distance()))) {

      data_structures::RingBufferTree<_NodeData> subtree =
        std::move(document_stream_->subtree(subtree_root));

      // auto it = already_verified_.find(subtree);
      // if (it != already_verified_.end()) {
      //   ++verificationsduplicates_;
      //   return it->second;
      // }

      std::vector<int> subtree_kr = std::move(subtree.kr());

      // gather labels query, gather labels subtree (only those!)
      // intersection labels
      // lower bound = |Q| - |label intersection|
      // |Q| - |label intersection| > ranking.front().get_distance() => continue
      // otherwise, skip subtree and continue ring buffer examination
      /*
      // TASMPostorder+
      if (ranking_.full()) {
        // we assumne that we can prune and revert this assumption if necessary
        bool prune = true;

        // we start with |Q| and decrement it for each matched label
        // if this is still > max(R) after all labels of the subtree have been
        // considered, we can prune the subtree
        int remaining_nodes = query_.node_count();

        // pre-store some values to avoid unnecessary lookups in every iteration
        const int root_node_index = subtree_kr.size() - 1;
        const int root_node = subtree_kr.at(root_node_index);
        const int max_distance = ranking_.front().get_distance();
        const auto& label_map = query_.label_map();

        auto query_data_ids_temp = query_data_ids_;

        // traverse subtree label ids and match label ids with query label ids
        for (int dd = subtree.lmld(root_node); dd <= root_node; ++dd) {
          if (!label_map.contains(subtree.data(dd))) {
            continue;
          }

          const int data_id = label_map.at(subtree.data(dd));
          if (data_id < query_data_ids_temp.size() && query_data_ids_temp.at(data_id) > 0) {
            --query_data_ids_temp.at(data_id);
            --remaining_nodes;

            // once |Q| - x_i <= max(R), we are not allowed to prune the subtree,
            // i.e., x is the no. of matches after the i-th label of the subtree
            if (remaining_nodes <= max_distance) {
              prune = false;
              break;
            }
          }
        }

        if (prune) {
          subtree_root -= subtree.node_count();
          continue;
        }
      }*/

      for (size_t d = 1; d < subtree_kr.size(); ++d) {
        int computed_prefix_size = -1;
        for (size_t q = 1; q < query_kr_.size(); ++q) {
          computed_prefix_size = std::max(computed_prefix_size,
            tasm_dynamic_.prefix_distance(
              query_kr_.at(q), subtree_kr.at(d), query_, subtree, pd_,
              td_, ranking_
            )
          );
        }
      }

      ++verificationstotal_;

      subtree_root -= subtree.node_count();
    } else {
      --subtree_root;
    }
  }
}

} // namespace tasm

#endif // TASM_TASM_POSTORDER_H
