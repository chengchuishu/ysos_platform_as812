/**
 *@file ioctlstrategycallback.cpp
 *@brief ioctl strategy call back
 *@version 1.0
 *@author l.j..
 *@date Created on: 2016-10-20 15:16:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
/// Private Headers //  NOLINT
#include "../include/ioctlstrategycallback.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/filesystem.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <json/json.h>
#include "../../../../../../core/trunk/main/public/include/os_hal_package/bufferutility.h"
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/manager_declare.h"
#include "../../../../../../core/trunk/main/public/include/sys_interface_package/protocolinterface.h"
#include "../../../../../../core/trunk/main/public/include/plugins/commonenumdefine.h"

namespace ysos {
DECLARE_PLUGIN_REGISTER_INTERFACE(IoctlStrategyCallback, CallbackInterface);  ///<  插件的入口，一定要加上 //  NOLINT
IoctlStrategyCallback::IoctlStrategyCallback(const std::string &strClassName):BaseStrategyCallbackImpl(strClassName) {
  req_service_ = "";
  req_id_ = "";
  req_type_ = "";
  req_value_ = "";
  event_notify_ = "";
  ignore_ = "";
  format_ = "";
  get_service_ = "";
  logger_ = GetUtility()->GetLogger("ysos.ioctlstrategy");
}

IoctlStrategyCallback::~IoctlStrategyCallback(void) {

}

int IoctlStrategyCallback::HandleMessage(const std::string &event_name, BufferInterfacePtr input_buffer, CallbackInterface *context) {
  int n_return = YSOS_ERROR_FAILED;
  lock_.Lock();
  do {
      YSOS_LOG_DEBUG("event_name = " << event_name);
    int ctl_id = -1;
    if ("run_event" == event_name)
      ctl_id = PROP_RUN;
    else if ("pause_event" == event_name)
      ctl_id = PROP_PAUSE;
    else if ("stop_event" == event_name)
      ctl_id = PROP_STOP;
    else if ("close_event" == event_name)
      ctl_id = PROP_CLOSE;
    if (-1 != ctl_id) {
      YSOS_LOG_DEBUG("get ctl_id = " << ctl_id);
      n_return = RequestService(req_service_, GetUtility()->ConvertToString(ctl_id), "", "", context);
      break;
    }

    std::string str_service = req_service_;
    std::string str_id = req_id_;
    std::string str_value = "";
    if (NULL != input_buffer) {
      UINT8* buffer;
      UINT32 buffer_size;
      n_return = input_buffer->GetBufferAndLength(&buffer, &buffer_size);
      if (YSOS_ERROR_SUCCESS != n_return) {
        YSOS_LOG_DEBUG("GetBufferAndLength error");
        break;
      }
      if ("true" == get_service_) {
        std::string str_data = std::string((char*)buffer);
        if (str_data.empty()) {
          YSOS_LOG_DEBUG("get service data empty");
          break;
        }
        YSOS_LOG_DEBUG("need get service, str_data = " << str_data);
        Json::Reader json_reader;
        Json::Value json_service;
        json_reader.parse(str_data, json_service, true);
        str_service = json_service["service"].asString();
        str_id = json_service["id"].asString();
        std::string value_data = json_service["value"].asString();
        memset(buffer, 0, buffer_size);
        if (str_service.empty() || str_id.empty()) {
          YSOS_LOG_DEBUG("service_data or id_data empty");
          break;
        }
        if (!value_data.empty()) {
          memcpy(buffer, value_data.c_str(), value_data.length());
        }
      }

      if (!format_.empty()) { //< 格式化数据
        YSOS_LOG_DEBUG("format_ = " << format_);
        ProtocolInterfacePtr format_protocol_ptr = GetProtocolInterfaceManager()->FindInterface(format_);
        if (NULL == format_protocol_ptr) {
          YSOS_LOG_DEBUG("find protocol interface: " << format_ << " failed");
          break;
        }
        ProtocolInterface::ProtocolFormatId format_id = 0;
        BufferInterfacePtr out_buffer = GetBufferUtility()->CreateBuffer(1024);
        int ret = format_protocol_ptr->FormMessage(input_buffer, out_buffer, format_id, NULL);
        if (0 != ret) {
          YSOS_LOG_DEBUG("format message failed");
          break;
        }
        input_buffer = out_buffer;
      }

      std::string str_data = std::string((char*)buffer);
      if (str_data.empty()) {
        YSOS_LOG_DEBUG("str_data empty");
        str_value = req_value_;
      }
      if ("true" == ignore_) //< 使用配置value
        str_value = req_value_;
      else
        str_value = str_data;
    } else {
      str_value = req_value_;
    }
    if (req_service_.empty() || req_id_.empty()) {
      YSOS_LOG_DEBUG("param error, req_service_ = " << req_service_ << ", req_id_ = " << req_id_);
      break;
    }
    n_return = RequestService(str_service, str_id, req_type_, str_value, context);
    if (YSOS_ERROR_SUCCESS != n_return)
      break;
    if ("true" == event_notify_) {
      Json::FastWriter js_writer;
      Json::Value js_value, js_result;
      js_value["result_code"] = n_return;
	  js_value["str_service"] = str_service;
	  js_value["str_id"] = str_id;
      js_value["data"] = "";
      js_result["type"] = "ioctl_result_event";
      js_result["data"] = js_value;
      str_value = js_writer.write(js_result);
      str_value = GetUtility()->ReplaceAllDistinct ( str_value, "\\r\\n", "" );
      YSOS_LOG_DEBUG("str_value = " << str_value);
      n_return = DoEventNotifyService("ioctl_result_event", "ioctl_result_callback", str_value, context);
      if (YSOS_ERROR_SUCCESS != n_return) {
        YSOS_LOG_DEBUG("execute DoEventNotifyService failed, n_return = " << n_return);
        break;
      }
    }

    n_return = YSOS_ERROR_SUCCESS;
  } while (0);
  lock_.Unlock();

  return n_return;
}

int IoctlStrategyCallback::Initialized(const std::string &key, const std::string &value) {
  int n_return = YSOS_ERROR_SUCCESS;

  if ("service" == key)
    req_service_ = value;
  else if ("id" == key)
    req_id_ = value;
  else if ("type" == key)
    req_type_ = value;
  else if ("value" == key)
    req_value_ = value;
  else if ("notify" == key)
    event_notify_ = value;
  else if ("ignore" == key)
    ignore_ = value;
  else if ("format" == key)
    format_ = value;
  else if ("getservice" == key)
    get_service_ = value;

  return n_return;
}

int IoctlStrategyCallback::RequestService(std::string service_name, std::string id, std::string type, std::string json_value, CallbackInterface *context) {
  YSOS_LOG_DEBUG("RequestService execute");
  int n_return = YSOS_ERROR_SUCCESS;

  YSOS_LOG_DEBUG("RequestService service_name = " << service_name);
  YSOS_LOG_DEBUG("RequestService id = " << id);
  YSOS_LOG_DEBUG("RequestService text = " << type);
  YSOS_LOG_DEBUG("RequestService json_value = " << json_value);

  do {
    ReqIOCtlServiceParam req_service;
    req_service.id = id;
    req_service.service_name = service_name;
    if (!type.empty()) {
      req_service.type = type;
    }
    if (!json_value.empty()) {
      req_service.value = json_value;
    }
    n_return = DoIoctlService(req_service, context);
    if (YSOS_ERROR_SUCCESS != n_return) {
      YSOS_LOG_DEBUG("execute DoIoctlService failed, n_return = " << n_return);
      n_return = YSOS_ERROR_FAILED;
      break;
    }
    YSOS_LOG_DEBUG("RequestService done and success");
  } while (0);

  return n_return;
}

}