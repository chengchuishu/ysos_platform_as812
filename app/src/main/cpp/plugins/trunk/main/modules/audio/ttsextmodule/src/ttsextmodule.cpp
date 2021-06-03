/**
 *@file ttsextmodule.h
 *@brief ttsext module interface
 *@version 1.0
 *@author l.j..
 *@date Created on: 2016-10-20 14:50:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
/// self header
#include "../include/ttsextmodule.h"

/// boost headers
#include <boost/filesystem.hpp>

/// ysos headers
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/manager_declare.h"
#include "../../../../../../core/trunk/main/public/include/os_hal_package/bufferutility.h"
#include "../../../../../../core/trunk/main/public/include/sys_interface_package/ysos_enum.h"
#include "../../../../../../core/trunk/main/public/include/core_help_package/jsonutility.h"

namespace ysos {
DECLARE_PLUGIN_REGISTER_INTERFACE(TtsExtModule, ModuleInterface);
TtsExtModule::TtsExtModule(const std::string &strClassName):BaseModuleImpl(strClassName) {
  logger_ = GetUtility()->GetLogger("ysos.asrext");
  driver_prt_ = NULL;
}

TtsExtModule::~TtsExtModule() {
  driver_prt_ = NULL;
}

int TtsExtModule::Initialized(const std::string &key, const std::string &value)  {
  int n_return = YSOS_ERROR_SUCCESS;

  do {
    if ("relate_driver" == key) {
      relate_driver_ = value;
    } else if ("instead_input_type" == key) {
      instead_input_type_ = value;
    }
  } while (0);

  return n_return;
}

int TtsExtModule::RealOpen(LPVOID param) {
  YSOS_LOG_DEBUG("module RealOpen execute");
  int n_return = YSOS_ERROR_FAILED;

  do {
    if (NULL == driver_prt_) {
      driver_prt_ = GetDriverInterfaceManager()->FindInterface(relate_driver_);
      if (NULL == driver_prt_) {
        YSOS_LOG_DEBUG("get driver_prt_ failed");
        break;
      }
    }
    n_return = driver_prt_->Open(param);
    if (YSOS_ERROR_SUCCESS != n_return)
      break;

    n_return = YSOS_ERROR_SUCCESS;
  } while (0);
  YSOS_LOG_DEBUG("module RealOpen done");

  return n_return;
}

int TtsExtModule::RealRun(void) {
  return YSOS_ERROR_SUCCESS;
}

int TtsExtModule::RealPause(void) {
  return YSOS_ERROR_SUCCESS;
}

int TtsExtModule::Ioctl(INT32 control_id, LPVOID param) {
  int n_return = YSOS_ERROR_FAILED;

  switch (control_id) {
  case CMD_CUSTOM_MODE: {
    std::string *str = reinterpret_cast<std::string*>(param);
    YSOS_LOG_DEBUG("CMD_CUSTOM_MODE get data = " << *str);
    n_return = CustomAudioData(*str);
    break;
  }

  case CMD_SET_CUSTOM_DATA: {
    std::string *str = reinterpret_cast<std::string*>(param);
    YSOS_LOG_DEBUG("get tts CMD_SET_CUSTOM_DATA get data = " << *str);
    JsonValue json_value;
    n_return = GetJsonUtil()->JsonObjectFromString(*str, json_value);
    if (YSOS_ERROR_SUCCESS != n_return) {
      YSOS_LOG_ERROR("CMD_SET_CUSTOM_DATA parse to json failed: " << *str << " | " << logic_name_);
      break;
    }

    if (true == json_value.empty()) {
      YSOS_LOG_ERROR("json_value is empty");
      break;
    }

    std::string text_data;
    if (true == json_value.isMember("text") &&
        true == json_value["text"].isString()) {
      YSOS_LOG_DEBUG("parse success");
      text_data = json_value["text"].asString();
    } else {
      YSOS_LOG_ERROR("parse failed");
      n_return = YSOS_ERROR_FAILED;
      break;
    }
    YSOS_LOG_ERROR("text_data [" << text_data << "]");
    n_return = CustomAudioData(text_data);
    break;
  }

  case CMD_AUDIO_TTS_MODE: {
    YSOS_LOG_DEBUG("CMD_AUDIO_TTS_MODE in");
    std::string param_value = *reinterpret_cast<std::string*>(param);
    BufferInterfacePtr in_buffer_ptr = GetBufferUtility()->CreateBuffer(param_value);
    if (NULL == in_buffer_ptr) {
      YSOS_LOG_DEBUG("ioctl create buffer failed");
      break;
    }
    driver_prt_->Ioctl(CMD_AUDIO_TTS_MODE, in_buffer_ptr, NULL);
    n_return = buffer_pool_ptr_->ReleaseBuffer(in_buffer_ptr);
    if (n_return != YSOS_ERROR_SUCCESS) {
      YSOS_LOG_DEBUG("execute ReleaseBuffer failed");
    }
    break;
  }
  case CMD_AUDIO_TTS_SPEAKER: {
    YSOS_LOG_DEBUG("CMD_AUDIO_TTS_SPEAKER in");
    std::string param_value = *reinterpret_cast<std::string*>(param);
    BufferInterfacePtr in_buffer_ptr = GetBufferUtility()->CreateBuffer(param_value);
    if (NULL == in_buffer_ptr) {
      YSOS_LOG_DEBUG("ioctl create buffer failed");
      break;
    }
    driver_prt_->Ioctl(CMD_AUDIO_TTS_SPEAKER, in_buffer_ptr, NULL);
    n_return = buffer_pool_ptr_->ReleaseBuffer(in_buffer_ptr);
    if (n_return != YSOS_ERROR_SUCCESS) {
      YSOS_LOG_DEBUG("execute ReleaseBuffer failed");
    }
    break;
  }

  default:
    BaseModuleImpl::Ioctl(control_id,param);
  }

  return n_return;
}

int TtsExtModule::Flush(LPVOID param /*= nullptr*/) {
  return YSOS_ERROR_SUCCESS;
}

int TtsExtModule::Initialize(LPVOID param /*= nullptr*/) {
  int n_return = YSOS_ERROR_FAILED;

  do {
    std::string data_path = GetPackageConfigImpl()->GetConfPath() + "../data/";
    if (!boost::filesystem::exists(data_path)) {
      boost::filesystem::create_directory(data_path);
    }
    data_path = data_path + "ttsextmodule/";
    if (!boost::filesystem::exists(data_path)) {
      boost::filesystem::create_directory(data_path);
    }
    data_path = data_path + "pcm/";
    if (!boost::filesystem::exists(data_path)) {
      if (!boost::filesystem::create_directory(data_path)) {
        YSOS_LOG_ERROR("create directory failed, data_path = " << data_path);
        break;
      }
    }

    n_return = BaseModuleImpl::Initialize(param);
  } while (0);


  return n_return;
}

int TtsExtModule::UnInitialize(LPVOID param /*= nullptr*/) {
  return YSOS_ERROR_SUCCESS;
}

int TtsExtModule::RealClose() {

  driver_prt_->Close();

  return YSOS_ERROR_SUCCESS;
}

int TtsExtModule::InitalDataInfo(void) {
  return YSOS_ERROR_SUCCESS;
}

int TtsExtModule::RealStop() {
  BufferInterfacePtr ablity_name_buffptr = GetBufferUtility()->CreateBuffer("tts");
  if (driver_prt_)
    driver_prt_->Ioctl(CMD_ICC_STOP, ablity_name_buffptr, NULL);

  return YSOS_ERROR_SUCCESS;
}

int TtsExtModule::GetProperty(int iTypeId, void *piType) {
  int n_return = YSOS_ERROR_FAILED;

  do {
    switch (iTypeId) {
    case PROP_FUN_CALLABILITY: {
      //< 如果当前状态是realstop状态，则callback输出为空
      FunObjectCommon2* pobject = reinterpret_cast<FunObjectCommon2*>(piType);
      if (NULL == pobject) {
        YSOS_LOG_DEBUG("error call property PROP_Fun_CallAbility, the param is null");
        break;
      }
      AbilityParam* pin = reinterpret_cast<AbilityParam*>(pobject->pparam1);
      AbilityParam* pout = reinterpret_cast<AbilityParam*>(pobject->pparam2);

      {
        //< 如果当前module没有在运行，则输出结果都是空，在能力调以前判断一次，在能力调之后判断一次
        INT32 module_state = 0;
        int igetstate_ret = GetState(3000, &module_state, NULL); //< 3秒超时，如果拿不到module的状态，则报错
        if (YSOS_ERROR_SUCCESS != igetstate_ret) {
          YSOS_LOG_DEBUG("error can't get module status in 3000 miliseconds");
          break;
        }
        if (PROP_RUN != module_state || YSOS_ERROR_SUCCESS != igetstate_ret) {
          //< 如果当前module没有在运行或获取状态失败，则输出结果都是空，callbck拿到结果为空后则不会向下一个callback处理
          YSOS_LOG_DEBUG("current is not run, call ability return null");
          if (pout && pout->buf_interface_ptr) {
            pout->buf_interface_ptr->SetLength(0); //<  设置长度为空
          }
          n_return = YSOS_ERROR_SKIP; //< 通知callback module已经停了，callback也不要再往下传了
          break;
        }
      }

      if (pin) { ///< 输入类型转换
        //strcpy_s(pin->data_type_name, sizeof(pin->data_type_name), instead_input_type_.c_str());
        strcpy(pin->data_type_name, instead_input_type_.c_str());
      }
      n_return = driver_prt_->GetProperty(iTypeId, piType);

      {
        INT32 module_state = 0;
        int igetstate_ret = GetState(3000, &module_state, NULL);
        if (YSOS_ERROR_SUCCESS != igetstate_ret) {
          YSOS_LOG_DEBUG("error can't get module status in 3000 miliseconds");
          break;
        }
        if (PROP_RUN != module_state || YSOS_ERROR_SUCCESS != igetstate_ret) {
          //< 如果当前module没有在运行或获取状态失败，则输出结果都是空，callbck拿到结果为空后则不会向下一个callback处理
          YSOS_LOG_DEBUG("current is not run, call ability return null");
          if (pout && pout->buf_interface_ptr) {
            pout->buf_interface_ptr->SetLength(0); //<  设置长度为空
          }
          n_return = YSOS_ERROR_SKIP; //< 通知callback module已经停了，callback也不要再往下传了
          break;
        }
      }
    }
    break;
    default: {
      n_return = BaseModuleImpl::GetProperty(iTypeId, piType);
    }
    }
  } while (0);

  return n_return;
}

int TtsExtModule::CustomAudioData(const std::string &audio_data) {
  int n_return = YSOS_ERROR_FAILED;

  do {
    JsonValue json_value;
    json_value["text"] = audio_data.c_str();
    json_value["rc"] = 0;
    json_value["rg"] = 0;
    json_value["answer_best"] = audio_data.c_str();
    json_value["service"] = "chat";
    json_value["interrupt_type"] = 0;

    std::string json_str;
    n_return = GetJsonUtil()->JsonValueToString(json_value, json_str);
    if (YSOS_ERROR_SUCCESS != n_return) {
      YSOS_LOG_ERROR("CustomAudioData wrap to json failed: " << audio_data << " | " << logic_name_);
      break;
    }
    YSOS_LOG_DEBUG("json_str = " << json_str);
    
    json_str = GetUtility()->GbkToUtf8(json_str);//add for linux  for encode transfer
    BufferInterfacePtr buffer_ptr = GetBufferUtility()->CreateBuffer(json_str);
    if (NULL == buffer_ptr) {
      YSOS_LOG_ERROR("CustomAudioData wrap to buffer failed: " << audio_data << " | " << logic_name_);
      break;
    }

    CallbackInterfacePtr cur_callback_ptr;
    {
      AutoLockOper lock(pre_callback_queue_lock_);
      cur_callback_ptr = pre_callback_queue_->FindCallback(true);
    }

    if (NULL == cur_callback_ptr) {
      YSOS_LOG_ERROR("does not have next callback: " << logic_name_);
      break;
    }

    BaseModuelCallbackContext callback_contex;
    callback_contex.prev_callback_ptr = NULL;
    callback_contex.prev_module_ptr = NULL;
    callback_contex.cur_callback_ptr = cur_callback_ptr.get();
    callback_contex.cur_module_ptr = this;;
    n_return = cur_callback_ptr->Callback(buffer_ptr, NULL, &callback_contex);

  } while (0);

  return n_return;
}

}
