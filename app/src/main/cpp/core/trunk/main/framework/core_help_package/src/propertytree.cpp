/**
 *@file PropertyTree.cpp
 *@brief Definition of PropertyTree
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-26 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

/// Self Header
#include "../../../protect/include/core_help_package/propertytree.h"
/// stl headers //  NOLINT
#include <sstream>
/// ysos private headers //  NOLINT
#include "../../../protect/include/core_help_package/tinyxmltree.h"
#include "../../../protect/include/core_help_package/jsontree.h"
//#include <tinyxml2/tinyxml2.h>

namespace ysos {
DEFINE_SINGLETON(PropertyTree);
PropertyTree::PropertyTree(const std::string &strClassName) {
  logger_ = GetUtility()->GetLogger("ysos.framework");  // need update for linux

  level_ = 0;
  parent_ = NULL;
  empty_attribute_ = std::make_pair("", "");
}

PropertyTree::~PropertyTree() {
  Destroy();
}

void PropertyTree::Destroy() {
  child_list_.clear();
  sibling_list_.clear();
  attribute_map_.clear();
  parent_ = NULL;
}

TreeNodeIterator PropertyTree::GetRoot(void) {
  TreeNodeIterator root = this->shared_from_this();
  while(NULL != root->parent_) {
    root = root->parent_;
  }

  return root;
}

TreeNodeIterator PropertyTree::GetParent(void) {
  return parent_;
}

TreeNodeIterator PropertyTree::FindPrevSibling(const std::string &name) {
  if(NULL == parent_) {
    return NULL;
  }

  return parent_->FindPrevChild(this->shared_from_this(), name);
}

TreeNodeIterator PropertyTree::FindFirstSibling(const std::string &name) {
  /// root没有Sibling
  if(NULL == parent_) {
    if(0 != level_) {
      YSOS_LOG_DEBUG("Not a valid tree: " << level_);  // need update for linux
    }

    return NULL;
  }

  return parent_->FindFirstChild(name);
}

TreeNodeIterator PropertyTree::FindNextSibling(const std::string &name) {
  /// root没有Sibling
  if(NULL == parent_) {
    if(0 != level_) {
      YSOS_LOG_DEBUG("Not a valid tree: " << level_); // need update for linux
    }

    return NULL;
  }

  return parent_->FindNextChild(this->shared_from_this(), name);
}

TreeNodeIterator PropertyTree::FindFirstChild(const std::string &name) {
  if(name.empty()) {
    if(child_list_.size() > 0) {
      return *(child_list_.begin());
    }
    else {
      return NULL;
    }
  }

  for(TreeNodeListIterator it=child_list_.begin(); it!=child_list_.end(); ++it){
    TreeNodeIterator node = *it;
    assert(NULL != node);
    if(name == node->node_name_) {
      return node;
    }
  }

  return NULL;
}

TreeNodeListIterator PropertyTree::FindPrevChild(TreeNodeIterator cur_node) {
  assert(NULL != cur_node);

  TreeNodeListIterator it = std::find(child_list_.begin(), child_list_.end(), cur_node);
  if(it == child_list_.end()) {
    return it;
  }

  if(it == child_list_.begin()) {
    return child_list_.end();
  }

  return (--it);
}

TreeNodeListIterator PropertyTree::FindNextChild(TreeNodeIterator cur_node) {
  assert(NULL != cur_node);

  TreeNodeListIterator it = std::find(child_list_.begin(), child_list_.end(), cur_node);
  if(it == child_list_.end()) {
    return child_list_.end();
  }

  if(++it == child_list_.end()) {
    return child_list_.end();
  }

  return it;
}

TreeNodeIterator PropertyTree::FindPrevChild(TreeNodeIterator cur_node, const std::string &name) {
  if(NULL == cur_node) {
    return FindFirstChild(name);
  }


  TreeNodeListIterator it = FindPrevChild(cur_node);
  if(name.empty()) {
    return (*it);
  }

  for(; it!=child_list_.begin(); --it){
    if((*it)->node_name_ == name) {
      return (*it);
    }
  }

  if(it==child_list_.begin() && (*it)->node_name_ != name) {
    return NULL;
  }

  return (*it);
}

TreeNodeIterator PropertyTree::FindNextChild(TreeNodeIterator cur_node, const std::string &name) {
  if(NULL == cur_node) {
    return FindFirstChild(name);
  }

  TreeNodeListIterator it = FindNextChild(cur_node);
  if(it == child_list_.end()) {
    return NULL;
  }
  
  if(name.empty()) {
    return (*it);
  }

  for(; it!=child_list_.end(); ++it){
    TreeNodeIterator node = *it;
    assert(NULL != node);
    if(node->node_name_ == name) {
      return node;
    }
  }

  return NULL;
}

TreeAttributeNode PropertyTree::FindFirstAttribute(const std::string &name) {
  std::map<TreeKeyType, TreeValueType>::iterator it=attribute_map_.begin();
  if(name.empty()) {
    if(it == attribute_map_.end()) {
      return empty_attribute_;
    }
    return *it;
  }

  it = attribute_map_.find(name);
  if(it == attribute_map_.end()) {
    return empty_attribute_;
  }

  return *it;
}

TreeAttributeNode PropertyTree::FindNextAttribute(TreeAttributeNode &cur_attribute) {
  if(cur_attribute.first.empty()) {
    return FindFirstAttribute();

  }

  std::map<TreeKeyType, TreeValueType>::iterator it = attribute_map_.find(cur_attribute.first);
  if(it == attribute_map_.end() || ++it == attribute_map_.end()) {
    return empty_attribute_;
  }

  return *it;
}

TreeNodeIterator PropertyTree::FindNode(const int depth, const int level) {
  TreeNodeIterator dest_node = FindNodeByDepth(depth);
  if(NULL == dest_node) {
    return dest_node;
  }

  return dest_node->FindNodeByLevel(level);
}

TreeNodeIterator PropertyTree::FindNodeByDepth(const int depth) {
  TreeNodeIterator dest_node = this->shared_from_this();
  int cur_depth = depth;
  while(cur_depth++ < 0 && NULL != dest_node) {
    dest_node = dest_node->parent_;
  }
/*
  if(cur_depth == 0) {
    return dest_node;
  }*/

  while(cur_depth-- >0 && NULL != dest_node) {
    dest_node = dest_node->FindFirstChild();
  }

  return dest_node;
}

TreeNodeIterator PropertyTree::FindNodeByLevel(const int level) {
  TreeNodeIterator dest_node = this->shared_from_this();
  int cur_level = level;
  while(cur_level++ < 0 && NULL != dest_node) {
    dest_node = dest_node->FindPrevSibling();
  }
/*
  if(cur_level == 0) {
    return dest_node;
  }*/

  while(cur_level-- >0 && NULL != dest_node) {
    dest_node = dest_node->FindNextSibling();
  }

  return dest_node;
}
std::string PropertyTree::GetNodeName() {
  return (node_name_);
}

std::string PropertyTree::GetNodeValue() {
  return (node_value_);
}

//template<typename T>
std::string PropertyTree::GetAttribute(const std::string &name, const std::string &default_value) {
  if(name.empty()) {
    return default_value;
  }

  std::map<TreeKeyType, TreeValueType>::iterator it = attribute_map_.find(name);
  if(it == attribute_map_.end()){
    return default_value;
  } 

  return /*GetValue<T>*/(it->second);
}

//template<typename T>
int PropertyTree::SetValue(const std::string &value) {
  std::stringstream ss;
  ss << value;
  node_value_ = ss.str();

  return YSOS_ERROR_SUCCESS;
}

//template<typename T>
int PropertyTree::SetName(const std::string &name) {
  std::stringstream ss;
  ss << name;
  node_name_ = ss.str();

  return YSOS_ERROR_SUCCESS;
}

//template<typename T>
int PropertyTree::SetAttribute(const std::string &key, const std::string &value) {
  std::stringstream ss;
  ss << value;
  if(key.empty() || ss.str().empty()) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  std::map<TreeKeyType, TreeValueType>::iterator it = attribute_map_.find(key);
  if(it == attribute_map_.end()){
    attribute_map_.insert(std::make_pair(key, ss.str()));
  }
  else {
    it->second = ss.str();
  }

  return YSOS_ERROR_SUCCESS;
}

int PropertyTree::AddChild(TreeNodeIterator child, const int &position) {
  TreeNodeIterator cur_child = child;
  assert(NULL != cur_child);

  child->parent_ = this->shared_from_this();

  cur_child->level_ = level_ + 1;
  if(position <= 0) {
    child_list_.push_back(cur_child);

    return YSOS_ERROR_SUCCESS;
  }

  TreeNodeListIterator it=child_list_.begin();
  for(int i=0; i<position; ++i, ++it) {
    if(it == child_list_.end()) {
      break;
    }
  }

  if(it == child_list_.end()) {
    child_list_.push_back(cur_child);
  }
  else {
    child_list_.insert(it, cur_child);
  }

  return YSOS_ERROR_SUCCESS;
}

int PropertyTree::AddSibling(TreeNodeIterator sibling, const int &position) {
  if(NULL == parent_ || level_ == 0) {
    return YSOS_ERROR_INVALID_OPERATION;
  }

  return parent_->AddChild(sibling, position);
}

int PropertyTree::DeleteChild(TreeNodeIterator child) {
  if(NULL == child) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  TreeNodeListIterator it = std::find(child_list_.begin(), child_list_.end(), child);
  if(it == child_list_.end()) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  child_list_.erase(it);
  UpdateAllChildren(child);
  child->Destroy();

  return YSOS_ERROR_SUCCESS;
}

int PropertyTree::DeleteSibling(TreeNodeIterator sibling) {
  if(NULL == parent_) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  return parent_->DeleteChild(sibling);
}

void  PropertyTree::UpdateAllChildren(TreeNodeIterator update_node, bool is_add) {
  if(NULL == update_node) {
    return;
  }

  for(TreeNodeListIterator it=child_list_.begin(); it!=child_list_.end(); ++it){
    TreeNodeIterator node = *it;
    assert(NULL != node);
    node->UpdateSiblings(update_node, is_add);
  }
}

void PropertyTree::UpdateSiblings(TreeNodeIterator update_node, bool is_add) {
  assert(NULL == update_node);

  TreeNodeListIterator it=std::find(sibling_list_.begin(), sibling_list_.end(), update_node);
  if(it == sibling_list_.end()) {
    if(is_add) {
      TreeNodeIterator node = update_node;
      assert(NULL != node);
      sibling_list_.push_back(node);
    }
    return;
  }

  if(!is_add) {
    sibling_list_.erase(it);
  }
}

int PropertyTree::ParseString(const std::string &content, const PropertyTree::PropertyType type) {
  if (content.empty()) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  if (type == XML) {
    GetTinyXmlTree()->ParseString(this->shared_from_this(), content);
  } else if (type == JSON) {
    GetJsonTree()->ParseString(this->shared_from_this(), content);
  }

  return YSOS_ERROR_SUCCESS;
}

std::string PropertyTree::ConvertToString(
  PropertyType type,
  int flags,
  int item_index,
  bool at_top_level,
  int parent_flags) {
  std::string str;
  if (type == XML) {
    str =  GetTinyXmlTree()->ConvertToString(this->shared_from_this());
  } else if (type == JSON) {
    str =  GetJsonTree()->ConvertToString(this->shared_from_this(), flags, item_index, at_top_level, parent_flags);
  }
  return str;
}
}