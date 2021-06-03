/*
# JsonProtocolImpl.cpp
# Implemention of JsonProtocolImpl
# Created on: 2016-04-21 13:59:20
# Original author: XuLanyue
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
# version 0.1, 20160421, created by XuLanyue
*/
#if 1   //  disable warning 4250 and 4996
#include "../../../public/include/os_hal_package/config.h"  // NOLINT
#if (CURRENT_PLATFORM == PLATFORM_WINDOWS)
#pragma warning(disable: 4250)      //  dominance
#pragma warning(disable: 4996)      //  std::_Fill_n
#endif
#endif

#include "../../../protect/include/protocol_help_package/jsonprotocolimpl.h"
#include "../../../public/include/sys_interface_package/messageinterface.h"
#include "../../../public/include/sys_interface_package/messagehead.h"
#include <cstdio>
#include <string>
#include <sstream>
#include <algorithm>

namespace ysos {
JsonProtocolImpl::JsonProtocolImpl(const std::string& object_name):
BaseInterfaceImpl(object_name) {
  logger_ = GetUtility()->GetLogger("ysos.framework");
  YSOS_LOG_DEBUG("JsonProtocolImpl()");
}

JsonProtocolImpl::~JsonProtocolImpl() {
  YSOS_LOG_DEBUG("~JsonProtocolImpl()");
}

#if defined(_MSC_VER)  &&  _MSC_VER >= 1310
#pragma warning(disable: 4996)      // disable fopen deprecation warning
#endif
static int GetJsonStrFromFile(std::string& strJsonFromFile, const char* path) {
  int return_value = YSOS_ERROR_FAILED;
  FILE* file = fopen(path, "rb");
  if (!file) {
    strJsonFromFile = "";
  } else {

    // GetFileSize
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    std::string text;
    char* buffer = new char[size + 1];
    buffer[size] = 0;
    if (fread(buffer, 1, size, file) == (unsigned long)size) {
      strJsonFromFile = buffer;
      return_value = YSOS_ERROR_SUCCESS;
    }
    fclose(file);
    delete[] buffer;
  }
  return return_value;
}

static void GetNodeValueWithIndexList(
  const std::string &input,
  std::string& strNodeValue,
  Json::Value &value,
  unsigned uLevel,
  std::string& strIndexList,
  bool& bGot,
  const std::string path = ".") {
  unsigned int uLevel_ = 0;
  unsigned int uIndex_ = 0;
  unsigned int uOffset = 0;
  unsigned int uLength = 0;
  std::string strIndexList_ = "00000000";
  value.GetLevel_Index_IndexList(uLevel_, uIndex_, strIndexList_);
  switch (value.type()) {
  case Json::nullValue:
    if (strIndexList_ == strIndexList) {
      value.GetOffset_Length(uOffset, uLength);
      strNodeValue = input.substr(uOffset, uLength);
      bGot = true;
    }
    break;
  case Json::intValue:
    if (strIndexList_ == strIndexList) {
      value.GetOffset_Length(uOffset, uLength);
      strNodeValue = input.substr(uOffset, uLength);
      bGot = true;
    }
    break;
  case Json::uintValue:
    if (strIndexList_ == strIndexList) {
      value.GetOffset_Length(uOffset, uLength);
      strNodeValue = input.substr(uOffset, uLength);
      bGot = true;
    }
    break;
  case Json::realValue:
    if (strIndexList_ == strIndexList) {
      value.GetOffset_Length(uOffset, uLength);
      strNodeValue = input.substr(uOffset, uLength);
      bGot = true;
    }
    break;
  case Json::stringValue:
    if (strIndexList_ == strIndexList) {
      value.GetOffset_Length(uOffset, uLength);
      strNodeValue = input.substr(uOffset, uLength);
      bGot = true;
    }
    break;
  case Json::booleanValue:
    if (strIndexList_ == strIndexList) {
      value.GetOffset_Length(uOffset, uLength);
      strNodeValue = input.substr(uOffset, uLength);
      bGot = true;
    }
    break;
  case Json::arrayValue: {
    if (strIndexList_ == strIndexList) {
      value.GetOffset_Length(uOffset, uLength);
      strNodeValue = input.substr(uOffset, uLength);
      bGot = true;
    } else {
      int size = value.size();
      for (int index =0; index < size; ++index) {
        static char buffer[16];
        sprintf(buffer, "[%d]", index);
        GetNodeValueWithIndexList(
          input,
          strNodeValue,
          value[index],
          (uLevel + 1),
          strIndexList,
          bGot,
          (path + buffer));
        if (bGot) {
          break;
        }
      }
    }
  }
  break;
  case Json::objectValue: {
    if (strIndexList_ == strIndexList) {
      value.GetOffset_Length(uOffset, uLength);
      strNodeValue = input.substr(uOffset, uLength);
      bGot = true;
    } else {
      Json::Value::Members members(value.getMemberNames());
      std::sort(members.begin(), members.end());
      std::string suffix = *(path.end()-1) == '.' ? "" : ".";
      for (Json::Value::Members::iterator it = members.begin();
           it != members.end();
           ++it) {
        const std::string &name = *it;
        GetNodeValueWithIndexList(
          input,
          strNodeValue,
          value[name],
          (uLevel + 1),
          strIndexList,
          bGot,
          (path + suffix + name));
        if (bGot) {
          break;
        }
      }
    }
  }
  break;
  default:
    break;
  }
}

int JsonProtocolImpl::TravelValueTreeForNode(
  const std::string &input,
  Json::Value &root,
  const Json::Features &features,
  uint64_t uFormatId,
  std::string& strMember) {
  Json::Reader reader(features);
  bool parsingSuccessful = reader.parse(input, root, 1);
  if (!parsingSuccessful) {
    YSOS_LOG_ERROR("Failed to parse: " << reader.getFormatedErrorMessages());
    return 1;
  }
  HierarchyFormatIdStruct oHFIS(uFormatId);
  std::string strIndexList = "";
  if (uFormatId > 0) {
    char szBufTemp[64];
    memset(szBufTemp, 0, sizeof(szBufTemp));
    if (oHFIS.level_4 != 0) {
      sprintf(szBufTemp, "%d-%d-%d-%d-",
              oHFIS.level_1, oHFIS.level_2, oHFIS.level_3, oHFIS.level_4);
    } else if (oHFIS.level_3 != 0) {
      sprintf(szBufTemp, "%d-%d-%d-",
              oHFIS.level_1, oHFIS.level_2, oHFIS.level_3);
    } else if (oHFIS.level_2 != 0) {
      sprintf(szBufTemp, "%d-%d-",
              oHFIS.level_1, oHFIS.level_2);
    } else if (oHFIS.level_1 != 0) {
      sprintf(szBufTemp, "%d-",
              oHFIS.level_1);
    }
    strIndexList = szBufTemp;
  }
  bool bGot = false;
  GetNodeValueWithIndexList(input, strMember, root, 1, strIndexList, bGot);
  if (bGot) {
    YSOS_LOG_DEBUG(strMember);
  }

  return 0;
}

static void GetEachNodeValue(
  const std::string &input,
  std::string& strNodeValue,
  Json::Value &value,
  unsigned uLevel,
  bool& bGot,
  const std::string path = ".") {
  unsigned int uLevel_ = 0;
  unsigned int uIndex_ = 0;
  unsigned int uOffset = 0;
  unsigned int uLength = 0;
  std::string strIndexList_ = "00000000";
  switch (value.type()) {
  case Json::nullValue:
    value.GetOffset_Length(uOffset, uLength);
    strNodeValue = input.substr(uOffset, uLength);
    break;
  case Json::intValue:
    value.GetOffset_Length(uOffset, uLength);
    strNodeValue = input.substr(uOffset, uLength);
    break;
  case Json::uintValue:
    value.GetOffset_Length(uOffset, uLength);
    strNodeValue = input.substr(uOffset, uLength);
    break;
  case Json::realValue:
    value.GetOffset_Length(uOffset, uLength);
    strNodeValue = input.substr(uOffset, uLength);
    break;
  case Json::stringValue:
    value.GetOffset_Length(uOffset, uLength);
    strNodeValue = input.substr(uOffset, uLength);
    break;
  case Json::booleanValue:
    value.GetOffset_Length(uOffset, uLength);
    strNodeValue = input.substr(uOffset, uLength);
    break;
  case Json::arrayValue: {
    if (1) {
      value.GetOffset_Length(uOffset, uLength);
      strNodeValue = input.substr(uOffset, uLength);
    } else {
      int size = value.size();
      for (int index =0; index < size; ++index) {
        static char buffer[16];
        sprintf(buffer, "[%d]", index);
        GetEachNodeValue(
          input,
          strNodeValue,
          value[index],
          (uLevel + 1),
          bGot,
          (path + buffer));
      }
    }
  }
  break;
  case Json::objectValue: {
    if (1) {
      value.GetOffset_Length(uOffset, uLength);
      strNodeValue = input.substr(uOffset, uLength);
    } else {
      Json::Value::Members members(value.getMemberNames());
      std::sort(members.begin(), members.end());
      std::string suffix = *(path.end()-1) == '.' ? "" : ".";
      for (Json::Value::Members::iterator it = members.begin();
           it != members.end();
           ++it) {
        const std::string &name = *it;
        GetEachNodeValue(
          input,
          strNodeValue,
          value[name],
          (uLevel + 1),
          bGot,
          (path + suffix + name));
      }
    }
  }
  break;
  default:
  break;
  }
}

int JsonProtocolImpl::TravelWholeValueTree(
  const std::string &input,
  Json::Value &root,
  const Json::Features &features,
  uint64_t uFormatId,
  std::string& strMember) {
  Json::Reader reader(features);
  bool parsingSuccessful = reader.parse(input, root, 1);
  if (!parsingSuccessful) {
    YSOS_LOG_ERROR("Failed to parse: " << reader.getFormatedErrorMessages());
    return 1;
  }
  bool bGot = false;
  std::string strIndexList = "";
  GetNodeValueWithIndexList(input, strMember, root, 1, strIndexList, bGot);
  if (bGot) {
    YSOS_LOG_DEBUG(strMember);
  }

  return 0;
}

int JsonProtocolImpl::GetJsonMemberValue(
  bool bFromFile,
  std::string& strPathOrJson,
  ProtocolFormatId uFormatId,
  std::string& strMember) {
  int return_value = YSOS_ERROR_SUCCESS;
  std::string strJsonFromFile;
  Json::Features features = Json::Features::strictMode();
  if (bFromFile) {
    if (YSOS_ERROR_SUCCESS != GetJsonStrFromFile(
      strJsonFromFile, strPathOrJson.c_str())) {
      return_value = YSOS_ERROR_INVALID_ARGUMENTS;
    }
  }
  if (return_value == YSOS_ERROR_INVALID_ARGUMENTS ||
      (bFromFile && strJsonFromFile.empty()) ||
      (!bFromFile && strPathOrJson.empty())) {
    return_value = YSOS_ERROR_INVALID_ARGUMENTS;
  } else {
    Json::Value root;
    return_value = TravelValueTreeForNode(
      (bFromFile ? strJsonFromFile : strPathOrJson),
      root,
      features,
      uFormatId,
      strMember);
  }

  return return_value;
}

int JsonProtocolImpl::ParseMessage(
  BufferInterfacePtr in_buffer,
  BufferInterfacePtr out_buffer,
  ProtocolFormatId format_id,
  void* context) {
  int return_value = YSOS_ERROR_SUCCESS;
  int from_file = 0;
  bool is_from_file = false;
  JsonContext* json_context_ptr = NULL;
  do {
    //  parameter check
    if (!in_buffer || !out_buffer) {
      return_value = YSOS_ERROR_INVALID_ARGUMENTS;
      break;
    }

    if (NULL != context) {
      json_context_ptr = (JsonContext*)context;
    }
    if (NULL != json_context_ptr) {
      from_file = json_context_ptr->input_from;
    }
    is_from_file = (from_file != 0);

    //  get source json
    UINT32 in_buffer_length = 0;
    UINT32 out_buffer_length = 0;
    UINT8* in_data = NULL;
    UINT8* out_data = NULL;
    in_buffer->GetBufferAndLength(&in_data, &in_buffer_length);
    out_buffer->GetBufferAndLength(&out_data, &out_buffer_length);
#if 0
    if (NULL == in_data ||
        0 == in_buffer_length ||
        NULL == out_data ||
        0 == out_buffer_length) {
      return_value = YSOS_ERROR_INVALID_ARGUMENTS;
      break;
    }
#else
    uint32_t total_head_length = sizeof(MessageHead) + sizeof(MessageData);
    if (NULL == in_data ||
        total_head_length >= in_buffer_length ||
        NULL == out_data ||
        total_head_length > out_buffer_length) {
      return_value = YSOS_ERROR_INVALID_ARGUMENTS;
      break;
    }
#endif
    //  get json member value by format id
#if 0
    std::string path_or_json_string = (char*)&in_data[0];
#else
    std::string path_or_json_string =
      (char*)&in_data[total_head_length];
#endif
    std::string member_value_string = "";
    if (YSOS_ERROR_SUCCESS != GetJsonMemberValue(
          is_from_file,
          path_or_json_string,
          format_id,
          member_value_string)) {
      return_value = YSOS_ERROR_INVALID_ARGUMENTS;
      break;
    }

    //  save json member value to out buffer
#if 0
    uint32_t required_out_buffer_length = strlen(member_value_string.c_str()) + 1;
#else
    uint32_t required_out_buffer_length =
      total_head_length + strlen(member_value_string.c_str()) + 1;
#endif
    if (out_buffer_length < required_out_buffer_length) {
      //  the out buffer is too small
      return_value = YSOS_ERROR_INVALID_ARGUMENTS;
      break;
    }
#if 0
    memcpy(
      &out_data[0],
      member_value_string.c_str(),
      required_out_buffer_length - 1);
#else
    //  Make MessageHead
    MessageHead message_head_dst;
    memcpy(
      &message_head_dst,
      &in_data[0],
      sizeof(MessageHead));
    message_head_dst.message_length = required_out_buffer_length;

    //  Make MessageData
    MessageData message_data_dst;
    memcpy(
      &message_data_dst,
      &in_data[0] + sizeof(MessageHead),
      sizeof(MessageData));
    message_data_dst.message_data_ = out_buffer;

    //  Copy to dst buffer
    memcpy(
      &out_data[0],
      &message_head_dst,
      sizeof(MessageHead));
    memcpy(
      &out_data[sizeof(MessageHead)],
      &message_data_dst,
      sizeof(MessageData));
    memcpy(
      &out_data[total_head_length],
      member_value_string.c_str(),
      required_out_buffer_length - 1);
#endif
    out_data[required_out_buffer_length - 1] = '\0';
  } while (false);

  return return_value;
}

int JsonProtocolImpl::FormMessage(
  BufferInterfacePtr in_buffer,
  BufferInterfacePtr out_buffer,
  ProtocolFormatId format_id,
  void* context) {
  int return_value = YSOS_ERROR_SUCCESS;
  int from_file = 0;
  bool is_from_file = false;
  JsonContext* json_context_ptr = NULL;

  if (NULL != context) {
    json_context_ptr = (JsonContext*)context;
  }
  if (NULL != json_context_ptr) {
    from_file = json_context_ptr->input_from;
  }

  return YSOS_ERROR_SUCCESS;
}

int JsonProtocolImpl::GetLength(
  BufferInterfacePtr in_buffer,
  ProtocolFormatId format_id) {
  int return_value = YSOS_ERROR_SUCCESS;
  int from_file = 0;
  bool is_from_file = false;
  do {
    //  parameter check
    if (!in_buffer) {
      return_value = YSOS_ERROR_INVALID_ARGUMENTS;
      break;
    }

    is_from_file = (from_file != 0);

    //  get source json
    UINT32 in_buffer_length = 0;
    UINT8* in_data = NULL;
    in_buffer->GetBufferAndLength(&in_data, &in_buffer_length);
    if (NULL != in_data ||
        0 == in_buffer_length) {
      return_value = YSOS_ERROR_INVALID_ARGUMENTS;
      break;
    }

    //  get json member value by format id
    std::string path_or_json_string = (char*)&in_data[0];
    std::string member_value_string = "";
    if (YSOS_ERROR_SUCCESS != GetJsonMemberValue(
          is_from_file,
          path_or_json_string,
          format_id,
          member_value_string)) {
      return_value = YSOS_ERROR_INVALID_ARGUMENTS;
      break;
    }

    //  save json member value to out buffer
    uint32_t required_out_buffer_length = strlen(member_value_string.c_str()) + 1;
    return_value = static_cast<int>(required_out_buffer_length);
  } while (false);

  return return_value;
}

//ProtocolInterfacePtr CreateInstanceOfJsonProtocolInterface(std::string strClassName) {
//  return ProtocolInterfacePtr(
//    JsonProtocolImpl::CreateInstance()
//    //(new(JsonProtocolImpl(strClassName)))
//    );
//}

} // namespace ysos
