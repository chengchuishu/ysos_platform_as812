/**
 *@file XmlUtil.h
 *@brief Definition of XmlUtil
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-21 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef CHP_XML_UTIL_H  //NOLINT
#define CHP_XML_UTIL_H  //NOLINT

/// stl headers //  NOLINT
#include <string>
/// boost headers //  NOLINT
#include <boost/shared_ptr.hpp>
/// ysos private headers //  NOLINT
#include "../../../public/include/sys_interface_package/common.h"
#include <tinyxml2/tinyxml2.h>
#include "../../../public/include/sys_interface_package/baseinterface.h"
#include "../../../public/include/core_help_package/utility.h"

namespace ysos {
typedef tinyxml2::XMLNode XMLNode;
typedef tinyxml2::XMLDocument XmlDocument;
typedef tinyxml2::XMLElement  XmlElement;
typedef tinyxml2::XMLDeclaration XmlDeclaration;
typedef tinyxml2::XMLComment     XMLComment;
typedef tinyxml2::XMLText        XMLText;
typedef tinyxml2::XMLPrinter     XMLPrinter;
typedef tinyxml2::XMLAttribute   XMLAttribute;
typedef tinyxml2::XMLConstHandle XMLConstHandle;
typedef tinyxml2::XMLError       XMLError;
typedef tinyxml2::XMLUnknown     XMLUnknown;
typedef tinyxml2::XMLUtil        XMLUtil;
typedef tinyxml2::XMLVisitor     XMLVisitor;

class XmlUtil;
typedef boost::shared_ptr<XmlUtil>  XmlUtilPtr;
/**
  *@brief  Xml的工具类，使用方式：  //  NOLINT
  *        GetXmlUtil()->GetElementTextValue() ... //  NOLINT
  */
class YSOS_EXPORT XmlUtil: public BaseInterfaceImpl {
  DECLARE_CREATEINSTANCE(XmlUtil);
  DISALLOW_COPY_AND_ASSIGN(XmlUtil);
  DECLARE_PROTECT_CONSTRUCTOR(XmlUtil);

 public:
  ~XmlUtil();

  /**
   *@brief 通过XML结构获取对应的子结构text,text值会自动转化为小写  // NOLINT
   *@param element[Input]： 要获取值的父结点  // NOLINT
   *@param name[Input]：子结点的名称  // NOLINT
   *@return： 成功返回text值，失败返回""  // NOLINT
   */
  std::string GetElementTextValue(tinyxml2::XMLElement *element, const std::string &name);
  /**
   *@brief 通过XML结构获取对应的子结构text,text值会自动转化为INT64  // NOLINT
   *@param element[Input]： 要获取值的父结点  // NOLINT
   *@param name[Input]：子结点的名称  // NOLINT
   *@return： 成功返回Int64值，失败返回0  // NOLINT
   */
  INT32 GetElementInt32Value(tinyxml2::XMLElement *element, const std::string &name);
  /**
   *@brief 通过XML结构获取对应的子结构text,text值会自动转化为INT64  // NOLINT
   *@param element[Input]： 要获取值的父结点  // NOLINT
   *@param name[Input]：子结点的名称  // NOLINT
   *@return： 成功返回Int64值，失败返回0  // NOLINT
   */
  INT64 GetElementInt64Value(tinyxml2::XMLElement *element, const std::string &name);
  /**
   *@brief 通过XML结构获取对应的子结构text,text值会自动转化为bool  // NOLINT
   *@param element[Input]： 要获取值的父结点  // NOLINT
   *@param name[Input]：子结点的名称  // NOLINT
   *@return： 成功返回bool值，失败返回false  // NOLINT
   */
  bool GetElementBoolValue(tinyxml2::XMLElement *element, const std::string &name);
  /**
   *@brief 获取指定结点的text,text值会自动转化为小写  // NOLINT
   *@param element[Input]： 要获取值的结点  // NOLINT
   *@return： 成功返回字符串，失败返回""  // NOLINT
   */
  std::string GetElementTextValue(tinyxml2::XMLElement *element);
  /**
   *@brief 获取指定结点的Attribute,自动转化为小写  // NOLINT
   *@param element[Input]： 要获取值的结点  // NOLINT
   *@param attribute_name[Input]： 要获取值的属性名  // NOLINT
   *@return： 成功返回字符串，失败返回""  // NOLINT
   */
  std::string GetElementTextAttribute(tinyxml2::XMLElement *element, const std::string &attribute_name);
  /**
   *@brief 获取指定结点的Attribute,自动转化为INT32类型  // NOLINT
   *@param element[Input]： 要获取值的结点  // NOLINT
   *@param attribute_name[Input]： 要获取值的属性名  // NOLINT
   *@return： 成功返回数值，失败返回0  // NOLINT
   */
  INT32 GetElementInt32Attribute(tinyxml2::XMLElement *element, const std::string &attribute_name);
  /**
   *@brief 获取指定结点的Attribute,自动转化为INT64类型  // NOLINT
   *@param element[Input]： 要获取值的结点  // NOLINT
   *@param attribute_name[Input]： 要获取值的属性名  // NOLINT
   *@return： 成功返回数值，失败返回0  // NOLINT
   */
  INT64 GetElementInt64Attribute(tinyxml2::XMLElement *element, const std::string &attribute_name);
  /**
   *@brief 获取指定Xml字符串的第一层结点  // NOLINT
   *@param xml_doc[Input]： 操作xml 字符串的XmlDocument  // NOLINT
   *@param xml_str[Input]：要解析的Xml字符串  // NOLINT
   *@return： 成功返回结点，失败返回NULL  // NOLINT
   */
  tinyxml2::XMLElement *GetRootElement(tinyxml2::XMLDocument *xml_doc, const std::string &xml_str);
  /**
   *@brief 获取指定Xml字符串的第二层结点  // NOLINT
   *@param xml_doc[Input]： 操作xml 字符串的XmlDocument  // NOLINT
   *@param xml_str[Input]：要解析的Xml字符串  // NOLINT
   *@param element_name[Input]：子结点的名称  // NOLINT
   *@return： 成功返回结点，失败返回NULL  // NOLINT
   */
  tinyxml2::XMLElement *GetRootElement(tinyxml2::XMLDocument *xml_doc, const std::string &xml_str, const std::string &element_name);
  /**
   *@brief 将指定文本的内容，读取到内存中，通过返回值返回  // NOLINT
   *@param file_name[Input]： 要获取值的文件名称  // NOLINT
   *@return： 成功返回字符串，失败返回""  // NOLINT
   */
  std::string GetXmlStringFromFile(const std::string &file_name);
  /**
   *@brief 将XMLNode转化成Text  // NOLINT
   *@param xml_node[Input]： 要转化的xml node  // NOLINT
   *@return： 成功返回字符串，失败返回""  // NOLINT
   */
  std::string ConvertToString(const tinyxml2::XMLNode *xml_node);
  
  DECLARE_SINGLETON_VARIABLE(XmlUtil);
};
//IMPLEMENT_CREATESINGLEINSTANCE(XmlUtil);
}
#define GetXmlUtil  ysos::XmlUtil::Instance
#endif // CHP_MULTIMAP_H  //NOLINT
