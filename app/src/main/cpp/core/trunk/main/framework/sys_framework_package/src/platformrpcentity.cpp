/*
# platformrpcentity.cpp
# Definition of PlatformRPCEntity
# Created on: 2016-10-12 10:22:33
# Original author: jinchengzhe
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
# version 0.1, 20161012, created by JinChengZhe
*/

/// Private Headers
#include "../../../protect/include/sys_framework_package/platformrpcentity.h"
/// Platform Headers
#include "../../../protect/include/protocol_help_package/platformprotocolimpl.h"

namespace ysos {

#if _CERTUSNET
const char* g_request_service_param_service_name = "action";
#else
const char* g_request_service_param_service_name = "service_name";
#endif
const char* g_request_service_param_flag = "flag";
const char* g_request_service_param_event_spec = "event_spec";
const char* g_request_service_param_id = "id";
const char* g_request_service_param_type = "type";
const char* g_request_service_param_value = "value";
const char* g_request_service_param_config_file_name = "config_file_name";
const char* g_request_service_param_config_param = "config_param";
const char* g_request_service_param_state_name = "state_name";
const char* g_request_service_param_event_name = "event_name";
const char* g_request_service_param_callback_name = "callback_name";
const char* g_request_service_param_data = "data";
const char* g_request_service_param_app_name = "app_name";
const char* g_request_service_param_strategy_name = "strategy_name";
const char* g_request_service_param_log_content = "log_content";
#if _CERTUSNET
const char* g_request_service_param_service_param = "param";
#else
const char* g_request_service_param_service_param = "service_param";
#endif

const char* g_request_service_param_event_spec_event_key = "event_key";
const char* g_request_service_param_event_spec_callback_name = "callback_name";

const char* g_request_service_param_config_param_config_key = "config_key";
const char* g_request_service_param_config_param_load_name = "load_name";
const char* g_request_service_param_config_param_is_sync = "is_sync";
const char* g_request_service_param_config_param_config_value = "config_value";

#if _CERTUSNET
const char* g_response_result_status_code = "status";
#else
const char* g_response_result_status_code = "status_code";
#endif
const char* g_response_result_description = "description";
const char* g_response_result_detail = "detail";

const char* g_response_result_detail_cur_state = "cur_state";
const char* g_response_result_detail_service_list = "service_list";
const char* g_response_result_detail_service_name = "service_name";
const char* g_response_result_detail_state_param = "state_param";

const char* g_response_result_detail_service_list_name = "name";
const char* g_response_result_detail_service_list_alias = "alias";
const char* g_response_result_detail_service_list_description = "description";

const char* g_get_set_data = "data";
const char* g_get_set_data_key = "key";
const char* g_get_set_data_value = "value";

const char* g_response_result_description_succeeded = "succeeded";
const char* g_response_result_description_faild = "failed";

//////////////////////////////////////////////////////////////////////////
/*
Request Service Param
*/

/// ReqInitServiceParam
bool ReqInitServiceParam::ToString(std::string& to_string) {
  bool result = false;

  do {
    JsonValue json_value_service_param;

    PlatformProtocolImpl::PlatformProtocolParamPair platform_protocol_param_pair;

    platform_protocol_param_pair.first = g_request_service_param_app_name;
    platform_protocol_param_pair.second = app_name;
    PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_service_param);

    platform_protocol_param_pair.first = g_request_service_param_strategy_name;
    platform_protocol_param_pair.second = strategy_name;
    PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_service_param);

    if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonValueToString(json_value_service_param, to_string)) {
      break;
    }

    result = true;
  } while (0);

  return result;
}

bool ReqInitServiceParam::FromString(const std::string& from_string) {
  bool result = false;

  do {
    JsonValue json_value_service_param;
    if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonObjectFromString(from_string, json_value_service_param)) {
      break;
    }

    if (
      true == json_value_service_param.isMember(g_request_service_param_app_name) &&
      true == json_value_service_param[g_request_service_param_app_name].isString()
    ) {
      app_name = json_value_service_param[g_request_service_param_app_name].asString();
    }

    if (
      true == json_value_service_param.isMember(g_request_service_param_strategy_name) &&
      true == json_value_service_param[g_request_service_param_strategy_name].isString()
    ) {
      strategy_name = json_value_service_param[g_request_service_param_strategy_name].asString();
    }

    result = true;
  } while (0);

  return result;
}

/// ReqSwitchNotifyServiceParam
bool ReqSwitchNotifyServiceParam::ToString(std::string& to_string) {
  bool result = false;

  do {
    JsonValue json_value_service_param;

    PlatformProtocolImpl::PlatformProtocolParamPair platform_protocol_param_pair;

    platform_protocol_param_pair.first = g_request_service_param_state_name;
    platform_protocol_param_pair.second = state_name;
    PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_service_param);

    if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonValueToString(json_value_service_param, to_string)) {
      break;
    }

    result = true;
  } while (0);

  return result;
}

bool ReqSwitchNotifyServiceParam::FromString(const std::string& from_string) {
  bool result = false;

  do {
    JsonValue json_value_service_param;
    if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonObjectFromString(from_string, json_value_service_param)) {
      break;
    }

    if (
      true == json_value_service_param.isMember(g_request_service_param_state_name) &&
      true == json_value_service_param[g_request_service_param_state_name].isString()
    ) {
      state_name = json_value_service_param[g_request_service_param_state_name].asString();
    }

    result = true;
  } while (0);

  return result;
}

/// ReqSwitchServiceParam
bool ReqSwitchServiceParam::ToString(std::string& to_string) {
  bool result = false;

  do {
    JsonValue json_value_service_param;

    PlatformProtocolImpl::PlatformProtocolParamPair platform_protocol_param_pair;

    platform_protocol_param_pair.first = g_request_service_param_state_name;
    platform_protocol_param_pair.second = state_name;
    PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_service_param);

    if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonValueToString(json_value_service_param, to_string)) {
      break;
    }

    result = true;
  } while (0);

  return result;
}

bool ReqSwitchServiceParam::FromString(const std::string& from_string) {
  bool result = false;

  do {
    JsonValue json_value_service_param;
    if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonObjectFromString(from_string, json_value_service_param)) {
      break;
    }

    if (
      true == json_value_service_param.isMember(g_request_service_param_state_name) &&
      true == json_value_service_param[g_request_service_param_state_name].isString()
    ) {
      state_name = json_value_service_param[g_request_service_param_state_name].asString();
    }

    if (true == state_name.empty()) {
      break;
    }

    result = true;
  } while (0);

  return result;
}

/// ReqRegServiceEventServiceParam
bool ReqRegServiceEventServiceParam::ToString(std::string& to_string) {
  bool result = false;

  do {
    JsonValue json_value_service_param;
    PlatformProtocolImpl::PlatformProtocolParamPair platform_protocol_param_pair;

    /// service_name
    platform_protocol_param_pair.first = g_request_service_param_service_name;
    platform_protocol_param_pair.second = service_name;
    PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_service_param);

    /// falg
    platform_protocol_param_pair.first = g_request_service_param_flag;
    platform_protocol_param_pair.second = flag;
    PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_service_param);

    /// event_spec
    JsonValue json_value_unit_array;
    JsonValue json_value_unit;

    for (
      ServiceEventSpecUnits::iterator iterator = service_event_spec_units.begin();
      iterator != service_event_spec_units.end();
      ++iterator
    ) {
      platform_protocol_param_pair.first = g_request_service_param_event_spec_event_key;
      platform_protocol_param_pair.second = iterator->event_key;
      PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_unit);

      platform_protocol_param_pair.first = g_request_service_param_event_spec_callback_name;
      platform_protocol_param_pair.second = iterator->callback_name;
      PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_unit);

      /// add array
      PlatformProtocolImpl::JsonArrayFromJsonObject(json_value_unit, json_value_unit_array);
    }

    json_value_service_param[g_request_service_param_event_spec] = json_value_unit_array;

    if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonValueToString(json_value_service_param, to_string)) {
      break;
    }

    result = true;
  } while (0);

  return result;
}

bool ReqRegServiceEventServiceParam::FromString(const std::string& from_string) {

  bool result = false;

  do {
    JsonValue json_value_service_param;
    if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonObjectFromString(from_string, json_value_service_param)) {
      break;
    }

    /// service_name
    if (
      true == json_value_service_param.isMember(g_request_service_param_service_name) &&
      true == json_value_service_param[g_request_service_param_service_name].isString()
    ) {
      service_name = json_value_service_param[g_request_service_param_service_name].asString();
    }

    /// flag
    if (
      true == json_value_service_param.isMember(g_request_service_param_flag) &&
      true == json_value_service_param[g_request_service_param_flag].isString()
    ) {
      flag = json_value_service_param[g_request_service_param_flag].asString();
    }

    /// event_spec
    if (true == json_value_service_param.isMember(g_request_service_param_event_spec) &&
        true == json_value_service_param[g_request_service_param_event_spec].isArray()
       ) {
      JsonValue json_value_event_spec = json_value_service_param[g_request_service_param_event_spec];
      if (true == json_value_event_spec.empty()) {
        break;
      }

      if (0 < json_value_event_spec.size()) {
        JsonValue json_value_event_spec_unit;
        for (unsigned ii = 0; ii <json_value_event_spec.size(); ++ii) {
          json_value_event_spec_unit = json_value_event_spec[ii];
          JsonValue json_value_object;
          ServiceEventSpecUnit unit;
          for (
            JsonValueIterator iterator = json_value_event_spec_unit.begin();
            iterator != json_value_event_spec_unit.end();
            ++iterator
          ) {

            /// event_key
            if (0 == iterator.key().asString().compare(g_request_service_param_event_spec_event_key)) {
              json_value_object = json_value_event_spec_unit[iterator.key().asString().c_str()];
              if (true == json_value_object.isString()) {
                unit.event_key = json_value_object.asString();
              }
            }

            /// callback_name
            if (0 == iterator.key().asString().compare(g_request_service_param_event_spec_callback_name)) {
              json_value_object = json_value_event_spec_unit[iterator.key().asString().c_str()];
              if (true == json_value_object.isString()) {
                unit.callback_name = json_value_object.asString();
              }
            }
          }
          service_event_spec_units.push_back(unit);
        }
      }
    }

    result = true;

  } while (0);

  return result;
}

/// ReqUnregServiceEventServiceParam
bool ReqUnregServiceEventServiceParam::ToString(std::string& to_string) {
  bool result = false;

  do {
    JsonValue json_value_service_param;
    PlatformProtocolImpl::PlatformProtocolParamPair platform_protocol_param_pair;

    /// service_name
    platform_protocol_param_pair.first = g_request_service_param_service_name;
    platform_protocol_param_pair.second = service_name;
    PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_service_param);

    /// event_spec
    JsonValue json_value_unit_array;
    JsonValue json_value_unit;

    for (
      ServiceEventSpecUnits::iterator iterator = service_event_spec_units.begin();
      iterator != service_event_spec_units.end();
      ++iterator
    ) {
      platform_protocol_param_pair.first = g_request_service_param_event_spec_event_key;
      platform_protocol_param_pair.second = iterator->event_key;
      PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_unit);

      platform_protocol_param_pair.first = g_request_service_param_event_spec_callback_name;
      platform_protocol_param_pair.second = iterator->callback_name;
      PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_unit);

      PlatformProtocolImpl::JsonArrayFromJsonObject(json_value_unit, json_value_unit_array);
    }

    json_value_service_param[g_request_service_param_event_spec] = json_value_unit_array;

    if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonValueToString(json_value_service_param, to_string)) {
      break;
    }

    result = true;
  } while (0);

  return result;
}

bool ReqUnregServiceEventServiceParam::FromString(const std::string& from_string) {
  bool result = false;

  do {
    JsonValue json_value_service_param;
    if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonObjectFromString(from_string, json_value_service_param)) {
      break;
    }

    /// service_name
    if (
      true == json_value_service_param.isMember(g_request_service_param_service_name) &&
      true == json_value_service_param[g_request_service_param_service_name].isString()
    ) {
      service_name = json_value_service_param[g_request_service_param_service_name].asString();
    } else {
      break;
    }

    /// event_spec
    if (true == json_value_service_param.isMember(g_request_service_param_event_spec) &&
        true == json_value_service_param[g_request_service_param_event_spec].isArray()
       ) {
      JsonValue json_value_event_spec = json_value_service_param[g_request_service_param_event_spec];
      if (true == json_value_event_spec.empty()) {
        break;
      }

      if (0 < json_value_event_spec.size()) {
        JsonValue json_value_event_spec_unit;
        for (unsigned ii = 0; ii <json_value_event_spec.size(); ++ii) {
          json_value_event_spec_unit = json_value_event_spec[ii];
          JsonValue json_value_object;
          ServiceEventSpecUnit unit;
          for (
            JsonValueIterator iterator = json_value_event_spec_unit.begin();
            iterator != json_value_event_spec_unit.end();
            ++iterator
          ) {
            /// event_key
            if (0 == iterator.key().asString().compare(g_request_service_param_event_spec_event_key)) {
              json_value_object = json_value_event_spec_unit[iterator.key().asString().c_str()];
              if (true == json_value_object.isString()) {
                unit.event_key = json_value_object.asString();
              }
            }

            /// callback_name
            if (0 == iterator.key().asString().compare(g_request_service_param_event_spec_callback_name)) {
              json_value_object = json_value_event_spec_unit[iterator.key().asString().c_str()];
              if (true == json_value_object.isString()) {
                unit.callback_name = json_value_object.asString();
              }
            }
          }
          service_event_spec_units.push_back(unit);
        }
      }
    }

    result = true;

  } while (0);

  return result;
}

/// ReqReadyServiceParam
bool ReqReadyServiceParam::ToString(std::string& to_string) {
  to_string = JsonNullValue;
  return true;
}

bool ReqReadyServiceParam::FromString(const std::string& from_string) {
  return true;
}

/// ReqCustomEventServiceParam
bool ReqCustomEventServiceParam::ToString(std::string& to_string) {
  JsonValue json_value_service_param;
  PlatformProtocolImpl::PlatformProtocolParamPair platform_protocol_param_pair;

  /// service_name
  platform_protocol_param_pair.first = g_request_service_param_service_name;
  platform_protocol_param_pair.second = service_name;
  PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_service_param);

  /// service_param
  JsonValue json_value_object;
  if (true != service_param.empty()) {
    if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonObjectFromString(service_param, json_value_object)) {
      return false;
    }
  }

  json_value_service_param[g_request_service_param_service_param] = json_value_object;

  if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonValueToString(json_value_service_param, to_string)) {
    return false;
  }

  return true;
}

bool ReqCustomEventServiceParam::FromString(const std::string& from_string) {
  bool result = false;

  do {
    JsonValue json_value_service_param;
    if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonObjectFromString(from_string, json_value_service_param)) {
      break;
    }

    /// service_name
    if (
      true == json_value_service_param.isMember(g_request_service_param_service_name) &&
      true == json_value_service_param[g_request_service_param_service_name].isString()
    ) {
      service_name = json_value_service_param[g_request_service_param_service_name].asString();
    }

    /// service_param
    if (true == json_value_service_param.isMember(g_request_service_param_service_param) &&
        true != json_value_service_param[g_request_service_param_service_param].isNull() &&
        true == json_value_service_param[g_request_service_param_service_param].isObject()) {
      if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonValueToString(json_value_service_param[g_request_service_param_service_param], service_param)) {
        break;
      }
    }

    result = true;
  } while (0);

  return result;
}

/// ReqIOCtlServiceParam
bool ReqIOCtlServiceParam::ToString(std::string& to_string) {

  bool result = false;

  do {
    JsonValue json_value_service_param;

    PlatformProtocolImpl::PlatformProtocolParamPair platform_protocol_param_pair;

    /// service_name
    platform_protocol_param_pair.first = g_request_service_param_service_name;
    platform_protocol_param_pair.second = service_name;
    PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_service_param);

    /// id
    platform_protocol_param_pair.first = g_request_service_param_id;
    platform_protocol_param_pair.second = id;
    PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_service_param);

    /// type
    platform_protocol_param_pair.first = g_request_service_param_type;
    platform_protocol_param_pair.second = type;
    PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_service_param);

    /// value
    platform_protocol_param_pair.first = g_request_service_param_value;
    platform_protocol_param_pair.second = value;
    PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_service_param);

    if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonValueToString(json_value_service_param, to_string)) {
      break;
    }

    result = true;
  } while (0);

  return result;
}

bool ReqIOCtlServiceParam::FromString(const std::string& from_string) {
  bool result = false;

  do {
    JsonValue json_value_service_param;
    if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonObjectFromString(from_string, json_value_service_param)) {
      break;
    }

    /// service_name
    if (
      true == json_value_service_param.isMember(g_request_service_param_service_name) &&
      true == json_value_service_param[g_request_service_param_service_name].isString()
    ) {
      service_name = json_value_service_param[g_request_service_param_service_name].asString();
    }

    /// id
    if (
      true == json_value_service_param.isMember(g_request_service_param_id) &&
      true == json_value_service_param[g_request_service_param_id].isString()
    ) {
      id = json_value_service_param[g_request_service_param_id].asString();
    }

    /// type
    if (
      true == json_value_service_param.isMember(g_request_service_param_type) &&
      true == json_value_service_param[g_request_service_param_type].isString()
    ) {
      type = json_value_service_param[g_request_service_param_type].asString();
    }

    /// value
    if (
      true == json_value_service_param.isMember(g_request_service_param_value) &&
      true == json_value_service_param[g_request_service_param_value].isString()
    ) {
      value = json_value_service_param[g_request_service_param_value].asString();
    }

    result = true;
  } while (0);

  return result;
}

/// ReqGetServiceListServiceParam
bool ReqGetServiceListServiceParam::ToString(std::string& to_string) {
  to_string = JsonNullValue;
  return true;
}

bool ReqGetServiceListServiceParam::FromString(const std::string& from_string) {
  return true;
}

/// ReqGetServiceStateServiceParam
bool ReqGetServiceStateServiceParam::ToString(std::string& to_string) {
  bool result = false;

  do {
    JsonValue json_value_service_param;
    PlatformProtocolImpl::PlatformProtocolParamPair platform_protocol_param_pair;

    platform_protocol_param_pair.first = g_request_service_param_service_name;
    platform_protocol_param_pair.second = service_name;

    PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_service_param);

    if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonValueToString(json_value_service_param, to_string)) {
      break;
    }

    result = true;
  } while (0);

  return result;
}

bool ReqGetServiceStateServiceParam::FromString(const std::string& from_string) {
  bool result = false;

  do {
    JsonValue json_value_service_param;
    if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonObjectFromString(from_string, json_value_service_param)) {
      break;
    }

    /// service_name
    if (
      true == json_value_service_param.isMember(g_request_service_param_service_name) &&
      true == json_value_service_param[g_request_service_param_service_name].isString()
    ) {
      service_name = json_value_service_param[g_request_service_param_service_name].asString();
    }

    result = true;
  } while (0);

  return result;
}

/// ReqLoadConfigServiceParam
bool ReqLoadConfigServiceParam::ToString(std::string& to_string) {
  bool result = false;

  do {
    JsonValue json_value_service_param;

    PlatformProtocolImpl::PlatformProtocolParamPair platform_protocol_param_pair;

    /// config_file_name
    platform_protocol_param_pair.first = g_request_service_param_config_file_name;
    platform_protocol_param_pair.second = config_file_name;
    PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_service_param);

    JsonValue json_value_unit_array;
    JsonValue json_value_unit;

    for (
      ServiceConfigParamUnits::iterator iterator = service_config_param_units.begin();
      iterator != service_config_param_units.end();
      ++iterator
    ) {
      /// config_key
      platform_protocol_param_pair.first = g_request_service_param_config_param_config_key;
      platform_protocol_param_pair.second = iterator->config_key;
      PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_unit);

      /// load_name
      platform_protocol_param_pair.first = g_request_service_param_config_param_load_name;
      platform_protocol_param_pair.second = iterator->load_name;
      PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_unit);

      /// is_sync
      platform_protocol_param_pair.first = g_request_service_param_config_param_is_sync;
      platform_protocol_param_pair.second = iterator->is_sync;
      PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_unit);

      /// config_value
      platform_protocol_param_pair.first = g_request_service_param_config_param_config_value;
      platform_protocol_param_pair.second = iterator->config_value;
      PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_unit);

      PlatformProtocolImpl::JsonArrayFromJsonObject(json_value_unit, json_value_unit_array);
    }

    json_value_service_param[g_request_service_param_config_param] = json_value_unit_array;

    if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonValueToString(json_value_service_param, to_string)) {
      break;
    }

    result = true;
  } while (0);

  return result;
}

bool ReqLoadConfigServiceParam::FromString(const std::string& from_string) {
  bool result = false;

  do {
    JsonValue json_value_service_param;
    if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonObjectFromString(from_string, json_value_service_param)) {
      break;
    }

    /// config_file_name
    if (
      true == json_value_service_param.isMember(g_request_service_param_config_file_name) &&
      true == json_value_service_param[g_request_service_param_config_file_name].isString()
    ) {
      config_file_name = json_value_service_param[g_request_service_param_config_file_name].asString();
    }

    /// config_param
    if (true == json_value_service_param.isMember(g_request_service_param_config_param) &&
        true == json_value_service_param[g_request_service_param_config_param].isArray()
       ) {
      JsonValue json_value_config_param = json_value_service_param[g_request_service_param_config_param];
      if (true == json_value_config_param.empty()) {
        break;
      }

      if (0 < json_value_config_param.size()) {
        JsonValue json_value_config_param_unit;
        for (unsigned ii = 0; ii <json_value_config_param.size(); ++ii) {
          json_value_config_param_unit = json_value_config_param[ii];
          JsonValue json_value_object;
          ServiceConfigParamUnit unit;
          for (
            JsonValueIterator iterator = json_value_config_param_unit.begin();
            iterator != json_value_config_param_unit.end();
            ++iterator
          ) {
            /// config_key
            if (0 == iterator.key().asString().compare(g_request_service_param_config_param_config_key)) {
              json_value_object = json_value_config_param_unit[iterator.key().asString().c_str()];
              if (true == json_value_object.isString()) {
                unit.config_key = json_value_object.asString();
              }
            }

            /// load_name
            if (0 == iterator.key().asString().compare(g_request_service_param_config_param_load_name)) {
              json_value_object = json_value_config_param_unit[iterator.key().asString().c_str()];
              if (true == json_value_object.isString()) {
                unit.load_name = json_value_object.asString();
              }
            }

            /// is_sync
            if (0 == iterator.key().asString().compare(g_request_service_param_config_param_is_sync)) {
              json_value_object = json_value_config_param_unit[iterator.key().asString().c_str()];
              if (true == json_value_object.isString()) {
                unit.is_sync = json_value_object.asString();
              }
            }

            /// config_value
            if (0 == iterator.key().asString().compare(g_request_service_param_config_param_config_value)) {
              json_value_object = json_value_config_param_unit[iterator.key().asString().c_str()];
              if (true == json_value_object.isString()) {
                unit.config_value = json_value_object.asString();
              }
            }
          }
          service_config_param_units.push_back(unit);
        }
      }
    }

    result = true;
  } while (0);

  return result;
}

/// ReqEventNotifyServiceParam
bool ReqEventNotifyServiceParam::ToString(std::string& to_string) {
  bool result = false;

  do {
    JsonValue json_value_service_param;
    PlatformProtocolImpl::PlatformProtocolParamPair platform_protocol_param_pair;

    /// event_name
    platform_protocol_param_pair.first = g_request_service_param_event_name;
    platform_protocol_param_pair.second = event_name;
    PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_service_param);

    /// callback_name
    platform_protocol_param_pair.first = g_request_service_param_callback_name;
    platform_protocol_param_pair.second = callback_name;
    PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_service_param);

    /// data
    platform_protocol_param_pair.first = g_request_service_param_data;
    platform_protocol_param_pair.second = data;
    PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_service_param);

    if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonValueToString(json_value_service_param, to_string)) {
      break;
    }

    result = true;
  } while (0);

  return result;
}

bool ReqEventNotifyServiceParam::FromString(const std::string& from_string) {
  bool result = false;

  do {
    JsonValue json_value_service_param;
    if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonObjectFromString(from_string, json_value_service_param)) {
      break;
    }

    /// event_name
    if (
      true == json_value_service_param.isMember(g_request_service_param_event_name) &&
      true == json_value_service_param[g_request_service_param_event_name].isString()
    ) {
      event_name = json_value_service_param[g_request_service_param_event_name].asString();
    }

    /// callback_name
    if (
      true == json_value_service_param.isMember(g_request_service_param_callback_name) &&
      true == json_value_service_param[g_request_service_param_callback_name].isString()
    ) {
      callback_name = json_value_service_param[g_request_service_param_callback_name].asString();
    }

    /// data
    if (
      true == json_value_service_param.isMember(g_request_service_param_data) &&
      true == json_value_service_param[g_request_service_param_data].isString()
    ) {
      data = json_value_service_param[g_request_service_param_data].asString();
    }

    result = true;
  } while (0);

  return result;
}

/// ReqHeartbeatServiceParam
bool ReqHeartbeatServiceParam::ToString(std::string& to_string) {
  to_string = JsonNullValue;
  return true;
}

bool ReqHeartbeatServiceParam::FromString(const std::string& from_string) {
  return true;
}

/// ReqWriteLogServiceParam
bool ReqWriteLogServiceParam::ToString(std::string& to_string) {
  bool result = false;

  do {
    JsonValue json_value_service_param;
    PlatformProtocolImpl::PlatformProtocolParamPair platform_protocol_param_pair;

    /// event_name
    platform_protocol_param_pair.first = g_request_service_param_log_content;
    platform_protocol_param_pair.second = log_content;
    PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_service_param);

    if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonValueToString(json_value_service_param, to_string)) {
      break;
    }

    result = true;
  } while (0);

  return result;
}

bool ReqWriteLogServiceParam::FromString(const std::string& from_string) {
  bool result = false;

  do {
    JsonValue json_value_service_param;
    if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonObjectFromString(from_string, json_value_service_param)) {
      break;
    }

    /// log_content
    if (
      true == json_value_service_param.isMember(g_request_service_param_log_content) &&
      true == json_value_service_param[g_request_service_param_log_content].isString()
    ) {
      log_content = json_value_service_param[g_request_service_param_log_content].asString();
    }

    result = true;
  } while (0);

  return result;
}

//ReqGetDataServiceParam
bool ReqGetDataServiceParam::ToString(std::string& to_string) {
  bool result = false;

  do {
    JsonValue json_value_service_param;
    PlatformProtocolImpl::PlatformProtocolParamPair platform_protocol_param_pair;

    /// data
    JsonValue json_value_unit_array;
    JsonValue json_value_unit;

    for (
      KeyUnits::iterator iterator = key_units.begin();
      iterator != key_units.end();
      ++iterator
    ) {
      platform_protocol_param_pair.first = g_get_set_data_key;
      platform_protocol_param_pair.second = iterator->key;
      PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_unit);

      PlatformProtocolImpl::JsonArrayFromJsonObject(json_value_unit, json_value_unit_array);
    }

    json_value_service_param[g_get_set_data] = json_value_unit_array;

    if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonValueToString(json_value_service_param, to_string)) {
      break;
    }

    result = true;
  } while (0);

  return result;
}

bool ReqGetDataServiceParam::FromString(const std::string& from_string) {
  bool result = false;

  do {
    JsonValue json_value_service_param;
    if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonObjectFromString(from_string, json_value_service_param)) {
      break;
    }

    /// key_value_units
    if (true == json_value_service_param.isMember(g_get_set_data) &&
        true == json_value_service_param[g_get_set_data].isArray()
       ) {
      JsonValue json_value_key_units = json_value_service_param[g_get_set_data];
      if (true == json_value_key_units.empty()) {
        break;
      }

      if (0 < json_value_key_units.size()) {
        JsonValue json_value_key_value_unit;
        for (unsigned ii = 0; ii < json_value_key_units.size(); ++ii) {
          json_value_key_value_unit = json_value_key_units[ii];
          JsonValue json_value_object;
          KeyUnit unit;
          for (
            JsonValueIterator iterator = json_value_key_value_unit.begin();
            iterator != json_value_key_value_unit.end();
            ++iterator
          ) {
            /// key
            if (0 == iterator.key().asString().compare(g_get_set_data_key)) {
              json_value_object = json_value_key_value_unit[iterator.key().asString().c_str()];
              if (true == json_value_object.isString()) {
                unit.key = json_value_object.asString();
              }
            }
          }

          key_units.push_back(unit);
        }
      }
    }

    result = true;
  } while (0);

  return result;
}

//ReqSetDataServiceParam
bool ReqSetDataServiceParam::ToString(std::string& to_string) {
  bool result = false;

  do {
    JsonValue json_value_service_param;
    PlatformProtocolImpl::PlatformProtocolParamPair platform_protocol_param_pair;

    /// data
    JsonValue json_value_unit_array;
    JsonValue json_value_unit;

    for (
      KeyValueUnits::iterator iterator = key_value_units.begin();
      iterator != key_value_units.end();
      ++iterator
    ) {
      platform_protocol_param_pair.first = g_get_set_data_key;
      platform_protocol_param_pair.second = iterator->key;
      PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_unit);

      platform_protocol_param_pair.first = g_get_set_data_value;
      platform_protocol_param_pair.second = iterator->value;
      PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_unit);

      PlatformProtocolImpl::JsonArrayFromJsonObject(json_value_unit, json_value_unit_array);
    }

    json_value_service_param[g_get_set_data] = json_value_unit_array;

    if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonValueToString(json_value_service_param, to_string)) {
      break;
    }

    result = true;
  } while (0);

  return result;
}

bool ReqSetDataServiceParam::FromString(const std::string& from_string) {
  bool result = false;

  do {
    JsonValue json_value_service_param;
    if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonObjectFromString(from_string, json_value_service_param)) {
      break;
    }

    /// key_value_units
    if (true == json_value_service_param.isMember(g_get_set_data) &&
        true == json_value_service_param[g_get_set_data].isArray()
       ) {
      JsonValue json_value_key_value_units = json_value_service_param[g_get_set_data];
      if (true == json_value_key_value_units.empty()) {
        break;
      }

      if (0 < json_value_key_value_units.size()) {
        JsonValue json_value_key_value_unit;
        for (unsigned ii = 0; ii < json_value_key_value_units.size(); ++ii) {
          json_value_key_value_unit = json_value_key_value_units[ii];
          JsonValue json_value_object;
          KeyValueUnit unit;
          for (
            JsonValueIterator iterator = json_value_key_value_unit.begin();
            iterator != json_value_key_value_unit.end();
            ++iterator
          ) {
            /// key
            if (0 == iterator.key().asString().compare(g_get_set_data_key)) {
              json_value_object = json_value_key_value_unit[iterator.key().asString().c_str()];
              if (true == json_value_object.isString()) {
                unit.key = json_value_object.asString();
              }
            }

            /// value
            if (0 == iterator.key().asString().compare(g_get_set_data_value)) {
              json_value_object = json_value_key_value_unit[iterator.key().asString().c_str()];
              if (true == json_value_object.isString()) {
                unit.value = json_value_object.asString();
              }
            }
          }

          key_value_units.push_back(unit);
        }
      }
    }

    result = true;
  } while (0);

  return result;
}

/// ReqUninitServiceParam
bool ReqUninitServiceParam::ToString(std::string& to_string) {
  to_string = JsonNullValue;
  return true;
}

bool ReqUninitServiceParam::FromString(const std::string& from_string) {
  return true;
}

//////////////////////////////////////////////////////////////////////////
/*
Response Service Param
*/

/// ResInitServiceParam
bool ResInitServiceParam::ToString(std::string& to_string) {
  bool result = false;

  do {
    JsonValue json_value_result;
    PlatformProtocolImpl::PlatformProtocolParamPair platform_protocol_param_pair;

    platform_protocol_param_pair.first = g_response_result_status_code;
    platform_protocol_param_pair.second = status_code;
    PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_result);

    platform_protocol_param_pair.first = g_response_result_description;
    platform_protocol_param_pair.second = description;
    PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_result);

    json_value_result[g_response_result_detail] = JsonNullValue;

    if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonValueToString(json_value_result, to_string)) {
      break;
    }

    result = true;
  } while (0);

  return result;
}

bool ResInitServiceParam::FromString(const std::string& from_string) {
  bool result = false;

  do {
    JsonValue json_value_result;
    if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonObjectFromString(from_string, json_value_result)) {
      break;
    }

    /// status_code
    if (
      true == json_value_result.isMember(g_response_result_status_code) &&
      true == json_value_result[g_response_result_status_code].isString()
    ) {
      status_code = json_value_result[g_response_result_status_code].asString();
    }

    /// description
    if (
      true == json_value_result.isMember(g_response_result_description) &&
      true == json_value_result[g_response_result_description].isString()
    ) {
      description = json_value_result[g_response_result_description].asString();
    }

    result = true;
  } while (0);

  return result;
}

/// ResSwitchNotifyServiceParam
bool ResSwitchNotifyServiceParam::ToString(std::string& to_string) {
  bool result = false;

  do {
    JsonValue json_value_result;
    PlatformProtocolImpl::PlatformProtocolParamPair platform_protocol_param_pair;

    platform_protocol_param_pair.first = g_response_result_status_code;
    platform_protocol_param_pair.second = status_code;
    PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_result);

    platform_protocol_param_pair.first = g_response_result_description;
    platform_protocol_param_pair.second = description;
    PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_result);

    json_value_result[g_response_result_detail] = JsonNullValue;

    if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonValueToString(json_value_result, to_string)) {
      break;
    }

    result = true;
  } while (0);

  return result;
}

bool ResSwitchNotifyServiceParam::FromString(const std::string& from_string) {
  bool result = false;

  do {
    JsonValue json_value_object;
    if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonObjectFromString(from_string, json_value_object)) {
      break;
    }

    /// status_code
    if (
      true == json_value_object.isMember(g_response_result_status_code) &&
      true == json_value_object[g_response_result_status_code].isString()
    ) {
      status_code = json_value_object[g_response_result_status_code].asString();
    }

    /// description
    if (
      true == json_value_object.isMember(g_response_result_description) &&
      true == json_value_object[g_response_result_description].isString()
    ) {
      description = json_value_object[g_response_result_description].asString();
    }

    result = true;
  } while (0);

  return result;
}

/// ResSwitchServiceParam
bool ResSwitchServiceParam::ToString(std::string& to_string) {
  bool result = false;

  do {
    JsonValue json_value_result;
    PlatformProtocolImpl::PlatformProtocolParamPair platform_protocol_param_pair;

    platform_protocol_param_pair.first = g_response_result_status_code;
    platform_protocol_param_pair.second = status_code;
    PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_result);

    platform_protocol_param_pair.first = g_response_result_description;
    platform_protocol_param_pair.second = description;
    PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_result);

    JsonValue json_value_unit;
    platform_protocol_param_pair.first = g_response_result_detail_cur_state;
    platform_protocol_param_pair.second = switch_service_state_unit.cur_state;
    PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_unit);

    json_value_result[g_response_result_detail] = json_value_unit;

    if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonValueToString(json_value_result, to_string)) {
      break;
    }

    result = true;
  } while (0);

  return result;
}

bool ResSwitchServiceParam::FromString(const std::string& from_string) {
  bool result = false;

  do {
    JsonValue json_value_result;
    if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonObjectFromString(from_string, json_value_result)) {
      break;
    }

    /// status_code
    if (
      true == json_value_result.isMember(g_response_result_status_code) &&
      true == json_value_result[g_response_result_status_code].isString()
    ) {
      status_code = json_value_result[g_response_result_status_code].asString();
    }

    /// description
    if (
      true == json_value_result.isMember(g_response_result_description) &&
      true == json_value_result[g_response_result_description].isString()
    ) {
      description = json_value_result[g_response_result_description].asString();
    }

    /// detail
    JsonValue json_value_result_detail = json_value_result[g_response_result_detail];
    if (
      true != json_value_result_detail.empty() &&
      true == json_value_result_detail.isObject()
    ) {
      if (
        true == json_value_result_detail.isMember(g_response_result_detail_cur_state) &&
        true == json_value_result_detail[g_response_result_detail_cur_state].isString()
      ) {
        switch_service_state_unit.cur_state = json_value_result_detail[g_response_result_detail_cur_state].asString();
      }
    }

    result = true;
  } while (0);

  return result;
}

/// ResRegServiceEventServiceParam
bool ResRegServiceEventServiceParam::ToString(std::string& to_string) {
  bool result = false;

  do {
    JsonValue json_value_result;
    PlatformProtocolImpl::PlatformProtocolParamPair platform_protocol_param_pair;

    platform_protocol_param_pair.first = g_response_result_status_code;
    platform_protocol_param_pair.second = status_code;
    PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_result);

    platform_protocol_param_pair.first = g_response_result_description;
    platform_protocol_param_pair.second = description;
    PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_result);

    JsonValue json_value_detail;
    for (
      std::vector<std::pair<std::string, std::string>>::iterator iterator = details.begin();
      iterator != details.end();
      ++iterator
    ) {
      platform_protocol_param_pair.first = iterator->first;
      platform_protocol_param_pair.second = iterator->second;
      PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_detail);
    }

    json_value_result[g_response_result_detail] = json_value_detail;

    if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonValueToString(json_value_result, to_string)) {
      break;
    }

    result = true;
  } while (0);

  return result;
}

bool ResRegServiceEventServiceParam::FromString(const std::string& from_string) {
  bool result = false;

  do {
    JsonValue json_value_result;
    if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonObjectFromString(from_string, json_value_result)) {
      break;
    }

    /// status_code
    if (
      true == json_value_result.isMember(g_response_result_status_code) &&
      true == json_value_result[g_response_result_status_code].isString()
    ) {
      status_code = json_value_result[g_response_result_status_code].asString();
    }

    /// description
    if (
      true == json_value_result.isMember(g_response_result_description) &&
      true == json_value_result[g_response_result_description].isString()
    ) {
      description = json_value_result[g_response_result_description].asString();
    }

    /// detail
    if (
      true == json_value_result.isMember(g_response_result_detail) &&
      true == json_value_result[g_response_result_detail].isObject()
    ) {
      JsonValue json_value_result_detail = json_value_result[g_response_result_detail];
      if (true != json_value_result_detail.empty()) {
        JsonValue json_value_object;
        std::string key, value;
        for (
          JsonValueIterator iterator = json_value_result_detail.begin();
          iterator != json_value_result_detail.end();
          ++iterator
        ) {
          key = iterator.key().asString().c_str();

          json_value_object = json_value_result_detail[key.c_str()];
          if (true == json_value_object.isString()) {
            value = json_value_object.asString().c_str();
          }

          details.push_back(std::make_pair(key, value));
        }
      }
    }

    result = true;
  } while (0);

  return result;
}

/// ResUnregServiceEventServiceParam
bool ResUnregServiceEventServiceParam::ToString(std::string& to_string) {
  bool result = false;

  do {
    JsonValue json_value_result;
    PlatformProtocolImpl::PlatformProtocolParamPair platform_protocol_param_pair;

    platform_protocol_param_pair.first = g_response_result_status_code;
    platform_protocol_param_pair.second = status_code;
    PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_result);

    platform_protocol_param_pair.first = g_response_result_description;
    platform_protocol_param_pair.second = description;
    PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_result);

    json_value_result[g_response_result_detail] = JsonNullValue;

    if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonValueToString(json_value_result, to_string)) {
      break;
    }

    result = true;
  } while (0);

  return result;
}

bool ResUnregServiceEventServiceParam::FromString(const std::string& from_string) {
  bool result = false;

  do {
    JsonValue json_value_object;
    if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonObjectFromString(from_string, json_value_object)) {
      break;
    }

    /// status_code
    if (
      true == json_value_object.isMember(g_response_result_status_code) &&
      true == json_value_object[g_response_result_status_code].isString()
    ) {
      status_code = json_value_object[g_response_result_status_code].asString();
    }

    /// description
    if (
      true == json_value_object.isMember(g_response_result_description) &&
      true == json_value_object[g_response_result_description].isString()
    ) {
      description = json_value_object[g_response_result_description].asString();
    }

    result = true;
  } while (0);

  return result;
}

/// ResReadyServiceParam
bool ResReadyServiceParam::ToString(std::string& to_string) {
  bool result = false;

  do {
    JsonValue json_value_result;
    PlatformProtocolImpl::PlatformProtocolParamPair platform_protocol_param_pair;

    platform_protocol_param_pair.first = g_response_result_status_code;
    platform_protocol_param_pair.second = status_code;
    PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_result);

    platform_protocol_param_pair.first = g_response_result_description;
    platform_protocol_param_pair.second = description;
    PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_result);

    json_value_result[g_response_result_detail] = JsonNullValue;

    if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonValueToString(json_value_result, to_string)) {
      break;
    }

    result = true;
  } while (0);

  return result;
}

bool ResReadyServiceParam::FromString(const std::string& from_string) {
  bool result = false;

  do {
    JsonValue json_value_result;
    if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonObjectFromString(from_string, json_value_result)) {
      break;
    }

    /// status_code
    if (
      true == json_value_result.isMember(g_response_result_status_code) &&
      true == json_value_result[g_response_result_status_code].isString()
    ) {
      status_code = json_value_result[g_response_result_status_code].asString();
    }

    /// description
    if (
      true == json_value_result.isMember(g_response_result_description) &&
      true == json_value_result[g_response_result_description].isString()
    ) {
      description = json_value_result[g_response_result_description].asString();
    }

    result = true;
  } while (0);

  return result;
}

/// ResCustomEventServiceParam
bool ResCustomEventServiceParam::ToString(std::string& to_string) {
  bool result = false;

  do {
    JsonValue json_value_result;
    PlatformProtocolImpl::PlatformProtocolParamPair platform_protocol_param_pair;

    /// status_code
    platform_protocol_param_pair.first = g_response_result_status_code;
    platform_protocol_param_pair.second = status_code;
    PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_result);

    /// description
    platform_protocol_param_pair.first = g_response_result_description;
    platform_protocol_param_pair.second = description;
    PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_result);

    /// detail
    JsonValue json_value_result_detail;

    /// service_name
    if (true != custom_event_service_unit.service_name.empty()) {
      platform_protocol_param_pair.first = g_response_result_detail_service_name;
      platform_protocol_param_pair.second = custom_event_service_unit.service_name;
      PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_result_detail);
    }

    /// service_param
    if (true != custom_event_service_unit.service_param.empty()) {
      platform_protocol_param_pair.first = g_response_result_detail_state_param;
      platform_protocol_param_pair.second = custom_event_service_unit.service_param;
      PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_result_detail);
    }

    json_value_result[g_response_result_detail] = json_value_result_detail;

    if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonValueToString(json_value_result, to_string)) {
      break;
    }

    result = true;
  } while (0);

  return result;
}

bool ResCustomEventServiceParam::FromString(const std::string& from_string) {
  bool result = false;

  do {
    JsonValue json_value_result;
    if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonObjectFromString(from_string, json_value_result)) {
      break;
    }

    /// status_code
    if (true == json_value_result.isMember(g_response_result_status_code) &&
        true == json_value_result[g_response_result_status_code].isString()) {
      status_code = json_value_result[g_response_result_status_code].asString();
    }

    /// description
    if (true == json_value_result.isMember(g_response_result_description) &&
        true == json_value_result[g_response_result_description].isString()) {
      description = json_value_result[g_response_result_description].asString();
    }

    /// detail
    if (true == json_value_result.isMember(g_response_result_detail) ||
        true != json_value_result[g_response_result_detail].isNull() ||
        true == json_value_result[g_response_result_detail].isObject()) {
      JsonValue json_value_result_detail = json_value_result[g_response_result_detail];

      /// service_name
      if (true == json_value_result_detail.isMember(g_response_result_detail_service_name) &&
          true == json_value_result_detail[g_response_result_detail_service_name].isString()) {
        custom_event_service_unit.service_name = json_value_result_detail[g_response_result_detail_service_name].asString();
      }

      /// service_param
      if (true == json_value_result_detail.isMember(g_response_result_detail_state_param) &&
          true == json_value_result_detail[g_response_result_detail_state_param].isString()) {
        custom_event_service_unit.service_param = json_value_result_detail[g_response_result_detail_state_param].asString();
      }
    }

    result = true;
  } while (0);

  return result;
}

/// ResIOCtlServiceParam
bool ResIOCtlServiceParam::ToString(std::string& to_string) {
  bool result = false;

  do {
    JsonValue json_value_result;
    PlatformProtocolImpl::PlatformProtocolParamPair platform_protocol_param_pair;

    platform_protocol_param_pair.first = g_response_result_status_code;
    platform_protocol_param_pair.second = status_code;
    PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_result);

    platform_protocol_param_pair.first = g_response_result_description;
    platform_protocol_param_pair.second = description;
    PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_result);

    JsonValue json_value_detail;
    for (
      std::vector<std::pair<std::string, std::string>>::iterator iterator = details.begin();
      iterator != details.end();
      ++iterator
    ) {
      platform_protocol_param_pair.first = iterator->first;
      platform_protocol_param_pair.second = iterator->second;
      PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_detail);
    }

    json_value_result[g_response_result_detail] = json_value_detail;

    if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonValueToString(json_value_result, to_string)) {
      break;
    }

    result = true;
  } while (0);

  return result;
}

bool ResIOCtlServiceParam::FromString(const std::string& from_string) {
  bool result = false;

  do {
    JsonValue json_value_result;
    if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonObjectFromString(from_string, json_value_result)) {
      break;
    }

    /// status_code
    if (
      true == json_value_result.isMember(g_response_result_status_code) &&
      true == json_value_result[g_response_result_status_code].isString()
    ) {
      status_code = json_value_result[g_response_result_status_code].asString();
    }

    /// description
    if (
      true == json_value_result.isMember(g_response_result_description) &&
      true == json_value_result[g_response_result_description].isString()
    ) {
      description = json_value_result[g_response_result_description].asString();
    }

    /// detail
    if (
      true == json_value_result.isMember(g_response_result_detail) &&
      true == json_value_result[g_response_result_detail].isObject()
    ) {
      JsonValue json_value_result_detail = json_value_result[g_response_result_detail];
      if (true != json_value_result_detail.empty()) {
        JsonValue json_value_object;
        std::string key, value;
        for (
          JsonValueIterator iterator = json_value_result_detail.begin();
          iterator != json_value_result_detail.end();
          ++iterator
        ) {
          key = iterator.key().asString().c_str();

          json_value_object = json_value_result_detail[key.c_str()];
          if (true == json_value_object.isString()) {
            value = json_value_object.asString().c_str();
          }

          details.push_back(std::make_pair(key, value));
        }
      }
    }

    result = true;
  } while (0);

  return result;
}

/// ResGetServiceListServiceParam
bool ResGetServiceListServiceParam::ToString(std::string& to_string) {
  bool result = false;

  do {
    JsonValue json_value_result;

    PlatformProtocolImpl::PlatformProtocolParamPair platform_protocol_param_pair;

    /// status_code
    platform_protocol_param_pair.first = g_response_result_status_code;
    platform_protocol_param_pair.second = status_code;
    PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_result);

    /// description
    platform_protocol_param_pair.first = g_response_result_description;
    platform_protocol_param_pair.second = description;
    PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_result);

    JsonValue json_value_unit_array;
    JsonValue json_value_unit;

    for (
      ServiceListUnits::iterator iterator = service_list_units.begin();
      iterator != service_list_units.end();
      ++iterator
    ) {
      /// name
      platform_protocol_param_pair.first = g_response_result_detail_service_list_name;
      platform_protocol_param_pair.second = iterator->name;
      PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_unit);

      /// alias
      platform_protocol_param_pair.first = g_response_result_detail_service_list_alias;
      platform_protocol_param_pair.second = iterator->alias;
      PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_unit);

      /// description
      platform_protocol_param_pair.first = g_response_result_detail_service_list_description;
      platform_protocol_param_pair.second = iterator->description;
      PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_unit);

      PlatformProtocolImpl::JsonArrayFromJsonObject(json_value_unit, json_value_unit_array);
    }

    json_value_result[g_response_result_detail_service_list] = json_value_unit_array;

    if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonValueToString(json_value_result, to_string)) {
      break;
    }

    result = true;
  } while (0);

  return result;
}

bool ResGetServiceListServiceParam::FromString(const std::string& from_string) {
  bool result = false;

  do {
    JsonValue json_value_result;
    if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonObjectFromString(from_string, json_value_result)) {
      break;
    }

    /// status_code
    if (
      true == json_value_result.isMember(g_response_result_status_code) &&
      true == json_value_result[g_response_result_status_code].isString()
    ) {
      status_code = json_value_result[g_response_result_status_code].asString();
    }

    /// description
    if (
      true == json_value_result.isMember(g_response_result_description) &&
      true == json_value_result[g_response_result_description].isString()
    ) {
      description = json_value_result[g_response_result_description].asString();
    }

    /// detail
    JsonValue json_value_result_detail = json_value_result[g_response_result_detail];
    if (
      true != json_value_result_detail.empty() &&
      true == json_value_result_detail.isObject()
    ) {

      /// service_list
      if (true == json_value_result_detail.isMember(g_response_result_detail_service_list) &&
          true == json_value_result_detail[g_response_result_detail_service_list].isArray()
         ) {
        JsonValue json_value_service_list = json_value_result_detail[g_response_result_detail_service_list];
        if (true == json_value_service_list.empty()) {
          break;
        }

        if (0 < json_value_service_list.size()) {
          JsonValue json_value_service_list_unit;
          for (unsigned ii = 0; ii <json_value_service_list.size(); ++ii) {
            json_value_service_list_unit = json_value_service_list[ii];
            JsonValue json_value_object;
            ServiceListUnit unit;
            for (
              JsonValueIterator iterator = json_value_service_list_unit.begin();
              iterator != json_value_service_list_unit.end();
              ++iterator
            ) {
              /// name
              if (0 == iterator.key().asString().compare(g_response_result_detail_service_list_name)) {
                json_value_object = json_value_service_list_unit[iterator.key().asString().c_str()];
                if (true == json_value_object.isString()) {
                  unit.name = json_value_object.asString();
                }
              }

              /// alias
              if (0 == iterator.key().asString().compare(g_response_result_detail_service_list_alias)) {
                json_value_object = json_value_service_list_unit[iterator.key().asString().c_str()];
                if (true == json_value_object.isString()) {
                  unit.alias = json_value_object.asString();
                }
              }

              /// description
              if (0 == iterator.key().asString().compare(g_response_result_detail_service_list_description)) {
                json_value_object = json_value_service_list_unit[iterator.key().asString().c_str()];
                if (true == json_value_object.isString()) {
                  unit.description = json_value_object.asString();
                }
              }
            }
            service_list_units.push_back(unit);
          }
        }
      }
    }

    result = true;

  } while (0);

  return result;
}

/// ResGetServiceStateServiceParam
bool ResGetServiceStateServiceParam::ToString(std::string& to_string) {
  bool result = false;

  do {
    JsonValue json_value_result;
    PlatformProtocolImpl::PlatformProtocolParamPair platform_protocol_param_pair;

    platform_protocol_param_pair.first = g_response_result_status_code;
    platform_protocol_param_pair.second = status_code;
    PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_result);

    platform_protocol_param_pair.first = g_response_result_description;
    platform_protocol_param_pair.second = description;
    PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_result);

    JsonValue json_value_unit;
    platform_protocol_param_pair.first = g_response_result_detail_service_name;
    platform_protocol_param_pair.second = service_state_unit.service_name;
    PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_unit);

    platform_protocol_param_pair.first = g_response_result_detail_state_param;
    platform_protocol_param_pair.second = service_state_unit.state_param;
    PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_unit);

    json_value_result[g_response_result_detail] = json_value_unit;

    if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonValueToString(json_value_result, to_string)) {
      break;
    }

    result = true;
  } while (0);

  return result;
}

bool ResGetServiceStateServiceParam::FromString(const std::string& from_string) {
  bool result = false;

  do {
    JsonValue json_value_result;
    if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonObjectFromString(from_string, json_value_result)) {
      break;
    }

    /// status_code
    if (
      true == json_value_result.isMember(g_response_result_status_code) &&
      true == json_value_result[g_response_result_status_code].isString()
    ) {
      status_code = json_value_result[g_response_result_status_code].asString();
    }

    /// description
    if (
      true == json_value_result.isMember(g_response_result_description) &&
      true == json_value_result[g_response_result_description].isString()
    ) {
      description = json_value_result[g_response_result_description].asString();
    }

    /// detail
    JsonValue json_value_result_detail = json_value_result[g_response_result_detail];
    if (
      true != json_value_result_detail.empty() &&
      true == json_value_result_detail.isObject()
    ) {

      /// service_name
      if (
        true == json_value_result_detail.isMember(g_response_result_detail_service_name) &&
        true == json_value_result_detail[g_response_result_detail_service_name].isString()
      ) {
        service_state_unit.service_name = json_value_result_detail[g_response_result_detail_service_name].asString();
      }

      /// state_param
      if (
        true == json_value_result_detail.isMember(g_response_result_detail_state_param) &&
        true == json_value_result_detail[g_response_result_detail_state_param].isString()
      ) {
        service_state_unit.state_param = json_value_result_detail[g_response_result_detail_state_param].asString();
      }
    }

    result = true;
  } while (0);

  return result;
}

/// ResLoadConfigServiceParam
bool ResLoadConfigServiceParam::ToString(std::string& to_string) {
  bool result = false;

  do {
    JsonValue json_value_result;
    PlatformProtocolImpl::PlatformProtocolParamPair platform_protocol_param_pair;

    platform_protocol_param_pair.first = g_response_result_status_code;
    platform_protocol_param_pair.second = status_code;
    PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_result);

    platform_protocol_param_pair.first = g_response_result_description;
    platform_protocol_param_pair.second = description;
    PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_result);

    json_value_result[g_response_result_detail] = JsonNullValue;

    if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonValueToString(json_value_result, to_string)) {
      break;
    }

    result = true;
  } while (0);

  return result;
}

bool ResLoadConfigServiceParam::FromString(const std::string& from_string) {
  bool result = false;

  do {
    JsonValue json_value_result;
    if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonObjectFromString(from_string, json_value_result)) {
      break;
    }

    /// status_code
    if (
      true == json_value_result.isMember(g_response_result_status_code) &&
      true == json_value_result[g_response_result_status_code].isString()
    ) {
      status_code = json_value_result[g_response_result_status_code].asString();
    }

    /// description
    if (
      true == json_value_result.isMember(g_response_result_description) &&
      true == json_value_result[g_response_result_description].isString()
    ) {
      description = json_value_result[g_response_result_description].asString();
    }

    result = true;
  } while (0);

  return result;
}

/// ResEventNotifyServiceParam
bool ResEventNotifyServiceParam::ToString(std::string& to_string) {
  bool result = false;

  do {
    JsonValue json_value_result;
    PlatformProtocolImpl::PlatformProtocolParamPair platform_protocol_param_pair;

    platform_protocol_param_pair.first = g_response_result_status_code;
    platform_protocol_param_pair.second = status_code;
    PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_result);

    platform_protocol_param_pair.first = g_response_result_description;
    platform_protocol_param_pair.second = description;
    PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_result);

    json_value_result[g_response_result_detail] = JsonNullValue;

    if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonValueToString(json_value_result, to_string)) {
      break;
    }

    result = true;
  } while (0);

  return result;
}

bool ResEventNotifyServiceParam::FromString(const std::string& from_string) {
  bool result = false;

  do {
    JsonValue json_value_result;
    if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonObjectFromString(from_string, json_value_result)) {
      break;
    }

    /// status_code
    if (
      true == json_value_result.isMember(g_response_result_status_code) &&
      true == json_value_result[g_response_result_status_code].isString()
    ) {
      status_code = json_value_result[g_response_result_status_code].asString();
    }

    /// description
    if (
      true == json_value_result.isMember(g_response_result_description) &&
      true == json_value_result[g_response_result_description].isString()
    ) {
      description = json_value_result[g_response_result_description].asString();
    }

    result = true;
  } while (0);

  return result;
}

/// ResHeartbeatServiceParam
bool ResHeartbeatServiceParam::ToString(std::string& to_string) {
  bool result = false;

  do {
    JsonValue json_value_result;
    PlatformProtocolImpl::PlatformProtocolParamPair platform_protocol_param_pair;

    platform_protocol_param_pair.first = g_response_result_status_code;
    platform_protocol_param_pair.second = status_code;
    PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_result);

    platform_protocol_param_pair.first = g_response_result_description;
    platform_protocol_param_pair.second = description;
    PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_result);

    json_value_result[g_response_result_detail] = JsonNullValue;

    if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonValueToString(json_value_result, to_string)) {
      break;
    }

    result = true;
  } while (0);

  return result;
}

bool ResHeartbeatServiceParam::FromString(const std::string& from_string) {
  bool result = false;

  do {
    JsonValue json_value_result;
    if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonObjectFromString(from_string, json_value_result)) {
      break;
    }

    /// status_code
    if (
      true == json_value_result.isMember(g_response_result_status_code) &&
      true == json_value_result[g_response_result_status_code].isString()
    ) {
      status_code = json_value_result[g_response_result_status_code].asString();
    }

    /// description
    if (
      true == json_value_result.isMember(g_response_result_description) &&
      true == json_value_result[g_response_result_description].isString()
    ) {
      description = json_value_result[g_response_result_description].asString();
    }

    result = true;
  } while (0);

  return result;
}

/// ResWriteLogServiceParam
bool ResWriteLogServiceParam::ToString(std::string& to_string) {
  bool result = false;

  do {
    JsonValue json_value_result;
    PlatformProtocolImpl::PlatformProtocolParamPair platform_protocol_param_pair;

    platform_protocol_param_pair.first = g_response_result_status_code;
    platform_protocol_param_pair.second = status_code;
    PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_result);

    platform_protocol_param_pair.first = g_response_result_description;
    platform_protocol_param_pair.second = description;
    PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_result);

    json_value_result[g_response_result_detail] = JsonNullValue;

    if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonValueToString(json_value_result, to_string)) {
      break;
    }

    result = true;
  } while (0);

  return result;
}

bool ResWriteLogServiceParam::FromString(const std::string& from_string) {
  bool result = false;

  do {
    JsonValue json_value_result;
    if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonObjectFromString(from_string, json_value_result)) {
      break;
    }

    /// status_code
    if (
      true == json_value_result.isMember(g_response_result_status_code) &&
      true == json_value_result[g_response_result_status_code].isString()
    ) {
      status_code = json_value_result[g_response_result_status_code].asString();
    }

    /// description
    if (
      true == json_value_result.isMember(g_response_result_description) &&
      true == json_value_result[g_response_result_description].isString()
    ) {
      description = json_value_result[g_response_result_description].asString();
    }

    result = true;
  } while (0);

  return result;
}

//ResGetDataServiceParam
bool ResGetDataServiceParam::ToString(std::string& to_string) {
  bool result = false;

  do {
    JsonValue json_value_result;
    PlatformProtocolImpl::PlatformProtocolParamPair platform_protocol_param_pair;

    platform_protocol_param_pair.first = g_response_result_status_code;
    platform_protocol_param_pair.second = status_code;
    PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_result);

    platform_protocol_param_pair.first = g_response_result_description;
    platform_protocol_param_pair.second = description;
    PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_result);

    JsonValue json_value_unit_array;
    JsonValue json_value_unit;

    for (
      KeyValueUnits::iterator iterator = key_value_units.begin();
      iterator != key_value_units.end();
      ++iterator
    ) {
      /// key
      platform_protocol_param_pair.first = g_get_set_data_key;
      platform_protocol_param_pair.second = iterator->key;
      PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_unit);

      /// value
      platform_protocol_param_pair.first = g_get_set_data_value;
      platform_protocol_param_pair.second = iterator->value;
      PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_unit);

      /// add array
      PlatformProtocolImpl::JsonArrayFromJsonObject(json_value_unit, json_value_unit_array);
    }

    JsonValue json_value_unit_array_object;
    json_value_unit_array_object[g_get_set_data] = json_value_unit_array;

    json_value_result[g_response_result_detail] = json_value_unit_array_object;

    if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonValueToString(json_value_result, to_string)) {
      break;
    }

    result = true;
  } while (0);

  return result;
}

bool ResGetDataServiceParam::FromString(const std::string& from_string) {
  bool result = false;

  do {
    JsonValue json_value_result;
    if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonObjectFromString(from_string, json_value_result)) {
      break;
    }

    /// status_code
    if (
      true == json_value_result.isMember(g_response_result_status_code) &&
      true == json_value_result[g_response_result_status_code].isString()
    ) {
      status_code = json_value_result[g_response_result_status_code].asString();
    }

    /// description
    if (
      true == json_value_result.isMember(g_response_result_description) &&
      true == json_value_result[g_response_result_description].isString()
    ) {
      description = json_value_result[g_response_result_description].asString();
    }

    /// detail
    JsonValue json_value_result_detail = json_value_result[g_response_result_detail];
    if (
      true != json_value_result_detail.empty() &&
      true == json_value_result_detail.isObject()
    ) {

      /// key_value_units
      if (true == json_value_result.isMember(g_get_set_data) &&
          true == json_value_result[g_get_set_data].isArray()
         ) {
        JsonValue json_value_key_value_units = json_value_result[g_get_set_data];
        if (true == json_value_key_value_units.empty()) {
          break;
        }

        if (0 < json_value_key_value_units.size()) {
          JsonValue json_value_key_value_unit;
          for (unsigned ii = 0; ii < json_value_key_value_units.size(); ++ii) {
            json_value_key_value_unit = json_value_key_value_units[ii];
            JsonValue json_value_object;
            KeyValueUnit unit;
            for (
              JsonValueIterator iterator = json_value_key_value_unit.begin();
              iterator != json_value_key_value_unit.end();
              ++iterator
            ) {
              /// key
              if (0 == iterator.key().asString().compare(g_get_set_data_key)) {
                json_value_object = json_value_key_value_unit[iterator.key().asString().c_str()];
                if (true == json_value_object.isString()) {
                  unit.key = json_value_object.asString();
                }
              }

              /// value
              if (0 == iterator.key().asString().compare(g_get_set_data_value)) {
                json_value_object = json_value_key_value_unit[iterator.key().asString().c_str()];
                if (true == json_value_object.isString()) {
                  unit.value = json_value_object.asString();
                }
              }
            }

            key_value_units.push_back(unit);
          }
        }
      }
    }

    result = true;
  } while (0);

  return result;
}

//ResSetDataServiceParam
bool ResSetDataServiceParam::ToString(std::string& to_string) {
  bool result = false;

  do {
    JsonValue json_value_result;
    PlatformProtocolImpl::PlatformProtocolParamPair platform_protocol_param_pair;

    platform_protocol_param_pair.first = g_response_result_status_code;
    platform_protocol_param_pair.second = status_code;
    PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_result);

    platform_protocol_param_pair.first = g_response_result_description;
    platform_protocol_param_pair.second = description;
    PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_result);

    json_value_result[g_response_result_detail] = JsonNullValue;

    if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonValueToString(json_value_result, to_string)) {
      break;
    }

    result = true;
  } while (0);

  return result;
}

bool ResSetDataServiceParam::FromString(const std::string& from_string) {
  bool result = false;

  do {
    JsonValue json_value_result;
    if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonObjectFromString(from_string, json_value_result)) {
      break;
    }

    /// status_code
    if (
      true == json_value_result.isMember(g_response_result_status_code) &&
      true == json_value_result[g_response_result_status_code].isString()
    ) {
      status_code = json_value_result[g_response_result_status_code].asString();
    }

    /// description
    if (
      true == json_value_result.isMember(g_response_result_description) &&
      true == json_value_result[g_response_result_description].isString()
    ) {
      description = json_value_result[g_response_result_description].asString();
    }

    result = true;
  } while (0);

  return result;
}

/// ResUninitServiceParam
bool ResUninitServiceParam::ToString(std::string& to_string) {
  bool result = false;

  do {
    JsonValue json_value_result;
    PlatformProtocolImpl::PlatformProtocolParamPair platform_protocol_param_pair;

    platform_protocol_param_pair.first = g_response_result_status_code;
    platform_protocol_param_pair.second = status_code;
    PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_result);

    platform_protocol_param_pair.first = g_response_result_description;
    platform_protocol_param_pair.second = description;
    PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_result);

    json_value_result[g_response_result_detail] = JsonNullValue;

    if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonValueToString(json_value_result, to_string)) {
      break;
    }

    result = true;
  } while (0);

  return result;
}

bool ResUninitServiceParam::FromString(const std::string& from_string) {
  bool result = false;

  do {
    JsonValue json_value_result;
    if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonObjectFromString(from_string, json_value_result)) {
      break;
    }

    /// status_code
    if (
      true == json_value_result.isMember(g_response_result_status_code) &&
      true == json_value_result[g_response_result_status_code].isString()
    ) {
      status_code = json_value_result[g_response_result_status_code].asString();
    }

    /// description
    if (
      true == json_value_result.isMember(g_response_result_description) &&
      true == json_value_result[g_response_result_description].isString()
    ) {
      description = json_value_result[g_response_result_description].asString();
    }

    result = true;
  } while (0);

  return result;
}

}
