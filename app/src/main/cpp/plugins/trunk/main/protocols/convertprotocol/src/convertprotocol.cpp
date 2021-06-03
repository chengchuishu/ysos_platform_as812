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
#include "../include/convertprotocol.h"
/// Ysos Headers
#include "../../../../../../core/trunk/main/public/include/os_hal_package/bufferutility.h"
#include "../../../../../../core/trunk/main/protect/include/core_help_package/propertytree.h"
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/manager_declare.h"

namespace ysos {
DECLARE_PLUGIN_REGISTER_INTERFACE(ConvertProtocol, ProtocolInterface);
ConvertProtocol::ConvertProtocol(const std::string &strClassName /* =FinancialCommonProtocol */): BaseProtocolImpl(strClassName)
  ,data_ptr_(GetDataInterfaceManager()->GetData(GLOBAL_DATA_KEY))
  ,first_answer()
  ,again_answer()
  ,default_answer()
{
  logger_ = GetUtility()->GetLogger("ysos.framework.strategy");
}

ConvertProtocol::~ConvertProtocol() {
  data_ptr_ = NULL;
}

int ConvertProtocol::Initialized( const std::string &key, const std::string &value ) {
  YSOS_LOG_DEBUG(key << ": " << value);
  if (key == "default_answer") {
    default_answer = value;
  }
  else if(key == "first_answer") {
    first_answer = value;
  }
  else if(key == "again_answer") {
    again_answer = value;
  }
  return YSOS_ERROR_SUCCESS;
}

// BufferInterfacePtr FinancialCommonProtocol::ParseMessageImpl(BufferInterfacePtr in_buffer) {
//
// }

int ConvertProtocol::ParseMessage(BufferInterfacePtr in_buffer, BufferInterfacePtr out_buffer, ProtocolFormatId format_id, void* context_ptr) {
  return YSOS_ERROR_SUCCESS;
}

int ConvertProtocol::FormMessage(BufferInterfacePtr in_buffer, BufferInterfacePtr out_buffer, ProtocolFormatId format_id, void* context_ptr) {
  YSOS_LOG_DEBUG("ConvertProtocol::FormMessage");

  std::string src_str;
  if (in_buffer != NULL && data_ptr_ != NULL) {
    const char *data = (const char*)(GetBufferUtility()->GetBufferData(in_buffer));
    data_ptr_->GetData(data, src_str);
    data_ptr_->SetData(data, "");
  }

  if (src_str.empty()) {
    GetBufferUtility()->CopyStringToBuffer(default_answer, out_buffer);
  }
  else if (src_str == "skip") {
  }
  else if (src_str == "again") {
    GetBufferUtility()->CopyStringToBuffer(again_answer, out_buffer);
  }
  else {
    if (!first_answer.empty()) {
      src_str += ',';
      src_str += first_answer;
      GetBufferUtility()->CopyStringToBuffer(src_str, out_buffer);
    }
  }

  return YSOS_ERROR_SUCCESS;
}

int ConvertProtocol::GetLength(BufferInterfacePtr in_buffer, ProtocolFormatId format_id) {
  return YSOS_ERROR_SUCCESS;
}
}