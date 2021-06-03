/**
 *@file nlpextcallback.cpp
 *@brief nlp ext callback
 *@version 1.0
 *@author l.j..
 *@date Created on: 2016-10-20 15:16:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
/// Private Headers //  NOLINT
#include "../include/nlpextcallback.h"

#include "../../../../../../core/trunk/main/public/include/os_hal_package/bufferutility.h"

#include "../../../public/include/common/commonenumdefine.h"
#include "../../../public/include/common/commonstructdefine.h"

namespace ysos {
DECLARE_PLUGIN_REGISTER_INTERFACE(NlpExtCallback, CallbackInterface);  ///<  插件的入口，一定要加上 //  NOLINT
NlpExtCallback::NlpExtCallback(const std::string &strClassName):BaseModuleCallbackImpl(strClassName) {
  logger_ = GetUtility()->GetLogger("ysos.audio.nlpext");
}

NlpExtCallback::~NlpExtCallback(void) {

}

int NlpExtCallback::IsReady(void) {
  return YSOS_ERROR_SUCCESS;
}

int NlpExtCallback::RealCallback(BufferInterfacePtr input_buffer, BufferInterfacePtr cur_output_buffer, BufferInterfacePtr output_buffer, void *context) {
  int n_return = YSOS_ERROR_SUCCESS;
  YSOS_LOG_DEBUG("NlpExtCallback RealCallback in");

  do {
    BaseModuelCallbackContext* module_callback_context_ptr = NULL;
    module_callback_context_ptr = reinterpret_cast<BaseModuelCallbackContext*>(context);
    if (NULL == module_callback_context_ptr) {
      YSOS_LOG_DEBUG("module_callback_context_ptr is null");
      n_return = YSOS_ERROR_FAILED;
      break;
    }

    BaseInterface* pModule= dynamic_cast<BaseInterface*>(module_callback_context_ptr->cur_module_ptr);
    if (NULL == pModule) {
      YSOS_LOG_DEBUG("pModule is null");
      n_return = YSOS_ERROR_FAILED;
      break;
    }
    AbilityParam input_param, out_param;
#ifdef _WIN32
    sprintf_s(input_param.ability_name, sizeof(input_param.ability_name), input_ability_name_.c_str());
    sprintf_s(input_param.data_type_name, sizeof(input_param.data_type_name), input_type_.c_str());
#else
    sprintf(input_param.ability_name, "%s", input_ability_name_.c_str());
    sprintf(input_param.data_type_name, "%s", input_type_.c_str());    
#endif
    input_param.buf_interface_ptr = input_buffer;
    UINT8* data_ptr = GetBufferUtility()->GetBufferData(input_buffer);
    if (NULL != data_ptr) { //test
      YSOS_LOG_DEBUG("nlpext receive request data = " << data_ptr);
      std::string input_data((char*)data_ptr);
      if ("DetectAsrResult" == input_data) {
        if (NULL != cur_output_buffer) {
          uint8_t* buffer_data_ptr = GetBufferUtility()->GetBufferData(cur_output_buffer);
          int buffer_length = GetBufferUtility()->GetBufferUsableLength(cur_output_buffer);
          if (input_data.length() <= buffer_length - 1) {
            memcpy(buffer_data_ptr, input_data.c_str(), input_data.length());
            buffer_data_ptr[input_data.length()] = '\0';
            YSOS_LOG_DEBUG("nlp set DetectAsrResult done, result = " << buffer_data_ptr);
            break;
          }
        }
      }
    }
#ifdef _WIN32
    sprintf_s(out_param.ability_name, sizeof(out_param.ability_name), output_ability_name_.c_str());
    sprintf_s(out_param.data_type_name, sizeof(out_param.data_type_name), output_type_.c_str());
#else
    sprintf(out_param.ability_name, "%s", output_ability_name_.c_str());
    sprintf(out_param.data_type_name, "%s", output_type_.c_str());
#endif
    out_param.buf_interface_ptr = cur_output_buffer;
    out_param.noresult_return_empty_ = true; //< 如果没有结果，则返回为空

    FunObjectCommon2 funobject2;
    funobject2.pparam1 = &input_param;
    funobject2.pparam2 = &out_param;
    n_return = pModule->GetProperty(PROP_FUN_CALLABILITY, &funobject2);
    YSOS_LOG_DEBUG("pModule->GetProperty result = " << n_return);
    if (YSOS_ERROR_SUCCESS != n_return)
      break;

    //< 输出buff可能为空的呢
    if (NULL != out_param.buf_interface_ptr) {
      UINT8* audio_data = NULL;
      UINT32 data_size = 0;
      out_param.buf_interface_ptr->GetBufferAndLength(&audio_data,&data_size);
      if (0 == data_size) { //< 表示解析成功，但当前没有数据，所以返因true
        n_return = YSOS_ERROR_SKIP;
        break;
      } else {
        YSOS_LOG_DEBUG("nlpext output data: " << GetUtility()->AsciiToUtf8((char*)audio_data));
        std::cout << "nlpext output data: " << (char*)audio_data << std::endl;
      }
    }
  } while (0);

  YSOS_LOG_DEBUG("NlpExtCallback RealCallback out");
  return n_return;
}

int NlpExtCallback::Initialized(const std::string &key, const std::string &value) {
  BaseModuleCallbackImpl::Initialized(key, value);
#ifdef _WIN32
  if (stricmp(key.c_str(), "input_type") ==0) {
    input_type_ = value;
  } else if (stricmp(key.c_str(), "output_type")==0) {
    output_type_ = value;
  } else if (stricmp(key.c_str(), "input_ability") ==0) {
    input_ability_name_ = value;
  } else if (stricmp(key.c_str(), "output_ability") ==0) {
    output_ability_name_ = value;
  }
#else
  if (strcasecmp(key.c_str(), "input_type") ==0) {
    input_type_ = value;
  } else if (strcasecmp(key.c_str(), "output_type")==0) {
    output_type_ = value;
  } else if (strcasecmp(key.c_str(), "input_ability") ==0) {
    input_ability_name_ = value;
  } else if (strcasecmp(key.c_str(), "output_ability") ==0) {
    output_ability_name_ = value;
  }
#endif
  YSOS_LOG_DEBUG("logic name = " << logic_name_ << "; input_type = " << input_type_ << "; output_type = " << output_type_ << "; input_ability_name_ = " << input_ability_name_ << "; output_ability_name_ = " << output_ability_name_);
  return YSOS_ERROR_SUCCESS;
}

}