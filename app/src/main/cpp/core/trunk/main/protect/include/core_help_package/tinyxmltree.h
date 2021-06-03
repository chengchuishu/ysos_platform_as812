/**
 *@file PropertyTree.h
 *@brief Definition of PropertyTree
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-26 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef CHP_TINY_XML_TREE_H_          //NOLINT
#define CHP_TINY_XML_TREE_H_          //NOLINT

/// ysos private headers //  NOLINT
#include "propertytree.h"
#include "../../../public/include/core_help_package/utility.h"
//#include <tinyxml2/tinyxml2.h>
#include <tinyxml2/tinyxml2.h>

#include "../../../public/include/core_help_package/baseinterfaceimpl.h"

namespace ysos {
class TinyXmlTree;
typedef boost::shared_ptr<TinyXmlTree>  TinyXmlTreePtr;
/**
  *@brief XML配置文件解析树结点
  */
class YSOS_EXPORT TinyXmlTree {
 public:
  TinyXmlTree(const std::string &strClassName="TinyXmlTree");
  virtual ~TinyXmlTree();

  virtual int ParseString(TreeNodeIterator root_node, const std::string &content);
  virtual TreeNodeIterator ConvertToTreeNode(tinyxml2::XMLElement *element, bool is_self=false);
  virtual void ConvertToPropertyTree(TreeNodeIterator parent, tinyxml2::XMLElement *child);
  virtual std::string ConvertToString(TreeNodeIterator tree);

  /**
     *@brief 静态函数，对外使用，如：GetTinyXmlTree()->ReadAllDataFromFile(file_name);  // NOLINT
     *@return： 成功返回XmlUtil指针，失败返回NULL  // NOLINT
     */
  //static const TinyXmlTreePtr Instance(void);

 protected:
  TreeNodeIterator root_node_;
   log4cplus::Logger logger_;

  DECLARE_SINGLETON_VARIABLE(TinyXmlTree);
};
}
#define GetTinyXmlTree  ysos::TinyXmlTree::Instance
#endif  // CHP_PROPERTY_TREE_H_       //NOLINT