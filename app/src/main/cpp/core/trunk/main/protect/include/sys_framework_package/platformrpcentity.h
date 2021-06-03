/*
# platformrpcentity.h
# Definition of PlatformRPCEntity
# Created on: 2016-10-12 10:22:33
# Original author: jinchengzhe
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
# version 0.1, 20161012, created by JinChengZhe
*/

#ifndef SFP_PLATFORM_RPC_ENTITY_H
#define SFP_PLATFORM_RPC_ENTITY_H

/// C++ Standard Headers //  NOLINT
#include <iostream>
#include <vector>
#include <algorithm>
/// Platform Headers
#include "../../../public/include/sys_interface_package/common.h"

namespace ysos {

extern const char* g_request_service_param_service_name;
extern const char* g_request_service_param_flag;
extern const char* g_request_service_param_event_spec;
extern const char* g_request_service_param_id;
extern const char* g_request_service_param_type;
extern const char* g_request_service_param_value;
extern const char* g_request_service_param_config_file_name;
extern const char* g_request_service_param_config_param;
extern const char* g_request_service_param_state_name;
extern const char* g_request_service_param_app_name;
extern const char* g_request_service_param_strategy_name;
extern const char* g_request_service_param_log_content;

extern const char* g_request_service_param_event_spec_event_key;
extern const char* g_request_service_param_event_spec_callback_name;

extern const char* g_request_service_param_config_param_config_key;
extern const char* g_request_service_param_config_param_load_name;
extern const char* g_request_service_param_config_param_is_sync;
extern const char* g_request_service_param_config_param_config_value;

extern const char* g_response_result_status_code;
extern const char* g_response_result_description;
extern const char* g_response_result_detail;

extern const char* g_get_set_data;
extern const char* g_get_set_data_key;
extern const char* g_get_set_data_value;

extern const char* g_response_result_description_succeeded;
extern const char* g_response_result_description_faild;

//////////////////////////////////////////////////////////////////////////
/*
Request Service Param
*/
typedef struct YSOS_EXPORT ReqInitServiceParam {
  std::string app_name;
  std::string strategy_name;

  ReqInitServiceParam() :
    app_name(""),
    strategy_name("") {
  }

  ~ReqInitServiceParam() {
    app_name.clear();
    strategy_name.clear();
  }

  bool ToString(std::string& to_string);
  bool FromString(const std::string& from_string);
} *ReqInitServiceParamPtr;

typedef struct YSOS_EXPORT ReqSwitchNotifyServiceParam {
  std::string state_name;

  ReqSwitchNotifyServiceParam() :
    state_name("") {}

  ~ReqSwitchNotifyServiceParam() {
    state_name.clear();
  }

  bool ToString(std::string& to_string);
  bool FromString(const std::string& from_string);
} *ReqSwitchNotifyServiceParamPtr;

typedef struct YSOS_EXPORT ReqSwitchServiceParam {
  std::string state_name;

  ReqSwitchServiceParam() :
    state_name("") {
  }

  ~ReqSwitchServiceParam() {
    state_name.clear();
  }

  bool ToString(std::string& to_string);
  bool FromString(const std::string& from_string);
} *ReqSwitchServiceParamPtr;

typedef struct YSOS_EXPORT ServiceEventSpecUnit {
  std::string event_key;
  std::string callback_name;

  ServiceEventSpecUnit() :
    event_key(""),
    callback_name("") {
  }

  ~ServiceEventSpecUnit() {
    event_key.clear();
    callback_name.clear();
  }
}  *ServiceEventSpecUnitPtr;

typedef std::vector<ServiceEventSpecUnit> ServiceEventSpecUnits;

typedef struct YSOS_EXPORT ReqRegServiceEventServiceParam {
  std::string service_name;
  std::string flag;
  ServiceEventSpecUnits service_event_spec_units;

  ReqRegServiceEventServiceParam() :
    service_name(""),
    flag("") {
    service_event_spec_units.clear();
  }

  ~ReqRegServiceEventServiceParam() {
    service_name.clear();
    flag.clear();
    service_event_spec_units.clear();
  }

  bool ToString(std::string& to_string);
  bool FromString(const std::string& from_string);
} *ReqRegServiceEventServiceParamPtr;

typedef struct YSOS_EXPORT ReqUnregServiceEventServiceParam {
  std::string service_name;
  ServiceEventSpecUnits service_event_spec_units;

  ReqUnregServiceEventServiceParam() :
    service_name("") {
    service_event_spec_units.clear();
  }

  ~ReqUnregServiceEventServiceParam() {
    service_name.clear();
    service_event_spec_units.clear();
  }

  bool ToString(std::string& to_string);
  bool FromString(const std::string& from_string);
} *ReqUnregServiceEventServiceParamPtr;

typedef struct YSOS_EXPORT ReqReadyServiceParam {
  ReqReadyServiceParam() {}
  ~ReqReadyServiceParam() {}

  bool ToString(std::string& to_string);
  bool FromString(const std::string& from_string);
} *ReqReadyServiceParamPtr;

typedef struct YSOS_EXPORT ReqCustomEventServiceParam {
  std::string service_name;
  std::string service_param;

  ReqCustomEventServiceParam() :
    service_name(""),
    service_param("") {}
  ~ReqCustomEventServiceParam() {
    service_name.clear();
    service_param.clear();
  }

  bool ToString(std::string& to_string);
  bool FromString(const std::string& from_string);
} *ReqCustomEventServiceParamPtr;

typedef struct YSOS_EXPORT ReqIOCtlServiceParam {
  std::string service_name;
  std::string id;
  std::string type;
  std::string value;

  ReqIOCtlServiceParam() :
    service_name(""),
    id(""),
    type(""),
    value("") {
  }

  ~ReqIOCtlServiceParam() {
    service_name.clear();
    id.clear();
    type.clear();
    value.clear();
  }

  bool ToString(std::string& to_string);
  bool FromString(const std::string& from_string);
} *ReqIOCtlServiceParamPtr;

typedef struct YSOS_EXPORT ReqGetServiceListServiceParam {
  ReqGetServiceListServiceParam() {}
  ~ReqGetServiceListServiceParam() {}

  bool ToString(std::string& to_string);
  bool FromString(const std::string& from_string);
} *ReqGetServiceListServiceParamPtr;

typedef struct YSOS_EXPORT ReqGetServiceStateServiceParam {
  std::string service_name;

  ReqGetServiceStateServiceParam() :
    service_name("") {
  }

  ~ReqGetServiceStateServiceParam() {
    service_name.clear();
  }

  bool ToString(std::string& to_string);
  bool FromString(const std::string& from_string);
} *ReqGetServiceStateServiceParamPtr;

typedef struct YSOS_EXPORT ServiceConfigParamUnit {
  std::string config_key;
  std::string load_name;
  std::string is_sync;
  std::string config_value;

  ServiceConfigParamUnit() :
    config_key(""),
    load_name(""),
    is_sync(""),
    config_value("") {
  }

  ~ServiceConfigParamUnit() {
    config_key.clear();
    load_name.clear();
    is_sync.clear();
    config_value.clear();
  }
} *ServiceConfigParamUnitPtr;

typedef std::vector<ServiceConfigParamUnit> ServiceConfigParamUnits;

typedef struct YSOS_EXPORT ReqLoadConfigServiceParam {
  std::string config_file_name;
  ServiceConfigParamUnits service_config_param_units;

  ReqLoadConfigServiceParam() :
    config_file_name("") {
    service_config_param_units.clear();
  }

  ~ReqLoadConfigServiceParam() {
    config_file_name.clear();
    service_config_param_units.clear();
  }

  bool ToString(std::string& to_string);
  bool FromString(const std::string& from_string);
} *ReqLoadConfigServiceParamPtr;

typedef struct YSOS_EXPORT ReqEventNotifyServiceParam {
  std::string event_name;
  std::string callback_name;
  std::string data;

  ReqEventNotifyServiceParam() :
    event_name(""),
    callback_name(""),
    data("") {
  }

  ~ReqEventNotifyServiceParam() {
    event_name.clear();
    callback_name.clear();
    data.clear();
  }

  bool ToString(std::string& to_string);
  bool FromString(const std::string& from_string);
} *ReqEventNotifyServiceParamPtr;

typedef struct YSOS_EXPORT ReqHeartbeatServiceParam {
  ReqHeartbeatServiceParam() {
  }

  ~ReqHeartbeatServiceParam() {
  }

  bool ToString(std::string& to_string);
  bool FromString(const std::string& from_string);
} *ReqHeartbeatServiceParamPtr;

typedef struct YSOS_EXPORT ReqWriteLogServiceParam {
  std::string log_content;

  ReqWriteLogServiceParam() :
    log_content("") {
  }

  ~ReqWriteLogServiceParam() {
    log_content.clear();
  }

  bool ToString(std::string& to_string);
  bool FromString(const std::string& from_string);
} *ReqWriteLogServiceParamPtr;

typedef struct YSOS_EXPORT KeyUnit {
  std::string key;

  KeyUnit() :
    key("") {
  }

  ~KeyUnit() {
    key.clear();
  }

  void Clear() {
    key.clear();
  }
} *KeyUnitPtr;

typedef std::vector<KeyUnit> KeyUnits;

typedef struct YSOS_EXPORT ReqGetDataServiceParam {
  KeyUnits key_units;

  ReqGetDataServiceParam() {
  }

  ~ReqGetDataServiceParam() {
    key_units.clear();
  }

  bool ToString(std::string& to_string);
  bool FromString(const std::string& from_string);
} *ReqGetDataServiceParamPtr;

typedef struct YSOS_EXPORT KeyValueUnit {
  std::string key;
  std::string value;

  KeyValueUnit() :
    key(""),
    value("") {
  }

  ~KeyValueUnit() {
    key.clear();
    value.clear();
  }

  void Clear() {
    key.clear();
    value.clear();
  }
} *KeyValueUnitPtr;

typedef std::vector<KeyValueUnit> KeyValueUnits;

typedef struct YSOS_EXPORT ReqSetDataServiceParam {
  KeyValueUnits key_value_units;

  ReqSetDataServiceParam() {
  }

  ~ReqSetDataServiceParam() {
    key_value_units.clear();
  }

  bool ToString(std::string& to_string);
  bool FromString(const std::string& from_string);
} *ReqSetDataServiceParamPtr;

typedef struct YSOS_EXPORT ReqUninitServiceParam {
  ReqUninitServiceParam() {}

  ~ReqUninitServiceParam() {}

  bool ToString(std::string& to_string);
  bool FromString(const std::string& from_string);
} *ReqUninitServiceParamPtr;
//////////////////////////////////////////////////////////////////////////
/*
Response Service Param
*/
typedef struct YSOS_EXPORT ResInitServiceParam {
  std::string status_code;
  std::string description;

  ResInitServiceParam() :
    status_code(""),
    description("") {
  }

  ~ResInitServiceParam() {
    status_code.clear();
    description.clear();
  }

  bool ToString(std::string& to_string);
  bool FromString(const std::string& from_string);
} *ResInitServiceParamPtr;

typedef struct YSOS_EXPORT ResSwitchNotifyServiceParam {
  std::string status_code;
  std::string description;

  ResSwitchNotifyServiceParam() :
    status_code(""),
    description("") {
  }

  ~ResSwitchNotifyServiceParam() {
    status_code.clear();
    description.clear();
  }

  bool ToString(std::string& to_string);
  bool FromString(const std::string& from_string);
} *ResSwitchNotifyServiceParamPtr;

typedef struct YSOS_EXPORT SwitchServiceStateUnit {
  std::string cur_state;

  SwitchServiceStateUnit() :
    cur_state("") {
  }

  ~SwitchServiceStateUnit() {
    cur_state.clear();
  }
} *SwitchServiceStateUnitPtr;

typedef struct YSOS_EXPORT ResSwitchServiceParam {
  std::string status_code;
  std::string description;
  SwitchServiceStateUnit switch_service_state_unit;

  ResSwitchServiceParam() :
    status_code(""),
    description("") {
  }

  ~ResSwitchServiceParam() {
    status_code.clear();
    description.clear();
  }

  bool ToString(std::string& to_string);
  bool FromString(const std::string& from_string);
} *ResSwitchServiceParamPtr;

typedef struct YSOS_EXPORT ResRegServiceEventServiceParam {
  std::string status_code;
  std::string description;
  std::vector<std::pair<std::string, std::string>> details;

  ResRegServiceEventServiceParam() :
    status_code(""),
    description("") {
    details.clear();
  }

  ~ResRegServiceEventServiceParam() {
    status_code.clear();
    description.clear();
    details.clear();
  }

  bool ToString(std::string& to_string);
  bool FromString(const std::string& from_string);
} *ResRegServiceEventServiceParamPtr;

typedef struct YSOS_EXPORT ResUnregServiceEventServiceParam {
  std::string status_code;
  std::string description;

  ResUnregServiceEventServiceParam() :
    status_code(""),
    description("") {
  }

  ~ResUnregServiceEventServiceParam() {
    status_code.clear();
    description.clear();
  }

  bool ToString(std::string& to_string);
  bool FromString(const std::string& from_string);
} *ResUnregServiceEventServiceParamPtr;

typedef struct YSOS_EXPORT ResReadyServiceParam {
  std::string status_code;
  std::string description;

  ResReadyServiceParam() :
    status_code(""),
    description("") {}

  ~ResReadyServiceParam() {
    status_code.clear();
    description.clear();
  }

  bool ToString(std::string& to_string);
  bool FromString(const std::string& from_string);
} *ResReadyServiceParamPtr;

typedef struct YSOS_EXPORT CustomEventServiceUnit {
  std::string service_name;
  std::string service_param;

  CustomEventServiceUnit() :
    service_name(""),
    service_param("") {
  }

  ~CustomEventServiceUnit() {
    service_name.clear();
    service_param.clear();
  }
} *CustomEventServiceUnitPtr;

typedef struct YSOS_EXPORT ResCustomEventServiceParam {
  std::string status_code;
  std::string description;
  CustomEventServiceUnit custom_event_service_unit;

  ResCustomEventServiceParam() :
    status_code(""),
    description("") {}

  ~ResCustomEventServiceParam() {
    status_code.clear();
    description.clear();
  }

  bool ToString(std::string& to_string);
  bool FromString(const std::string& from_string);
} *ResCustomEventServiceParamPtr;

typedef struct YSOS_EXPORT ResIOCtlServiceParam {
  std::string status_code;
  std::string description;
  std::vector<std::pair<std::string, std::string>> details;

  ResIOCtlServiceParam() :
    status_code(""),
    description("") {
    details.clear();
  }

  ~ResIOCtlServiceParam() {
    status_code.clear();
    description.clear();
    details.clear();
  }

  bool ToString(std::string& to_string);
  bool FromString(const std::string& from_string);
} *ResIOCtlServiceParamPtr;

typedef struct YSOS_EXPORT ServiceListUnit {
  std::string name;
  std::string alias;
  std::string description;

  ServiceListUnit() :
    name(""),
    alias(""),
    description("") {
  }

  ~ServiceListUnit() {
    name.clear();
    alias.clear();
    description.clear();
  }
} *ServiceListUnitPtr;

typedef std::vector<ServiceListUnit> ServiceListUnits;

typedef struct YSOS_EXPORT ResGetServiceListServiceParam {
  std::string status_code;
  std::string description;
  ServiceListUnits service_list_units;

  ResGetServiceListServiceParam() :
    status_code(""),
    description("") {
    service_list_units.clear();
  }

  ~ResGetServiceListServiceParam() {
    status_code.clear();
    description.clear();
    service_list_units.clear();
  }

  bool ToString(std::string& to_string);
  bool FromString(const std::string& from_string);
} *ResGetServiceListServiceParamPtr;

typedef struct YSOS_EXPORT ServiceStateUnit {
  std::string service_name;
  std::string state_param;

  ServiceStateUnit() :
    service_name(""),
    state_param("") {
  }

  ~ServiceStateUnit() {
    service_name.clear();
    state_param.clear();
  }
} *ServiceStateUnitPtr;

typedef struct YSOS_EXPORT ResGetServiceStateServiceParam {
  std::string status_code;
  std::string description;
  ServiceStateUnit service_state_unit;

  ResGetServiceStateServiceParam() :
    status_code(""),
    description("") {
  }

  ~ResGetServiceStateServiceParam() {
    status_code.clear();
    description.clear();
  }

  bool ToString(std::string& to_string);
  bool FromString(const std::string& from_string);
} *ResGetServiceStateServiceParamPtr;

typedef struct YSOS_EXPORT ResLoadConfigServiceParam {
  std::string status_code;
  std::string description;

  ResLoadConfigServiceParam() :
    status_code(""),
    description("") {
  }

  ~ResLoadConfigServiceParam() {
    status_code.clear();
    description.clear();
  }

  bool ToString(std::string& to_string);
  bool FromString(const std::string& from_string);
} *ResLoadConfigServiceParamPtr;

typedef struct YSOS_EXPORT ResEventNotifyServiceParam {
  std::string status_code;
  std::string description;

  ResEventNotifyServiceParam() :
    status_code(""),
    description("") {
  }

  ~ResEventNotifyServiceParam() {
    status_code.clear();
    description.clear();
  }

  bool ToString(std::string& to_string);
  bool FromString(const std::string& from_string);
} *ResEventNotifyServiceParamPtr;

typedef struct YSOS_EXPORT ResHeartbeatServiceParam {
  std::string status_code;
  std::string description;

  ResHeartbeatServiceParam() :
    status_code(""),
    description("") {
  }

  ~ResHeartbeatServiceParam() {
    status_code.clear();
    description.clear();
  }

  bool ToString(std::string& to_string);
  bool FromString(const std::string& from_string);
} *ResHeartbeatServiceParamPtr;

typedef struct YSOS_EXPORT ResWriteLogServiceParam {
  std::string status_code;
  std::string description;

  ResWriteLogServiceParam() :
    status_code(""),
    description("") {
  }

  ~ResWriteLogServiceParam() {
    status_code.clear();
    description.clear();
  }

  bool ToString(std::string& to_string);
  bool FromString(const std::string& from_string);
} *ResWriteLogServiceParamPtr;

typedef struct YSOS_EXPORT ResGetDataServiceParam {
  std::string status_code;
  std::string description;
  KeyValueUnits key_value_units;

  ResGetDataServiceParam():
    status_code(""),
    description("") {
  }

  ~ResGetDataServiceParam() {
    status_code.clear();
    description.clear();
    key_value_units.clear();
  }

  bool ToString(std::string& to_string);
  bool FromString(const std::string& from_string);
} *ResGetDataServiceParamPtr;

typedef struct YSOS_EXPORT ResSetDataServiceParam {
  std::string status_code;
  std::string description;

  ResSetDataServiceParam() :
    status_code(""),
    description("") {
  }

  ~ResSetDataServiceParam() {
    status_code.clear();
    description.clear();
  }

  bool ToString(std::string& to_string);
  bool FromString(const std::string& from_string);
} *ResSetDataServiceParamPtr;

typedef struct YSOS_EXPORT ResUninitServiceParam {
  std::string status_code;
  std::string description;

  ResUninitServiceParam() :
    status_code(""),
    description("") {}

  ~ResUninitServiceParam() {
    status_code.clear();
    description.clear();
  }

  bool ToString(std::string& to_string);
  bool FromString(const std::string& from_string);
} *ResUninitServiceParamPtr;

}

#endif
