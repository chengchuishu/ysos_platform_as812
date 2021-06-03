/**
 *@file hardwaremodule.cpp
 *@brief hardwar module interface
 *@version 1.0
 *@author z.gg..
 *@date Created on: 2020-04-14 09:00:00
 *@copyright Copyright (c) 2016 YSOS Technology. All rights reserved.
 * 
 */
/// Private Headers //  NOLINT
#include "../include/hardwaremodule.h"

#include <fstream>
#include <sstream>
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/locale.hpp>
#include <boost/filesystem.hpp>
#include <json/json.h>
#include "../../../../../../core/trunk/main/public/include/os_hal_package/bufferutility.h"

namespace ysos {
DECLARE_PLUGIN_REGISTER_INTERFACE(HardwareModule, ModuleInterface);  ///<  插件的入口，一定要加上 //  NOLINT

HardwareModule::HardwareModule(const std::string &strClassName)
    : BaseThreadModuleImpl(strClassName) {
  logger_ = GetUtility()->GetLogger("ysos.hardware");
}

HardwareModule::~HardwareModule(void) {
}

int HardwareModule::Ioctl(INT32 control_id, LPVOID param /* = nullptr */) {
  YSOS_LOG_DEBUG("module Ioctl execute");
  int n_return = YSOS_ERROR_SUCCESS;

  do {
    switch (control_id) {
      case CMD_HARDWARE_CHECK: {
        YSOS_LOG_DEBUG("HARDWARE_CHECK command");
        n_return = CheckHardware();
        break;
      }
      default: {
        YSOS_LOG_DEBUG("Ioctl id error, control_id = " << control_id);
        n_return = YSOS_ERROR_FAILED;
        break;
      }
    }
  } while (0);

  YSOS_LOG_DEBUG("module Ioctl done");

  return n_return;
}

int HardwareModule::Flush(LPVOID param /* = nullptr */) {
  return YSOS_ERROR_SUCCESS;
}

int HardwareModule::Initialize(LPVOID param /* = nullptr */) {
  YSOS_LOG_DEBUG("module Initialize execute");
  int n_return = YSOS_ERROR_SUCCESS;

  do {
    if (IsInitSucceeded()) {
      n_return = YSOS_ERROR_FINISH_ALREADY;
      break;
    }
    
    n_return = BaseThreadModuleImpl::Initialize(param);
    if (n_return != YSOS_ERROR_SUCCESS) {
      YSOS_LOG_DEBUG("Initialize BaseThreadModuleImpl failed");
      break;
    }
  } while (0);

  YSOS_LOG_DEBUG("module Initialize done: "<< n_return);
  return n_return;
}

int HardwareModule::UnInitialize(LPVOID param /* = nullptr */) {
  return BaseThreadModuleImpl::UnInitialize(param);
}

int HardwareModule::GetProperty(int iTypeId, void *piType) {
  int ret = YSOS_ERROR_SUCCESS;

  switch (iTypeId) {
    case PROP_DATA: {
      if (data_.empty()){
        //data_ = GetNetworkStatus();
      }
      
      YSOS_LOG_DEBUG("PROP_DATA: " << data_);
      if (data_.empty()) {
        ret = YSOS_ERROR_NOT_EXISTED;
        break;
      }
      BufferInterfacePtr *buffer_ptr_ptr = reinterpret_cast<BufferInterfacePtr*>(piType);
      if (NULL == buffer_ptr_ptr) {
        ret = YSOS_ERROR_INVALID_ARGUMENTS;
        break;
      }
      BufferInterfacePtr buffer_ptr = *buffer_ptr_ptr;
      if (NULL == buffer_ptr) {
        ret = YSOS_ERROR_INVALID_ARGUMENTS;
        break;
      }
      uint8_t *data = GetBufferUtility()->GetBufferData(buffer_ptr);
      size_t buffer_length = GetBufferUtility()->GetBufferLength(buffer_ptr);
      if (NULL == data || 0 == buffer_length || buffer_length <= data_.length()) {
        ret = YSOS_ERROR_INVALID_ARGUMENTS;
        break;
      }

      memcpy(data, data_.c_str(), data_.length());
      data[data_.length()] = '\0';
      data_.clear();
      break;
    }
    default:
      ret = BaseThreadModuleImpl::GetProperty(iTypeId, piType);
  }

  return ret;
}

int HardwareModule::InitalDataInfo(void) {
  YSOS_LOG_DEBUG("module InitalDataInfo execute");
  int n_return = YSOS_ERROR_SUCCESS;

  YSOS_LOG_DEBUG("module InitalDataInfo done");

  return n_return;
}

int HardwareModule::Initialized(const std::string &key, const std::string &value) {
  YSOS_LOG_DEBUG("module Initialized execute");
  int n_return = YSOS_ERROR_SUCCESS;

  do {

  } while (0);
  YSOS_LOG_DEBUG("module Initialized done");

  return n_return;
}

int HardwareModule::RealOpen(LPVOID param /* = nullptr */) {
  YSOS_LOG_DEBUG("module RealOpen execute");
  int n_return = YSOS_ERROR_SUCCESS;
  do {
    /*
    if (NULL == http_client_) {
      http_client_ = new HttpClient();
      if (http_client_ == NULL) {
        YSOS_LOG_DEBUG("get http_client_ failed");
        n_return = YSOS_ERROR_FAILED;
        break;
      }
    }*/
    n_return = BaseThreadModuleImpl::RealOpen(param);
    if (n_return != YSOS_ERROR_SUCCESS)
      break;
  } while (0);

  YSOS_LOG_DEBUG("module RealOpen done: " << n_return);
  return n_return;
}

int HardwareModule::RealClose(void) {
  YSOS_LOG_DEBUG("module RealClose execute");
  int n_return = YSOS_ERROR_SUCCESS;
  do {
    n_return = BaseThreadModuleImpl::RealClose();
    if (n_return != YSOS_ERROR_SUCCESS)
      break;
  } while (0);

  YSOS_LOG_DEBUG("module RealClose done: " << n_return);
  return n_return;
}

int HardwareModule::RealPause(void) {
  return YSOS_ERROR_SUCCESS;
}

int HardwareModule::RealStop(void) {
  return YSOS_ERROR_SUCCESS;
}

int HardwareModule::RealRun(void) {
  return YSOS_ERROR_SUCCESS;
}

int HardwareModule::CheckHardware() {
  int ret = YSOS_ERROR_SUCCESS;
  Json::Value json_body;

  do {
    
    data_ptr_ = GetDataInterfaceManager()->GetData(GLOBAL_DATA_KEY);
    if (!data_ptr_) {
      YSOS_LOG_DEBUG("get data_ptr failed.");
      ret = YSOS_ERROR_FAILED;
      json_body["type"] = "hardware_result_no";
      json_body["hardware_result"] = -1;
      break;
    }

    json_body["type"] = "hardware_result_yes";
    json_body["hardware_result"] = 0;
     
  } while (0);

  {
    Json::FastWriter writer;
    data_ = writer.write(json_body);
    data_ = GetUtility()->ReplaceAllDistinct ( data_, "\\r\\n", "" );
  }
   
  YSOS_LOG_DEBUG("HardwareModule::CheckHardware  [data_]=" << data_);
  
  SetProperty(PROP_THREAD_NOTIFY, NULL);

  return YSOS_ERROR_SUCCESS;
}

}