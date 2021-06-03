/*
# ysossdkinterface.cpp
# Definition of YSOSSDKInterface
# Created on: 2018-01-24 16:26:38
# Original author: jinchengzhe
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
# version 0.1, 20180124, created by JinChengZhe
*/

/// Platform Headers
#include "../../../public/include/sys_interface_package/ysossdkinterface.h"
#include "../../../protect/include/sys_platform_sdk/ysossdk.h"
#include "../../../protect/include/protocol_help_package/platformprotocolimpl.h"
#include "../../../public/include/core_help_package/utility.h"

namespace ysos {

namespace ysos_sdk {

#ifdef __cplusplus
extern "C" {
#endif

int OpenYSOSSDK(void* params, char* session_id) {
  if (NULL == GetYSOSSDK()) {
    return YSOS_ERROR_FAILED;
  }

  return (GetYSOSSDK()->Open(params, session_id));
}

int CloseYSOSSDK(const char* session_id) {
  if (NULL == GetYSOSSDK()) {
    return YSOS_ERROR_FAILED;
  }

  return (GetYSOSSDK()->Close(session_id));
}

int YSOSSDKDoService(const char* session_id, const char* service_name, const char* service_param) {
  if (NULL == GetYSOSSDK()) {
    return YSOS_ERROR_FAILED;
  }

  return (GetYSOSSDK()->DoService(session_id, service_name, service_param));
}

int YSOSSDKOnDispatchMessage(const char* session_id, const char* service_name, const char* result) {
  if (NULL == GetYSOSSDK()) {
    return YSOS_ERROR_FAILED;
  }

  return (GetYSOSSDK()->OnDispatchMessage(session_id, service_name, result));
}

bool InitYSOSSDKLogger(const char* log_file_path_ptr) {
  if (NULL == GetYSOSSDK()) {
    return false;
  }

  return (GetYSOSSDK()->InitYSOSSDKLogger(log_file_path_ptr));
}

int SetYSOSSDKOpenParams(OpenParams& open_params, const unsigned type, const char* value) {
  int result = YSOS_ERROR_SUCCESS;

  do {
    switch (type) {
    case OpenParams::SET_ADDRESS: {
      if (NULL == value || 0 == strlen(value)) {
        result = YSOS_ERROR_INVALID_ARGUMENTS;
        break;
      }

      memset(open_params.address_, 0, YSOS_SDK_OPEN_PARAM_ADDRESS_LENGTH);
      #ifdef _WIN32
        strcpy_s(open_params.address_, YSOS_SDK_OPEN_PARAM_ADDRESS_LENGTH, value);
      #else
        strcpy(open_params.address_, value);
      #endif
    }
    break;
    case OpenParams::SET_APP_NAME: {
      if (NULL == value || 0 == strlen(value)) {
        result = YSOS_ERROR_INVALID_ARGUMENTS;
        break;
      }

      memset(open_params.app_name_, 0, YSOS_SDK_OPEN_PARAM_APP_NAME_LENGTH);
      #ifdef _WIN32
        strcpy_s(open_params.app_name_, YSOS_SDK_OPEN_PARAM_APP_NAME_LENGTH, value);
      #else
        strcpy(open_params.app_name_, value);
      #endif
    }
    break;
    case OpenParams::SET_STRATEGY_NAME: {
      if (NULL == value || 0 == strlen(value)) {
        result = YSOS_ERROR_INVALID_ARGUMENTS;
        break;
      }

      memset(open_params.strategy_name_, 0, YSOS_SDK_OPEN_PARAM_STRATEGY_NAME_LENGTH);
      #ifdef _WIN32
        strcpy_s(open_params.strategy_name_, YSOS_SDK_OPEN_PARAM_STRATEGY_NAME_LENGTH, value);
      #else
        strcpy(open_params.strategy_name_, value);
      #endif
    }
    break;
    case OpenParams::SET_CONF_FILE_NAME: {
      if (NULL == value || 0 == strlen(value)) {
        result = YSOS_ERROR_INVALID_ARGUMENTS;
        break;
      }

      memset(open_params.conf_file_name_, 0, YSOS_SDK_OPEN_PARAM_CONF_FILE_NAME_LENGTH);
      #ifdef _WIN32
        strcpy_s(open_params.conf_file_name_, YSOS_SDK_OPEN_PARAM_CONF_FILE_NAME_LENGTH, value);
      #else
        strcpy(open_params.conf_file_name_, value);
      #endif
    }
    break;
    case OpenParams::SET_CONF_STR: {
      if (NULL == value || 0 == strlen(value)) {
        result = YSOS_ERROR_INVALID_ARGUMENTS;
        break;
      }

      std::string deplicated_params = value;

      ysos::JsonValue json_value_object;
      if (YSOS_ERROR_SUCCESS != ysos::PlatformProtocolImpl::JsonObjectFromString(deplicated_params, json_value_object)) {
        result = YSOS_ERROR_LOGIC_ERROR;
        break;
      }

      if (true == json_value_object.empty()) {
        result = YSOS_ERROR_LOGIC_ERROR;
        break;
      }

      std::string duplicated_type,
          duplicated_mode,
          duplicated_address,
          duplicated_port,
          duplicated_max_reconnect_amount,
          duplicated_reconnect_time,
          duplicated_app_name,
          duplicated_strategy_name,
          duplicated_conf_file_name;

      if (true == json_value_object.isMember(YSOS_SDK_OPEN_PARAM_TYPE) &&
          true == json_value_object[YSOS_SDK_OPEN_PARAM_TYPE].isString()) {
        duplicated_type = json_value_object[YSOS_SDK_OPEN_PARAM_TYPE].asString();
      }

      if (true == json_value_object.isMember(YSOS_SDK_OPEN_PARAM_MODE) &&
          true == json_value_object[YSOS_SDK_OPEN_PARAM_MODE].isString()) {
        duplicated_mode = json_value_object[YSOS_SDK_OPEN_PARAM_MODE].asString();
      }

      if (true == json_value_object.isMember(YSOS_SDK_OPEN_PARAM_ADDRESS) &&
          true == json_value_object[YSOS_SDK_OPEN_PARAM_ADDRESS].isString()) {
        duplicated_address = json_value_object[YSOS_SDK_OPEN_PARAM_ADDRESS].asString();
      }

      if (true == json_value_object.isMember(YSOS_SDK_OPEN_PARAM_PORT) &&
          true == json_value_object[YSOS_SDK_OPEN_PARAM_PORT].isString()) {
        duplicated_port = json_value_object[YSOS_SDK_OPEN_PARAM_PORT].asString();
      }

      if (true == json_value_object.isMember(YSOS_SDK_OPEN_PARAM_MAX_RECONNECT_AMOUNT) &&
          true == json_value_object[YSOS_SDK_OPEN_PARAM_MAX_RECONNECT_AMOUNT].isString()) {
        duplicated_max_reconnect_amount = json_value_object[YSOS_SDK_OPEN_PARAM_MAX_RECONNECT_AMOUNT].asString();
      }

      if (true == json_value_object.isMember(YSOS_SDK_OPEN_PARAM_RECONNECT_TIME) &&
          true == json_value_object[YSOS_SDK_OPEN_PARAM_RECONNECT_TIME].isString()) {
        duplicated_reconnect_time = json_value_object[YSOS_SDK_OPEN_PARAM_RECONNECT_TIME].asString();
      }

      if (true == json_value_object.isMember(YSOS_SDK_OPEN_PARAM_APPLICATION_NAME) &&
          true == json_value_object[YSOS_SDK_OPEN_PARAM_APPLICATION_NAME].isString()) {
        duplicated_app_name = json_value_object[YSOS_SDK_OPEN_PARAM_APPLICATION_NAME].asString();
      }

      if (true == json_value_object.isMember(YSOS_SDK_OPEN_PARAM_STRATEGY_NAME) &&
          true == json_value_object[YSOS_SDK_OPEN_PARAM_STRATEGY_NAME].isString()) {
        duplicated_strategy_name = json_value_object[YSOS_SDK_OPEN_PARAM_STRATEGY_NAME].asString();
      }

      if (true == json_value_object.isMember(YSOS_SDK_OPEN_PARAM_CONF_FILE_NAME) &&
          true == json_value_object[YSOS_SDK_OPEN_PARAM_CONF_FILE_NAME].isString()) {
        duplicated_conf_file_name = json_value_object[YSOS_SDK_OPEN_PARAM_CONF_FILE_NAME].asString();
      }

      if (true != duplicated_type.empty()) {
        open_params.type_ = boost::lexical_cast<unsigned>(duplicated_type.c_str());
      }

      if (true != duplicated_mode.empty()) {
        open_params.mode_ = boost::lexical_cast<unsigned>(duplicated_mode.c_str());
      }

      if (true != duplicated_address.empty()) {
        result = SetYSOSSDKOpenParams(open_params, OpenParams::SET_ADDRESS, duplicated_address.c_str());
        if (YSOS_ERROR_SUCCESS != result) {
          break;
        }
      }

      if (true != duplicated_port.empty()) {
        open_params.port_ = boost::lexical_cast<unsigned>(duplicated_port.c_str());
      }

      if (true != duplicated_max_reconnect_amount.empty()) {
        open_params.max_reconnect_amount_ = boost::lexical_cast<unsigned>(duplicated_max_reconnect_amount.c_str());
      }

      if (true != duplicated_reconnect_time.empty()) {
        open_params.reconnect_time_ = boost::lexical_cast<unsigned>(duplicated_reconnect_time.c_str());
      }

      if (true != duplicated_app_name.empty()) {
        result = SetYSOSSDKOpenParams(open_params, OpenParams::SET_APP_NAME, duplicated_app_name.c_str());
        if (YSOS_ERROR_SUCCESS != result) {
          break;
        }
      }

      if (true != duplicated_strategy_name.empty()) {
        result = SetYSOSSDKOpenParams(open_params, OpenParams::SET_STRATEGY_NAME, duplicated_strategy_name.c_str());
        if (YSOS_ERROR_SUCCESS != result) {
          break;
        }
      }

      if (true != duplicated_conf_file_name.empty()) {
        result = SetYSOSSDKOpenParams(open_params, OpenParams::SET_CONF_FILE_NAME, duplicated_conf_file_name.c_str());
        if (YSOS_ERROR_SUCCESS != result) {
          break;
        }
      }
    }
    break;
    case OpenParams::SET_DEFAULT_VALUE: {
      open_params.type_ = OpenParams::PLATFORM_RPC;
      open_params.mode_ = YSOS_SDK_CLIENT_MODE;
      open_params.port_ = YSOS_SDK_PORT;
      open_params.max_reconnect_amount_ = YSOS_SDK_MAX_RECONNECT_AMOUNT;
      open_params.reconnect_time_ = YSOS_SDK_MAX_RECONNECT_AMOUNT;
      open_params.error_handler_ = NULL;
      open_params.do_service_handler_ = NULL;
      open_params.on_dispatch_message_handler_ = NULL;
      memset(open_params.app_name_, 0, YSOS_SDK_OPEN_PARAM_APP_NAME_LENGTH);
      memset(open_params.strategy_name_, 0, YSOS_SDK_OPEN_PARAM_STRATEGY_NAME_LENGTH);
      memset(open_params.conf_file_name_, 0, YSOS_SDK_OPEN_PARAM_CONF_FILE_NAME_LENGTH);
      result = SetYSOSSDKOpenParams(open_params, OpenParams::SET_ADDRESS, YSOS_SDK_ADDRESS);
      if (YSOS_ERROR_SUCCESS != result) {
        break;
      }
    }
    break;
    case OpenParams::RESET_VALUE: {
      memset(open_params.address_, 0, YSOS_SDK_OPEN_PARAM_ADDRESS_LENGTH);
      memset(open_params.app_name_, 0, YSOS_SDK_OPEN_PARAM_APP_NAME_LENGTH);
      memset(open_params.strategy_name_, 0, YSOS_SDK_OPEN_PARAM_STRATEGY_NAME_LENGTH);
      memset(open_params.conf_file_name_, 0, YSOS_SDK_OPEN_PARAM_CONF_FILE_NAME_LENGTH);
      open_params.type_ = 0;
      open_params.mode_ = 0;
      open_params.port_ = 0;
      open_params.max_reconnect_amount_ = 0;
      open_params.reconnect_time_ = 0;
      open_params.error_handler_ = NULL;
      open_params.do_service_handler_ = NULL;
      open_params.on_dispatch_message_handler_ = NULL;
    }
    break;
    default:
      result = YSOS_ERROR_INVALID_ARGUMENTS;
      break;
    }
  } while (0);

  return result;
}

#ifdef __cplusplus
}
#endif

}

}
