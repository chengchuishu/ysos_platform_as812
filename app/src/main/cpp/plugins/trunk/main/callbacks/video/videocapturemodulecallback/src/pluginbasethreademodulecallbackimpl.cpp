/**   //NOLINT
  *@file PluginBaseThreadModuleCallbackImpl.cpp
  *@brief Definition of 
  *@version 0.1
  *@author wanfenfen
  *@date Created on: 2016:12:15   13:54
  *@copyright Copyright(c) 2016 YunShen Technology. All rights reserved.
  * 
  *@howto Usage:
  *@todo
  */

/// self header
#include "../../../public/include/common/pluginbasethreademodulecallbackimpl.h"

/// boost headers
#include <boost/thread/thread.hpp>

/// ysos headers
#include "../../../../../../core/trunk/main/public/include/os_hal_package/lock.h"              //NOLINT
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"

/// third party ifly headers
//#include <ifly/include/qisr.h>
//#include <ifly/include/msp_cmn.h>
//#include <ifly/include/msp_errors.h>

/// private headers
#include "../../../public/include/common/commonenumdefine.h"
#include "../../../public/include/common/commonstructdefine.h"

namespace ysos {
//DECLARE_PLUGIN_REGISTER_INTERFACE(PluginBaseThreadModuleCallbackImpl, CallbackInterface);
PluginBaseThreadModuleCallbackImpl::PluginBaseThreadModuleCallbackImpl(const std::string &strClassName /* =ASRCallback */):/*BaseModuleCallbackImpl*/BaseThreadModuleCallbackImpl(strClassName) {
  //input_type_ = /*"PCM"*/DTSTR_STREAM_PCMX1X;
  //output_type_ = /*"TEXT"*/DTSTR_TEXT_JSON;
  logger_ = GetUtility()->GetLogger("ysos.audio");
}

int PluginBaseThreadModuleCallbackImpl::IsReady() {
  return YSOS_ERROR_SUCCESS;
}


int PluginBaseThreadModuleCallbackImpl::Initialized(const std::string &key, const std::string &value) {
  BaseModuleCallbackImpl::Initialized(key, value);
#ifdef _WIN32
  if (stricmp(key.c_str(), "input_type") ==0)
  {
    input_type_ = value;
  }else if(stricmp(key.c_str(), "output_type" )==0) {
    output_type_ = value;
  }else if(stricmp(key.c_str(), "input_ability") ==0) {
    input_ability_name_ = value;
  }else if(stricmp(key.c_str(), "output_ability") ==0) {
    output_ability_name_ = value;
  }
#else
  if (strcasecmp(key.c_str(), "input_type") ==0)
  {
    input_type_ = value;
  }else if(strcasecmp(key.c_str(), "output_type" )==0) {
    output_type_ = value;
  }else if(strcasecmp(key.c_str(), "input_ability") ==0) {
    input_ability_name_ = value;
  }else if(strcasecmp(key.c_str(), "output_ability") ==0) {
    output_ability_name_ = value;
  }
#endif

  YSOS_LOG_DEBUG("logic name:" << logic_name_ << " input_type:" << input_type_ <<"  output_type:" << output_type_);
  return YSOS_ERROR_SUCCESS;
}

int PluginBaseThreadModuleCallbackImpl::RealCallback(BufferInterfacePtr input_buffer, BufferInterfacePtr next_output_buffer, BufferInterfacePtr pre_output_buf, void *context) {
  BaseModuelCallbackContext* module_callback_context_ptr = NULL;
  YSOS_LOG_DEBUG("PluginBaseThreadModuleCallbackImpl::RealCallback[Enter]");
  module_callback_context_ptr = reinterpret_cast<BaseModuelCallbackContext*>(context);
  //assert(module_callback_context_ptr);

  BaseInterface* pModule= dynamic_cast<BaseInterface*>(module_callback_context_ptr->cur_module_ptr);
  //assert(pModule);
  AbilityParam /*input_param,*/ out_param;
  /*sprintf_s(input_param.ability_name, sizeof(input_param.ability_name), ABSTR_ASR);
  sprintf_s(input_param.data_type_name, sizeof(input_param.data_type_name), DTSTR_TEXT_PURE);
  input_param.buf_interface_ptr = input_buffer;*/
#ifdef _WIN32
  sprintf_s(out_param.ability_name, sizeof(out_param.ability_name), /*ABSTR_TTS*//*ABSTR_VIDEOCAPTURE*/output_ability_name_.c_str());
  sprintf_s(out_param.data_type_name, sizeof(out_param.data_type_name), /*DTSTR_STREAM_PCMX1X_P2*//*DTSTR_STREAM_FRAME_RGB24_P6*/output_type_.c_str());
#else
    snprintf(out_param.ability_name, sizeof(out_param.ability_name), "%s", /*ABSTR_TTS*//*ABSTR_VIDEOCAPTURE*/output_ability_name_.c_str());
    snprintf(out_param.data_type_name, sizeof(out_param.data_type_name), "%s", /*DTSTR_STREAM_PCMX1X_P2*//*DTSTR_STREAM_FRAME_RGB24_P6*/output_type_.c_str());   
#endif
  out_param.buf_interface_ptr = next_output_buffer;

  FunObjectCommon2 funobject2;
  funobject2.pparam1 = NULL; ///&input_param;
  funobject2.pparam2 = &out_param;

  int iloop_max_times = 10000;  ///< 最多一万次，防止死循环
  int i=0;
  for(i=0; i<iloop_max_times; ++i) {  ///< 由于数据不是一次性合成的，是由多次调用后合成的，所以这里会循环调用。
    int iret= pModule->GetProperty(PROP_FUN_CALLABILITY, &funobject2);
    if(YSOS_ERROR_SUCCESS != iret)
      return iret;
    // 输出buff可能为空的呢。
    if(NULL != out_param.buf_interface_ptr) {
      UINT8* audio_data = NULL;
      UINT32 data_size =0;
      out_param.buf_interface_ptr->GetBufferAndLength(&audio_data,&data_size);
      if(0 == data_size) {  ///< 表示解析成功，但当前没有数据，所以返因true
        continue;
      }
    }
    
    iret = InvokeNextCallback(out_param.buf_interface_ptr, NULL, context);
    if(0 == out_param.is_not_finish)  ///< have finish tts all voice data
      break;
  }
  //assert(i < iloop_max_times);
  YSOS_LOG_DEBUG("i < iloop_max_times :" << i <<" < " << iloop_max_times);
  return YSOS_ERROR_SKIP;
}


}
