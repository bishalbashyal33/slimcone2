#ifndef TASM_TASM_DYNAMIC_H
#define TASM_TASM_DYNAMIC_H

namespace tasm {

template<class _NodeData, class _Costs, template<class...> class _QueryTreeType, template<class...> class _DocumentTreeType>
class TASMDynamic {
private:
  data_structures::Array2D<double> pd_{}; // forest distances
  data_structures::Array2D<double> td_{}; // tree distances

public:
  TASMDynamic();
  ~TASMDynamic();

  data_structures::KHeap<wrappers::NodeDistancePair<int>> compute_tasm(
    _QueryTreeType<_NodeData>& query_,
    _DocumentTreeType<_NodeData>& document_,
    const int k = 0);
  
  const int prefix_distance(
    const int q,
    const int d,
    const _QueryTreeType<_NodeData>& query,
    const _DocumentTreeType<_NodeData>& subtree,
    data_structures::Array2D<double>& pd,
    data_structures::Array2D<double>& td,
    data_structures::KHeap<wrappers::NodeDistancePair<int>>& ranking,
    const int edit_bound = (-1));
};

template<class _NodeData, class _Costs, template<class...> class _QueryTreeType, template<class...> class _DocumentTreeType>
TASMDynamic<_NodeData, _Costs, _QueryTreeType, _DocumentTreeType>::TASMDynamic() { }

template<class _NodeData, class _Costs, template<class...> class _QueryTreeType, template<class...> class _DocumentTreeType>
TASMDynamic<_NodeData, _Costs, _QueryTreeType, _DocumentTreeType>::~TASMDynamic()
{ }

template<class _NodeData, class _Costs, template<class...> class _QueryTreeType, template<class...> class _DocumentTreeType>
data_structures::KHeap<wrappers::NodeDistancePair<int>> TASMDynamic<_NodeData, _Costs, _QueryTreeType, _DocumentTreeType>::compute_tasm(
    _QueryTreeType<_NodeData>& query,
    _DocumentTreeType<_NodeData>& document,
    const int k) {

  data_structures::KHeap<wrappers::NodeDistancePair<int>> ranking(k);

  if (k <= 0) {
    return std::move(ranking);
  }

  std::vector<int> query_kr = query.kr();
  std::vector<int> document_kr = document.kr();

  pd_.set_dimensions(query.node_count() + 1, document.node_count() + 1);
  td_.set_dimensions(query.node_count() + 1, document.node_count() + 1);

  for (size_t d = 1; d < document_kr.size(); ++d) {
    int computed_prefix_size = -1;
    for (size_t q = 1; q < query_kr.size(); ++q) {
      computed_prefix_size = std::max(computed_prefix_size, 
        prefix_distance(query_kr.at(q), document_kr.at(d), std::move(query),
          document, pd_, td_, ranking
        )
      );
    }
  }

  return std::move(ranking);
}

template<class _NodeData, class _Costs, template<class...> class _QueryTreeType, template<class...> class _DocumentTreeType>
const int TASMDynamic<_NodeData, _Costs, _QueryTreeType, _DocumentTreeType>::prefix_distance(
    const int q,
    const int d,
    const _QueryTreeType<_NodeData>& query,
    const _DocumentTreeType<_NodeData>& subtree,
    data_structures::Array2D<double>& pd,
    data_structures::Array2D<double>& td,
    data_structures::KHeap<wrappers::NodeDistancePair<int>>& ranking,
    const int edit_bound) {

  _Costs costs;
  pd[query.lmld(q) - 1][subtree.lmld(d) - 1] = 0;
  int document_prefix_size = -1;

  //std::cout << "dd[" << subtree.lmld(d) << "; " << d << "]\n";
  //std::cout << "dq[" << query.lmld(q) << "; " << q << "]" << std::endl;

  for (int dd = subtree.lmld(d); dd <= d; ++dd) {
    const _NodeData& document_data = subtree.data(dd);
    double insert_costs = costs.ins(&document_data);

    pd[query.lmld(q) - 1][dd] = pd[query.lmld(q) - 1][dd - 1] + insert_costs;

    if (edit_bound < 0) {
      // TASM-only
      document_prefix_size = dd - subtree.lmld(d) + 1;
      double lower_bound = document_prefix_size - query.node_count();

      if (ranking.full() && (lower_bound >= ranking.front().get_distance())) {
        return document_prefix_size;
      }
    }

    for (int dq = query.lmld(q); dq <= q; ++dq) {
      const _NodeData& query_data = query.data(dq);
      double delete_costs = costs.del(&query_data);

      pd[dq][subtree.lmld(d) - 1] =
        pd[dq - 1][subtree.lmld(d) - 1] + delete_costs;

      /*std::cout << "query.lmld(" << dq << ") = " << query.lmld(dq)
        << ", query.lmld(" << q << ") = " << query.lmld(q) << std::endl;
      std::cout << "subtree.lmld(" << dd << ") = " << subtree.lmld(dd)
        << ", subtree.lmld(" << dd << ") = " << subtree.lmld(d) << std::endl;*/

      if ((query.lmld(dq) == query.lmld(q)) &&
          (subtree.lmld(dd) == subtree.lmld(d))) {

        pd[dq][dd] = std::min({
          pd[dq - 1][dd] + delete_costs,
          pd[dq][dd - 1] + insert_costs,
          pd[dq - 1][dd - 1] + costs.ren(&query_data, &document_data)
        });

        td[dq][dd] = pd[dq][dd];
        
        //std::cout << "store [" << dq << "][" << dd << "] = " << td[dq][dd] << std::endl;

        if (edit_bound < 0) {
          // TASM-only
          wrappers::NodeDistancePair<int> candidate(subtree.global_id(dd), td[dq][dd]);

          if ((dq == query.root()) &&
              (!ranking.full() || (candidate < ranking.front()))) {

            if (!ranking.insert(candidate)) {
              ranking.replace_front(candidate);
            }
          }
        }
      } else {
        pd[dq][dd] = std::min({
          pd[dq - 1][dd] + delete_costs,
          pd[dq][dd - 1] + insert_costs,
          pd[query.lmld(dq) - 1][subtree.lmld(dd) - 1] + td[dq][dd]
        });
      }
    }
  }

  return document_prefix_size;
}

}

#endif // TASM_TASM_DYNAMIC_H
