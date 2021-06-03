/**
 *@file networkmodule.cpp
 *@brief network module interface
 *@version 1.0
 *@author z.gg..
 *@date Created on: 2020-04-14 09:00:00
 *@copyright Copyright (c) 2016 YSOS Technology. All rights reserved.
 * 
 */
/// Private Headers //  NOLINT
#include "../include/networkmodule.h"

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
DECLARE_PLUGIN_REGISTER_INTERFACE(NetworkModule, ModuleInterface);  ///<  插件的入口，一定要加上 //  NOLINT

NetworkModule::NetworkModule(const std::string &strClassName)
    : BaseThreadModuleImpl(strClassName) {
  http_client_ = NULL;
  thread_data_->timeout = 3000;
  err_count_ = 0;
  logger_ = GetUtility()->GetLogger("ysos.network");
}

NetworkModule::~NetworkModule(void) {
}

int NetworkModule::Ioctl(INT32 control_id, LPVOID param /* = nullptr */) {
  YSOS_LOG_DEBUG("module Ioctl execute");
  int n_return = YSOS_ERROR_SUCCESS;

  do {
    switch (control_id) {
      case CMD_NETWORK_CHECK: {
        YSOS_LOG_DEBUG("NETWORK_CHECK command");
        n_return = CheckNetwork();
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

int NetworkModule::Flush(LPVOID param /* = nullptr */) {
  return YSOS_ERROR_SUCCESS;
}

int NetworkModule::Initialize(LPVOID param /* = nullptr */) {
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

int NetworkModule::UnInitialize(LPVOID param /* = nullptr */) {
  return BaseThreadModuleImpl::UnInitialize(param);
}

int NetworkModule::GetProperty(int iTypeId, void *piType) {
  int ret = YSOS_ERROR_SUCCESS;

  switch (iTypeId) {
    case PROP_DATA: {
      if (data_.empty()) {
        data_ = GetNetworkStatus();
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

int NetworkModule::InitalDataInfo(void) {
  YSOS_LOG_DEBUG("module InitalDataInfo execute");
  int n_return = YSOS_ERROR_SUCCESS;

  YSOS_LOG_DEBUG("module InitalDataInfo done");

  return n_return;
}

int NetworkModule::Initialized(const std::string &key, const std::string &value) {
  YSOS_LOG_DEBUG("module Initialized execute");
  int n_return = YSOS_ERROR_SUCCESS;

  do {

  } while (0);
  YSOS_LOG_DEBUG("module Initialized done");

  return n_return;
}

int NetworkModule::RealOpen(LPVOID param /* = nullptr */) {
  YSOS_LOG_DEBUG("module RealOpen execute");
  int n_return = YSOS_ERROR_SUCCESS;
  do {
    if (NULL == http_client_) {
      http_client_ = new HttpClient();
      if (http_client_ == NULL) {
        YSOS_LOG_DEBUG("get http_client_ failed");
        n_return = YSOS_ERROR_FAILED;
        break;
      }
    }
    n_return = BaseThreadModuleImpl::RealOpen(param);
    if (n_return != YSOS_ERROR_SUCCESS)
      break;
  } while (0);

  YSOS_LOG_DEBUG("module RealOpen done: " << n_return);
  return n_return;
}

int NetworkModule::RealClose(void) {
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

int NetworkModule::RealPause(void) {
  return YSOS_ERROR_SUCCESS;
}

int NetworkModule::RealStop(void) {
  return YSOS_ERROR_SUCCESS;
}

int NetworkModule::RealRun(void) {
  return YSOS_ERROR_SUCCESS;
}

std::string NetworkModule::GetNetworkStatus() {
  boost::shared_ptr<HttpClient> http_client(new HttpClient());
  http_client->SetHttpUrl("http://www.baidu.com");
  http_client->SetHttpHeader("Content-Type", "application/x-www-form-urlencoded");
  http_client->SetTimeOut(2, 2);
  std::string response_data;
  int ret = http_client->Request(response_data, HTTP_POST);
  response_data = boost::locale::conv::from_utf<char>(response_data.c_str(), "gbk");
  YSOS_LOG_INFO("Network check response " << response_data.size() << " bytes.");

  std::string result = "{\"type\":\"network_status\",\"data\":{\"status\":";
  if (response_data.empty()) {
    result.append("0}}");
    err_count_ = 0;//err_count_ + 1 ;
  } else {
    result.append("1}}");
    err_count_ = 0;
  }
  return result;
}

int NetworkModule::CheckNetwork() {
  int ret = YSOS_ERROR_SUCCESS;
  Json::Value json_body;

  do {
    
    data_ptr_ = GetDataInterfaceManager()->GetData(GLOBAL_DATA_KEY);
    if (!data_ptr_) {
      YSOS_LOG_DEBUG("get data_ptr failed.");
      ret = YSOS_ERROR_FAILED;
      json_body["type"] = "network_result_no";
      json_body["network_result"] = -2;
      break;
    }

    if (err_count_ > 2 ) {
      json_body["type"] = "network_result_no";
      json_body["network_result"] = -1;   
    } else {
      json_body["type"] = "network_result_yes";
      json_body["network_result"] = 0;
    }
    
  } while (0);

  {
    Json::FastWriter writer;
    data_ = writer.write(json_body);
    data_ = GetUtility()->ReplaceAllDistinct ( data_, "\\r\\n", "" );
  }

  YSOS_LOG_DEBUG("NetworkModule::CheckNetwork  [data_]=" << data_);

  SetProperty(PROP_THREAD_NOTIFY, NULL);

  return YSOS_ERROR_SUCCESS;
}

}