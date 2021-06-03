/**
  *@file JsonUtility.h
  *@brief JsonUtility
  *@version 1.0
  *@author Donghongqian
  *@date Created on: 2016-05-31 18:03:27
  *@copyright Copyright(c) 2016 YunShen Technology. All rights reserved.
  * 
  */

/// Private Header //  NOLINT
#include "../../../public/include/core_help_package/jsonutility.h"


namespace ysos {

DEFINE_SINGLETON(JsonUtil);
JsonUtil::JsonUtil(const std::string &strClassName /* =JsonUtil */): BaseInterfaceImpl(strClassName) {

}

JsonUtil::~JsonUtil() {

}

int JsonUtil::JsonObjectFromString(const std::string& json_string, JsonValue& json_object) {
  int result = YSOS_ERROR_FAILED;

  do {
    if (json_string.empty()) {
      break;
    }

    JsonFeatures json_features = JSON_FEATURES_STRICT_MODE;
    JsonReader json_reader(json_features);

    if (!json_reader.parse(json_string, json_object, true)) {
      break;
    }

    if (json_object.empty()) {
      break;
    }

    result = YSOS_ERROR_SUCCESS;

  } while (0);

  return result;
}

void JsonUtil::JsonArrayFromJsonObject(const JsonValue& json_object, JsonValue& json_array) {
  json_array.append(json_object);
  return;
}

std::string JsonUtil::GetTextElement(const std::string &key, JsonValue &json_object) {
  if(key.empty() || !json_object.isMember(key)) {
    YSOS_LOG_ERROR("get tex element failed: " << key);  // need update for linux
    return "";
  }

  JsonValue value = json_object[key];
  if(value.empty() || !value.isString()) {
    YSOS_LOG_ERROR("json_object is not string: " << key);  // need update for linux
    //return false;
    return "";  // need update for linux
  }

  return value.asString();
}

int JsonUtil::JsonValueToString(const JsonValue& json_value, std::string& json_string) {
  int result = YSOS_ERROR_FAILED;

  do {
    JsonFastWriter json_writer;
    json_string = json_writer.write(json_value);
    json_string = GetUtility()->ReplaceAllDistinct ( json_string, "\\r\\n", "" );

    if (json_string.empty()) {
      break;
    }

    result  = YSOS_ERROR_SUCCESS;
  } while (0);

  return result;
}

void JsonUtil::PrintJsonValue(const JsonValue& json_value) {

  if (0 < json_value.size()) {
    JsonValue json_value_object;
    for (
      JsonValueIterator iterator = json_value.begin();
      iterator != json_value.end();
    ++iterator
      ) {

        json_value_object = json_value[iterator.key().asString().c_str()];

        if (true == json_value_object.isObject()) {

          std::cout << std::endl << iterator.key().asString().c_str() << ": " << "{" << std::endl;

          PrintJsonValue(json_value_object);

          std::cout << "}" << std::endl << std::endl;

        } else if (true == json_value_object.isArray()) {

          std::cout << std::endl << iterator.key().asString().c_str() << ": " << "[" << std::endl;

          JsonValue json_value_array_unit;
          for (unsigned ii = 0; ii < json_value_object.size(); ++ii) {
            std::cout << std::endl << "{" << std::endl;

            json_value_array_unit = json_value_object[ii];
            PrintJsonValue(json_value_array_unit);

            std::cout <<"}" << std::endl << std::endl;
          }

          std::cout <<"]" << std::endl << std::endl;

        } else if (true == json_value_object.isString()) {
          std::cout << iterator.key().asString().c_str() << ": " << json_value_object.asString().c_str() << std::endl;
        } else if (true == json_value_object.isBool()) {
          std::cout << iterator.key().asString().c_str() << ": " << json_value_object.asBool() << std::endl;
        } else if (true == json_value_object.isInt()) {
          std::cout << iterator.key().asString().c_str() << ": " << json_value_object.asInt() << std::endl;
        } else if (true == json_value_object.isUInt()) {
          std::cout << iterator.key().asString().c_str() << ": " << json_value_object.asUInt() << std::endl;
        } else if (true == json_value_object.isDouble()) {
          std::cout << iterator.key().asString().c_str() << ": " << json_value_object.asDouble() << std::endl;
        } else {
          std::cout << "unknown type" << std::endl;
        }
    }
  }

  return;
}
}
