/**
 *@file JsonTreeUtil.cpp
 *@brief Definition of JsonTreeUtil
  该实用工具集包括: 根据key和value生成PropertyTree节点, 以及, 从PropertyTree节点获取key和value.//NOLINT
 *@version 0.1
 *@author XuLanyue
 *@date Created on: 2016-06-23 10:45:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#if 1   //  disable warning 4250 and 4996
#include "../../../public/include/os_hal_package/config.h"  // NOLINT
#if (CURRENT_PLATFORM == PLATFORM_WINDOWS)
#pragma warning(disable: 4250)      //  dominance
#pragma warning(disable: 4996)      //  std::_Fill_n
#endif
#endif
#include "../../../public/include/core_help_package/jsontreeutil.h"
#include "../../../protect/include/core_help_package/propertytree.h"
#include <string>

namespace ysos {
  TreeNodeIterator GenerateTreeNode_Part1(
  std::string& element_key,
  bool at_top_level) {
  TreeNodeIterator tree_node = NULL;
  do {
    //  参数检测//NOLINT
    if (!at_top_level && element_key.length() == 0) {
      break;
    }

    //  创建树节点//NOLINT
    tree_node = TreeNodeIterator(new PropertyTree());
    if (!tree_node) {
      break;
    }

    //  若必要则置默认的根节点名//NOLINT
    if (at_top_level && element_key.length() == 0) {
      element_key = DEFAULT_ROOT_NODE_KEY;
    }
  } while (false);
  return tree_node;
}

void GenerateTreeNode_Part2(
  TreeNodeIterator &tree_node,
  const std::string& element_key,
  const std::string& element_value,
  uint32_t parent_flags,
  bool at_top_level) {
  do {
    if (parent_flags == JSON_NODE_PARENT_IS_ARRAY) {
      tree_node->SetName(DEFAULT_ARRAY_ELEMENT_NAME);
      tree_node->SetValue(element_value);
    } else if (parent_flags == JSON_NODE_PARENT_IS_STRUCT) {
      std::string key = DEFAULT_STRUCT_ELEMENT_ATTRIBUTE_KEY;
      std::string value = element_key;
      tree_node->SetAttribute(key, value);
      tree_node->SetName(DEFAULT_STRUCT_ELEMENT_NAME);
      tree_node->SetValue(element_value);
    } else {
      tree_node->SetName(element_key);
      tree_node->SetValue(element_value);
    }
  } while (false);
}
TreeNodeIterator JsonTreeUtil::GenerateTreeNodeForInt(
  const std::string& element_key,
  int element_value,
  uint32_t parent_flags,
  bool at_top_level) {
  TreeNodeIterator tree_node = NULL;
  do {
    std::string element_key_ = element_key;
    tree_node = GenerateTreeNode_Part1(element_key_, at_top_level);
    if (!tree_node) {
      break;
    }

    char buf_for_number[129];
    memset(buf_for_number, 0, sizeof(buf_for_number));
    sprintf(buf_for_number, "%d", element_value);
    std::string element_value_ = buf_for_number;

    GenerateTreeNode_Part2(
      tree_node, element_key_, element_value_, parent_flags, at_top_level);
  } while (false);
  return tree_node;
}
TreeNodeIterator JsonTreeUtil::GenerateTreeNodeForUint(
  const std::string& element_key,
  unsigned element_value,
  uint32_t parent_flags,
  bool at_top_level) {
  TreeNodeIterator tree_node = NULL;
  do {
    std::string element_key_ = element_key;
    tree_node = GenerateTreeNode_Part1(element_key_, at_top_level);
    if (!tree_node) {
      break;
    }

    char buf_for_number[129];
    memset(buf_for_number, 0, sizeof(buf_for_number));
    sprintf(buf_for_number, "%u", element_value);
    std::string element_value_ = buf_for_number;

    GenerateTreeNode_Part2(
      tree_node, element_key_, element_value_, parent_flags, at_top_level);
  } while (false);
  return tree_node;
}
TreeNodeIterator JsonTreeUtil::GenerateTreeNodeForDouble(
  const std::string& element_key,
  double element_value,
  uint32_t parent_flags,
  bool at_top_level) {
  TreeNodeIterator tree_node = NULL;
  do {
    std::string element_key_ = element_key;
    tree_node = GenerateTreeNode_Part1(element_key_, at_top_level);
    if (!tree_node) {
      break;
    }

    char buf_for_number[129];
    memset(buf_for_number, 0, sizeof(buf_for_number));
    sprintf(buf_for_number, "%f", element_value);
    std::string element_value_ = buf_for_number;

    GenerateTreeNode_Part2(
      tree_node, element_key_, element_value_, parent_flags, at_top_level);
  } while (false);
  return tree_node;
}
TreeNodeIterator JsonTreeUtil::GenerateTreeNodeForBool(
  const std::string& element_key,
  bool element_value,
  uint32_t parent_flags,
  bool at_top_level) {
  TreeNodeIterator tree_node = NULL;
  do {
    std::string element_key_ = element_key;
    tree_node = GenerateTreeNode_Part1(element_key_, at_top_level);
    if (!tree_node) {
      break;
    }

    std::string element_value_ = "false";
    if (element_value == true) {
      element_value_ = "true";
    }

    GenerateTreeNode_Part2(
      tree_node, element_key_, element_value_, parent_flags, at_top_level);
  } while (false);
  return tree_node;
}
TreeNodeIterator JsonTreeUtil::GenerateTreeNodeForNull(
  const std::string& element_key,
  uint32_t parent_flags,
  bool at_top_level) {
  TreeNodeIterator tree_node = NULL;
  do {
    std::string element_key_ = element_key;
    tree_node = GenerateTreeNode_Part1(element_key_, at_top_level);
    if (!tree_node) {
      break;
    }

    std::string element_value_ = "null";
    GenerateTreeNode_Part2(
      tree_node, element_key_, element_value_, parent_flags, at_top_level);
  } while (false);
  return tree_node;
}
TreeNodeIterator JsonTreeUtil::GenerateTreeNodeForString(
  const std::string& element_key,
  const std::string& element_value,
  uint32_t parent_flags,
  bool at_top_level) {
  TreeNodeIterator tree_node = NULL;
  do {
    std::string element_key_ = element_key;
    tree_node = GenerateTreeNode_Part1(element_key_, at_top_level);
    if (!tree_node) {
      break;
    }

    std::string element_value_ = "\"" + element_value + "\"";
    GenerateTreeNode_Part2(
      tree_node, element_key_, element_value_, parent_flags, at_top_level);
  } while (false);
  return tree_node;
}
TreeNodeIterator JsonTreeUtil::GenerateTreeNodeForObject(
  const std::string& element_key,
  uint32_t parent_flags,
  bool at_top_level) {
  TreeNodeIterator tree_node = NULL;
  do {
    std::string element_key_ = element_key;
    tree_node = GenerateTreeNode_Part1(element_key_, at_top_level);
    if (!tree_node) {
      break;
    }

    std::string element_value_ = "";
    GenerateTreeNode_Part2(
      tree_node, element_key_, element_value_, parent_flags, at_top_level);
  } while (false);
  return tree_node;
}
TreeNodeIterator JsonTreeUtil::GenerateTreeNodeForArray(
  const std::string& element_key,
  uint32_t parent_flags,
  bool at_top_level) {
  TreeNodeIterator tree_node = NULL;
  do {
    std::string element_key_ = element_key;
    tree_node = GenerateTreeNode_Part1(element_key_, at_top_level);
    if (!tree_node) {
      break;
    }

    std::string element_value_ = "";
    GenerateTreeNode_Part2(
      tree_node, element_key_, element_value_, parent_flags, at_top_level);
  } while (false);
  return tree_node;
}
}

