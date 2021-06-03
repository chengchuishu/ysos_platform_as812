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
#include "../include/audiocaptureextmodule.h"

/// boost headers
#include <boost/thread/thread.hpp>

/// ysos headers
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/manager_declare.h"
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"
#include "../../../../../../core/trunk/main/public/include/os_hal_package/bufferutility.h"
#include "../../../../../../core/trunk/main/public/include/sys_interface_package/ysos_enum.h"


namespace ysos {
DECLARE_PLUGIN_REGISTER_INTERFACE(AudioCaptureExtModule, ModuleInterface);
AudioCaptureExtModule::AudioCaptureExtModule(const std::string &strClassName):BaseThreadModuleImpl(strClassName) {
  logger_ = GetUtility()->GetLogger("ysos.audiocaptureext");
  driver_prt_ = NULL;
  pcm_dir_ = "";
}

AudioCaptureExtModule::~AudioCaptureExtModule() {
  driver_prt_ = NULL;
}

int AudioCaptureExtModule::Initialized(const std::string &key, const std::string &value)  {
  int n_return = YSOS_ERROR_FAILED;

  do {
    if (strcasecmp(key.c_str(), "relate_driver") ==0) { ///< 关联的驱动 driver的逻辑名, 目前只支持一个driver
      releate_driver_name_= value;
    } else if (strcasecmp(key.c_str(), "thread_interval") ==0) { ///< 线程module 的timeout 参数（单位毫秒)
      int thread_interval = atoi(value.c_str());
      thread_data_->timeout = thread_interval;
    } else if (strcasecmp(key.c_str(), "pcm_dir") ==0) {
      pcm_dir_ = value;
    }
    n_return = YSOS_ERROR_SUCCESS;
  } while (0);

  return n_return;
}

int AudioCaptureExtModule::RealOpen(LPVOID param) {
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

    data_ptr_ = GetDataInterfaceManager()->GetData(GLOBAL_DATA_KEY);
    if (!data_ptr_) {
      YSOS_LOG_DEBUG("get data_ptr failed");
      break;
    }

    if (!pcm_dir_.empty()) {
      n_return = driver_prt_->SetProperty(PROP_INPUT_SOUND, &pcm_dir_);
      if (YSOS_ERROR_SUCCESS != n_return) {
        break;
      }
    }

    n_return = driver_prt_->Open(param);
    if (YSOS_ERROR_SUCCESS != n_return) {
      break;
    }

    n_return = BaseThreadModuleImpl::RealOpen(param);
    if (n_return != YSOS_ERROR_SUCCESS)
      break;
  } while (0);
  YSOS_LOG_DEBUG("module RealOpen done");

  return n_return;
}

int AudioCaptureExtModule::RealRun(void) {
  int n_return = YSOS_ERROR_FAILED;

  if (driver_prt_) {
    n_return = driver_prt_->Ioctl(CMD_ICC_START, NULL, NULL);
  }

  return n_return;
}

int AudioCaptureExtModule::RealPause(void) {
  return YSOS_ERROR_SUCCESS;
}

int AudioCaptureExtModule::Ioctl(INT32 control_id, LPVOID param) {
  int n_return = YSOS_ERROR_FAILED;
  YSOS_LOG_DEBUG("AudioCaptureExtModule::Ioctl in, control_id = " << control_id);
  do {
    switch (control_id) {
    case CMD_AUDIO_CAPTURE_RUN: {
      n_return = driver_prt_->Ioctl(CMD_AUDIO_CAPTURE_RUN, NULL, NULL);
      break;
    }
    case CMD_AUDIO_CAPTURE_STOP: {
      n_return = driver_prt_->Ioctl(CMD_AUDIO_CAPTURE_STOP, NULL, NULL);
      break;
    }
    default: {
      YSOS_LOG_DEBUG("unsupport ioctl function, control_id = " << control_id);
      BaseThreadModuleImpl::Ioctl(control_id,param);
      n_return = YSOS_ERROR_FAILED;
      break;
    }
    }
  } while (0);

  return n_return;
}

int AudioCaptureExtModule::Flush(LPVOID param /*= nullptr*/) {
  return YSOS_ERROR_SUCCESS;
}

int AudioCaptureExtModule::Initialize(LPVOID param /*= nullptr*/) {
  return BaseThreadModuleImpl::Initialize(param);
}

int AudioCaptureExtModule::UnInitialize(LPVOID param /*= nullptr*/) {
  return YSOS_ERROR_SUCCESS;
}

int AudioCaptureExtModule::RealClose() {
  int n_return = YSOS_ERROR_FAILED;

  driver_prt_->Close();
  driver_prt_ = NULL;

  n_return = BaseThreadModuleImpl::RealClose();

  return n_return;
}

int AudioCaptureExtModule::InitalDataInfo(void) {
  return YSOS_ERROR_SUCCESS;
}

int AudioCaptureExtModule::RealStop() {
  int n_return = YSOS_ERROR_FAILED;

  if (driver_prt_) {
    n_return = driver_prt_->Ioctl(CMD_ICC_STOP, NULL, NULL);
  }

  return n_return;
}

int AudioCaptureExtModule::GetProperty(int iTypeId, void *piType) {
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
      n_return = BaseThreadModuleImpl::GetProperty(iTypeId, piType);
    }
    }
  } while (0);

  return n_return;
}

bool AudioCaptureExtModule::NeedConvertDataTypeInfo(const std::string source_input_datatype, const std::string source_output_datatypa, ModuleDataTypeConvertInfo &convert_info) {
  bool result = false;

  do {
    if (source_input_datatype.length() <=0 && source_output_datatypa.length() <=0) {
      YSOS_LOG_DEBUG("param error");
      break;
    }

    std::list<ModuleDataTypeConvertInfo>::iterator it = list_module_datatype_convert_info_.begin();
    for (it; it != list_module_datatype_convert_info_.end(); ++it) {
      ModuleDataTypeConvertInfo* pinfo = &(*it);
      if (source_input_datatype.length() > 0 && pinfo->source_input_datatype.length() > 0) {
        if (strcasecmp(source_input_datatype.c_str(), pinfo->source_input_datatype.c_str()) != 0)
          continue;
      }
      if (source_output_datatypa.length() > 0 && pinfo->source_output_datatype.length() > 0) {
        if (strcasecmp(source_output_datatypa.c_str(), pinfo->source_output_datatype.c_str()) != 0)
          continue;
      }
      //< 匹配上了
      YSOS_LOG_DEBUG("convert success, source_input = "<< source_input_datatype <<",source_output = " << source_output_datatypa << ",dest_input = " << pinfo->dest_input_datatype << ",dest_output = " << pinfo->dest_output_datatype);
      convert_info = *pinfo;
      result = true;
    }
  } while (0);

  return result;
}

}
