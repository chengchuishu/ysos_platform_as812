/**
  *@file JsonUtility.h
  *@brief JsonUtility
  *@version 1.0
  *@author Donghongqian
  *@date Created on: 2016-05-31 18:03:27
  *@copyright Copyright(c) 2016 YunShen Technology. All rights reserved.
  * 
  */
#ifndef CFP_JSON_UTILITY_H_
#define CFP_JSON_UTILITY_H_

/// Ysos Headers //  NOLINT
#include "../../../public/include/core_help_package/utility.h"
//#include <jsoncpp-src-0.5.0/include/json/json.h>
#include <json/json.h>

namespace ysos {

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
  *@brief  Xml的工具类，使用方式：  //  NOLINT
  *        GetXmlUtil()->GetElementTextValue() ... //  NOLINT
  */
class YSOS_EXPORT JsonUtil: public BaseInterfaceImpl {
  DECLARE_CREATEINSTANCE(JsonUtil);
  DISALLOW_COPY_AND_ASSIGN(JsonUtil);
  DECLARE_PROTECT_CONSTRUCTOR(JsonUtil);

//  friend class Singleton<JsonUtil>;

 public:
  ~JsonUtil();

  /**
  *@brief 使用字符串生成Json结构体  // NOLINT
  *@param json_string[IN]:  Json字符串  // NOLINT
  *@param json_object[OUT]:  Json结构体  // NOLINT
  *@return:  返回 0 成功，返回 -1  // NOLINT
  */
  int JsonObjectFromString(const std::string& json_string, JsonValue& json_object);
  /**
  *@brief 使用Json结构体生成Json数组  // NOLINT
  *@param json_object[IN]:  Json结构体  // NOLINT
  *@param json_array[OUT]:  Json数组  // NOLINT
  *@return:  返回 0 成功，返回 -1  // NOLINT
  */
  void JsonArrayFromJsonObject(const JsonValue& json_object, JsonValue& json_array);
  /**
  *@brief 从JsonObject中直接获取key对应的text值  // NOLINT
  *@param key[IN]:  关键字  // NOLINT
  *@param json_object[OUT]:  Json结构体  // NOLINT
  *@return:  返回值非空，成功，返回空，失败  // NOLINT
  */
  std::string GetTextElement(const std::string &key, JsonValue &json_object);

  /**
  *@brief Json数据结构转换为字符串  // NOLINT
  *@param json_value[IN]:  Json数据结构  // NOLINT
  *@param json_string[OUT]:  Json字符串  // NOLINT
  *@return:  返回 0 成功，返回 -1  // NOLINT
  */
  int JsonValueToString(const JsonValue& json_value, std::string& json_string);
  /**
  *@brief 打印任意Json数据类型结构  // NOLINT
  *@param json_value[IN]:  Json数据类型结构体  // NOLINT
  *@return:  无  // NOLINT
  */
  void PrintJsonValue(const JsonValue& json_value);

  DECLARE_SINGLETON_VARIABLE(JsonUtil);
};
}
#define GetJsonUtil  ysos::JsonUtil::Instance
#endif  ///<  CFP_JSON_UTILITY_H_ //  NOLINT