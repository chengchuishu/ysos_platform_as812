/**
 *@file XmlUtil.cpp
 *@brief Definition of XmlUtil
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-21 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

/// ysos private headers //  NOLINT
#include "../../../public/include/core_help_package/xmlutil.h"
/// c headers //  NOLINT
#include <fstream>
#include <sstream>
/// boost headers //  NOLINT
#include <boost/algorithm/string.hpp>
/// Ysos Headers
#include "../../../public/include/core_help_package/utility.h"
#include "../../../public/include/os_hal_package/lock.h"

namespace ysos {
DEFINE_SINGLETON(XmlUtil);
XmlUtil::XmlUtil(const std::string &strClassName): BaseInterfaceImpl(strClassName) {
  logger_ = GetUtility()->GetLogger();
}

XmlUtil::~XmlUtil() {

}

std::string XmlUtil::GetElementTextValue(tinyxml2::XMLElement *element) {
  if (NULL == element) {
    YSOS_LOG_ERROR("get element failed: ");  // need update for linux

    return "";
  }

  const char *text = element->GetText();
  return text==NULL ? "" : text;
}

std::string XmlUtil::GetElementTextValue(tinyxml2::XMLElement *element, const std::string &name) {
  tinyxml2::XMLElement *child = element->FirstChildElement(name.c_str());

  return GetElementTextValue(child);
}

INT32 XmlUtil::GetElementInt32Value(tinyxml2::XMLElement *element, const std::string &name) {
  tinyxml2::XMLElement *child = element->FirstChildElement(name.c_str());
  if (NULL == child) {
    YSOS_LOG_ERROR("get child element failed: " << name);  // need update for linux

    return 0;
  }

  const char *text = child->GetText();
  return text==NULL ? 0 : std::atoi(text);
}

INT64 XmlUtil::GetElementInt64Value(tinyxml2::XMLElement *element, const std::string &name) {
  tinyxml2::XMLElement *child = element->FirstChildElement(name.c_str());
  if (NULL == child) {
    YSOS_LOG_ERROR("get child element failed: " << name);  // need update for linux

    return 0;
  }

  const char *text = child->GetText();
  return text==NULL ? 0 : std::atoi(text);
}

bool XmlUtil::GetElementBoolValue(tinyxml2::XMLElement *element, const std::string &name) {
  tinyxml2::XMLElement *child = element->FirstChildElement(name.c_str());
  if (NULL == child) {
    YSOS_LOG_ERROR("get child element failed: " << name);  // need update for linux

    return false;
  }

  const char *text = child->GetText();
  if (NULL == text) {
    return false;
  }

  std::string str = boost::to_lower_copy(std::string(text));
  return std::strcmp(str.c_str(), "true") == 0;
}

std::string XmlUtil::GetElementTextAttribute(tinyxml2::XMLElement *element, const std::string &attribute_name) {
  if(NULL == element) {
    return "";
  }

  const char *value = element->Attribute(attribute_name.c_str());
  if(NULL == value) {
    return "";
  }

  return value;
}

INT32 XmlUtil::GetElementInt32Attribute(tinyxml2::XMLElement *element, const std::string &attribute_name) {
  if(NULL == element) {
    return 0;
  }

  const char *value = element->Attribute(attribute_name.c_str());
  if(NULL == value) {
    return 0;
  }

  return std::atoi(value);
}

INT64 XmlUtil::GetElementInt64Attribute(tinyxml2::XMLElement *element, const std::string &attribute_name) {
  if(NULL == element) {
    return 0;
  }

  const char *value = element->Attribute(attribute_name.c_str());
  if(NULL == value) {
    return 0;
  }

  return std::atoi(value);
}

tinyxml2::XMLElement *XmlUtil::GetRootElement(tinyxml2::XMLDocument *xml_doc, const std::string &xml_str) {
  xml_doc->Parse(xml_str.c_str());

  return xml_doc->RootElement();
}

tinyxml2::XMLElement *XmlUtil::GetRootElement(tinyxml2::XMLDocument *xml_doc, const std::string &xml_str, const std::string &element_name) {
  xml_doc->Parse(xml_str.c_str());

  tinyxml2::XMLElement * root = xml_doc->RootElement();
  if (NULL == root) {
    return root;
  }

  return root->FirstChildElement(element_name.c_str());
}

std::string XmlUtil::ConvertToString(const tinyxml2::XMLNode *xml_node) {
  XMLPrinter printer;
  xml_node->Accept(&printer);

  std::stringstream ss;
  ss << printer.CStr();

  return ss.str();
}

std::string XmlUtil::GetXmlStringFromFile(const std::string &file_name) {
  return GetUtility()->ReadAllDataFromFile(file_name);
}

}
