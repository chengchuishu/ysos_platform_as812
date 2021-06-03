/**
 *@file TinyXmlTree.cpp
 *@brief Definition of TinyXmlTree
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-26 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

/// Self Header
#include "../../protect/include/core_help_package/tinyxmltree.h"
/// stl headers //  NOLINT
#include <sstream>
/// ysos private headers //  NOLINT
//#include <tinyxml2/tinyxml2.h>


namespace ysos {
DEFINE_SINGLETON(TinyXmlTree);
TinyXmlTree::TinyXmlTree(const std::string &strClassName) { /*: BaseInterfaceImpl("TinyXmlTree") */
  logger_ = GetUtility()->GetLogger("ysos.framework");
  root_node_ = NULL;
}

TinyXmlTree::~TinyXmlTree() {
  root_node_ = NULL;
}

int TinyXmlTree::ParseString(TreeNodeIterator root_node, const std::string &content) {
  if (NULL == root_node) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }
  root_node_ = root_node;

  tinyxml2::XMLDocument doc;
  doc.Parse(content.c_str());
  if (tinyxml2::XML_NO_ERROR != doc.ErrorID()) {
    YSOS_LOG_DEBUG("Parse xml string failed");  // need update for linux

    return YSOS_ERROR_LOGIC_ERROR;
  }

  tinyxml2::XMLElement *root = doc.RootElement();
  if (NULL == root) {
    YSOS_LOG_DEBUG("get root failed"); // need update for linux

    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  ConvertToTreeNode(root, true);
  if (NULL == root_node) {

    return YSOS_ERROR_LOGIC_ERROR;
  }

  ConvertToPropertyTree(root_node, root->FirstChildElement());

  return YSOS_ERROR_SUCCESS;
}

TreeNodeIterator TinyXmlTree::ConvertToTreeNode(tinyxml2::XMLElement *element, bool is_self) {
  assert(NULL != element);
  TreeNodeIterator tree_node = NULL;
  if (is_self) {
    tree_node = root_node_;
  } else {
    tree_node = TreeNodeIterator(new PropertyTree());
  }

  if (NULL == element) {
    YSOS_LOG_DEBUG("not enough memory"); // need update for linux
    return NULL;
  }

  tree_node->SetValue((NULL != element->GetText()) ? (element->GetText()) : "");
  tree_node->SetName((NULL != element->Name()) ? element->Name() : "");
  const tinyxml2::XMLAttribute *attribute = element->FirstAttribute();
  while (NULL != attribute) {
    std::string key = (NULL == attribute->Name()) ? "" : attribute->Name();
    std::string value = (NULL==attribute->Value()) ? "" : attribute->Value();
    tree_node->SetAttribute(key, value);

    attribute = attribute->Next();
  }

  return tree_node;
}

void TinyXmlTree::ConvertToPropertyTree(TreeNodeIterator parent, tinyxml2::XMLElement *child) {
  while (NULL != child) {
    TreeNodeIterator cur_node = ConvertToTreeNode(child);
    if (NULL == cur_node) {
      return;
    }

    if (NULL != parent) {
      parent->AddChild(cur_node);
    }

    ConvertToPropertyTree(cur_node, child->FirstChildElement());
    child = child->NextSiblingElement();
  }
}


std::string TinyXmlTree::ConvertToString(TreeNodeIterator tree) {
  std::stringstream ss;

  ss << "<" << tree->GetNodeName();

  TreeAttributeNode attribute = tree->FindFirstAttribute();
  while (!attribute.first.empty()) {
    ss << " " << attribute.first << "=\"" << attribute.second << "\"";
    attribute = tree->FindNextAttribute(attribute);
  }
  ss << ">";

  ss << tree->GetNodeValue();

  TreeNodeIterator child = tree->FindFirstChild();
  while (NULL != child) {
    ss << child->ConvertToString();
    child = child->FindNextSibling();
  }

  ss << "</" << tree->GetNodeName() << ">";

  return ss.str();
}
}