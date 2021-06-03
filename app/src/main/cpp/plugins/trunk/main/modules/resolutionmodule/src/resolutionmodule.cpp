/**
 *@file resolutionmodule.cpp
 *@brief resolution module
 *@version 1.0
 *@author l.j..
 *@date Created on: 2016-10-20 14:50:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
/// Private Headers //  NOLINT
#include "../include/resolutionmodule.h"
#include <json/json.h>
#include <boost/filesystem.hpp>

namespace ysos {
DECLARE_PLUGIN_REGISTER_INTERFACE(ResolutionModule, ModuleInterface);  ///<  插件的入口，一定要加上 //  NOLINT

ResolutionModule::ResolutionModule(const std::string &strClassName) : BaseThreadModuleImpl(strClassName) {

  exe_cmd_ = CMD_CONFIG_AUTO;
  thread_data_->timeout = 3000;

  logger_ = GetUtility()->GetLogger("ysos.resolution");
}

ResolutionModule::~ResolutionModule(void) {

}

int ResolutionModule::GetProperty(int type_id, void *type) {
  int n_return = YSOS_ERROR_SUCCESS;
  int n_result = -1;

  do {
    if (type_id == CMD_SCREEN_CALLBACKAUTO) {
      std::string str_result = "";
      Json::Reader js_reader;
      Json::FastWriter js_writer;
      Json::Value js_value;
      Json::Value js_ret;
      Json::Value js_child;

      BufferInterfacePtr *buffer_ptr = static_cast<BufferInterfacePtr*>(type);
      BufferInterfacePtr in_buffer_ptr = *buffer_ptr;
      if (in_buffer_ptr == NULL) {
        YSOS_LOG_DEBUG("in_buffer_ptr is null");
        n_return = YSOS_ERROR_FAILED;
        break;
      }

      UINT8* buffer;
      UINT32 buffer_size;
      n_return = in_buffer_ptr->GetBufferAndLength(&buffer, &buffer_size);
      if (YSOS_ERROR_SUCCESS != n_return) {
        YSOS_LOG_DEBUG("GetBufferAndLength error");
        n_return = YSOS_ERROR_FAILED;
        break;
      }
      memset(buffer, 0, buffer_size);
      lock_.Lock();
      switch (exe_cmd_) {
      case CMD_SCREEN_CALLBACKINFO:
        driver_prt_->Ioctl(CMD_SCREEN_CALLBACKINFO, in_buffer_ptr, in_buffer_ptr);
        exe_cmd_ = CMD_SCREEN_CALLBACKAUTO;

        break;
      }
      lock_.Unlock();
    } else {
      n_result = BaseThreadModuleImpl::GetProperty(type_id, type);
      if (n_result != YSOS_ERROR_SUCCESS) {
        n_return = YSOS_ERROR_FAILED;
        YSOS_LOG_DEBUG("execute BaseThreadModuleImpl GetProperty failed");
        break;
      }
    }
  } while (0);

  return n_return;
}

int ResolutionModule::Ioctl(INT32 control_id, LPVOID param /* = nullptr */) {
  YSOS_LOG_DEBUG("module Ioctl execute");
  int n_return = YSOS_ERROR_SUCCESS;

  do {
    BufferInterfacePtr in_buffer_ptr;
    std::string str_param = "";
    Json::Reader js_reader;
    Json::FastWriter js_writer;
    Json::Value js_value;

    n_return = buffer_pool_ptr_->GetBuffer(&in_buffer_ptr);
    if (YSOS_ERROR_SUCCESS!= n_return) {
      YSOS_LOG_DEBUG("GetBuffer error");
      break;
    }
    //< 需要修改
    in_buffer_ptr->SetLength(module_data_info_->buffer_length);

    UINT8* buffer;
    UINT32 buffer_size;
    n_return = in_buffer_ptr->GetBufferAndLength(&buffer, &buffer_size);
    if (YSOS_ERROR_SUCCESS != n_return) {
      YSOS_LOG_DEBUG("GetBufferAndLength error");
      break;
    }
    YSOS_LOG_DEBUG("buffer_size = " << buffer_size);
    memset(buffer, 0, buffer_size);
    lock_.Lock();
    switch (control_id) {
    case CMD_SCREEN_INFO: {
      exe_cmd_ = CMD_SCREEN_CALLBACKINFO;
      break;
    }
    case CMD_SCREEN_MODIFY: {
      str_param = *(reinterpret_cast<std::string*>(param));
      if (str_param.empty()) {
        YSOS_LOG_DEBUG("param error");
        n_return = YSOS_ERROR_FAILED;
        break;
      }
      js_reader.parse(str_param, js_value, true);
      std::string value = js_value["value"].asString();
      memcpy(buffer, value.c_str(), strlen(value.c_str()));

      n_return = driver_prt_->Ioctl(CMD_SCREEN_MODIFY, in_buffer_ptr, in_buffer_ptr);
      break;
    }
    case CMD_SCREEN_DELCACHE: {
      n_return = driver_prt_->Ioctl(CMD_SCREEN_DELCACHE, in_buffer_ptr, in_buffer_ptr);
      break;
    }
    default: {
      n_return = YSOS_ERROR_FAILED;
      break;
    }
    }

    int n_result = buffer_pool_ptr_->ReleaseBuffer(in_buffer_ptr);
    if (n_result != YSOS_ERROR_SUCCESS) {
      YSOS_LOG_DEBUG("execute ReleaseBuffer failed");
      n_return = n_result;
      break;
    }
    lock_.Unlock();
  } while (0);

  int iBase = BaseModuleImpl::Ioctl(control_id, param);
  if (iBase != YSOS_ERROR_SUCCESS)
    n_return = iBase;

  YSOS_LOG_DEBUG("module Ioctl done");

  return n_return;
}

int ResolutionModule::Flush(LPVOID param /* = nullptr */) {
  return YSOS_ERROR_SUCCESS;
}

int ResolutionModule::Initialize(LPVOID param /* = nullptr */) {
  YSOS_LOG_DEBUG("module Initialize execute");
  int n_return = YSOS_ERROR_SUCCESS;

  do {
    if (IsInitSucceeded()) {
      n_return = YSOS_ERROR_FINISH_ALREADY;
      break;
    }

    //ysos_sdk\Public\conf\instance\driver_instance.xml
    driver_prt_ = ysos::GetDriverInterfaceManager()->FindInterface("default@ResolutionDriver");


    n_return = BaseThreadModuleImpl::Initialize(param);
    if (n_return != YSOS_ERROR_SUCCESS) {
      YSOS_LOG_DEBUG("Initialize BaseThreadModuleImpl failed");
      break;
    }

  } while (0);

  YSOS_LOG_DEBUG("module Initialize done");

  return n_return;
}

int ResolutionModule::UnInitialize(LPVOID param /* = nullptr */) {
  return BaseThreadModuleImpl::UnInitialize(param);
}

int ResolutionModule::InitalDataInfo(void) {
  YSOS_LOG_DEBUG("module InitalDataInfo execute");
  int n_return = YSOS_ERROR_SUCCESS;

  YSOS_LOG_DEBUG("module InitalDataInfo done");

  return n_return;
}

int ResolutionModule::RealOpen(LPVOID param /* = nullptr */) {
  YSOS_LOG_DEBUG("module RealOpen execute");
  int n_return = YSOS_ERROR_SUCCESS;
  do {
    YSOS_LOG_DEBUG("xml_path_ = " << xml_path_);
    driver_prt_->Open((char *)xml_path_.c_str());

    if (n_return != YSOS_ERROR_SUCCESS)
      break;
  } while (0);

  YSOS_LOG_DEBUG("module RealOpen done");

  return BaseThreadModuleImpl::RealOpen(param);
}

int ResolutionModule::RealClose(void) {
  YSOS_LOG_DEBUG("module RealClose execute");
  int n_return = YSOS_ERROR_SUCCESS;

  YSOS_LOG_DEBUG("module RealClose done");

  return BaseThreadModuleImpl::RealClose();
}

int ResolutionModule::RealPause(void) {
  return YSOS_ERROR_SUCCESS;
}

int ResolutionModule::RealStop(void) {
  return YSOS_ERROR_SUCCESS;
}

int ResolutionModule::RealRun(void) {
  return YSOS_ERROR_SUCCESS;
}

int ResolutionModule::Initialized(const std::string &key, const std::string &value) {
  YSOS_LOG_DEBUG("module Initialized execute");
  int n_return = YSOS_ERROR_SUCCESS;

  if ("xml_path" == key)
    xml_path_ = value;
  YSOS_LOG_DEBUG("module Initialized done");

  return n_return;
}

}