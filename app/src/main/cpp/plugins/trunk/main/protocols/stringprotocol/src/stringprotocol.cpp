/**
 *@file protocolinterface.h
 *@brief Definition of protocol
 *@version 0.1
 *@author Steven.Shi
 *@date Created on: 2016-04-13 08:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

/// Self Header
#include "../include/stringprotocol.h"
/// Ysos Headers
#include "../../../../../../core/trunk/main/public/include/os_hal_package/bufferutility.h"
#include "../../../../../../core/trunk/main/protect/include/core_help_package/propertytree.h"
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/manager_declare.h"

namespace ysos {
DECLARE_PLUGIN_REGISTER_INTERFACE(StringProtocol, ProtocolInterface);
StringProtocol::StringProtocol(const std::string &strClassName /* =FinancialCommonProtocol */): BaseProtocolImpl(strClassName) {
  logger_ = GetUtility()->GetLogger("ysos.ysos");
}

StringProtocol::~StringProtocol() {
}

BufferInterfacePtr StringProtocol::FormatValue(const std::string &value, const std::string &format) {
  if(value.empty() || format.empty()) {
    return GetBufferUtility()->CreateBuffer(value);
  }

  ProtocolInterfacePtr format_protocol_ptr = GetProtocolInterfaceManager()->FindInterface(format);
  if(NULL == format_protocol_ptr) {
    YSOS_LOG_ERROR("find protocol interface: " << format << " failed");
    return NULL;
  }

  BufferInterfacePtr in_buffer = GetBufferUtility()->CreateBuffer(value);
  BufferInterfacePtr out_buffer = GetBufferUtility()->CreateBuffer(1024);

  ProtocolFormatId format_id = 0;
  int ret = format_protocol_ptr->FormMessage(in_buffer, out_buffer, format_id, NULL);
  if(0 != ret) {
    YSOS_LOG_ERROR("format message failed: " << value << " | " << format);
    return NULL;
  }

  return out_buffer;
}

int StringProtocol::ParseMessage(BufferInterfacePtr in_buffer, BufferInterfacePtr out_buffer, ProtocolFormatId format_id, void* context_ptr) {
  return YSOS_ERROR_SUCCESS;
}

int StringProtocol::FormMessage(BufferInterfacePtr in_buffer, BufferInterfacePtr out_buffer, ProtocolFormatId format_id, void* context_ptr) {
  if(NULL == in_buffer) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  uint8_t *data = GetBufferUtility()->GetBufferData(in_buffer);
  int length = GetBufferUtility()->GetBufferLength(in_buffer);
  if(NULL == data || 0 >= length) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  uint8_t *dst_data = GetBufferUtility()->GetBufferData(out_buffer);
  int dst_length = GetBufferUtility()->GetBufferLength(out_buffer);
  if(NULL == dst_data) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  memcpy(dst_data, data, length);
  return YSOS_ERROR_SUCCESS;
}

int StringProtocol::GetLength(BufferInterfacePtr in_buffer, ProtocolFormatId format_id) {
  return YSOS_ERROR_SUCCESS;
}

}