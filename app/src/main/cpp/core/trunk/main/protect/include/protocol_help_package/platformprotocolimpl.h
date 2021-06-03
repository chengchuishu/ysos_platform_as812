/*
# platformprotocolimpl.h
# Definition of PlatformProtocolImpl
# Created on: 2016-09-12 15:22
# Original author: jinchengzhe
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
# version 0.1, 20160912, created by JinChengZhe
*/
#ifndef PHP_PLATFORM_PROTOCOL_IMPL_H
#define PHP_PLATFORM_PROTOCOL_IMPL_H

/// C++ Standard Headers
#include <vector>
#include <algorithm>
/// ThirdParty Headers
#include <boost/shared_ptr.hpp>
//#include <jsoncpp-src-0.5.0/include/json/json.h>
#include <json/json.h>
/// Platform Headers
#include "../../../public/include/sys_interface_package/common.h"
#include "../../../public/include/core_help_package/baseinterfaceimpl.h"
#include "../../../public/include/sys_interface_package/protocolinterface.h"

namespace ysos {

extern const char* g_platform_protocolimpl_content_tag_request_verb;
extern const char* g_platform_protocolimpl_content_tag_response_verb;

/**
*@brief 重定义Json类参数  // NOLINT
*/
typedef Json::Value JsonValue;
typedef Json::FastWriter JsonFastWriter;
typedef Json::StyledWriter JsonStyledWriter;
typedef Json::Reader JsonReader;
typedef Json::Features JsonFeatures;
typedef Json::ValueIterator JsonValueIterator;
#define JsonNullValue Json::nullValue;

#define JSON_FEATURES_STRICT_MODE (Json::Features::strictMode());

/**
*@brief PlatformProtocolImpl的具体实现  // NOLINT
*/
class PlatformProtocolImpl;
typedef boost::shared_ptr<PlatformProtocolImpl> PlatformProtocolImplPtr;
class YSOS_EXPORT PlatformProtocolImpl : public ProtocolInterface, public BaseInterfaceImpl {
  DISALLOW_COPY_AND_ASSIGN(PlatformProtocolImpl)  //  禁止拷贝和复制
  DECLARE_PROTECT_CONSTRUCTOR(PlatformProtocolImpl)  // 构造函数保护
  DECLARE_CREATEINSTANCE(PlatformProtocolImpl)  // 定义类的全局静态创建函数

 public:
  /**
  *@brief 定义用于解析的数据结构  // NOLINT
  */
  typedef std::pair<std::string, std::string> PlatformProtocolParamPair;
  typedef std::vector<PlatformProtocolParamPair> PlatformProtocolParamPairVector;
  typedef PlatformProtocolParamPairVector::iterator PlatformProtocolParamPairVectorIterator;

  typedef struct JsonArrayUnit {
    unsigned section;
    PlatformProtocolParamPairVector vector;
  } *JsonArrayUnitPtr;

  typedef std::vector<JsonArrayUnit> PlatformProtocolJsonArrayUnitVector;
  typedef PlatformProtocolJsonArrayUnitVector::iterator PlatformProtocolJsonArrayUnitVectorIterator;

  typedef struct MessageIDnServiceNameInfo {
    unsigned int message_id;
    const char* message_id_string;
    const char* service_name;
  }*MessageIDnServiceNameInfoPtr;

  /**
  *@brief 定义用于解析和封装的结构体  // NOLINT
  */
  typedef struct MessageHeader {
    std::string tag;
    std::string version;
    std::string type;
    std::string time_stamp;
    std::string session_id;
    std::string serial_number;

    MessageHeader() :
      tag(""),
      version(""),
      type(""),
      time_stamp(""),
      session_id(""),
      serial_number("") {
    }

    ~MessageHeader() {
      tag.clear();
      version.clear();
      type.clear();
      time_stamp.clear();
      session_id.clear();
      serial_number.clear();
    }
  } *MessageHeaderPtr;

  struct RequestMessageParam {
    std::string service_name;
    std::string service_param;

    RequestMessageParam() :
      service_name(""),
      service_param("") {
    }

    ~RequestMessageParam() {
      service_name.clear();
      service_param.clear();
    }
  } *RequestMessageParamPtr;

  struct ResponseMessageParamResult {
    std::string status_code;
    std::string description;
    std::string detail;

    ResponseMessageParamResult() :
      status_code(""),
      description(""),
      detail("") {
    }

    ~ResponseMessageParamResult() {
      status_code.clear();
      description.clear();
      detail.clear();
    }
  }*ResponseMessageParamResultPtr;

  struct ResponseMessageParam {
    std::string service_name;
    ResponseMessageParamResult result;
    std::string result_string;

    ResponseMessageParam() :
      service_name("") {
    }

    ~ResponseMessageParam() {
      service_name.clear();
    }
  } *ResponseMessageParamPtr;

  typedef struct RequestMessageBody {
    std::string from;
    std::string to;
    std::string verb;
    RequestMessageParam param;

    RequestMessageBody() :
      from(""),
      to(""),
      verb("") {
    }

    ~RequestMessageBody() {
      from.clear();
      to.clear();
      verb.clear();
    }
  }*RequestMessageBodyPtr;

  typedef struct ResponseMessageBody {
    std::string from;
    std::string to;
    std::string verb;
    ResponseMessageParam param;

    ResponseMessageBody() :
      from(""),
      to(""),
      verb("") {
    }

    ~ResponseMessageBody() {
      from.clear();
      to.clear();
      verb.clear();
    }
  } *ResponseMessageBodyPtr;

  typedef struct NormalMessageBody {
    std::string from;
    std::string to;
    std::string verb;
    std::string service_name;
    std::string others;

    NormalMessageBody() :
      from(""),
      to(""),
      verb(""),
      service_name(""),
      others("") {
    }

    ~NormalMessageBody() {
      from.clear();
      to.clear();
      verb.clear();
      service_name.clear();
      others.clear();
    }
  } *NormalMessageBodyPtr;

  typedef struct RequestMessagePack {
    MessageHeader message_header;
    RequestMessageBody message_body;
  } *RequestMessagePackPtr;

  typedef struct ResponseMessagePack {
    MessageHeader message_header;
    ResponseMessageBody message_body;
  } *ResponseMessagePackPtr;

  typedef struct NormalMessagePack {
    MessageHeader message_header;
    NormalMessageBody message_body;
  } *NormalMessagePackPtr;

  /**
  *@brief  析构函数  // NOLINT
  *@param  无  // NOLINT
  *@return:  无  // NOLINT
  */
  virtual ~PlatformProtocolImpl();

  /**
  *@brief 解析数据  // NOLINT
  *@param in_buffer[IN]:  需解析数据的缓冲，内部包含实际读取数据长度  // NOLINT
  *@param out_buffer[IN]:  NULL  // NOLINT
  *@param format_id[IN]:  数据属性，只可以设置REQUEST或者RESPONSE  // NOLINT
  *@param context_ptr[IN/OUT]:  ResponseMessagePack/RequestMessagePack结构体指针，解析后保存到相应的结构体中  // NOLINT
  *@return:  返回 0 成功，返回 -1  // NOLINT
  */
  int ParseMessage(BufferInterfacePtr in_buffer, BufferInterfacePtr out_buffer, ProtocolFormatId format_id, void *context_ptr);

  /**
  *@brief 封装数据  // NOLINT
  *@param in_buffer[IN]:  NULL  // NOLINT
  *@param out_buffer[IN]:  需封装数据的缓冲，内部包含实际读取数据长度  // NOLINT
  *@param format_id[IN]:  数据属性，只可以设置REQUEST或者RESPONSE  // NOLINT
  *@param context_ptr[IN/OUT]:  ResponseMessagePack/RequestMessagePack结构体指针，需封装的数据保存到相应的结构体中  // NOLINT
  *@return:  返回 0 成功，返回 -1  // NOLINT
  */
  int FormMessage(BufferInterfacePtr in_buffer, BufferInterfacePtr out_buffer, ProtocolFormatId format_id, void *context_ptr);

  /**
  *@brief 使用字符串生成Json结构体  // NOLINT
  *@param json_string[IN]:  Json字符串  // NOLINT
  *@param json_object[OUT]:  Json结构体  // NOLINT
  *@return:  返回 0 成功，返回 -1  // NOLINT
  */
  static int JsonObjectFromString(const std::string& json_string, JsonValue& json_object);

  /**
  *@brief 使用字符串对生成Json结构体  // NOLINT
  *@param key_value[IN]:  字符串对  // NOLINT
  *@param json_object[OUT]:  Json结构体  // NOLINT
  *@return:  返回 0 成功，返回 -1  // NOLINT
  */
  static void JsonObjectFromStringPair(const PlatformProtocolParamPair& key_value, JsonValue& json_object);

  /**
  *@brief 使用Json结构体生成Json数组  // NOLINT
  *@param json_object[IN]:  Json结构体  // NOLINT
  *@param json_array[OUT]:  Json数组  // NOLINT
  *@return:  返回 0 成功，返回 -1  // NOLINT
  */
  static void JsonArrayFromJsonObject(const JsonValue& json_object, JsonValue& json_array);

  /**
  *@brief Json数据结构转换为字符串  // NOLINT
  *@param json_value[IN]:  Json数据结构  // NOLINT
  *@param json_string[OUT]:  Json字符串  // NOLINT
  *@return:  返回 0 成功，返回 -1  // NOLINT
  */
  static int JsonValueToString(const JsonValue& json_value, std::string& json_string);

  /**
  *@brief 解析Json数组，保存为自定义Vector  // NOLINT
  *@param json_array[IN]:  Json数组  // NOLINT
  *@param platform_protocol_json_Array_unit_vector[OUT]:  自定义Vector，解析后数据将保存在相应的Vector里  // NOLINT
  *@return:  返回 0 成功，返回 -1  // NOLINT
  */
  static int JsonArrayToVector(const JsonValue& json_array, PlatformProtocolJsonArrayUnitVector& platform_protocol_json_Array_unit_vector);

  /**
  *@brief 打印请求数据结构体信息  // NOLINT
  *@param message_pack[IN]:  请求数据结构体  // NOLINT
  *@return:  无  // NOLINT
  */
  static void PrintRequestMessagePack(const RequestMessagePack& message_pack);

  /**
  *@brief 打印响应数据结构体信息  // NOLINT
  *@param message_pack[IN]:  响应数据结构体  // NOLINT
  *@return:  无  // NOLINT
  */
  static void PrintResponseMessagePack(const ResponseMessagePack& message_pack);

  /**
  *@brief 打印任意Json数据类型结构  // NOLINT
  *@param json_value[IN]:  Json数据类型结构体  // NOLINT
  *@return:  无  // NOLINT
  */
  static void PrintJsonValue(const JsonValue& json_value);

  /**
  *@brief 获取message_id的字符串和对应服务名  // NOLINT
  *@param message_id[IN]:  message id  // NOLINT
  *@param message_id_string[OUT]:  message id的字符串  // NOLINT
  *@param service_name[OUT]:  message id对应的服务名  // NOLINT
  *@return:  无  // NOLINT
  */
  static void GetMessageInfoByMessageID(const unsigned int message_id, std::string& message_id_string, std::string& service_name);

  /**
  *@brief 获取message_id和message_id的字符串  // NOLINT
  *@param service_name[IN]:  服务名  // NOLINT
  *@param message_id[OUT]:  message id  // NOLINT
  *@param message_id_string[OUT]:  message id的字符串  // NOLINT
  *@return:  无  // NOLINT
  */
  static void GetMessageInfoByServiceName(const std::string& service_name, ProtocolFormatId format_id, unsigned int& message_id, std::string& message_id_string);

 private:
  /**
  *@brief  空实现函数  // NOLINT
  */
  int GetLength(BufferInterfacePtr in_buffer, ProtocolFormatId format_id) {
    return YSOS_ERROR_SUCCESS;
  }
  int GetProperty(int type_id, void *type) {
    return YSOS_ERROR_SUCCESS;
  }
  int SetProperty(int type_id, void *type) {
    return YSOS_ERROR_SUCCESS;
  }

  /**
  *@brief 封装请求数据  // NOLINT
  *@param in_buffer[IN]:  NULL  // NOLINT
  *@param out_buffer[IN]:  需封装数据的缓冲，内部包含实际读取数据长度  // NOLINT
  *@param context_ptr[IN/OUT]:  ResponseMessagePack/RequestMessagePack结构体指针，需封装的数据保存到相应的结构体中  // NOLINT
  *@return:  返回 0 成功，返回 -1  // NOLINT
  */
  int FormRequestMessage(BufferInterfacePtr in_buffer, BufferInterfacePtr out_buffer, void *context_ptr);

  /**
  *@brief 封装响应数据  // NOLINT
  *@param in_buffer[IN]:  NULL  // NOLINT
  *@param out_buffer[IN]:  需封装数据的缓冲，内部包含实际读取数据长度  // NOLINT
  *@param context_ptr[IN/OUT]:  ResponseMessagePack/RequestMessagePack结构体指针，需封装的数据保存到相应的结构体中  // NOLINT
  *@return:  返回 0 成功，返回 -1  // NOLINT
  */
  int FormResponseMessage(BufferInterfacePtr in_buffer, BufferInterfacePtr out_buffer, void *context_ptr);

  /**
  *@brief 解析标准数据  // NOLINT
  *@param in_buffer[IN]:  需解析数据的缓冲，内部包含实际读取数据长度  // NOLINT
  *@param out_buffer[IN]:  NULL  // NOLINT
  *@param context_ptr[IN/OUT]:  NormalMessagePack结构体指针，解析后保存到相应的结构体中  // NOLINT
  *@return:  返回 0 成功，返回 -1  // NOLINT
  */
  int ParseNormalMessage(BufferInterfacePtr in_buffer, BufferInterfacePtr out_buffer, void *context_ptr);

  /**
  *@brief 解析请求数据  // NOLINT
  *@param in_buffer[IN]:  需解析数据的缓冲，内部包含实际读取数据长度  // NOLINT
  *@param out_buffer[IN]:  NULL  // NOLINT
  *@param context_ptr[IN/OUT]:  RequestMessagePack结构体指针，解析后保存到相应的结构体中  // NOLINT
  *@return:  返回 0 成功，返回 -1  // NOLINT
  */
  int ParseRequestMessage(BufferInterfacePtr in_buffer, BufferInterfacePtr out_buffer, void *context_ptr);

  /**
  *@brief 解析响应数据  // NOLINT
  *@param in_buffer[IN]:  需解析数据的缓冲，内部包含实际读取数据长度  // NOLINT
  *@param out_buffer[IN]:  NULL  // NOLINT
  *@param context_ptr[IN/OUT]:  ResponseMessagePack结构体指针，解析后保存到相应的结构体中  // NOLINT
  *@return:  返回 0 成功，返回 -1  // NOLINT
  */
  int ParseResponseMessage(BufferInterfacePtr in_buffer, BufferInterfacePtr out_buffer, void *context_ptr);

  static UINT64 serial_number_;  ///< 用于设置流水号
};

}

#endif
