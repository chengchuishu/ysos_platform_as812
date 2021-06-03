/*
# TreeNodeInterface.h
# Definition of TreeNodeInterface
# Created on: 2016-05-03 18:59:20
# Original author: dhongqian
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
*/

#ifndef SIF_TREE_NODE_INTERFACE__H                                          //  NOLINT
#define SIF_TREE_NODE_INTERFACE__H                                          //  NOLINT

#include "../../../public/include/sys_interface_package/common.h"
#include "../../../public/include/core_help_package/templatefactory.h"
#include "baseinterface.h"
#include <string>

namespace ysos {
class YSOS_EXPORT TreeNodeInterface: virtual public BaseInterface {
  IMPLEMENT_PUBLIC_EMPTY_DESTRUCTOR(TreeNodeInterface)
 public:
  /**
    *@brief  解析conent,构建一棵树 //  NOLINT
    *@param content 待解析的内容 //  NOLINT
    *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
    */
  virtual int ParseString(const std::string &content) = 0;
  /**
  *@brief  找到当前结点的第一个Sibling  //  NOLINT
  *@param name 待找的Sibling结点的名称 //  NOLINT
  *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
  */
  virtual TreeNodeInterface* FindFirstSibling(const std::string &name="")=0;
  /**
    *@brief  找到当前结点的下一个Sibling //  NOLINT
    *@param name 待找的Sibling结点的名称 //  NOLINT
    *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
    */
  virtual TreeNodeInterface* FindNextSibling(const std::string &name="")=0;
  /**
    *@brief  找到当前结点的第一个Child //  NOLINT
    *@param name 待找的Child结点的名称 //  NOLINT
    *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
    */
  virtual TreeNodeInterface* FindFirstChild(const std::string &name="")=0;
  /**
    *@brief  找到指定深度，指定层次的Node //  NOLINT
    *@param depth 距离当前结点的深度，0指当前结点 //  NOLINT
    *@param level 距离当前结点的层次，0为当前层次 //  NOLINT
    *@return 成功返回值，失败返回NULL  //  NOLINT
    */
  virtual TreeNodeInterface* FindNode(const int depth=0, const int level=0) = 0;
  /**
    *@brief  获取当前结点的值 //  NOLINT
    *@return 成功返回值，失败返回空  //  NOLINT
    */
  virtual std::string GetValue() = 0;
  /**
    *@brief  获取指点的属性值，若没有，返回设置的默认值 //  NOLINT
    *@param name 属性的名称 //  NOLINT
    *@param default_value 指定的默认值 //  NOLINT
    *@return 成功返回值，失败返回指定的默认值  //  NOLINT
    */
  virtual std::string GetAttribute(const std::string &name, const std::string &default_value="") = 0;
  /**
    *@brief  设置结点的Value //  NOLINT
    *@param value 待设置的Value //  NOLINT
    *@return 成功返回0，失败非0  //  NOLINT
    */
  virtual int SetValue(const std::string &value) = 0;
  /**
    *@brief  设置结点的属性值 //  NOLINT
    *@param key 待设置的属性key //  NOLINT
    *@param value 待设置的Value //  NOLINT
    *@return 成功返回0，失败非0  //  NOLINT
    */
  virtual int SetAttribute(const std::string &key, const std::string &value) = 0;
  /**
    *@brief  添加当前结点的Child //  NOLINT
    *@param child 待添加的Child //  NOLINT
    *@param position 添加的位置，-1为最后 //  NOLINT
    *@return 成功返回0，失败非0  //  NOLINT
    */
  virtual int AddChild(TreeNodeInterface *child, const int &position=-1) = 0;
  /**
    *@brief  添加当前结点的sibling //  NOLINT
    *@param sibling 待添加的sibling //  NOLINT
    *@param position 添加的位置，-1为最后 //  NOLINT
    *@return 成功返回0，失败非0  //  NOLINT
    */
  virtual int AddSibling(TreeNodeInterface *sibling, const int &position=-1) = 0;
  /**
    *@brief  删除指定的Child //  NOLINT
    *@param child 待删除的Child //  NOLINT
    *@return 成功返回0，失败非0  //  NOLINT
    */
  virtual int DeleteChild(TreeNodeInterface *child=NULL) = 0;
  /**
    *@brief  删除指定的sibling //  NOLINT
    *@param sibling 待删除的sibling //  NOLINT
    *@return 成功返回0，失败非0  //  NOLINT
    */
  virtual int DeleteSibling(TreeNodeInterface *sibling=NULL) = 0;
  /**
    *@brief  将该结点及其所有子结点，转化成字符串 //  NOLINT
    *@return 成功返回值，失败返回空  //  NOLINT
    */
  virtual std::string ConvertToString(void) = 0;
};
}
#endif  //  SIF_TREE_NODE_INTERFACE__H                                      //NOLINT
