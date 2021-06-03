/**
 *@file asrextcallback.cpp
 *@brief asr ext callback
 *@version 1.0
 *@author l.j..
 *@date Created on: 2016-10-20 15:16:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
/// self header
#include "../include/asrextcallback.h"

/// boost headers
#include <boost/thread/thread.hpp>

/// ysos headers
#include "../../../../../../core/trunk/main/public/include/os_hal_package/lock.h"              //NOLINT
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"
#include "../../../../../../core/trunk/main/public/include/os_hal_package/bufferutility.h"

/// private headers
#include "../../../public/include/common/commonenumdefine.h"
#include "../../../public/include/common/commonstructdefine.h"

namespace ysos {
DECLARE_PLUGIN_REGISTER_INTERFACE(AsrExtCallback, CallbackInterface);
AsrExtCallback::AsrExtCallback(const std::string &strClassName):BaseModuleThreadCallbackImpl(strClassName) {
  logger_ = GetUtility()->GetLogger("ysos.asrext");
}

AsrExtCallback::~AsrExtCallback(void) {

}

int AsrExtCallback::RealCallback(BufferInterfacePtr input_buffer, BufferInterfacePtr output_buffer, BufferInterfacePtr external_buf, void *context) {
  if (context == NULL) {
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }
  YSOS_LOG_DEBUG("AsrExtCallback RealCallback in");
  BaseModuelCallbackContext* module_callback_context_ptr = NULL;
  module_callback_context_ptr = reinterpret_cast<BaseModuelCallbackContext*>(context);

  BaseInterface* pModule= dynamic_cast<BaseInterface*>(module_callback_context_ptr->cur_module_ptr);
  AbilityParam input_param, out_param;
#ifdef _WIN32
  sprintf_s(input_param.ability_name, sizeof(input_param.ability_name), /*ABSTR_VOICERECORD*/input_ability_name_.c_str());
  sprintf_s(input_param.data_type_name, sizeof(input_param.data_type_name), /*DTSTR_STREAM_PCMX1X*/input_type_.c_str());
  input_param.buf_interface_ptr = input_buffer;
  sprintf_s(out_param.ability_name, sizeof(out_param.ability_name), /*ABSTR_ASRNLP*/output_ability_name_.c_str());
  sprintf_s(out_param.data_type_name, sizeof(out_param.data_type_name), /*DTSTR_TEXT_JSON*/output_type_.c_str());
#else
  sprintf(input_param.ability_name, "%s", /*ABSTR_VOICERECORD*/input_ability_name_.c_str());
  sprintf(input_param.data_type_name, "%s", /*DTSTR_STREAM_PCMX1X*/input_type_.c_str());
  input_param.buf_interface_ptr = input_buffer;
  sprintf(out_param.ability_name, "%s", /*ABSTR_ASRNLP*/output_ability_name_.c_str());
  sprintf(out_param.data_type_name, "%s", /*DTSTR_TEXT_JSON*/output_type_.c_str());
#endif
  out_param.buf_interface_ptr = output_buffer;

  FunObjectCommon2 funobject2;
  funobject2.pparam1 = &input_param;
  funobject2.pparam2 = &out_param;
  int iret= pModule->GetProperty(PROP_FUN_CALLABILITY, &funobject2);
  if (YSOS_ERROR_SUCCESS != iret) {
    return iret;
  }

  //< 输出buff可能为空
  if (NULL != out_param.buf_interface_ptr) {
    UINT8* audio_data = NULL;
    UINT32 data_size =0;
    out_param.buf_interface_ptr->GetBufferAndLength(&audio_data,&data_size);
    if (0 == data_size) { //< 表示解析成功，但当前没有数据，所以返因true
      return YSOS_ERROR_SKIP;
    } else {
      std::string asr_value = std::string((char*)audio_data);
      if (asr_value.empty()) {
        return YSOS_ERROR_SKIP;
      }

      //< 创建一个临时buff传给nlp，为了方便直接调用CloneBuffer，清空数据
      BufferInterfacePtr clone_buffer_ptr = GetBufferUtility()->CloneBuffer(input_buffer);
      if (NULL != clone_buffer_ptr) {
        if (YSOS_ERROR_SUCCESS != GetBufferUtility()->InitialBuffer(clone_buffer_ptr)) {
          YSOS_LOG_DEBUG("Initial buffer failed");
        }
        std::cout << "AsrExt output data: " << (const char*)audio_data << std::endl;
        YSOS_LOG_DEBUG("AsrExt output data: " << (const char*)audio_data);
        InvokeNextCallback(out_param.buf_interface_ptr, clone_buffer_ptr, context);
        return YSOS_ERROR_SKIP;
      }
    }
  }

  return YSOS_ERROR_SUCCESS;
}

int AsrExtCallback::Initialized(const std::string &key, const std::string &value) {
  if (strcasecmp(key.c_str(), "input_type") ==0) {
    input_type_ = value;
  } else if (strcasecmp(key.c_str(), "output_type")==0) {
    output_type_ = value;
  } else if (strcasecmp(key.c_str(), "input_ability") ==0) {
    input_ability_name_ = value;
  } else if (strcasecmp(key.c_str(), "output_ability") ==0) {
    output_ability_name_ = value;
  }

  YSOS_LOG_DEBUG("input_type = " << input_type_ <<"; output_type = " << output_type_ << "; input_ability = " << input_ability_name_ << "; output_ability = " << output_ability_name_);
  return YSOS_ERROR_SUCCESS;
}

}
