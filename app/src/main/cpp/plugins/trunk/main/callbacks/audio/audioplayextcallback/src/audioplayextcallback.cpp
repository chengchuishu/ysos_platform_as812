/**
 *@file ttsextcallback.cpp
 *@brief tts ext callback
 *@version 1.0
 *@author l.j..
 *@date Created on: 2016-10-20 15:16:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
/// self header
#include "../include/audioplayextcallback.h"

/// ysos headers
#include "../../../../../../core/trunk/main/public/include/os_hal_package/lock.h"              //NOLINT
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"
#include "../../../../../../core/trunk/main/public/include/os_hal_package/bufferutility.h"


/// private headers
#include "../../../public/include/common/commonenumdefine.h"
#include "../../../public/include/common/commonstructdefine.h"

namespace ysos {
DECLARE_PLUGIN_REGISTER_INTERFACE(AudioPlayExtCallback, CallbackInterface);
AudioPlayExtCallback::AudioPlayExtCallback(const std::string &strClassName):BaseModuleCallbackImpl(strClassName) {
  logger_ = GetUtility()->GetLogger("ysos.audioplayext");
}

AudioPlayExtCallback::~AudioPlayExtCallback(void) {

}

int AudioPlayExtCallback::RealCallback(BufferInterfacePtr input_buffer, BufferInterfacePtr output_buffer, BufferInterfacePtr external_buf, void *context) {

  int n_return = YSOS_ERROR_FAILED;
  YSOS_LOG_DEBUG("AudioPlayExtCallback RealCallback in");
  do {
    BaseModuelCallbackContext* module_callback_context_ptr = reinterpret_cast<BaseModuelCallbackContext*>(context);
    BaseInterface* pModule= dynamic_cast<BaseInterface*>(module_callback_context_ptr->cur_module_ptr);
    AbilityParam input_param, out_param;
#ifdef _WIN32
    sprintf_s(input_param.ability_name, sizeof(input_param.ability_name), input_ability_name_.c_str());
    sprintf_s(input_param.data_type_name, sizeof(input_param.data_type_name), input_type_.c_str());
    input_param.buf_interface_ptr = input_buffer;
    sprintf_s(out_param.ability_name, sizeof(out_param.ability_name), output_ability_name_.c_str());
    sprintf_s(out_param.data_type_name, sizeof(out_param.data_type_name), output_type_.c_str());
#else
    sprintf(input_param.ability_name, "%s", input_ability_name_.c_str());
    sprintf(input_param.data_type_name, "%s", input_type_.c_str());
    input_param.buf_interface_ptr = input_buffer;
    sprintf(out_param.ability_name, "%s", output_ability_name_.c_str());
    sprintf(out_param.data_type_name, "%s", output_type_.c_str());
#endif
    out_param.buf_interface_ptr = output_buffer;
    FunObjectCommon2 funobject2;
    funobject2.pparam1 = &input_param;
    funobject2.pparam2 = &out_param;
    n_return = pModule->GetProperty(PROP_FUN_CALLABILITY, &funobject2);
    if (YSOS_ERROR_SUCCESS != n_return) {
      break;
    }
  } while (0);
  YSOS_LOG_DEBUG("AudioPlayExtCallback RealCallback out");
  return n_return;
}

int AudioPlayExtCallback::Initialized(const std::string &key, const std::string &value) {
  BaseModuleCallbackImpl::Initialized(key, value);
  if (strcasecmp(key.c_str(), "input_type") ==0) {
    input_type_ = value;
  } else if (strcasecmp(key.c_str(), "output_type")==0) {
    output_type_ = value;
  } else if (strcasecmp(key.c_str(), "input_ability") ==0) {
    input_ability_name_ = value;
  } else if (strcasecmp(key.c_str(), "output_ability") ==0) {
    output_ability_name_ = value;
  }

  YSOS_LOG_DEBUG("input_type: " << input_type_ << " ,output_type: " << output_type_ << " ,input_ability: " << input_ability_name_ << " ,output_ability: " << output_ability_name_);
  return YSOS_ERROR_SUCCESS;
}

}
