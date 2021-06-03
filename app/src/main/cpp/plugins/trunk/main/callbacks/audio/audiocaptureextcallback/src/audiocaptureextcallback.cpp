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
#include "../include/audiocaptureextcallback.h"


/// boost headers
#include <boost/thread/thread.hpp>

/// ysos headers
#include "../../../../../../core/trunk/main/public/include/os_hal_package/lock.h"             //NOLINT
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"
#include "../../../../../../core/trunk/main/public/include/os_hal_package/bufferutility.h"


/// private headers
#include "../../../public/include/common/commonenumdefine.h"
#include "../../../public/include/common/commonstructdefine.h"

namespace ysos {
DECLARE_PLUGIN_REGISTER_INTERFACE(AudioCaptureExtCallback, CallbackInterface);
AudioCaptureExtCallback::AudioCaptureExtCallback(const std::string &strClassName):BaseThreadModuleCallbackImpl(strClassName) {
  logger_ = GetUtility()->GetLogger("ysos.audiocaptureext");
}

AudioCaptureExtCallback::~AudioCaptureExtCallback(void) {

}

int AudioCaptureExtCallback::RealCallback(BufferInterfacePtr input_buffer, BufferInterfacePtr output_buffer, BufferInterfacePtr external_buf, void *context) {

  int n_return = YSOS_ERROR_FAILED;
  YSOS_LOG_DEBUG("RealCallback in");

#ifdef _DEBUGVIEW
  ::OutputDebugString((std::string("ysos.audiocaptureext RealCallback")).c_str());
#endif

  do {
    BaseModuelCallbackContext* module_callback_context_ptr = reinterpret_cast<BaseModuelCallbackContext*>(context);
    BaseInterface* pModule = dynamic_cast<BaseInterface*>(module_callback_context_ptr->cur_module_ptr);
    AbilityParam out_param;
#ifdef _WIN32
    sprintf_s(out_param.ability_name, sizeof(out_param.ability_name), output_ability_name_.c_str());
    sprintf_s(out_param.data_type_name, sizeof(out_param.data_type_name), output_type_.c_str());
#else
    sprintf(out_param.ability_name, "%s", output_ability_name_.c_str());
    sprintf(out_param.data_type_name, "%s", output_type_.c_str());
#endif
    out_param.buf_interface_ptr = output_buffer;
    FunObjectCommon2 funobject2;
    funobject2.pparam1 = NULL;
    funobject2.pparam2 = &out_param;
    int iloop_max_times = 10000; ///< 最多一万次，防止死循环
    int i = 0;
    for (i=0; i<iloop_max_times; ++i) { ///< 由于数据不是一次性合成的，是由多次调用后合成的，所以这里会循环调用
      n_return = pModule->GetProperty(PROP_FUN_CALLABILITY, &funobject2);
      if (YSOS_ERROR_SUCCESS != n_return) {
        break;
      }

      ///< 输出buff可能为空的呢
      if (NULL != out_param.buf_interface_ptr) {
        UINT8* audio_data = NULL;
        UINT32 data_size = 0;
        out_param.buf_interface_ptr->GetBufferAndLength(&audio_data,&data_size);
        if (0 == data_size) { ///< 表示解析成功，但当前没有数据，所以返因true
          continue;
        }
      }
      YSOS_LOG_DEBUG("AudioCaptureExtCallback InvokeNextCallback");
      n_return = InvokeNextCallback(out_param.buf_interface_ptr, NULL, context);
      if (0 == out_param.is_not_finish) { ///< have finish tts all voice data
        n_return = YSOS_ERROR_SKIP;
        break;
      }
    }
    if (i > iloop_max_times) {
      YSOS_LOG_DEBUG("execute too much, i = " << i);
    }
  } while (0);

  YSOS_LOG_DEBUG("RealCallback out");
  return n_return;
}

int AudioCaptureExtCallback::Initialized(const std::string &key, const std::string &value) {
  BaseThreadModuleCallbackImpl::Initialized(key, value);

  if (strcasecmp(key.c_str(), "input_type") == 0) {
    input_type_ = value;
  } else if (strcasecmp(key.c_str(), "output_type") == 0) {
    output_type_ = value;
  } else if (strcasecmp(key.c_str(), "input_ability") == 0) {
    input_ability_name_ = value;
  } else if (strcasecmp(key.c_str(), "output_ability") == 0) {
    output_ability_name_ = value;
  }

  YSOS_LOG_DEBUG("input_type = " << input_type_ <<"; output_type = " << output_type_ << "; input_ability = " << input_ability_name_ << "; output_ability = " << output_ability_name_);
  return YSOS_ERROR_SUCCESS;
}

}
