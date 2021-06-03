/**
 *@file JsonTreeUtil.h
 *@brief Definition of JsonTreeUtil
  该实用工具集包括: 根据key和value生成PropertyTree节点, 以及, 从PropertyTree节点获取key和value.//NOLINT
 *@version 0.1
 *@author XuLanyue
 *@date Created on: 2016-06-23 10:45:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef CHP_JSON_UTIL_H         //NOLINT
#define CHP_JSON_UTIL_H         //NOLINT

#include "../../../public/include/sys_interface_package/common.h"
#include "../../../protect/include/core_help_package/propertytree.h"
#include <string>

#define DEFAULT_ROOT_NODE_KEY                   "CoreFramework"
#define DEFAULT_ARRAY_ELEMENT_NAME              "ysos_json_array_item"
#define DEFAULT_STRUCT_ELEMENT_NAME             "ysos_json_struct_item"
#define DEFAULT_STRUCT_ELEMENT_ATTRIBUTE_KEY    "ysos_json_struct_item_attribute"

#define JSON_NODE_PARENT_IS_NORMAL              0
#define JSON_NODE_PARENT_IS_STRUCT              1
#define JSON_NODE_PARENT_IS_ARRAY               2

namespace ysos {
class JsonTreeUtil {
 public:
  static TreeNodeIterator GenerateTreeNodeForInt(
    const std::string& element_key,
    int element_value,
    uint32_t parent_flags = JSON_NODE_PARENT_IS_STRUCT,
    bool at_top_level = false);
  static TreeNodeIterator GenerateTreeNodeForUint(
    const std::string& element_key,
    unsigned element_value,
    uint32_t parent_flags = JSON_NODE_PARENT_IS_STRUCT,
    bool at_top_level = false);
  static TreeNodeIterator GenerateTreeNodeForDouble(
    const std::string& element_key,
    double element_value,
    uint32_t parent_flags = JSON_NODE_PARENT_IS_STRUCT,
    bool at_top_level = false);
  static TreeNodeIterator GenerateTreeNodeForBool(
    const std::string& element_key,
    bool element_value,
    uint32_t parent_flags = JSON_NODE_PARENT_IS_STRUCT,
    bool at_top_level = false);
  static TreeNodeIterator GenerateTreeNodeForNull(
    const std::string& element_key,
    uint32_t parent_flags = JSON_NODE_PARENT_IS_STRUCT,
    bool at_top_level = false);
  static TreeNodeIterator GenerateTreeNodeForString(
    const std::string& element_key,
    const std::string& element_value,
    uint32_t parent_flags = JSON_NODE_PARENT_IS_STRUCT,
    bool at_top_level = false);
  static TreeNodeIterator GenerateTreeNodeForObject(
    const std::string& element_key,
    uint32_t parent_flags = JSON_NODE_PARENT_IS_STRUCT,
    bool at_top_level = false);
  static TreeNodeIterator GenerateTreeNodeForArray(
    const std::string& element_key,
    uint32_t parent_flags = JSON_NODE_PARENT_IS_STRUCT,
    bool at_top_level = false);
};
}

#endif  //  CHP_JSON_UTIL_H     //NOLINT
