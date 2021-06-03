/**
 *@file asrextmodule.h
 *@brief asrext module interface
 *@version 1.0
 *@author l.j..
 *@date Created on: 2016-10-20 14:50:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
/// self header
#include "../include/asrextmodule.h"

/// boost headers
#include <boost/thread/thread.hpp>

/// ysos headers
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/manager_declare.h"
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"
#include "../../../../../../core/trunk/main/public/include/os_hal_package/bufferutility.h"
#include "../../../../../../core/trunk/main/public/include/sys_interface_package/ysos_enum.h"

#include <json/json.h>

namespace ysos {
DECLARE_PLUGIN_REGISTER_INTERFACE(AsrExtModule, ModuleInterface);
AsrExtModule::AsrExtModule(const std::string &strClassName /* =ASRModule */):BaseModuleImpl(strClassName) {
  logger_ = GetUtility()->GetLogger("ysos.asrext");
  ioctl_cmd_ = CMD_AUDIO_AUTO;
  ioctl_value_ = "";
  ioctl_detect_ = false;
  driver_prt_ = NULL;
}

AsrExtModule::~AsrExtModule() {
  driver_prt_ = NULL;
}

int AsrExtModule::Initialized(const std::string &key, const std::string &value)  {
  int n_return = YSOS_ERROR_SUCCESS;

  if (strcasecmp(key.c_str(), "output_ability") ==0) { //< 输出能力,如果有多个能力，则使用 ｜ 分开
    string_output_abilities_ = value;
    GetUtility()->SplitString(value, "|", list_output_abilities_);
  }

  return n_return;
}

int AsrExtModule::RealOpen(LPVOID param) {
  YSOS_LOG_DEBUG("module RealOpen execute");
  int n_return = YSOS_ERROR_FAILED;

  do {
    if (NULL == driver_prt_) {
      driver_prt_ = GetDriverInterfaceManager()->FindInterface("default@AudioExtDriver");
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

int AsrExtModule::RealRun(void) {
  return YSOS_ERROR_SUCCESS;
}

int AsrExtModule::RealPause(void) {
  return YSOS_ERROR_SUCCESS;
}

int AsrExtModule::Ioctl(INT32 control_id, LPVOID param) {
  int n_return = YSOS_ERROR_FAILED;

  do {
    switch (control_id) {
    case CMD_AUDIO_ASR: {
      YSOS_LOG_DEBUG("CMD_AUDIO_ASR in");
      ioctl_cmd_ = CMD_AUDIO_ASR;
      ioctl_value_ = *reinterpret_cast<std::string*>(param);
      YSOS_LOG_DEBUG("ioctl_value_ = " << ioctl_value_);
      break;
    }

    case CMD_AUDIO_JSON_ASR: {
      YSOS_LOG_DEBUG("CMD_AUDIO_JSON_ASR in");
      ioctl_cmd_ = CMD_AUDIO_ASR;
      std::string param_value = *reinterpret_cast<std::string*>(param);
      YSOS_LOG_DEBUG("param_value = " << param_value);
      Json::Value json_value;
      Json::Reader json_reader;
      json_reader.parse(param_value, json_value, true);
      ioctl_value_ = json_value.get("text", "").asString();
      YSOS_LOG_DEBUG("ioctl_value_ = " << ioctl_value_);
      break;
    }

    case CMD_AUDIO_ASR_MODE: {
      YSOS_LOG_DEBUG("CMD_AUDIO_ASR_MODE in");
      std::string param_value = *reinterpret_cast<std::string*>(param);
      BufferInterfacePtr in_buffer_ptr = GetBufferUtility()->CreateBuffer(param_value);
      if (NULL == in_buffer_ptr) {
        YSOS_LOG_DEBUG("ioctl create buffer failed");
        break;
      }
      driver_prt_->Ioctl(CMD_AUDIO_ASR_MODE, in_buffer_ptr, NULL);
      n_return = buffer_pool_ptr_->ReleaseBuffer(in_buffer_ptr);
      if (n_return != YSOS_ERROR_SUCCESS) {
        YSOS_LOG_DEBUG("execute ReleaseBuffer failed");
        break;
      }
      break;
    }

    case CMD_AUDIO_ASR_ACCENT: {
      YSOS_LOG_DEBUG("CMD_AUDIO_ASR_ACCENT in");
      std::string param_value = *reinterpret_cast<std::string*>(param);
      BufferInterfacePtr in_buffer_ptr = GetBufferUtility()->CreateBuffer(param_value);
      if (NULL == in_buffer_ptr) {
        YSOS_LOG_DEBUG("ioctl create buffer failed");
        break;
      }
      driver_prt_->Ioctl(CMD_AUDIO_ASR_ACCENT, in_buffer_ptr, NULL);
      n_return = buffer_pool_ptr_->ReleaseBuffer(in_buffer_ptr);
      if (n_return != YSOS_ERROR_SUCCESS) {
        YSOS_LOG_DEBUG("execute ReleaseBuffer failed");
        break;
      }
      break;
    }

    case CMD_AUDIO_ASR_BEGIN: {
      YSOS_LOG_DEBUG("CMD_AUDIO_ASR_BEGIN in");
      std::string param_value = *reinterpret_cast<std::string*>(param);
      BufferInterfacePtr in_buffer_ptr = GetBufferUtility()->CreateBuffer(param_value);
      if (NULL == in_buffer_ptr) {
        YSOS_LOG_DEBUG("ioctl create buffer failed");
        break;
      }
      driver_prt_->Ioctl(CMD_AUDIO_ASR_BEGIN, in_buffer_ptr, NULL);
      n_return = buffer_pool_ptr_->ReleaseBuffer(in_buffer_ptr);
      if (n_return != YSOS_ERROR_SUCCESS) {
        YSOS_LOG_DEBUG("execute ReleaseBuffer failed");
        break;
      }
      break;
    }

    case CMD_AUDIO_ASR_END: {
      YSOS_LOG_DEBUG("CMD_AUDIO_ASR_END in");
      std::string param_value = *reinterpret_cast<std::string*>(param);
      BufferInterfacePtr in_buffer_ptr = GetBufferUtility()->CreateBuffer(param_value);
      if (NULL == in_buffer_ptr) {
        YSOS_LOG_DEBUG("ioctl create buffer failed");
        break;
      }
      driver_prt_->Ioctl(CMD_AUDIO_ASR_END, in_buffer_ptr, NULL);
      n_return = buffer_pool_ptr_->ReleaseBuffer(in_buffer_ptr);
      if (n_return != YSOS_ERROR_SUCCESS) {
        YSOS_LOG_DEBUG("execute ReleaseBuffer failed");
        break;
      }
      break;
    }

    default: {
      YSOS_LOG_DEBUG("unsupport Ioctl ability");
      break;
    }
    }

    n_return = YSOS_ERROR_SUCCESS;
  } while (0);

  BaseModuleImpl::Ioctl(control_id,param);

  return n_return;
}

int AsrExtModule::Flush(LPVOID param /*= nullptr*/) {
  return YSOS_ERROR_SUCCESS;
}

int AsrExtModule::Initialize(LPVOID param /*= nullptr*/) {
  return BaseModuleImpl::Initialize(param);
}

int AsrExtModule::UnInitialize(LPVOID param /*= nullptr*/) {
  return YSOS_ERROR_SUCCESS;
}

int AsrExtModule::RealClose() {

  driver_prt_->Close();

  return YSOS_ERROR_SUCCESS;
}

int AsrExtModule::InitalDataInfo(void) {
  return YSOS_ERROR_SUCCESS;
}

int AsrExtModule::RealStop() {

  BufferInterfacePtr ablity_name_buffptr = GetBufferUtility()->CreateBuffer(ABSTR_ASRNLP);
  if (driver_prt_)
    driver_prt_->Ioctl(CMD_ICC_STOP, ablity_name_buffptr, NULL);

  return YSOS_ERROR_SUCCESS;
}

int AsrExtModule::GetProperty(int iTypeId, void *piType) {
  int n_return = YSOS_ERROR_FAILED;

  do {
    FunObjectCommon2* pobject = NULL;
    AbilityParam* pin = NULL;
    AbilityParam* pout = NULL;

    if (PROP_FUN_CALLABILITY == iTypeId && CMD_AUDIO_AUTO != ioctl_cmd_) {
      YSOS_LOG_DEBUG("AsrExtModule get ioctl_cmd_");
      iTypeId = ioctl_cmd_;
    }
    switch (iTypeId) {
    case PROP_FUN_CALLABILITY: {
      //< 如果当前状态是realstop状态，则callback输出为空
      pobject = reinterpret_cast<FunObjectCommon2*>(piType);
      if (NULL == pobject) {
        YSOS_LOG_DEBUG("error call property PROP_Fun_CallAbility, the param is null");
        break;
      }
      pin = reinterpret_cast<AbilityParam*>(pobject->pparam1);
      pout = reinterpret_cast<AbilityParam*>(pobject->pparam2);

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

      int driver_ret =  driver_prt_->GetProperty(iTypeId, piType);

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
      n_return = driver_ret;
    }
    break;

    case CMD_AUDIO_ASR: {
      pobject = reinterpret_cast<FunObjectCommon2*>(piType);
      if (NULL == pobject) {
        YSOS_LOG_DEBUG("error call property PROP_Fun_CallAbility, the param is null");
        break;
      }
      pin = reinterpret_cast<AbilityParam*>(pobject->pparam1);
      pout = reinterpret_cast<AbilityParam*>(pobject->pparam2);

#ifdef AUDIO_INTERRUPT
      if (!ioctl_detect_) {
        YSOS_LOG_DEBUG("AsrExtModule begin send DetectAsrResult");
        UINT8* buffer;
        UINT32 buffer_size;
        pout->buf_interface_ptr->GetBufferAndLength(&buffer, &buffer_size);
        memset(buffer, 0, buffer_size);
        YSOS_LOG_DEBUG("buffer_size = " << buffer_size);
        std::string detect("DetectAsrResult");
        memcpy(buffer, detect.c_str(), detect.length());
        pout->buf_interface_ptr->SetLength(detect.length());
        ioctl_detect_ = true;
        n_return = YSOS_ERROR_SUCCESS;
        break;
      }
#endif

      UINT8* buffer;
      UINT32 buffer_size;
      pin->buf_interface_ptr->GetBufferAndLength(&buffer, &buffer_size);
      memset(buffer, 0, buffer_size);
      memcpy(buffer, ioctl_value_.c_str(), ioctl_value_.length());
      //sprintf_s(pin->data_type_name, sizeof(pin->data_type_name), DTSTR_TEXT_PURE);
      sprintf(pin->data_type_name, DTSTR_TEXT_PURE);
      //sprintf_s(pout->data_type_name, sizeof(pout->data_type_name), DTSTR_TEXT_JSON);
      sprintf(pout->data_type_name, DTSTR_TEXT_JSON);
      //sprintf_s(pout->ability_name, sizeof(pout->ability_name), ABSTR_NLP);
      sprintf(pout->ability_name, ABSTR_NLP);
      n_return = driver_prt_->GetProperty(PROP_FUN_CALLABILITY, piType);
      ioctl_cmd_ = CMD_AUDIO_AUTO;
      ioctl_value_ = "";
      ioctl_detect_ = false;
    }
    break;

    default: {
      n_return = BaseModuleImpl::GetProperty(iTypeId, piType);
    }
    }
  } while (0);

  return n_return;
}

}
