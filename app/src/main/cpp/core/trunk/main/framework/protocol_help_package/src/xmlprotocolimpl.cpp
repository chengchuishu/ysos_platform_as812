/**
 *@file XmlProtocolImpl.cpp
 *@brief Definition of XmlProtocolImpl
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-21 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#include "../../../public/include/sys_interface_package/bufferpool.h"  //  NOLINT
#include "../../../protect/include/protocol_help_package/xmlprotocolimpl.h"  //  NOLINT
#include <exception>

namespace ysos {

XmlProtocolImpl::XmlProtocolImpl(const std::string& name):
  BaseInterfaceImpl(name) {  //  NOLINT
  try {
    xml_doc_ = new tinyxml2::XMLDocument();
  } catch(std::bad_alloc &) {
    // deal exception
  }
}

XmlProtocolImpl::~XmlProtocolImpl(void) {
  if (NULL != xml_doc_) {
    delete xml_doc_;
    xml_doc_ = NULL;
  }
}

/*
   @brief 从BufferInterfacePtr中提取出Buffer结点  //  NOLINT
   @param buffer_ptr：待提取的参数  //  NOLINT
   @param length:  Buffer的最大长度  //  NOLINT
   @return:        返回得到的Buffer  //  NOLINT
*/
UINT8* XmlProtocolImpl::GetBuffer(
  BufferInterfacePtr buffer_ptr,
  UINT32 *length) {
  UINT8 *ptr = NULL;
  buffer_ptr->GetBufferAndLength(&ptr, length);

  return ptr;
}

/*
   @brief 获取指定的第num个位置的兄弟结点，第1个结点是自身  //  NOLINT
   @param element：第1个兄弟结点，以这个结点开始，寻找  //  NOLINT
   @param num:     第几个兄弟结点  //  NOLINT
   @return: 成功返回得到的XML结点，否则返回NULL  //  NOLINT
*/
tinyxml2::XMLElement * XmlProtocolImpl::GetSpecifyElement(
  tinyxml2::XMLElement *element, 
  uint64_t num) {
  // 1 即是本身，不需要遍历  //  NOLINT
  while (NULL != element && num-- > 1) {
    element = element->NextSiblingElement();
  }

  // 如果num != 1,说明没有找到指定的结点，返回NULL  //  NOLINT
  return (1 != num) ? element : NULL;
}

tinyxml2::XMLElement *XmlProtocolImpl::GetSpecifyLevelElement(tinyxml2::XMLElement *parent, const uint64_t level) {
    if(0 == level) {
      return NULL;
    }

    return GetSpecifyElement(parent, level);
}

/*
   @brief 解析输入的XML字符串，根据format_id的要求，返回指定的子结点  //  NOLINT
   @param in_buffer:  待解析的XML字符串  //  NOLINT
   @param format_id：指定要取的子结点的位置  //  NOLINT
   @return: 成功返回得到的XML结点，否则返回NULL  //  NOLINT
*/
tinyxml2::XMLElement* XmlProtocolImpl::ParseMesage(
  BufferInterfacePtr in_buffer, 
  ProtocolFormatId format_id) {
  UINT32 length = 0;
  UINT8 *buffer_ptr = GetBuffer(in_buffer, &length);
  if (NULL == buffer_ptr || 0 == length) {
    return NULL;
  }

  int ret = xml_doc_->Parse(reinterpret_cast<char*>(buffer_ptr));  //  NOLINT
  if (0 != ret) {
    return NULL;
  }

  tinyxml2::XMLElement *element= xml_doc_->RootElement();
  if(NULL == element) {
    return element;
  }

  HierarchyFormatIdStruct formtStr(format_id);
  tinyxml2::XMLElement *child = element;

  // 获取第一层  //  NOLINT
  if(0 == formtStr.level_1 && 0 == formtStr.level_2 && \
    0==formtStr.level_3 && 0==formtStr.level_4) {
      return element;
  }

  element = GetSpecifyLevelElement(element, formtStr.level_1);
  if(NULL == element) {
    return element;
  }

  // 获取第二层  //  NOLINT
  if(0 == formtStr.level_2 && 0==formtStr.level_3
    && 0==formtStr.level_4) {
      return element;
  }
  element = GetSpecifyLevelElement(element->FirstChildElement(), formtStr.level_2);
  if(NULL == element) {
    return element;
  }

  // 获取第三层  //  NOLINT
  if(0==formtStr.level_3 && 0==formtStr.level_4) {
      return element;
  }
  element = GetSpecifyLevelElement(element->FirstChildElement(), formtStr.level_3);
  if(NULL == element) {
    return element;
  }

  // 获取第四层  //  NOLINT
  if(0==formtStr.level_4) {
    return element;
  }
  element = GetSpecifyLevelElement(element->FirstChildElement(), formtStr.level_3);
  if(NULL == element) {
    return element;
  }

  return child;
}

/*
   @brief 根据iMessageID的要求，解析输入的XML字符串，并把得到的字符串，通过pOutBuffer，返回  //  NOLINT
   @param pInBuffer： 待解析的XML字符串  //  NOLINT
   @param pOutBuffer: 得到指定的客串后，通过这个参数返回  //  NOLINT
   @param iMessageID: 指定要取的子结点的位置  //  NOLINT
   @param pContext:   上午文信息  //  NOLINT
   @return:           成功返回0，否则解析失败  //  NOLINT
*/
int XmlProtocolImpl::ParseMessage(
  BufferInterfacePtr in_buffer,
  BufferInterfacePtr out_buffer,
  ProtocolFormatId format_id,
  void *context_ptr) {
  if (NULL == xml_doc_ || NULL == in_buffer || NULL == out_buffer) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  tinyxml2::XMLElement *element = ParseMesage(in_buffer, format_id);

  if (NULL == element) {
    return 1;
  }

  UINT32 length=0;
  UINT8* pBuffer = GetBuffer(out_buffer, &length);
  if (NULL == pBuffer || 0 == length) {
    return 1;
  }

  XMLPrinter printer;
  element->Accept(&printer);
  const char* xmlcstr = printer.CStr();
  // 留一个位置给'\0'  //  NOLINT
  std::memcpy(
    pBuffer,
    xmlcstr, 
    length>(UINT32)strlen(xmlcstr) ? strlen(xmlcstr): length - 1);

  return 0;
}

/*
   @brief 根据iMessageID的要求，拼装输入的XML字符串，并把得到的字符串，通过pOutBuffer，返回  //  NOLINT
   @param pInBuffer： 待拼装的的XML结构  //  NOLINT
   @param pOutBuffer: 得到指定的客串后，通过这个参数返回  //  NOLINT
   @param iMessageID: 指定要取的子结点的位置  //  NOLINT
   @param pContext:   上午文信息  //  NOLINT
   @return:           成功返回0，否则解析失败  //  NOLINT
*/
int XmlProtocolImpl::FormMessage (
  BufferInterfacePtr in_buffer, 
  BufferInterfacePtr out_buffer, 
  ProtocolFormatId format_id, 
  void *pContext) {
  return 0;
}

/*
   @brief 根据iMessageID的要求，解析输入的XML字符串，并把得到的字符串，并返回字符串的长度  //  NOLINT
   @param in_buffer： 待解析的XML字符串  //  NOLINT
   @param format_id: 指定要取的子结点的位置  //  NOLINT
   @return:           成功返回实际的长度，否则返回0  //  NOLINT
*/
int XmlProtocolImpl::GetLength(
  BufferInterfacePtr in_buffer, 
  ProtocolFormatId format_id) {
  tinyxml2::XMLElement *element = ParseMesage(in_buffer, format_id);

  if (NULL == element) {
    return 0;
  }

  XMLPrinter printer;
  element->Accept(&printer);
  const char* xmlcstr = printer.CStr();

  return NULL == xmlcstr ? 0 : std::strlen(xmlcstr);
}
}  /*  NOLINT*/