/**
 *@file audiocaptureextmodule.h
 *@brief audio capture ext module interface
 *@version 1.0
 *@author l.j..
 *@date Created on: 2016-10-20 14:50:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
/// self header
#include "../include/audioplayextmodule.h"

/// ysos headers
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/manager_declare.h"
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"
#include "../../../../../../core/trunk/main/public/include/os_hal_package/bufferutility.h"
#include "../../../../../../core/trunk/main/public/include/sys_interface_package/ysos_enum.h"

namespace ysos {
DECLARE_PLUGIN_REGISTER_INTERFACE(AudioPlayExtModule, ModuleInterface);
AudioPlayExtModule::AudioPlayExtModule(const std::string &strClassName):BaseModuleImpl(strClassName) {
  logger_ = GetUtility()->GetLogger("ysos.audioplayext");
  driver_prt_ = NULL;
}

AudioPlayExtModule::~AudioPlayExtModule() {
  driver_prt_ = NULL;
}

int AudioPlayExtModule::Initialized(const std::string &key, const std::string &value)  {
  int n_return = YSOS_ERROR_FAILED;

  do {
    if (strcasecmp(key.c_str(), "relate_driver") ==0) { ///< 关联的驱动 driver的逻辑名, 目前只支持一个driver
      releate_driver_name_= value;
    }

    n_return = YSOS_ERROR_SUCCESS;
  } while (0);

  return n_return;
}

int AudioPlayExtModule::RealOpen(LPVOID param) {
  YSOS_LOG_DEBUG("module RealOpen execute");
  int n_return = YSOS_ERROR_FAILED;

  do {
    if (NULL == driver_prt_) {
      driver_prt_ = GetDriverInterfaceManager()->FindInterface(releate_driver_name_);
      if (NULL == driver_prt_) {
        YSOS_LOG_DEBUG("get driver_prt_ failed");
        break;
      }
    }

    SetStatusEventCallback(driver_prt_);

    n_return = driver_prt_->Open(param);
    if (YSOS_ERROR_SUCCESS != n_return) {
      break;
    }
  } while (0);
  YSOS_LOG_DEBUG("module RealOpen done");

  return n_return;
}

int AudioPlayExtModule::RealRun(void) {
    return YSOS_ERROR_SUCCESS;
}

int AudioPlayExtModule::RealPause(void) {
  return YSOS_ERROR_SUCCESS;
}

int AudioPlayExtModule::Ioctl(INT32 control_id, LPVOID param) {
  return BaseModuleImpl::Ioctl(control_id,param);
}

int AudioPlayExtModule::Flush(LPVOID param /*= nullptr*/) {
  return YSOS_ERROR_SUCCESS;
}

int AudioPlayExtModule::Initialize(LPVOID param /*= nullptr*/) {
  return BaseModuleImpl::Initialize(param);
}

int AudioPlayExtModule::UnInitialize(LPVOID param /*= nullptr*/) {
  return YSOS_ERROR_SUCCESS;
}

int AudioPlayExtModule::RealClose() {

  int n_return = YSOS_ERROR_FAILED;

  driver_prt_->Close();
  driver_prt_ = NULL;

  return n_return;
}

int AudioPlayExtModule::InitalDataInfo(void) {
  return YSOS_ERROR_SUCCESS;
}

int AudioPlayExtModule::RealStop() {

  int n_return = YSOS_ERROR_FAILED;

  if (driver_prt_) {
    BufferInterfacePtr ablity_name_buffptr = GetBufferUtility()->CreateBuffer(ABSTR_VOICEPLAY);
    n_return = driver_prt_->Ioctl(CMD_ICC_STOP, ablity_name_buffptr, NULL);
  }

  return n_return;
}

int AudioPlayExtModule::GetProperty(int iTypeId, void *piType) {
  int n_return = YSOS_ERROR_FAILED;

  do {
    switch (iTypeId) {
    case PROP_FUN_CALLABILITY: {
      FunObjectCommon2* pobject = reinterpret_cast<FunObjectCommon2*>(piType);
      if (NULL == pobject) {
        YSOS_LOG_DEBUG("error call property PROP_Fun_CallAbility, the param is null");
        break;
      }
      AbilityParam* pin = reinterpret_cast<AbilityParam*>(pobject->pparam1);
      AbilityParam* pout = reinterpret_cast<AbilityParam*>(pobject->pparam2);

      {
        ///< 如果当前module没有在运行，则输出结果都是空，在能力调以前判断一次，在能力调之后判断一次
        INT32 module_state = 0;
        int igetstate_ret = GetState(3000, &module_state, NULL);  ///< 3秒超时，如果拿不到module的状态，则报错
        if (YSOS_ERROR_SUCCESS != igetstate_ret) {
          YSOS_LOG_DEBUG("error my cant get module status in 3000 miliseconds");
        }
        if (PROP_RUN != module_state
            || YSOS_ERROR_SUCCESS != igetstate_ret) {
          ///< 如果当前module没有在运行或获取状态失败，则输出结果都是空，callbck 拿到结果为空后则不会向下一个callback处理
          YSOS_LOG_DEBUG("current is not run (current state:%" << module_state <<" ) getsatatus fun return:" << igetstate_ret << " , call ability return null");
          if (pout && pout->buf_interface_ptr) {
            pout->buf_interface_ptr->SetLength(0);  ///< 设置长度为空
          }
          n_return = YSOS_ERROR_SKIP; ///< 通知callback ,module 已经停了，callback 也不要再往下传了
          break;
        }
      }

      if (NULL == driver_prt_) {
        break; ///< 表示不支持此调用
      }
      n_return = driver_prt_->GetProperty(iTypeId, piType);
      if (YSOS_ERROR_SKIP == n_return) {
        break;
      }

      {
        ///< 如果当前module没有在运行，则输出结果都是空，在能力调以前判断一次，在能力调之后判断一次
        INT32 module_state = 0;
        int igetstate_ret = GetState(3000, &module_state, NULL);  ///< 3秒超时，如果拿不到module的状态，则报错
        if (YSOS_ERROR_SUCCESS != igetstate_ret) {
          YSOS_LOG_DEBUG("error my cant get module status in 3000 miliseconds");
        }
        if (PROP_RUN != module_state
            || YSOS_ERROR_SUCCESS != igetstate_ret) {
          ///< 如果当前module没有在运行或获取状态失败，则输出结果都是空，callbck 拿到结果为空后则不会向下一个callback处理
          YSOS_LOG_DEBUG("current is not run (current state:%" << module_state <<" ) getsatatus fun return:" << igetstate_ret << " , call ability return null");
          if (pout && pout->buf_interface_ptr) {
            pout->buf_interface_ptr->SetLength(0);  ///< 设置长度为空
          }
          n_return = YSOS_ERROR_SKIP; ///< 通知callback ,module 已经停了，callback 也不要再往下传了
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

}
