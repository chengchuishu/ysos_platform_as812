/**
 *@file JsonTree.cpp
 *@brief Definition of JsonTree
 *@version 0.1
 *@author XuLanyue
 *@date Created on: 2016-06-20 17:50:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

#include "../../../protect/include/core_help_package/jsontree.h"
//#include <jsoncpp-src-0.5.0/include/json/json.h>
#include <json/json.h>
#include <sstream>

namespace ysos {
//  静态成员初始化//NOLINT
//JsonTreePtr  JsonTree::s_jsontree_ = NULL;
//boost::shared_ptr<LightLock> JsonTree::s_jsontree_lock_
//  = boost::shared_ptr<LightLock>(new LightLock());
DEFINE_SINGLETON(JsonTree);
JsonTree::JsonTree(const std::string &class_name) :
  BaseInterfaceImpl(class_name) {
  // //YSOS_LOG_DEBUG("JsonTree::JsonTree");  //need update for linux
  root_node_ = NULL;
  root_node_name_ = "CoreFramework";
}

JsonTree::~JsonTree() {
  root_node_ = NULL;}

//  单例//NOLINT
//const JsonTreePtr JsonTree::Instance(void) {
//  if (NULL == s_jsontree_) {
//    AutoLockOper lock(s_jsontree_lock_.get());
//    s_jsontree_ = JsonTreePtr(new JsonTree());
//  }
//  return s_jsontree_;
//}

#if ENABLE_DERIVED_FROM_BASEINTERFACE
int JsonTree::GetProperty(int type_id, void *type) {
  if (NULL == type) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  switch (type_id) {
  case TREE_ROOT_NAME: {
    //AutoLockOper lock(s_jsontree_lock_.get());
    std::string *root_node_name = static_cast<std::string *>(type);
    *root_node_name = root_node_name_;
  }
  break;
  default:
  break;
  }

  return YSOS_ERROR_SUCCESS;
}

int JsonTree::SetProperty(int type_id, void *type) {
  if (NULL == type) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  switch (type_id) {
  case TREE_ROOT_NAME: {
    //AutoLockOper lock(s_jsontree_lock_.get());
    std::string *root_node_name = static_cast<std::string *>(type);
    root_node_name_ = *root_node_name;
  }
  break;
  default:
  break;
  }

  return YSOS_ERROR_SUCCESS;
}
#endif

//  解析//NOLINT
int JsonTree::ParseString(TreeNodeIterator root_node, const std::string &content) {
  int return_value = YSOS_ERROR_SUCCESS;
  do {
    //  参数检测//NOLINT
    if (!root_node || content.length() < 2) {
      return_value = YSOS_ERROR_INVALID_ARGUMENTS;
      YSOS_LOG_DEBUG("ParseString failed for invalid param.");  //need update for linux
      break;
    }

    root_node_ = root_node;

    //  获取解析器并进行解析//NOLINT
    Json::Features features = Json::Features::strictMode();
    Json::Reader reader(features);
    Json::Value root;
    bool parsingSuccessful = reader.parse(content, root, 1);
    if (!parsingSuccessful) {
      return_value = YSOS_ERROR_FAILED;
      YSOS_LOG_DEBUG("ParseString failed for Json parse failed.");  //need update for linux
      break;
    }
    if (root.isNull()) {
      YSOS_LOG_DEBUG("null struct");  //need update for linux
      break;
    }

    if (!ConvertToTreeNode(&root, false, true, root_node_name_)) {
      return_value = YSOS_ERROR_LOGIC_ERROR;
      break;
    }

    //  NextToDo: root_node check
    if (NULL == root_node) {
      return_value = YSOS_ERROR_LOGIC_ERROR;
      break;
    }
#if 1
    if (root.isArray()) {
      Json::Value::Members names;
#if ENABLE_DERIVED_FROM_BASEINTERFACE
      std::string root_name = root_node_name_;
#else
      std::string root_name = "CoreFramework";
#endif
      names.push_back(root_name);
      ConvertToPropertyTree(root_node, &root, true, &names);
    } else
#endif
    if (root.isObject()) {
      Json::Value::Members names(root.getMemberNames());
      ConvertToPropertyTree(root_node, &root, false, &names);
    }
  } while (false);
  return return_value;
}

//  转换//NOLINT
TreeNodeIterator JsonTree::ConvertToTreeNode(
  void *element,
  bool is_parent_struct,
  bool is_self,
  const std::string& root_name) {
  TreeNodeIterator tree_node = NULL;
  do {
    if (!element || !root_node_) {
      break;
    }
    if (is_self) {
      tree_node = root_node_;
    } else {
      tree_node = TreeNodeIterator(new PropertyTree());
      if (!tree_node) {
        YSOS_LOG_DEBUG("not enough memory");  //need update for linux
        break;
      }
    }
    if (is_parent_struct) {
      std::string struct_item_name = "ysos_json_struct_item";
      tree_node->SetName(struct_item_name);
      Json::Value* element_ptr = (Json::Value*)element;
      if (element_ptr->isArray() || element_ptr->isObject()) {
        tree_node->SetValue("");
      } else {
        tree_node->SetValue(element_ptr->toStyledString());
      }

      std::string key = "ysos_json_struct_item_attribute";
      std::string value = root_name;
      tree_node->SetAttribute(key, value);
    } else {
      tree_node->SetName(root_name);
      Json::Value* element_ptr = (Json::Value*)element;
      if (element_ptr->isArray() || element_ptr->isObject()) {
        tree_node->SetValue("");
      } else {
        tree_node->SetValue(element_ptr->toStyledString());
      }
    }
  } while (false);

  return tree_node;
}

/*
bool AddChild_(TreeNodeIterator* cur_node_ptr, const std::string& current_root_name) {

  *cur_node_ptr = ConvertToTreeNode(&element_in[index], false, current_root_name);
  if (!cur_node) {
    break;
  }
  if (parent)
  {
    parent->AddChild(cur_node);
  }
  if (element_in[index].isArray() || element_in[index].isObject()) {
    Json::Value::Members names;
    if (element_in[index].isArray()) {
      names.push_back(current_root_name);
    } else {
      names = element_in[index].getMemberNames();
    }
    ConvertToPropertyTree(cur_node, &element_in[index], element_in[index].isArray(), &names);
  }
}
*/
void JsonTree::ConvertToPropertyTree(
  TreeNodeIterator parent,
  void *element,
  bool is_array,
  void *child) {
  bool will_break = false;
  Json::Value& element_in = *(Json::Value*)element;
  Json::UInt size = element_in.size();
  std::string root_name = "";
  Json::Value::Members* names_ptr = (Json::Value::Members*)child;
  TreeNodeIterator cur_node = NULL;
  if (is_array) {
    root_name = names_ptr->front();
    for (Json::UInt index =0; index < size; ++index) {
      std::string current_root_name = "ysos_json_array_item";
      cur_node = ConvertToTreeNode(&element_in[index], false, false, current_root_name);
      if (!cur_node) {
        break;
      }
      if (parent)
      {
        parent->AddChild(cur_node);
      }
      if (element_in[index].isArray() || element_in[index].isObject()) {
        Json::Value::Members names;
        if (element_in[index].isArray()) {
          names.push_back(current_root_name);
        } else/* if (element_in[index].isObject())*/ {
          names = element_in[index].getMemberNames();
        }
        ConvertToPropertyTree(cur_node, &element_in[index], element_in[index].isArray(), &names);
      }
    }
  } else {  //  结构体//NOLINT
    for ( Json::Value::Members::iterator it = names_ptr->begin();
      it != names_ptr->end();
      ++it)
    {
      root_name = *it;
      cur_node = ConvertToTreeNode(&element_in[root_name], true, false, root_name);
      if (!cur_node) {
        break;
      }
      if (parent)
      {
        parent->AddChild(cur_node);
      }
      if (element_in[root_name].isArray() || element_in[root_name].isObject()) {
        Json::Value::Members names;
        if (element_in[root_name].isArray()) {
          names.push_back(root_name);
        } else/* if (element_in[index].isObject())*/ {
          names = element_in[root_name].getMemberNames();
        }
        ConvertToPropertyTree(cur_node, &element_in[root_name], element_in[root_name].isArray(), &names);
      }
    }
  }
}

//  0, normal; 1, struct; 2, array
int GetPropertyTreeNodeFlag(TreeNodeIterator element) {
  int return_value = 0;
  TreeNodeIterator child = NULL;
  do {
    if (!element) {
      break;
    }
    child = element->FindFirstChild();
    if (!child) {
      break;
    }
    TreeAttributeNode attribute = child->FindFirstAttribute();
    return_value = 2;
    std::string ysos_json_struct_item_attribute = "ysos_json_struct_item_attribute";
    if (attribute.first.empty() || attribute.first != ysos_json_struct_item_attribute)
    {
      break;
    }
    return_value = 1;
  } while (false);
  return return_value;
}

/*
//  若非首个成员,加分隔符","//NOLINT
//  结构体成员要加"\"name\":"//NOLINT
//  若为顶层,或,数组成员,则不要"\"name\":"
//  若为数组或结构体,加开始符号"["或"{"
//  若有子节点,加子节点
//  若为数组或结构体,加结束符号"]"或"}"
*/
std::string JsonTree::ConvertToString(
  TreeNodeIterator tree,
  int flags,  //  0, normal; 1, struct; 2, array
  int item_index,
  bool at_top_level,
  int parent_flags) {
  std::stringstream ss;

  //  若为结构体或数组的非首个成员, 则加分隔符//NOLINT
  if (item_index > 0) {
    ss << ",";
    //std::cout << ",";
  }
  TreeAttributeNode attribute = tree->FindFirstAttribute();

  std::string ysos_json_struct_item_attribute = "ysos_json_struct_item_attribute";
  //  非顶层元素//NOLINT
  if (!at_top_level) {
    //  若为结构体的元素//NOLINT
    if (!attribute.first.empty()
      && attribute.first == ysos_json_struct_item_attribute) {
      ss << "\"" << attribute.second << "\":";
    } else {
      if (parent_flags != 2 && tree->GetNodeName().length() > 0)
      {
        ss << "\"" << tree->GetNodeName() << "\":";
      }
    }
  }

  //  结构体或数组开始符号//NOLINT
  if (flags == 1) {
    ss << "{";
    //std::cout << "{";
  } else if (flags == 2) {
    ss << "[";
    //std::cout << "{";
  }

  //  添加节点的文本//NOLINT
  ss << tree->GetNodeValue();
  //std::cout << tree->GetNodeValue();

  //  添加各子节点的文本//NOLINT
  TreeNodeIterator child = tree->FindFirstChild();
  int item_index_inner = 0;
  while (NULL != child) {
    ss << child->ConvertToString(
      ysos::PropertyTree::JSON,
      GetPropertyTreeNodeFlag(child),
      item_index_inner,
      false,
      GetPropertyTreeNodeFlag(tree));
    ++item_index_inner;
    child = child->FindNextSibling();
  }

  //  结构体或数组结束符号//NOLINT
  if (flags == 1) {
    ss << "}";
    //std::cout << "}";
  } else if (flags == 2) {
    ss << "]";
    //std::cout << "}";
  }

  return ss.str();
}
}   // namespace ysos
