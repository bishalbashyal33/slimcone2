#ifndef ZHANG_SHASHA_H
#define ZHANG_SHASHA_H

#include <vector>
#include <algorithm>

#include "node.h"
#include "string_node_data.h"
#include "common.h"
#include "array_2d.h"

//  TODO:
//  - Update documentation (kr)

namespace zhang_shasha {

//Computes the left most leaf descendant for all subtrees of the input tree
//
//Param: root of the tree, a vector which stores the left most leaf descendant for each subtree
template<class _NodeData = nodes::StringNodeData>
int lmld(nodes::Node<_NodeData>* root, int& id, std::vector<int>& l) {
  int lml = 0;
  // call lmld recursively and compute the left-most-leaf descendants
  for (int i = 0; i < root->get_children_number(); ++i) {
    if (i == 0) {
      lml = lmld<_NodeData>(root->get_children().at(i), id, l);
    } else {
      lmld<_NodeData>(root->get_children().at(i), id, l);
    }
  }
  
  ++id;
  if (root->get_children_number() == 0) {
    // is leaf
    l.at(id) = id;
    return id;
  }
  
  // no leaf
  l.at(id) = lml;
  return lml; // used for the root node!
}

//Computes the key-roots of a specific tree. A key-root is either the root or a node with a left sibling.
//
//Param: the left-most leaf descendats for all subtrees of the input-tree, the number of leaves of the input-tree
//
//Return: A vector storing all key-roots of the input-tree
void kr(std::vector<int>& l, std::vector<int>& kr) {
  std::vector<int> visit(l.size(), 0); // was node visited

  int k = kr.size() - 1;
  int i = l.size() - 1;
  while (k >= 1) {
    if (visit.at(l.at(i)) == 0) {
      kr.at(k--) = i;
      visit.at(l.at(i)) = 1;
    }
    i -= 1;
  }
}

template<class _NodeData = nodes::StringNodeData>
void set_leaves(std::vector<nodes::Node<_NodeData>*>* leaves_tree1,
  std::vector<nodes::Node<_NodeData>*>* leaves_tree2, nodes::Node<_NodeData>* root,
  int which_tree)
{  
  if (which_tree == 1) {
    if (root) {
      if (root->get_children_number() > 0) {
        for (int i = 0; i < root->get_children_number(); ++i) {
          set_leaves<_NodeData>(leaves_tree1, leaves_tree2, root->get_child(i), 1);
        }
      } else {
        leaves_tree1->push_back(root);
      }
    }
  } else {
    if ((root != nullptr) && (root->get_children_number() > 0)) {
      for (int i = 0; i < root->get_children_number(); ++i) {
        set_leaves<_NodeData>(leaves_tree1, leaves_tree2, root->get_child(i), 2);
      }
    } else {
      leaves_tree2->push_back(root);
    }
  }
}

// Computes for each input-tree a vector consisting of all leaves of the specific
// tree.
//
// Parameters: left-hand tree, right-hand tree
template<class _NodeData = nodes::StringNodeData>
void make_leaves(std::vector<nodes::Node<_NodeData>*>* leaves_tree1,
  std::vector<nodes::Node<_NodeData>*>* leaves_tree2, nodes::Node<_NodeData>* tree1,
  nodes::Node<_NodeData>* tree2)
{
  // TODO: remove which_tree from set_leaves
  set_leaves<_NodeData>(leaves_tree1, leaves_tree2, tree1, 1);
  set_leaves<_NodeData>(leaves_tree1, leaves_tree2, tree2, 2);
}

// Computes the forest-distance of:
// T1[l(i)...di], where di is element of desc(T1[i]) and
// T2[l(j)...dj], where dj is element of desc(T2[j])
//
// Param: i, j defined as above and a optional cost-model
template<class _NodeData = nodes::StringNodeData, class _costs = nodes::Costs<_NodeData>>
void forest_dist(std::vector<nodes::Node<_NodeData>*>& tree1_postorder,
  std::vector<nodes::Node<_NodeData>*>& tree2_postorder, int i, int j,
  data_structures::Array2D<double>& fd, data_structures::Array2D<double>& td,
  std::vector<int>& l1, std::vector<int>& l2, _costs costs = _costs())
{
  fd[l1.at(i) - 1][l2.at(j) - 1] = 0;

  int di = 0, dj = 0;
  for (di = l1.at(i); di <= i; ++di) {
    fd[di][l2.at(j) - 1] = fd[di - 1][l2.at(j) - 1] + costs.del(); // TODO replace del()
  }
 
  for (dj = l2.at(j); dj <= j; ++dj) {
    fd[l1.at(i) - 1][dj] = fd[l1.at(i) - 1][dj - 1] + costs.ins(); // TODO replace ins()
  }

  for (di = l1.at(i); di <= i; ++di) {
    for (dj = l2.at(j); dj <= j; ++dj) {
      if (l1.at(di) == l1.at(i) && l2.at(dj) == l2.at(j)) {
        fd[di][dj] = std::min(
          std::min(fd[di - 1][dj] + costs.del(), fd[di][dj - 1] + costs.ins()), // TODO replace del(), ins()
          fd[di - 1][dj - 1] + costs.ren(tree1_postorder.at(di - 1)->get_data(), tree2_postorder.at(dj - 1)->get_data())
        );

        td[di][dj] = fd[di][dj];
      } else {
        fd[di][dj] = std::min(
          std::min(fd[di - 1][dj] + costs.del(), fd[di][dj - 1] + costs.ins()), // TODO replace del(), ins()
          fd[l1.at(di) - 1][l2.at(dj) - 1] + td[di][dj]
        );
      }
    }
  }
}

/*
// TODO: remove get_id calls!
//
// Prints the given edit mapping
//
// Params:  edm     the edit mapping
template<class _node = nodes::StringNode>
void print_pretty_edit_mapping(
  std::vector<std::array<nodes::Node<_node>>*, 2>> edit_mapping)
{
  typename std::array<nodes::Node<_node>*, 2> em;
  typename std::vector<std::array<nodes::Node_node>*, 2>>::iterator it;
  for (it = --edit_mapping.end();
        it >= edit_mapping.begin(); --it)
  {
    em = *it;
    std::cout << "(";
    if (em[0] == nullptr) {
      std::cout << "0";
    } else {
      std::cout << em[0]->get_id();
    }
    std::cout << "->";
    if (em[1] == nullptr) {
      std::cout << "0";
    } else {
      std::cout << em[1]->get_id();
    }
    std::cout << ")" << std::endl;
  }
}

// TODO: remove get_id calls!
//
// "Returns"/Fills a two dimensional integer array for the given edit mapping
//                 array size: 2 x max(nodes_t1, nodes_t2)+1 or 2 x edm.size();
//
// Params:  edm     the edit mapping
//
// "Returns"/Fills: a two dimensional integer array, where arr[0][id] is the
//          mapping for a the node in the first tree (depends on the mapping !)
template<class _node = nodes::StringNode>
void get_edit_mapping_int_array(
  std::vector<std::array<nodes::Node<_node>*, 2>> edit_mapping,
  int** array_to_fill)
{
  std::array<_node*, 2> em;
  typename std::vector<std::array<nodes::Node<_node>*, 2> >::iterator it;
  for (it = --edit_mapping.end();
        it >= edit_mapping.begin(); --it)
  {
    em = *it;
    if (em[0] == nullptr) {
      // insert
      array_to_fill[1][em[1]->get_id()] = 0;
    } else if (em[1] == nullptr) {
      // delete
      array_to_fill[0][em[0]->get_id()] = 0;
    } else {
      array_to_fill[0][em[0]->get_id()] = em[1]->get_id();
      array_to_fill[1][em[1]->get_id()] = em[0]->get_id();
    }
  }
}
*/

// Computes the edit mapping for two trees (optional: custom costs)
//
// Params:  tree1      the root of the first tree
//          tree2      the root of the second tree
//
// Returns: a vector of node* arrays, which shows the mapping for each node,
//          e.g.: node_in_t1  -> node_in_t2   mapping or rename operation
//                nullptr     -> node_in_t2   insert operation
//                node_in_t1  -> nullptr      delete operation
template<class _NodeData = nodes::StringNodeData, class _costs = nodes::Costs<_NodeData>>
std::vector<std::array<nodes::Node<_NodeData>*, 2> > compute_edit_mapping(
  nodes::Node<_NodeData>* tree1, nodes::Node<_NodeData>* tree2, _costs costs = _costs())
{
  std::vector<nodes::Node<_NodeData>*> tree1_postorder;
  std::vector<nodes::Node<_NodeData>*> tree2_postorder;
  // fill vectors
  common::generate_postorder<_NodeData>(tree1, tree1_postorder);
  common::generate_postorder<_NodeData>(tree2, tree2_postorder);

  // left-most-leaf-descendants of tree 1 and 2
  std::vector<int> l1(tree1_postorder.size() + 1);
  std::vector<int> l2(tree2_postorder.size() + 1);

  // leaves of tree 1 and 2
  std::vector<nodes::Node<_NodeData>*> leaves_tree1;
  std::vector<nodes::Node<_NodeData>*> leaves_tree2;

  data_structures::Array2D<double> fd(tree1_postorder->size() + 1, tree2_postorder->size() + 1);
  data_structures::Array2D<double> td(tree1_postorder->size() + 1, tree2_postorder->size() + 1);

  make_leaves<_NodeData>(&leaves_tree1, &leaves_tree2, tree1, tree2);

  int id = 0;
  lmld<_NodeData>(tree1, id, l1);
  id = 0;
  lmld<_NodeData>(tree2, id, l2);

  std::vector<int> kr1(leaves_tree1.size() + 1);
  std::vector<int> kr2(leaves_tree2.size() + 1);
  kr(l1, kr1);
  kr(l2, kr2);

  //compute the distance
  for ( std::vector<int>::iterator kr1_it = std::next(kr1.begin());
        kr1_it != kr1.end(); ++kr1_it)
  {
    for ( std::vector<int>::iterator kr2_it = std::next(kr2.begin());
          kr2_it != kr2.end(); ++kr2_it)
    {
      forest_dist<_NodeData, _costs>(tree1_postorder, tree2_postorder, *kr1_it, *kr2_it, fd, td, l1, l2, costs);
    }
  }
 
  typename std::vector<std::array<int, 2> > tree_pairs;
  typename std::vector<std::array<nodes::Node<_NodeData>*, 2> > edit_mapping;
  tree_pairs.push_back({ tree1->get_subtree_size(true), tree2->get_subtree_size(true) });
  std::array<int, 2> tree_pair;
  bool root_node_pair = true;

  while (!tree_pairs.empty()) {
    tree_pair = tree_pairs.back();
    tree_pairs.pop_back();

    int last_row = tree_pair.at(0);
    int last_col = tree_pair.at(1);

    if (!root_node_pair) {
      forest_dist(tree1_postorder, tree2_postorder, last_row, last_col, costs);
    }
    root_node_pair = false;

    int first_row = l1.at(last_row) - 1;
    int first_col = l2.at(last_col) - 1;
    int row = last_row;
    int col = last_col;

    while ((row > first_row) || (col > first_col)) {
      int cost_delete = costs.del(); // TODO replace del()
      int cost_insert = costs.ins(); // TODO replace ins()

      if ((row > first_row) && (fd[row - 1][col] + cost_delete == fd[row][col]))
      {
        edit_mapping.push_back({ tree1_postorder->at(row-1), nullptr });
        --row;
      } else if ( (col > first_col)
                  && (fd[row][col - 1] + cost_insert == fd[row][col]))
      {
        edit_mapping.push_back({ nullptr, tree2_postorder->at(col - 1) });
        --col;
      } else {
        if (l1.at(row) == l1.at(last_row)
            && l2.at(col) == l2.at(last_col))
        {  
          edit_mapping.push_back(
            { tree1_postorder->at(row - 1), tree2_postorder->at(col - 1) }
          );
          --row;
          --col;
        } else {
          tree_pairs.push_back({ row, col });
          row = l1.at(row) - 1;
          col = l2.at(col) - 1;
        }
      }
    }
  }

  delete tree1_postorder;
  delete tree2_postorder;

  return edit_mapping;
}

// Generic function to compute the distance between two trees under a specified
// cost model. Each tree is represented by its respective root node. A root node
// type is specified as first template parameter, the cost model type as second
// template parameter.
template<class _NodeData = nodes::StringNodeData, class _costs = nodes::Costs<_NodeData>>
double compute_zhang_shasha(nodes::Node<_NodeData>* tree1, nodes::Node<_NodeData>* tree2,
  _costs costs = _costs())
{
  // TODO: instead of running generate_postorder we could integrate this with
  // the lmld function

  std::vector<nodes::Node<_NodeData>*> tree1_postorder;
  std::vector<nodes::Node<_NodeData>*> tree2_postorder;
  // fill vectors
  common::generate_postorder<_NodeData>(tree1, tree1_postorder);
  common::generate_postorder<_NodeData>(tree2, tree2_postorder);

  // left-most-leaf-descendants of tree 1 and 2
  std::vector<int> l1(tree1_postorder.size() + 1);
  std::vector<int> l2(tree2_postorder.size() + 1);

  // leaves of tree 1 and 2
  std::vector<nodes::Node<_NodeData>*> leaves_tree1;
  std::vector<nodes::Node<_NodeData>*> leaves_tree2;

  data_structures::Array2D<double> fd(tree1_postorder.size() + 1,
    tree2_postorder.size() + 1
  );
  data_structures::Array2D<double> td(tree1_postorder.size() + 1,
    tree2_postorder.size() + 1
  );

  make_leaves<_NodeData>(&leaves_tree1, &leaves_tree2, tree1, tree2);

  int id = 0;
  lmld<_NodeData>(tree1, id, l1);
  id = 0;
  lmld<_NodeData>(tree2, id, l2);

  std::vector<int> kr1(leaves_tree1.size() + 1);
  std::vector<int> kr2(leaves_tree2.size() + 1);
  kr(l1, kr1);
  kr(l2, kr2);

  //compute the distance
  for ( std::vector<int>::iterator kr1_it = std::next(kr1.begin());
        kr1_it != kr1.end(); ++kr1_it)
  {
    for ( std::vector<int>::iterator kr2_it = std::next(kr2.begin());
          kr2_it != kr2.end(); ++kr2_it)
    {
      forest_dist<_NodeData, _costs>(tree1_postorder, tree2_postorder, *kr1_it,
        *kr2_it, fd, td, l1, l2, costs
      );
    }
  }

  return td[tree1_postorder.size()][tree2_postorder.size()];
}

} // namespace zhang_shasha

#endif // ZHANG_SHASHA_H
