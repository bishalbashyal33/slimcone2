#ifndef PARSER_H
#define PARSER_H

#include "node.h"
#include "string_node_data.h"

namespace parser {

typedef std::unordered_map<std::string, int> LabelIDMap;

template<class _NodeData = nodes::StringNodeData>
nodes::Node<_NodeData>* create_tree_from_string(char* str,
  LabelIDMap& hashtable, int& labelid)
{ 
  int length = std::strlen(str);
  int scope = -1;
   
  std::vector<nodes::Node<_NodeData>*> scope_parent_list;
  typename std::vector<nodes::Node<_NodeData>*>::const_iterator it;
  std::string label = "";
    
  nodes::Node<_NodeData>* root = new nodes::Node<_NodeData>(new _NodeData());
  for (int i = 0; i < length; i++) {
    if (i != 0 && scope <= -1) {
      break;
    }

    if (str[i] == '{') {
      if (label.length() != 0) {
        if (!hashtable.count(label)) {
          hashtable.emplace(label, labelid++);
        }
        
        nodes::Node<_NodeData>* tmp_node = new nodes::Node<_NodeData>(
          new _NodeData(label)
        );
        scope_parent_list.push_back(tmp_node);
        
        if (scope > 0) {
          it = scope_parent_list.begin() + scope_parent_list.size() - 2;
          //_NodeData<_NodeData>* tmp = *it;
          //std::cout << label << ".parent_id:" << tmp->get_label_id() << " scope: " << scope << std::endl;
          //tmp->add_child(tmp_node);
          (*it)->add_child(tmp_node);
        } else {
          delete root;
          root = tmp_node;
          //std::cout << label << ".parent_id:/" << " scope: " << scope << std::endl;
        }
        
        label = "";
      }
      ++scope;
    } else if (str[i] == '}') {
      if (label.length() != 0) {
        if (!hashtable.count(label)) {
          hashtable.emplace(label, labelid++);
        }

        nodes::Node<_NodeData>* tmp_node = new nodes::Node<_NodeData>(
          new _NodeData(label)
        );
        scope_parent_list.push_back(tmp_node);
        
        if (scope > 0 && scope_parent_list.size() > 1) {
          it = scope_parent_list.begin() + scope_parent_list.size() - 2;
          //nodes::Node<_NodeData>* tmp = *it;
          //std::cout << label << ".parent_id:" << tmp->get_label_id() << " scope: " << scope << std::endl;
          //tmp->add_child(tmpnode);
          (*it)->add_child(tmp_node);
        } else {
          delete root;
          root = tmp_node;
          //std::cout << label << ".parent_id:/" << " scope: " << scope << std::endl;
        }
        
        label = "";
      }
      scope_parent_list.resize(scope);
      --scope;
    } else {
      label += str[i];
    }
  }
  return root;
}

} // namespace parser

#endif // PARSER_H
