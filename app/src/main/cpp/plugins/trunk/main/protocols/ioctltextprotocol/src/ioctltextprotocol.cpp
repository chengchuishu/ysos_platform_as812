/**
 *@file IoctlTextProtocol.h
 *@brief Definition of protocol
 *@version 0.1
 *@author Steven.Shi
 *@date Created on: 2016-04-13 08:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

/// Self Header
#include "../include/ioctltextprotocol.h"
/// Ysos Headers
#include "../../../../../../core/trunk/main/public/include/os_hal_package/bufferutility.h"
#include "../../../../../../core/trunk/main/protect/include/core_help_package/propertytree.h"
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/manager_declare.h"
#include "../../../../../../core/trunk/main/public/include/core_help_package/jsonutility.h"

namespace ysos {
DECLARE_PLUGIN_REGISTER_INTERFACE(IoctlTextProtocol, ProtocolInterface);
IoctlTextProtocol::IoctlTextProtocol(const std::string &strClassName /* =FinancialCommonProtocol */): BaseProtocolImpl(strClassName) {
  logger_ = GetUtility()->GetLogger("ysos.framework.strategy");
}

IoctlTextProtocol::~IoctlTextProtocol() {

}

int IoctlTextProtocol::Initialized(const std::string &key, const std::string &value) {
  if ("key" == key) {
    key_ = value;
  } else {
    YSOS_LOG_ERROR("unsupport key: " << key << " : " << logic_name_);
  }

  return BaseProtocolImpl::Initialized(key, value);
}

// BufferInterfacePtr IoctlTextProtocol::FormatValue(const std::string &value, const std::string &format) {
//   return out_buffer;
// }

// BufferInterfacePtr FinancialCommonProtocol::ParseMessageImpl(BufferInterfacePtr in_buffer) {
//
// }

int IoctlTextProtocol::ParseMessage(BufferInterfacePtr in_buffer, BufferInterfacePtr out_buffer, ProtocolFormatId format_id, void* context_ptr) {
  if (NULL == in_buffer) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }
  uint8_t *data = GetBufferUtility()->GetBufferData(in_buffer);
  int length = GetBufferUtility()->GetBufferLength(in_buffer);
  if (NULL == data || 0 >= length) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  uint8_t *dst_data = GetBufferUtility()->GetBufferData(out_buffer);
  int dst_length = GetBufferUtility()->GetBufferLength(out_buffer);
  if (NULL == dst_data) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  JsonValue src_json;
  int ret = GetJsonUtil()->JsonObjectFromString((const char*)data, src_json);
  if (YSOS_ERROR_SUCCESS != ret) {
    YSOS_LOG_ERROR("invalid json value: " << data << " : " << logic_name_);
    return ret;
  }

  bool is_member = src_json.isMember(key_);
  if (!is_member) {
    YSOS_LOG_ERROR("not " << key_ << " member : " << data << " : " << logic_name_);
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  bool is_str = src_json[key_.c_str()].isString();
  if (!is_str) {
    YSOS_LOG_ERROR(key_ << " is not string : " << data << " : " << logic_name_);
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  std::string key_value = src_json[key_.c_str()].asString();
  if (key_value.length() > (size_t)dst_length) {
    YSOS_LOG_ERROR(" dst buffer is not enough : " << dst_length << " : " << key_value.length() << " key_value: " << key_value << " | " << logic_name_);
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  memcpy(dst_data, key_value.c_str(), key_value.length());

  return YSOS_ERROR_SUCCESS;
}

int IoctlTextProtocol::FormMessage(BufferInterfacePtr in_buffer, BufferInterfacePtr out_buffer, ProtocolFormatId format_id, void* context_ptr) {
  return YSOS_ERROR_SUCCESS;
}

int IoctlTextProtocol::GetLength(BufferInterfacePtr in_buffer, ProtocolFormatId format_id) {
  return YSOS_ERROR_SUCCESS;
}
}