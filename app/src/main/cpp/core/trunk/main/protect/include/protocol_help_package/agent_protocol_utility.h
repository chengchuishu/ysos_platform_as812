/**
 *@file AgentProtocolUtility.h
 *@brief Definition of AgentProtocolUtility
 *@version 0.1
 *@author XuLanyue
 *@date Created on: 2016-06-28 22:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef CHP_AGENT_MESSAGE_DEF_H     //NOLINT
#define CHP_AGENT_MESSAGE_DEF_H     //NOLINT

#include <boost/shared_ptr.hpp>
#include "../../../public/include/sys_interface_package/common.h"
#include <json/json.h>
#include <map>
#include <list>

#define PARAM_ONDISPATCHMESSAGEPARAM_P2A_AS_STRING  1
/*
完整json串格式://NOLINT
{
  "header":
  {
    "tag":"ysos",//固定//NOLINT
    "version":"1.0.0.1234",//根据实际版本设置//NOLINT
    "type":"31006",//根据实际情况设置:verb+方向//NOLINT
    "time_stamp":"2016-06-20 20:04:00 123",//长度固定//NOLINT
    "serial_number":"nnnnnnnnnn"//长度固定//NOLINT
  },
  "content":
  {
    "from":"127.0.0.1:6001",//根据实际情况设置//NOLINT
    "to":"127.0.0.1:6002",//根据实际情况设置//NOLINT
    "verb":"RegDoServiceEvent",//根据实际情况设置//NOLINT
    "param":PARAM//根据实际情况设置, 其格式由verb和type确定//NOLINT
  }
}
*/
#define YSOS_AGENT_PROTOCOL_TAG                     "ysos"
#define YSOS_AGENT_PROTOCOL_VERSION                 "1.0.0.1"

//  消息流水号Agent通讯协议中, 每个消息的唯一标识符.//NOLINT
//  无效的消息流水号, 表示可以忽略.//NOLINT
//  A_Agent和对应的P_Agent各自维护其下一个流水号.//NOLINT
//  如果A_Agent或P_Agent认为其给出的流水号P_Agent或A_Agent无需关心,//NOLINT
//  那么给出YSOS_AGENT_INVALID_SERIAL_NUMBER.//NOLINT
#define YSOS_AGENT_INVALID_SERIAL_NUMBER            "0000000000"

namespace ysos {
YSOS_EXPORT std::string GetCurrentTimeStamp();
YSOS_EXPORT std::string ConvertUint32ToString(uint32_t number);
//  获取/content/param的值//NOLINT
YSOS_EXPORT std::string GetContentParam(const std::string& json_string);

//  将type = "31006"转为verb = "RegDoServiceEvent", from_app = false
bool TypeToVerbAndDirection(
  const std::string& type,
  std::string& verb,
  bool& from_app);

//  将type = 31006转为type_string = "31006", verb = "RegDoServiceEvent", from_app = false
bool TypeToVerbAndDirection(
  int type,
  std::string& type_string,
  std::string& verb,
  bool& from_app);

//  将type = 31006转为verb = "RegDoServiceEvent", from_app = false
bool TypeToVerbAndDirection(
  int type,
  std::string& verb,
  bool& from_app);

//  将type_string = "31006"转为type = 31006
YSOS_EXPORT bool TypeStringToType(const std::string& type_string, int& type);

//  将type = 31006转为type_string = "31006"
YSOS_EXPORT bool TypeToTypeString(int type, std::string& type_string);
/*
{
  "tag":"ysos",//固定//NOLINT
  "version":"1.0.0.1234",//根据实际版本设置//NOLINT
  "type":"31006",//根据实际情况设置:verb+方向//NOLINT
  "time_stamp":"2016-06-20 20:04:00 123",//长度固定//NOLINT
  "serial_number":"nnnnnnnnnn"//长度固定//NOLINT
}
*/
struct YSOS_EXPORT AgentMessageHead {
  std::string tag;            //固定为ysos//NOLINT
  std::string version;        //根据实际版本设置//NOLINT
  std::string type;           //根据实际情况设置:verb+方向//NOLINT
  std::string time_stamp;     //长度固定;调试时使用本地时刻,发布时使用GMT时刻//NOLINT
  std::string session_id;     //32位无符号十进制整数对应的串//NOLINT
  std::string serial_number;  //长度固定//NOLINT
  AgentMessageHead(int type, uint32_t serial_number, uint32_t session_id);
  AgentMessageHead(int type, uint32_t serial_number);
  /*
  AgentMessageHead(int type, uint32_t session_id);
  */
  AgentMessageHead(int type);
  AgentMessageHead();
  std::string ToString();
  bool FromString(const std::string& json_string);
};

struct YSOS_EXPORT Param_GetServiceList_A2P {
  Param_GetServiceList_A2P();
  ~Param_GetServiceList_A2P();
  std::string ToString();
  bool FromString(const std::string& json_string);
};

/*
{
  "name":"name_of_servicexxx",
  "alias":"alias_of_servicexxx",
  "description":"description_of_servicexxx"
}
*/
struct YSOS_EXPORT Param_GetServiceListItem_P2A {
  std::string name;
  std::string alias;
  std::string description;
  Param_GetServiceListItem_P2A();
  ~Param_GetServiceListItem_P2A();
  std::string ToString();
  bool FromString(const std::string& json_string);
};
typedef std::list<Param_GetServiceListItem_P2A> Param_ServiceList;

struct YSOS_EXPORT Param_GetServiceList_P2A {
  std::string result;
  std::string description;
  std::string serial_number;
  Param_ServiceList service_list;
  Param_GetServiceList_P2A();
  ~Param_GetServiceList_P2A();
  std::string ToString();
  bool FromString(const std::string& json_string);
};

struct YSOS_EXPORT Param_GetServiceState_A2P {
  std::string service_name;
  std::string serial_number;
  Param_GetServiceState_A2P();
  ~Param_GetServiceState_A2P();
  std::string ToString();
  bool FromString(const std::string& json_string);
};
struct YSOS_EXPORT Param_GetServiceState_P2A {
  std::string result;
  std::string description;
  std::string state_param;
  std::string serial_number;
  Param_GetServiceState_P2A();
  ~Param_GetServiceState_P2A();
  std::string ToString();
  bool FromString(const std::string& json_string);
};

/*
For RegDoServiceEvent/UnregDoServiceEvent/DoService
{
  "service_name":"ASRByiFly",
  "event_param"://null , if Unreg each event of the service
  [
    {"event_key":"event_key_1", "callback_name":"callbackName1"},
    {"event_key":"event_key_1", "callback_name":"callbackName2"},
    {"event_key":"event_key_4", "callback_name":"callbackName3"}
  ]
}
*/
typedef std::pair<std::string, std::string> EventCallbackPair;
typedef std::multimap<std::string, std::string> EventCallbackMap;
struct YSOS_EXPORT Param_ServiceEventHandleSpec_A2P {
  std::string service_name;
  std::string service_param;
  std::string event_name;
  std::string event_string;
  EventCallbackMap event_map;
  Param_ServiceEventHandleSpec_A2P();
  ~Param_ServiceEventHandleSpec_A2P();
  std::string ToString();
  bool FromString(const std::string& json_string);
};

/*
{
  "result":"0",
  "description":"succeeded",
  "service_name":"ASRByiFly"
}
*/
struct YSOS_EXPORT Param_ServiceEventHandleSpec_P2A {
  std::string result;
  std::string description;
  std::string serial_number;
  Param_ServiceEventHandleSpec_P2A();
  ~Param_ServiceEventHandleSpec_P2A();
  std::string ToString();
  bool FromString(const std::string& json_string);
};

/*
{
  "result":"0",
  "description":"succeeded",
  "serial_number":"cccccccccc"
  //"config_file_name":"xxx_app_config.xml"
}
*/
struct YSOS_EXPORT Param_LoadConfig_P2A {
  std::string result;
  std::string description;
  std::string serial_number;
  Param_LoadConfig_P2A();
  ~Param_LoadConfig_P2A();
  std::string ToString();
  bool FromString(const std::string& json_string);
};

/*
{
  "config_file_name":"xxx_app_config.xml",
  "config_param":[
    {
      "config_key":"config_key_1",
      "load_name":"callbackName1",
      "is_sync":"0",
      "config_value":"xxx"
    },
    {
      "config_key":"config_key_4",
      "load_name":"callbackName4",
      "is_sync":"1",
      "config_value":"yyy"
    }
  ]
}
*/
struct YSOS_EXPORT Param_LoadConfigParam_A2P {
  std::string config_key;
  std::string load_name;
  std::string is_sync;
  std::string config_value;
  Param_LoadConfigParam_A2P();
  ~Param_LoadConfigParam_A2P();
  std::string ToString();
  bool FromString(const std::string& json_string);
};
typedef std::list<Param_LoadConfigParam_A2P> Param_LoadConfigParam_A2PList;

struct YSOS_EXPORT Param_LoadConfig_A2P {
  std::string config_file_name;
  Param_LoadConfigParam_A2PList config_param;
  Param_LoadConfig_A2P();
  ~Param_LoadConfig_A2P();
  std::string ToString();
  bool FromString(const std::string& json_string);
};

/*
{
  "service_name":"default@ASRModuleByiFly",
  "event_key":"event_key_1",
  "callback_name":"callbackName1",
  "event_param":
  {
    "status":"0",
    "description":"succeeded",
    //...
  }
}
*/
#if PARAM_ONDISPATCHMESSAGEPARAM_P2A_AS_STRING
typedef std::string Param_OnDispatchMessageParam_P2A;
#else
struct YSOS_EXPORT Param_OnDispatchMessageParam_P2A {
  std::string status;
  std::string description;
  //...
  Param_OnDispatchMessageParam_P2A();
  ~Param_OnDispatchMessageParam_P2A();
  std::string ToString();
  bool FromString(const std::string& json_string);
};
#endif

struct YSOS_EXPORT Param_OnDispatchMessage_P2A {
  std::string service_name;
  std::string event_key;
  std::string callback_name;
  Param_OnDispatchMessageParam_P2A event_param;
  Param_OnDispatchMessage_P2A();
  ~Param_OnDispatchMessage_P2A();
  std::string ToString();
  bool FromString(const std::string& json_string);
};

/*
{
  "result":"0",
  "description":"succeeded",
  "serial_number":"cccccccccc"//长度固定//NOLINT
  //"service_name":"ASRByiFly",
  //"event_key":"event_key_1",
  //"callback_name":"callbackName1"
}
*/
struct YSOS_EXPORT Param_OnDispatchMessage_A2P {
  std::string result;
  std::string description;
  std::string serial_number;
  Param_OnDispatchMessage_A2P();
  ~Param_OnDispatchMessage_A2P();
  std::string ToString();
  bool FromString(const std::string& json_string);
};

YSOS_EXPORT bool NewParamInContent(void** param_ptr_ptr, int param_type);
YSOS_EXPORT bool NewParamInContent(void** param_ptr_ptr, int param_type, Json::Value& root);
YSOS_EXPORT void DeleteParamInContent(void** param_ptr_ptr, int param_type);
/*
    "from":"127.0.0.1:6001",//根据实际情况设置//NOLINT
    "to":"127.0.0.1:6002",//根据实际情况设置//NOLINT
    "verb":"RegDoServiceEvent",//根据实际情况设置//NOLINT
*/
struct YSOS_EXPORT AgentMessageContent {
  std::string from_ip_port;   //根据实际情况设置//NOLINT
  std::string to_ip_port;     //根据实际情况设置//NOLINT
  std::string verb;           //根据实际情况设置//NOLINT
  void* content_param_ptr;    //根据实际情况设置//NOLINT

  int type;                   //根据实际情况设置, 与AgentMessageHead的一致//NOLINT
  AgentMessageContent();
  ~AgentMessageContent();
  bool SetType(int type);
  std::string ToString();
  bool FromString(const std::string& json_string);
};

struct YSOS_EXPORT AgentMessage {
  AgentMessageHead head_;
  AgentMessageContent content_;
  std::string ToString();
  bool FromString(const std::string& json_string);
};

typedef boost::shared_ptr<AgentMessage> AgentMessagePtr;

struct YSOS_EXPORT ServiceInfo {
  typedef std::multimap<std::string, std::string>  ServiceInfoMap;
  std::string service_name;
  std::string service_param;
  std::string event_name;
  std::string event_param;
  ServiceInfoMap event_map;
};
typedef  boost::shared_ptr<ServiceInfo>  ServiceInfoPtr;

YSOS_EXPORT ServiceInfoPtr ParseAgentServiceInfo(const std::string &service_xml);

//  解析各类AppDispatch返回的event param功能函数
typedef std::list<std::pair<std::string, std::string>> OnDispatchMessageParamList;
typedef std::list<std::pair<std::string, std::string>>::iterator OnDispatchMessageParamListIterator;
YSOS_EXPORT int ParseOnDispatchMessageParam(const std::string& event_param, OnDispatchMessageParamList& app_dispatch_event_param_list);

}   // namespace ysos

#endif  // CHP_AGENT_MESSAGE_DEF_H  //NOLINT
