/**
 *@file ReadKeyProtocol.cpp
 *@brief Implement of moving aim protocol
 *@version 0.1
 *@author lishengjun
 *@date Created on: 2017-11-08 10:00:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

/// Self Header
#include "../include/readkeyprotocol.h"
/// Ysos Headers
#include "../../../../../../core/trunk/main/public/include/os_hal_package/bufferutility.h"
#include "../../../../../../core/trunk/main/protect/include/core_help_package/propertytree.h"
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/manager_declare.h"

namespace ysos {
DECLARE_PLUGIN_REGISTER_INTERFACE(ReadKeyProtocol, ProtocolInterface);
ReadKeyProtocol::ReadKeyProtocol(const std::string &strClassName /* =FinancialCommonProtocol */): BaseProtocolImpl(strClassName)
  ,data_ptr_(GetDataInterfaceManager()->GetData(GLOBAL_DATA_KEY))
{
  logger_ = GetUtility()->GetLogger("ysos.framework.strategy");
}

ReadKeyProtocol::~ReadKeyProtocol() {
  data_ptr_ = NULL;
}

int ReadKeyProtocol::Initialized( const std::string &key, const std::string &value ) {
  YSOS_LOG_DEBUG(key << ": " << value);
  return YSOS_ERROR_SUCCESS;
}

int ReadKeyProtocol::ParseMessage(BufferInterfacePtr in_buffer, BufferInterfacePtr out_buffer, ProtocolFormatId format_id, void* context_ptr) {
  return YSOS_ERROR_SUCCESS;
}

int ReadKeyProtocol::FormMessage(BufferInterfacePtr in_buffer, BufferInterfacePtr out_buffer, ProtocolFormatId format_id, void* context_ptr) {
  YSOS_LOG_DEBUG("ReadKeyProtocol::FormMessage");
  do {
    if (NULL == in_buffer || NULL == out_buffer || NULL == data_ptr_) {
      YSOS_LOG_DEBUG("buffer NULL (1)");
      break;
    }

    uint8_t *data_in = GetBufferUtility()->GetBufferData(in_buffer);
    uint8_t *data_out = GetBufferUtility()->GetBufferData(out_buffer);
    if (NULL == data_in || NULL == data_out) {
      YSOS_LOG_DEBUG("buffer NULL (2)");
      break;
    }

    std::string src_str;
    data_ptr_->GetData((char*)data_in, src_str);
    YSOS_LOG_DEBUG(src_str);

    strcpy((char*)data_out, src_str.c_str());
  } while(0);

  return YSOS_ERROR_SUCCESS;
}

int ReadKeyProtocol::GetLength(BufferInterfacePtr in_buffer, ProtocolFormatId format_id) {
  return YSOS_ERROR_SUCCESS;
}
}