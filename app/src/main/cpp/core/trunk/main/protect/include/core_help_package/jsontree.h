/**
 *@file JsonTree.h
 *@brief Definition of JsonTree
 *@version 0.1
 *@author XuLanyue
 *@date Created on: 2016-06-20 17:50:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef CHP_JSON_TREE_H_          //NOLINT
#define CHP_JSON_TREE_H_          //NOLINT

#include "../../../public/include/sys_interface_package/baseinterface.h"
#include "../../../public/include/core_help_package/baseinterfaceimpl.h"
#include "../../../public/include/core_help_package/utility.h"
#include "propertytree.h"

#define ENABLE_DERIVED_FROM_BASEINTERFACE       1

namespace ysos {
class JsonTree;
typedef boost::shared_ptr<JsonTree>   JsonTreePtr;
class YSOS_EXPORT JsonTree
#if ENABLE_DERIVED_FROM_BASEINTERFACE
  : public BaseInterfaceImpl
#endif
{
// DECLARE_CREATEINSTANCE(JsonTree);
  DISALLOW_COPY_AND_ASSIGN(JsonTree);
// DECLARE_PROTECT_CONSTRUCTOR(JsonTree);

  //friend class Singleton<JsonTree>;

 public:
  enum JsonTree_Property {
    TREE_ROOT_NAME = 0,     ///<  树的根节点的名字//NOLINT
    JSONTREE_PROPERTY_END
  };
 public:
  JsonTree(const std::string &class_name = "JsonTree");
  virtual ~JsonTree();

#if ENABLE_DERIVED_FROM_BASEINTERFACE
  virtual int GetProperty(int type_id, void *type);
  virtual int SetProperty(int type_id, void *type);
#endif
  virtual int ParseString(
    TreeNodeIterator root_node,
    const std::string &content);
  virtual TreeNodeIterator ConvertToTreeNode(
    void *element,
    bool is_parent_struct = false,
    bool is_self = false,
    const std::string& root_name = "");    ///<  对于Json, 若is_self为true且element为root, 则root_name不应为空//NOLINT

  /// 对于Json, 数组的成员是没有名字的.//NOLINT
  /// 因此, 若根为数组, 则根名定为"CoreFramework".//NOLINT
  /// 数组成员的名字定为:数组的名字,串上,基于1的索引号.//NOLINT
  /// XML转换为Json后再转回XML,XML中原属性节点转换回XML后可能被作为子节点.//NOLINT
  virtual void ConvertToPropertyTree(
    TreeNodeIterator parent,
    void *element,
    bool is_array,
    void *child);
  virtual std::string ConvertToString(
    TreeNodeIterator tree,
    int flags,//0, normal; 1, struct; 2, array
    int item_index = 0,
    bool at_top_level = true,
    int parent_flags = 1);

  /**
     *@brief 静态函数，对外使用，如：GetJsonTree()->ReadAllDataFromFile(file_name);  // NOLINT
     *@return： 成功返回JsonTreePtr，失败返回NULL  // NOLINT
     */
  //static const JsonTreePtr Instance(void);

 protected:
  TreeNodeIterator root_node_;
  std::string root_node_name_;

  DECLARE_SINGLETON_VARIABLE(JsonTree);
};
}
#define GetJsonTree  ysos::JsonTree::Instance
#endif  // CHP_JSON_TREE_H_       //NOLINT
