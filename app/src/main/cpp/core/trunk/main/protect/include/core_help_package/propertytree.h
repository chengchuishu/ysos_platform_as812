/**
 *@file PropertyTree.h
 *@brief Definition of PropertyTree
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-26 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef CHP_PROPERTY_TREE_H_          //NOLINT
#define CHP_PROPERTY_TREE_H_          //NOLINT

/// ysos private headers //  NOLINT
#include "../../../public/include/sys_interface_package/common.h"
/// stl headers //  NOLINT
#include <list>
#include <map>
/// boost headers //  NOLINT
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>  // NOLINT
/// 3rdparty headers //  NOLINT
#include <tinyxml2/tinyxml2.h>

/// Ysos Headers
#include "../../../public/include/core_help_package/baseinterfaceimpl.h"

#include "../../../protect/include/core_help_package/singletontemplate.h"

namespace ysos {

class PropertyTree;
typedef boost::shared_ptr<PropertyTree>  PropertyTreePtr;
typedef  std::string              TreeValueType;
typedef  std::string              TreeKeyType;
typedef  PropertyTreePtr          TreeNodeIterator;
typedef  std::list<TreeNodeIterator >::iterator  TreeNodeListIterator;
typedef  std::pair<TreeKeyType, TreeValueType>   TreeAttributeNode;

/** 
  *@brief 配置文件解析树基类
  */
class YSOS_EXPORT PropertyTree: public boost::enable_shared_from_this<PropertyTree> {
// DECLARE_CREATEINSTANCE(PropertyTree);
   DISALLOW_COPY_AND_ASSIGN(PropertyTree);
//  DECLARE_PROTECT_CONSTRUCTOR(PropertyTree);
  //PropertyTree(const std::string &strClassName="PropertyTree");

 public:
  PropertyTree(const std::string &strClassName="PropertyTree");
   virtual ~PropertyTree();

  enum PropertyType {JSON, XML, PropertyType_End};

  /**
    *@brief  解析conent,构建一棵树 //  NOLINT
    *@param content 待解析的内容 //  NOLINT
    *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
    */
  virtual int ParseString(const std::string &content, const PropertyType type=XML);
  /**
    *@brief  获得双亲结点 //  NOLINT
    *@return 成功返回parent，失败返回Null  //  NOLINT
    */
  TreeNodeIterator GetParent(void);
  /**
      *@brief  获得根结点 //  NOLINT
      *@return 成功返回根结点，失败返回Null  //  NOLINT
      */
  TreeNodeIterator GetRoot(void);
  /**
  *@brief  找到当前结点的前一个Sibling  //  NOLINT
  *@param name 待找的Sibling结点的名称 //  NOLINT
  *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
  */
  /*virtual*/ TreeNodeIterator FindPrevSibling(const std::string &name="");
  /**
  *@brief  找到当前结点的第一个Sibling  //  NOLINT
  *@param name 待找的Sibling结点的名称 //  NOLINT
  *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
  */
  /*virtual*/ TreeNodeIterator FindFirstSibling(const std::string &name="");
  /**
    *@brief  找到当前结点的下一个Sibling //  NOLINT
    *@param name 待找的Sibling结点的名称 //  NOLINT
    *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
    */
  /*virtual*/ TreeNodeIterator FindNextSibling(const std::string &name="");
  /**
    *@brief  找到当前结点的第一个Child //  NOLINT
    *@param name 待找的Child结点的名称 //  NOLINT
    *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
    */
  /*virtual*/ TreeNodeIterator FindFirstChild(const std::string &name="");
  /**
    *@brief  找到当前结点的第一个Attribute //  NOLINT
    *@param name 待找的Attribute结点的名称 //  NOLINT
    *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
    */
  TreeAttributeNode FindFirstAttribute(const std::string &name="");
  /**
    *@brief  找到当前结点的下一个Attribute //  NOLINT
    *@param cur_attribute 待找的Attribute结点的下一个结点 //  NOLINT
    *@param name 待找的Attribute结点的名称 //  NOLINT
    *@return 成功返回，失败返回NULL  //  NOLINT
    */
  TreeAttributeNode FindNextAttribute(TreeAttributeNode &cur_attribute);
  /**
    *@brief  找到指定深度，指定层次的Node //  NOLINT
    *@param depth 距离当前结点的深度，0指当前结点 //  NOLINT
    *@param level 距离当前结点的层次，0为当前层次 //  NOLINT
    *@return 成功返回值，失败返回NULL  //  NOLINT
    */
  /*virtual*/ TreeNodeIterator FindNode(const int depth=0, const int level=0);
  /**
    *@brief  获取当前结点的值 //  NOLINT
    *@return 成功返回值，失败返回空  //  NOLINT
    */
  //template<typename T>
  /*virtual*/ std::string GetNodeName();
  /**
    *@brief  获取当前结点的值 //  NOLINT
    *@return 成功返回值，失败返回空  //  NOLINT
    */
  //template<typename T>
  /*virtual*/ std::string GetNodeValue();
  /**
    *@brief  获取指点的属性值，若没有，返回设置的默认值 //  NOLINT
    *@param name 属性的名称 //  NOLINT
    *@param default_value 指定的默认值 //  NOLINT
    *@return 成功返回值，失败返回指定的默认值  //  NOLINT
    */
  //template<typename T>
  /*virtual*/ std::string GetAttribute(const std::string &name, const std::string &default_value="");
  /**
    *@brief  设置结点的Value //  NOLINT
    *@param value 待设置的Value //  NOLINT
    *@return 成功返回0，失败非0  //  NOLINT
    */
//template<typename T>
  /*virtual*/ int SetValue(const std::string &value);
  /**
    *@brief  设置结点的Name //  NOLINT
    *@param name) 待设置的Name //  NOLINT
    *@return 成功返回0，失败非0  //  NOLINT
    */
//template<typename T>
  /*virtual*/ int SetName(const std::string &name);
  /**
    *@brief  设置结点的属性值 //  NOLINT
    *@param key 待设置的属性key //  NOLINT
    *@param value 待设置的Value //  NOLINT
    *@return 成功返回0，失败非0  //  NOLINT
    */
  /*template<typename T>*/
  /*virtual*/ int SetAttribute(const std::string &key, const std::string &value);
  /**
    *@brief  添加当前结点的Child //  NOLINT
    *@param child 待添加的Child //  NOLINT
    *@param position 添加的位置，-1为最后 //  NOLINT
    *@return 成功返回0，失败非0  //  NOLINT
    */
  /*virtual*/ int AddChild(TreeNodeIterator child, const int &position=-1);
  /**
    *@brief  添加当前结点的sibling //  NOLINT
    *@param sibling 待添加的sibling //  NOLINT
    *@param position 添加的位置，-1为最后 //  NOLINT
    *@return 成功返回0，失败非0  //  NOLINT
    */
  /*virtual*/ int AddSibling(TreeNodeIterator sibling, const int &position=-1);
  /**
    *@brief  删除指定的Child //  NOLINT
    *@param child 待删除的Child //  NOLINT
    *@return 成功返回0，失败非0  //  NOLINT
    */
  /*virtual*/ int DeleteChild(TreeNodeIterator child=NULL);
  /**
    *@brief  删除指定的sibling //  NOLINT
    *@param sibling 待删除的sibling //  NOLINT
    *@return 成功返回0，失败非0  //  NOLINT
    */
  /*virtual*/ int DeleteSibling(TreeNodeIterator sibling=NULL);
  /**
    *@brief  将该结点及其所有子结点，转化成字符串 //  NOLINT
    *@return 成功返回值，失败返回空  //  NOLINT
    */
  virtual std::string ConvertToString(
    PropertyType type = XML,
    int flags = 1,//0, normal; 1, struct; 2, array
    int item_index = 0,
    bool at_top_level = true,
    int parent_flags = 1);
  /**
    *@brief  清空树 //  NOLINT
    *@return 无  //  NOLINT
    */
  virtual void Destroy();

 protected:
  /**
   *@brief  根据当前结点查找，如果为0，返回当前结点，如果-1，返回parent,如果1，返回第一个Child，依此类推 //  NOLINT
   *@param depth 待查找的结点深度 //  NOLINT
   *@return 成功返回结点，失败返回NULL  //  NOLINT
   */
  TreeNodeIterator FindNodeByDepth(const int depth);
  /**
    *@brief  根据当前结点查找，如果为0，返回当前结点，如果-1，返回pre sibling,如果1，返回next sibling，依此类推 //  NOLINT
    *@param level 待查找的结点Sibling //  NOLINT
    *@return 成功返回结点，失败返回NULL  //  NOLINT
    */
  TreeNodeIterator FindNodeByLevel(const int level);
  /**
      *@brief  找到当前结点的前一个Child //  NOLINT
      *@param cur_node 待找的Child结点的前一个结点 //  NOLINT
      *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
      */
  TreeNodeListIterator FindPrevChild(TreeNodeIterator cur_node);
  /**
    *@brief  找到当前结点的下一个Child //  NOLINT
    *@param cur_node 待找的Child结点的下一个结点 //  NOLINT
    *@param name 待找的Child结点的名称 //  NOLINT
    *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
    */
  TreeNodeListIterator FindNextChild(TreeNodeIterator cur_node);
  /**
      *@brief  找到当前结点的前一个Child //  NOLINT
      *@param cur_node 待找的Child结点的前一个结点 //  NOLINT
      *@param name 待找的Child结点的名称 //  NOLINT
      *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
      */
  TreeNodeIterator FindPrevChild(TreeNodeIterator cur_node, const std::string &name);
  /**
    *@brief  找到当前结点的下一个Child //  NOLINT
    *@param cur_node 待找的Child结点的下一个结点 //  NOLINT
    *@param name 待找的Child结点的名称 //  NOLINT
    *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
    */
  TreeNodeIterator FindNextChild(TreeNodeIterator cur_node, const std::string &name);
  /**
    *@brief  当结点增加、删除一个Child时，需要更新所有Child的Sibling信息 //  NOLINT
    *@param update_node 更新的结点 //  NOLINT
    *@param is_add true 增加 false 删除 //  NOLINT
    *@return 无  //  NOLINT
    */
  void  UpdateAllChildren(TreeNodeIterator update_node, bool is_add = true);
  /**
   *@brief  更新当前结点的Sibling信息 //  NOLINT
   *@param update_node 更新的结点 //  NOLINT
   *@param is_add true 增加 false 删除 //  NOLINT
   *@return 无  //  NOLINT
   */
  void  UpdateSiblings(TreeNodeIterator update_node, bool is_add = true);
  /**
    *@brief  解析conent,构建一棵树 //  NOLINT
    *@param content 待解析的内容 //  NOLINT
    *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
    */
// virtual int ParseString(const std::string &content, const PropertyType type=XML);
  /**
    *@brief  将该结点及其所有子结点，转化成字符串 //  NOLINT
    *@return 成功返回值，失败返回空  //  NOLINT
    */
// virtual std::string ConvertToString(PropertyType type=XML);

 protected:
  std::map<TreeKeyType, TreeValueType>   attribute_map_;
  std::list<TreeNodeIterator>        child_list_;
  std::list<TreeNodeIterator>        sibling_list_;
  TreeValueType                          node_value_;
  int                                level_;
  TreeValueType                          node_name_;
  TreeNodeIterator                   parent_;
  std::pair<TreeKeyType, TreeValueType>      empty_attribute_;

  log4cplus::Logger logger_;

  DECLARE_SINGLETON_VARIABLE(PropertyTree);
};

}
#define GetPropertyTree  ysos::PropertyTree::Instance
#endif  // CHP_PROPERTY_TREE_H_       //NOLINT