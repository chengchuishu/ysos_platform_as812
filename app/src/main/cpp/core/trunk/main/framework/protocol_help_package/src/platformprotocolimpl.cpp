/*
# platformprotocolimpl.cpp
# Definition of PlatformProtocolImpl
# Created on: 2016-09-12 15:22
# Original author: jinchengzhe
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
# version 0.1, 20160912, created by JinChengZhe
*/

/// Private Headers
#include "../../../protect/include/protocol_help_package/platformprotocolimpl.h"
/// ThirdParty Headers
#include <boost/date_time.hpp>
/// Platform Headers
#include "../../../public/include/core_help_package/utility.h"
#include "../../../public/include/os_hal_package/bufferutility.h"
#include "../../../public/include/sys_interface_package/messagetype.h"
#include "../../../public/include/sys_interface_package/ysos_enum.h"

namespace ysos {

const char* g_platform_protocolimpl_header_tag_header = "header";
const char* g_platform_protocolimpl_header_tag_tag = "tag";
const char* g_platform_protocolimpl_header_tag_version = "version";
const char* g_platform_protocolimpl_header_tag_type = "type";
const char* g_platform_protocolimpl_header_tag_time_stamp = "time_stamp";
const char* g_platform_protocolimpl_header_tag_session_id = "session_id";
const char* g_platform_protocolimpl_header_tag_serial_number = "serial_number";

const char* g_platform_protocolimpl_content_tag_content = "content";
const char* g_platform_protocolimpl_content_tag_from = "from";
const char* g_platform_protocolimpl_content_tag_to = "to";
const char* g_platform_protocolimpl_content_tag_verb = "verb";
const char* g_platform_protocolimpl_content_tag_param = "param";

#if _CERTUSNET
const char* g_platform_protocolimpl_content_param_tag_service_name = "action";
const char* g_platform_protocolimpl_request_content_param_tag_service_param = "param";
const char* g_platform_protocolimpl_response_content_param_result_tag_status_code = "status";
const char* g_platform_protocolimpl_response_content_param_tag_result = "param";
#else
const char* g_platform_protocolimpl_content_param_tag_service_name = "service_name";
const char* g_platform_protocolimpl_request_content_param_tag_service_param = "service_param";
const char* g_platform_protocolimpl_response_content_param_result_tag_status_code = "status_code";
const char* g_platform_protocolimpl_response_content_param_tag_result = "result";
#endif
const char* g_platform_protocolimpl_response_content_param_result_tag_description = "description";
const char* g_platform_protocolimpl_response_content_param_result_tag_detail = "detail";

const char* g_platform_protocolimpl_header_tag_tag_value = "ysos";
const char* g_platform_protocolimpl_header_tag_version_value = "1.0.0.1";
const char* g_platform_protocolimpl_content_tag_request_verb = "DoService";
const char* g_platform_protocolimpl_content_tag_response_verb = "OnDispatchMessage";

const PlatformProtocolImpl::MessageIDnServiceNameInfo g_message_id_service_name_info_req_direction[] = {
  {YSOS_AGENT_MESSAGE_HEART_BEAT_REQ, "31000", YSOS_AGENT_SERVICE_NAME_HEART_BEAT},
  {YSOS_AGENT_MESSAGE_GET_SERVICE_LIST_REQ, "31002", YSOS_AGENT_SERVICE_NAME_SERVICE_LIST},
  {YSOS_AGENT_MESSAGE_WRITE_LOG_REQ, "31004", YSOS_AGENT_SERVICE_NAME_WRITE_LOG},
  {YSOS_AGENT_MESSAGE_REG_EVENT_REQ, "31006", YSOS_AGENT_SERVICE_NAME_REG_EVENT},
  {YSOS_AGENT_MESSAGE_UNREG_EVENT_REQ, "31008", YSOS_AGENT_SERVICE_NAME_UNREG_EVENT},
  {YSOS_AGENT_MESSAGE_IOCTL_REQ, "31010", YSOS_AGENT_SERVICE_NAME_IOCTL},
  {YSOS_AGENT_MESSAGE_LOAD_CONFIG_REQ, "31012", YSOS_AGENT_SERVICE_NAME_LOAD_CONFIG},
  {YSOS_AGENT_MESSAGE_GET_SERVICE_STATE_REQ, "31014", YSOS_AGENT_SERVICE_NAME_GET_SERVICE_STATE},
  {YSOS_AGENT_MESSAGE_SWITCH_REQ, "31016", YSOS_AGENT_SERVICE_NAME_SWITCH},
  {YSOS_AGENT_MESSAGE_INIT_REQ, "31018", YSOS_AGENT_SERVICE_NAME_INIT},
  {YSOS_AGENT_MESSAGE_SWITCH_NOTIFY_RSQ, "31024", YSOS_AGENT_SERVICE_NAME_SWITCH_NOTIFY},
  {YSOS_AGENT_MESSAGE_READY_RSQ, "31026", YSOS_AGENT_SERVICE_NAME_READY},
  {YSOS_AGENT_MESSAGE_EVENT_NOTIFY_RSQ, "31028", YSOS_AGENT_SERVICE_NAME_EVENT_NOTIFY},
  {YSOS_AGENT_MESSAGE_GET_DATA_RSQ, "31030", YSOS_AGENT_SERVICE_NAME_GET_DATA},
  {YSOS_AGENT_MESSAGE_SET_DATA_RSQ, "31032", YSOS_AGENT_SERVICE_NAME_SET_DATA},
  {YSOS_AGENT_MESSAGE_CUSTOM_EVENT_RSQ, "31034", YSOS_AGENT_SERVICE_NAME_CUSTOM_EVENT},
  {YSOS_AGENT_MESSAGE_UNINIT_RSQ, "31036", YSOS_AGENT_SERVICE_NAME_UNINIT},
#if _CERTUSNET
  {YSOS_AGENT_MESSAGE_INIT_CONNECTION_REQ, "31038", YSOS_AGENT_SERVICE_NAME_INIT_CONNECTION},
  {YSOS_AGENT_MESSAGE_UNINIT_CONNECTION_REQ, "31040", YSOS_AGENT_SERVICE_NAME_UNINIT_CONNECTION},
#endif
};

const PlatformProtocolImpl::MessageIDnServiceNameInfo g_message_id_service_name_info_rsp_direction[] = {
  {YSOS_AGENT_MESSAGE_HEART_BEAT_RSP, "31001", YSOS_AGENT_SERVICE_NAME_HEART_BEAT},
  {YSOS_AGENT_MESSAGE_GET_SERVICE_LIST_RSP, "31003", YSOS_AGENT_SERVICE_NAME_SERVICE_LIST},
  {YSOS_AGENT_MESSAGE_WRITE_LOG_RSP, "31005", YSOS_AGENT_SERVICE_NAME_WRITE_LOG},
  {YSOS_AGENT_MESSAGE_REG_EVENT_RSP, "31007", YSOS_AGENT_SERVICE_NAME_REG_EVENT},
  {YSOS_AGENT_MESSAGE_UNREG_EVENT_RSP, "31009", YSOS_AGENT_SERVICE_NAME_UNREG_EVENT},
  {YSOS_AGENT_MESSAGE_IOCTL_RSP, "31011", YSOS_AGENT_SERVICE_NAME_IOCTL},
  {YSOS_AGENT_MESSAGE_LOAD_CONFIG_RSP, "31013", YSOS_AGENT_SERVICE_NAME_LOAD_CONFIG},
  {YSOS_AGENT_MESSAGE_GET_SERVICE_STATE_RSP, "31015", YSOS_AGENT_SERVICE_NAME_GET_SERVICE_STATE},
  {YSOS_AGENT_MESSAGE_SWITCH_RSP, "31017", YSOS_AGENT_SERVICE_NAME_SWITCH},
  {YSOS_AGENT_MESSAGE_INIT_RSP, "31019", YSOS_AGENT_SERVICE_NAME_INIT},
  {YSOS_AGENT_MESSAGE_SWITCH_NOTIFY_RSP, "31025", YSOS_AGENT_SERVICE_NAME_SWITCH_NOTIFY},
  {YSOS_AGENT_MESSAGE_READY_RSP, "31027", YSOS_AGENT_SERVICE_NAME_READY},
  {YSOS_AGENT_MESSAGE_EVENT_NOTIFY_RSP, "31029", YSOS_AGENT_SERVICE_NAME_EVENT_NOTIFY},
  {YSOS_AGENT_MESSAGE_GET_DATA_RSP, "31031", YSOS_AGENT_SERVICE_NAME_GET_DATA},
  {YSOS_AGENT_MESSAGE_SET_DATA_RSP, "31033", YSOS_AGENT_SERVICE_NAME_SET_DATA},
  {YSOS_AGENT_MESSAGE_CUSTOM_EVENT_RSP, "31035", YSOS_AGENT_SERVICE_NAME_CUSTOM_EVENT},
  {YSOS_AGENT_MESSAGE_UNINIT_RSP, "31037", YSOS_AGENT_SERVICE_NAME_UNINIT},
#if _CERTUSNET
  {YSOS_AGENT_MESSAGE_INIT_CONNECTION_RSP, "31039", YSOS_AGENT_SERVICE_NAME_INIT_CONNECTION},
  {YSOS_AGENT_MESSAGE_UNINIT_CONNECTION_RSP, "31041", YSOS_AGENT_SERVICE_NAME_UNINIT_CONNECTION},
#endif
};


UINT64 PlatformProtocolImpl::serial_number_ = 0;
PlatformProtocolImpl::PlatformProtocolImpl(const std::string &strClassName /* =AgentProtocolImpl */) : BaseInterfaceImpl(strClassName) {
  logger_ = GetUtility()->GetLogger("ysos.sdk");
}

PlatformProtocolImpl::~PlatformProtocolImpl() {

}

int PlatformProtocolImpl::ParseMessage(BufferInterfacePtr in_buffer, BufferInterfacePtr out_buffer, ProtocolFormatId format_id, void *context_ptr) {

  int result = YSOS_ERROR_FAILED;

  do {
    if (NULL == context_ptr || NULL == in_buffer) {
      break;
    }

    if (PROP_NORMAL == format_id) {
      if (YSOS_ERROR_SUCCESS != ParseRequestMessage(in_buffer, out_buffer, context_ptr)) {
        break;
      }
    } else if (PROP_REQUEST == format_id) {
      if (YSOS_ERROR_SUCCESS != ParseRequestMessage(in_buffer, out_buffer, context_ptr)) {
        break;
      }
    } else if (PROP_RESPONSE == format_id) {
      if (YSOS_ERROR_SUCCESS != ParseResponseMessage(in_buffer, out_buffer, context_ptr)) {
        break;
      }
    } else {
      break;
    }

    result = YSOS_ERROR_SUCCESS;

  } while (0);

  return result;
}

int PlatformProtocolImpl::FormMessage(BufferInterfacePtr in_buffer, BufferInterfacePtr out_buffer, ProtocolFormatId format_id, void *context_ptr) {

  int result = YSOS_ERROR_FAILED;

  do {
    if (NULL == context_ptr || NULL == out_buffer) {
      break;
    }

    if (PROP_REQUEST == format_id) {
      if (YSOS_ERROR_SUCCESS != FormRequestMessage(in_buffer, out_buffer, context_ptr)) {
        break;
      }
    } else if (PROP_RESPONSE == format_id) {
      if (YSOS_ERROR_SUCCESS != FormResponseMessage(in_buffer, out_buffer, context_ptr)) {
        break;
      }
    } else {
      break;
    }

    result = YSOS_ERROR_SUCCESS;

  } while (0);

  return result;
}

int PlatformProtocolImpl::FormRequestMessage(BufferInterfacePtr in_buffer, BufferInterfacePtr out_buffer, void *context_ptr) {

  int result = YSOS_ERROR_FAILED;

  do {

    if (NULL == context_ptr || NULL == out_buffer) {
      break;
    }

    RequestMessagePackPtr message_pack_ptr = static_cast<RequestMessagePackPtr>(context_ptr);
    if (NULL == message_pack_ptr) {
      break;
    }

    /// Message Header Info
    JsonValue json_value_header;

    /// tag
    if (true == message_pack_ptr->message_header.tag.empty()) {
      json_value_header[g_platform_protocolimpl_header_tag_tag] = g_platform_protocolimpl_header_tag_tag_value;
    } else {
      json_value_header[g_platform_protocolimpl_header_tag_tag] = message_pack_ptr->message_header.tag.c_str();
    }

    /// version
    if (true == message_pack_ptr->message_header.time_stamp.empty()) {
      json_value_header[g_platform_protocolimpl_header_tag_version] = g_platform_protocolimpl_header_tag_version_value;
    } else {
      json_value_header[g_platform_protocolimpl_header_tag_version] = message_pack_ptr->message_header.version.c_str();
    }

    /// type
    if (true == message_pack_ptr->message_header.type.empty()) {
      json_value_header[g_platform_protocolimpl_header_tag_type] = JsonNullValue;
    } else {
      json_value_header[g_platform_protocolimpl_header_tag_type] = message_pack_ptr->message_header.type.c_str();
    }

    /// time_stamp
    if (true == message_pack_ptr->message_header.time_stamp.empty()) {
      boost::posix_time::ptime ptime_locoal(boost::posix_time::microsec_clock::local_time());
      std::string local_time = boost::posix_time::to_iso_extended_string(ptime_locoal);
      int position = local_time.find('T');
      local_time.replace(position, 1, " ");
      position = local_time.find('.');
      local_time.replace(position, 1, " ");

      json_value_header[g_platform_protocolimpl_header_tag_time_stamp] = local_time.c_str();
    } else {
      json_value_header[g_platform_protocolimpl_header_tag_time_stamp] = message_pack_ptr->message_header.time_stamp.c_str();
    }

    /// session_id
    if (true == message_pack_ptr->message_header.session_id.empty()) {
      json_value_header[g_platform_protocolimpl_header_tag_session_id] = JsonNullValue;
    } else {
      json_value_header[g_platform_protocolimpl_header_tag_session_id] = message_pack_ptr->message_header.session_id.c_str();
    }

    /// serial_number
    if (true == message_pack_ptr->message_header.serial_number.empty()) {
      char serial_number[11] = {0,};
      #ifdef _WIN32
        sprintf_s(serial_number, 11, "%010d", serial_number_);  //need update for linux
      #else
        sprintf(serial_number, "%010d", (int)serial_number_);   //need update for linux
      #endif
      json_value_header[g_platform_protocolimpl_header_tag_serial_number] = serial_number;
      ++serial_number_;
    } else {
      json_value_header[g_platform_protocolimpl_header_tag_serial_number] = message_pack_ptr->message_header.serial_number.c_str();
    }

    /// Message Body Info
    JsonValue json_value_body;

    /// from
    if (true == message_pack_ptr->message_body.from.empty()) {
      json_value_body[g_platform_protocolimpl_content_tag_from] = JsonNullValue;
    } else {
      json_value_body[g_platform_protocolimpl_content_tag_from] = message_pack_ptr->message_body.from.c_str();
    }

    /// to
    if (true == message_pack_ptr->message_body.to.empty()) {
      json_value_body[g_platform_protocolimpl_content_tag_to] = JsonNullValue;
    } else {
      json_value_body[g_platform_protocolimpl_content_tag_to] = message_pack_ptr->message_body.to.c_str();
    }

    /// verb
    if (true == message_pack_ptr->message_body.verb.empty()) {
      json_value_body[g_platform_protocolimpl_content_tag_verb] = g_platform_protocolimpl_content_tag_request_verb;
    } else {
      json_value_body[g_platform_protocolimpl_content_tag_verb] = message_pack_ptr->message_body.verb.c_str();
    }

    /// param
    JsonValue json_value_param;

    /// service_name
    if (true == message_pack_ptr->message_body.param.service_name.empty()) {
      json_value_param[g_platform_protocolimpl_content_param_tag_service_name] = JsonNullValue;
    } else {
      json_value_param[g_platform_protocolimpl_content_param_tag_service_name] = message_pack_ptr->message_body.param.service_name.c_str();
    }

    /// service_param
    if (true == message_pack_ptr->message_body.param.service_param.empty()) {
      json_value_param[g_platform_protocolimpl_request_content_param_tag_service_param] = JsonNullValue;
    } else {
      JsonValue json_value_service_param;

      if (JsonObjectFromString(message_pack_ptr->message_body.param.service_param, json_value_service_param)) {
        break;
      }

      json_value_param[g_platform_protocolimpl_request_content_param_tag_service_param] = json_value_service_param;
    }

    json_value_body[g_platform_protocolimpl_content_tag_param] = json_value_param;

    /// Message Pack
    JsonValue json_value;
    json_value[g_platform_protocolimpl_header_tag_header] = json_value_header;
    json_value[g_platform_protocolimpl_content_tag_content] = json_value_body;

    /// Convert String
    Json::FastWriter json_fast_writer;
    std::string json_string = json_fast_writer.write(json_value);
    json_string = GetUtility()->ReplaceAllDistinct ( json_string, "\\r\\n", "" );

    if (true == json_string.empty()) {
      break;
    }

    /// Copy Memory
    if (YSOS_ERROR_SUCCESS != GetBufferUtility()->InitialBuffer(out_buffer)) {
      break;
    }

    UINT32 out_buffer_length = GetBufferUtility()->GetBufferMaxLength(out_buffer);
    if (0 == out_buffer_length) {
      break;
    }

    if (out_buffer_length < json_string.length()) {
      YSOS_LOG_DEBUG("PlatformProtocolImpl::FormRequestMessage[Fail][out_buffer_length is less than json_string.length().]");
      YSOS_LOG_DEBUG("PlatformProtocolImpl::FormRequestMessage[Fail][out_buffer_length][" << out_buffer_length << "]");
      YSOS_LOG_DEBUG("PlatformProtocolImpl::FormRequestMessage[Fail][json_string.length()][" << json_string.length() << "]");
      break;
    }

    if (YSOS_ERROR_SUCCESS != GetBufferUtility()->CopyStringToBuffer(json_string, out_buffer)) {
      break;
    }

    result = YSOS_ERROR_SUCCESS;

  } while (0);

  return result;
}

int PlatformProtocolImpl::FormResponseMessage(BufferInterfacePtr in_buffer, BufferInterfacePtr out_buffer, void *context_ptr) {
  int result = YSOS_ERROR_FAILED;

  do {

    if (NULL == context_ptr || NULL == out_buffer) {
      break;
    }

    ResponseMessagePackPtr message_pack_ptr = static_cast<ResponseMessagePackPtr>(context_ptr);
    if (NULL == message_pack_ptr) {
      break;
    }

    /// Message Header Info
    JsonValue json_value_header;

    /// tag
    if (true == message_pack_ptr->message_header.tag.empty()) {
      json_value_header[g_platform_protocolimpl_header_tag_tag] = g_platform_protocolimpl_header_tag_tag_value;
    } else {
      json_value_header[g_platform_protocolimpl_header_tag_tag] = message_pack_ptr->message_header.tag.c_str();
    }

    /// version
    if (true == message_pack_ptr->message_header.time_stamp.empty()) {
      json_value_header[g_platform_protocolimpl_header_tag_version] = g_platform_protocolimpl_header_tag_version_value;
    } else {
      json_value_header[g_platform_protocolimpl_header_tag_version] = message_pack_ptr->message_header.version.c_str();
    }

    /// type
    if (true == message_pack_ptr->message_header.type.empty()) {
      json_value_header[g_platform_protocolimpl_header_tag_type] = JsonNullValue;
    } else {
      json_value_header[g_platform_protocolimpl_header_tag_type] = message_pack_ptr->message_header.type.c_str();
    }

    /// time_stamp
    if (true == message_pack_ptr->message_header.time_stamp.empty()) {
      boost::posix_time::ptime ptime_locoal(boost::posix_time::microsec_clock::local_time());
      std::string local_time = boost::posix_time::to_iso_extended_string(ptime_locoal);
      int position = local_time.find('T');
      local_time.replace(position, 1, " ");
      position = local_time.find('.');
      local_time.replace(position, 1, " ");

      json_value_header[g_platform_protocolimpl_header_tag_time_stamp] = local_time.c_str();
    } else {
      json_value_header[g_platform_protocolimpl_header_tag_time_stamp] = message_pack_ptr->message_header.time_stamp.c_str();
    }

    /// session_id
    if (true == message_pack_ptr->message_header.session_id.empty()) {
      json_value_header[g_platform_protocolimpl_header_tag_session_id] = JsonNullValue;
    } else {
      json_value_header[g_platform_protocolimpl_header_tag_session_id] = message_pack_ptr->message_header.session_id.c_str();
    }

    /// serial_number
    if (true == message_pack_ptr->message_header.serial_number.empty()) {
      char serial_number[11] = {0,};
      #ifdef _WIN32
        sprintf_s(serial_number, 11, "%010d", serial_number_);    //need update for linux
      #else
        sprintf(serial_number, "%010d", (int)serial_number_);          //need update for linux
      #endif
      json_value_header[g_platform_protocolimpl_header_tag_serial_number] = serial_number;
      ++serial_number_;
    } else {
      json_value_header[g_platform_protocolimpl_header_tag_serial_number] = message_pack_ptr->message_header.serial_number.c_str();
    }

    /// Message Body Info
    JsonValue json_value_body;

    /// from
    if (true == message_pack_ptr->message_body.from.empty()) {
      json_value_body[g_platform_protocolimpl_content_tag_from] = JsonNullValue;
    } else {
      json_value_body[g_platform_protocolimpl_content_tag_from] = message_pack_ptr->message_body.from.c_str();
    }

    /// to
    if (true == message_pack_ptr->message_body.to.empty()) {
      json_value_body[g_platform_protocolimpl_content_tag_to] = JsonNullValue;
    } else {
      json_value_body[g_platform_protocolimpl_content_tag_to] = message_pack_ptr->message_body.to.c_str();
    }

    /// verb
    if (true == message_pack_ptr->message_body.verb.empty()) {
      json_value_body[g_platform_protocolimpl_content_tag_verb] = g_platform_protocolimpl_content_tag_response_verb;
    } else {
      json_value_body[g_platform_protocolimpl_content_tag_verb] = message_pack_ptr->message_body.verb.c_str();
    }

    /// param
    JsonValue json_value_param;

    /// service_name
    if (true == message_pack_ptr->message_body.param.service_name.empty()) {
      json_value_param[g_platform_protocolimpl_content_param_tag_service_name] = JsonNullValue;
    } else {
      json_value_param[g_platform_protocolimpl_content_param_tag_service_name] = message_pack_ptr->message_body.param.service_name.c_str();
    }

    /// result
    JsonValue json_value_result;

    if (true == message_pack_ptr->message_body.param.result_string.empty()) {
      /// status_code
      if (true == message_pack_ptr->message_body.param.result.status_code.empty()) {
        json_value_result[g_platform_protocolimpl_response_content_param_result_tag_status_code] = JsonNullValue;
      } else {
        json_value_result[g_platform_protocolimpl_response_content_param_result_tag_status_code] = message_pack_ptr->message_body.param.result.status_code.c_str();
      }

      /// description
      if (true == message_pack_ptr->message_body.param.result.description.empty()) {
        json_value_result[g_platform_protocolimpl_response_content_param_result_tag_description] = JsonNullValue;
      } else {
        json_value_result[g_platform_protocolimpl_response_content_param_result_tag_description] = message_pack_ptr->message_body.param.result.description.c_str();
      }

      /// detail
      if (true == message_pack_ptr->message_body.param.result.detail.empty()) {
        json_value_result[g_platform_protocolimpl_response_content_param_result_tag_detail] = JsonNullValue;
      } else {
        JsonValue json_value_detail;

        if (JsonObjectFromString(message_pack_ptr->message_body.param.result.detail, json_value_detail)) {
          break;
        }

        json_value_result[g_platform_protocolimpl_response_content_param_result_tag_detail] = json_value_detail;
      }
    } else {
      if (YSOS_ERROR_SUCCESS != JsonObjectFromString(message_pack_ptr->message_body.param.result_string, json_value_result)) {
        break;
      }
    }

    json_value_param[g_platform_protocolimpl_response_content_param_tag_result] = json_value_result;

    json_value_body[g_platform_protocolimpl_content_tag_param] = json_value_param;

    /// Message Pack
    JsonValue json_value;
    json_value[g_platform_protocolimpl_header_tag_header] = json_value_header;
    json_value[g_platform_protocolimpl_content_tag_content] = json_value_body;

    /// Convert String
    Json::FastWriter json_fast_writer;
    std::string json_string = json_fast_writer.write(json_value);
    json_string = GetUtility()->ReplaceAllDistinct ( json_string, "\\r\\n", "" );

    if (true == json_string.empty()) {
      break;
    }

    /// Copy Memory
    if (YSOS_ERROR_SUCCESS != GetBufferUtility()->InitialBuffer(out_buffer)) {
      break;
    }

    UINT32 out_buffer_length = GetBufferUtility()->GetBufferMaxLength(out_buffer);
    if (0 == out_buffer_length) {
      break;
    }

    if (out_buffer_length < json_string.length()) {
      YSOS_LOG_DEBUG("PlatformProtocolImpl::FormResponseMessage[Fail][out_buffer_length is less than json_string.length().]");
      YSOS_LOG_DEBUG("PlatformProtocolImpl::FormResponseMessage[Fail][out_buffer_length][" << out_buffer_length << "]");
      YSOS_LOG_DEBUG("PlatformProtocolImpl::FormResponseMessage[Fail][json_string.length()][" << json_string.length() << "]");
      break;
    }

    if (YSOS_ERROR_SUCCESS != GetBufferUtility()->CopyStringToBuffer(json_string, out_buffer)) {
      break;
    }

    result = YSOS_ERROR_SUCCESS;

  } while (0);

  return result;
}

int PlatformProtocolImpl::ParseNormalMessage(BufferInterfacePtr in_buffer, BufferInterfacePtr out_buffer, void *context_ptr) {
  int result = YSOS_ERROR_FAILED;

  do {
    if (NULL == in_buffer || NULL == context_ptr) {
      break;
    }

    UINT8* in_buffer_data_ptr = GetBufferUtility()->GetBufferData(in_buffer);
    if (NULL == in_buffer_data_ptr) {
      break;
    }

    std::string json_string = (char*)in_buffer_data_ptr;

    JsonFeatures json_features = JSON_FEATURES_STRICT_MODE;
    JsonReader json_reader(json_features);
    JsonValue json_value;

    if (true != json_reader.parse(json_string, json_value, true)) {
      break;
    }

    if (true == json_value.empty()) {
      break;
    }

    NormalMessagePackPtr message_pack_ptr = static_cast<NormalMessagePackPtr>(context_ptr);
    if (NULL == message_pack_ptr) {
      break;
    }

    /// Header
    if (true == json_value.isMember(g_platform_protocolimpl_header_tag_header)) {
      JsonValue json_value_header = json_value[g_platform_protocolimpl_header_tag_header];
      if (true != json_value_header.empty() && true == json_value_header.isObject()) {
        /// tag
        if (
          true == json_value_header.isMember(g_platform_protocolimpl_header_tag_tag) &&
          true == json_value_header[g_platform_protocolimpl_header_tag_tag].isString()
        ) {
          message_pack_ptr->message_header.tag = json_value_header[g_platform_protocolimpl_header_tag_tag].asString();
        }

        /// version
        if (
          true == json_value_header.isMember(g_platform_protocolimpl_header_tag_version) &&
          true == json_value_header[g_platform_protocolimpl_header_tag_version].isString()
        ) {
          message_pack_ptr->message_header.version = json_value_header[g_platform_protocolimpl_header_tag_version].asString();
        }

        /// type
        if (
          true == json_value_header.isMember(g_platform_protocolimpl_header_tag_type) &&
          true == json_value_header[g_platform_protocolimpl_header_tag_type].isString()
        ) {
          message_pack_ptr->message_header.type = json_value_header[g_platform_protocolimpl_header_tag_type].asString();
        }

        /// time_stamp
        if (
          true == json_value_header.isMember(g_platform_protocolimpl_header_tag_time_stamp) &&
          true == json_value_header[g_platform_protocolimpl_header_tag_time_stamp].isString()
        ) {
          message_pack_ptr->message_header.time_stamp = json_value_header[g_platform_protocolimpl_header_tag_time_stamp].asString();
        }

        /// session_id
        if (
          true == json_value_header.isMember(g_platform_protocolimpl_header_tag_session_id) &&
          true == json_value_header[g_platform_protocolimpl_header_tag_session_id].isString()
        ) {
          message_pack_ptr->message_header.session_id = json_value_header[g_platform_protocolimpl_header_tag_session_id].asString();
        }

        /// serial_number
        if (
          true == json_value_header.isMember(g_platform_protocolimpl_header_tag_serial_number) &&
          true == json_value_header[g_platform_protocolimpl_header_tag_serial_number].isString()
        ) {
          message_pack_ptr->message_header.serial_number = json_value_header[g_platform_protocolimpl_header_tag_serial_number].asString();
        }
      }
    }

    /// Content
    if (true == json_value.isMember(g_platform_protocolimpl_content_tag_content)) {
      JsonValue json_value_content = json_value[g_platform_protocolimpl_content_tag_content];
      if (true != json_value_content.empty() && true == json_value_content.isObject()) {

        /// from
        if (
          true == json_value_content.isMember(g_platform_protocolimpl_content_tag_from) &&
          true == json_value_content[g_platform_protocolimpl_content_tag_from].isString()
        ) {
          message_pack_ptr->message_body.from = json_value_content[g_platform_protocolimpl_content_tag_from].asString();
        }

        /// to
        if (
          true == json_value_content.isMember(g_platform_protocolimpl_content_tag_to) &&
          true == json_value_content[g_platform_protocolimpl_content_tag_to].isString()
        ) {
          message_pack_ptr->message_body.to = json_value_content[g_platform_protocolimpl_content_tag_to].asString();
        }

        /// verb
        if (
          true == json_value_content.isMember(g_platform_protocolimpl_content_tag_verb) &&
          true == json_value_content[g_platform_protocolimpl_content_tag_verb].isString()
        ) {
          message_pack_ptr->message_body.verb = json_value_content[g_platform_protocolimpl_content_tag_verb].asString();
        }

        /// param
        JsonValue json_value_param = json_value_content[g_platform_protocolimpl_content_tag_param];
        if (true != json_value_param.empty() && true == json_value_param.isObject()) {

          /// service_name
          if (
            true == json_value_param.isMember(g_platform_protocolimpl_content_param_tag_service_name) &&
            true == json_value_param[g_platform_protocolimpl_content_param_tag_service_name].isString()
          ) {
            message_pack_ptr->message_body.service_name = json_value_param[g_platform_protocolimpl_content_param_tag_service_name].asString();
          }

          /// others
          JsonValue json_value_others;
          if (true == json_value_param.isMember(g_platform_protocolimpl_request_content_param_tag_service_param)) {
            json_value_others = json_value_param[g_platform_protocolimpl_request_content_param_tag_service_param];
          } else if (true == json_value_param.isMember(g_platform_protocolimpl_response_content_param_tag_result)) {
            json_value_others = json_value_param[g_platform_protocolimpl_response_content_param_tag_result];
          }

          if (true != json_value_others.empty() && true == json_value_others.isObject()) {
            std::string json_string;
            if (YSOS_ERROR_SUCCESS != JsonValueToString(json_value_others, json_string)) {
              break;
            }

            message_pack_ptr->message_body.others = json_string;
          }
        }
      }
    }

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  return result;
}

int PlatformProtocolImpl::ParseRequestMessage(BufferInterfacePtr in_buffer, BufferInterfacePtr out_buffer, void *context_ptr) {
  int result = YSOS_ERROR_FAILED;

  do {
    if (NULL == in_buffer || NULL == context_ptr) {
      break;
    }

    UINT8* in_buffer_data_ptr = GetBufferUtility()->GetBufferData(in_buffer);
    if (NULL == in_buffer_data_ptr) {
      break;
    }

    std::string json_string = (char*)in_buffer_data_ptr;

    JsonFeatures json_features = JSON_FEATURES_STRICT_MODE;
    JsonReader json_reader(json_features);
    JsonValue json_value;

    if (true != json_reader.parse(json_string, json_value, true)) {
      break;
    }

    if (true == json_value.empty()) {
      break;
    }

    RequestMessagePackPtr message_pack_ptr = static_cast<RequestMessagePackPtr>(context_ptr);
    if (NULL == message_pack_ptr) {
      break;
    }

    /// Header
    if (true == json_value.isMember(g_platform_protocolimpl_header_tag_header)) {
      JsonValue json_value_header = json_value[g_platform_protocolimpl_header_tag_header];
      if (true != json_value_header.empty() && true == json_value_header.isObject()) {
        /// tag
        if (
          true == json_value_header.isMember(g_platform_protocolimpl_header_tag_tag) &&
          true == json_value_header[g_platform_protocolimpl_header_tag_tag].isString()
        ) {
          message_pack_ptr->message_header.tag = json_value_header[g_platform_protocolimpl_header_tag_tag].asString();
        }

        /// version
        if (
          true == json_value_header.isMember(g_platform_protocolimpl_header_tag_version) &&
          true == json_value_header[g_platform_protocolimpl_header_tag_version].isString()
        ) {
          message_pack_ptr->message_header.version = json_value_header[g_platform_protocolimpl_header_tag_version].asString();
        }

        /// type
        if (
          true == json_value_header.isMember(g_platform_protocolimpl_header_tag_type) &&
          true == json_value_header[g_platform_protocolimpl_header_tag_type].isString()
        ) {
          message_pack_ptr->message_header.type = json_value_header[g_platform_protocolimpl_header_tag_type].asString();
        }

        /// time_stamp
        if (
          true == json_value_header.isMember(g_platform_protocolimpl_header_tag_time_stamp) &&
          true == json_value_header[g_platform_protocolimpl_header_tag_time_stamp].isString()
        ) {
          message_pack_ptr->message_header.time_stamp = json_value_header[g_platform_protocolimpl_header_tag_time_stamp].asString();
        }

        /// session_id
        if (
          true == json_value_header.isMember(g_platform_protocolimpl_header_tag_session_id) &&
          true == json_value_header[g_platform_protocolimpl_header_tag_session_id].isString()
        ) {
          message_pack_ptr->message_header.session_id = json_value_header[g_platform_protocolimpl_header_tag_session_id].asString();
        }

        /// serial_number
        if (
          true == json_value_header.isMember(g_platform_protocolimpl_header_tag_serial_number) &&
          true == json_value_header[g_platform_protocolimpl_header_tag_serial_number].isString()
        ) {
          message_pack_ptr->message_header.serial_number = json_value_header[g_platform_protocolimpl_header_tag_serial_number].asString();
        }
      }
    }

    /// Content
    if (true == json_value.isMember(g_platform_protocolimpl_content_tag_content)) {
      JsonValue json_value_content = json_value[g_platform_protocolimpl_content_tag_content];
      if (true != json_value_content.empty() && true == json_value_content.isObject()) {

        /// from
        if (
          true == json_value_content.isMember(g_platform_protocolimpl_content_tag_from) &&
          true == json_value_content[g_platform_protocolimpl_content_tag_from].isString()
        ) {
          message_pack_ptr->message_body.from = json_value_content[g_platform_protocolimpl_content_tag_from].asString();
        }

        /// to
        if (
          true == json_value_content.isMember(g_platform_protocolimpl_content_tag_to) &&
          true == json_value_content[g_platform_protocolimpl_content_tag_to].isString()
        ) {
          message_pack_ptr->message_body.to = json_value_content[g_platform_protocolimpl_content_tag_to].asString();
        }

        /// verb
        if (
          true == json_value_content.isMember(g_platform_protocolimpl_content_tag_verb) &&
          true == json_value_content[g_platform_protocolimpl_content_tag_verb].isString()
        ) {
          message_pack_ptr->message_body.verb = json_value_content[g_platform_protocolimpl_content_tag_verb].asString();
        }

        /// param
        JsonValue json_value_param = json_value_content[g_platform_protocolimpl_content_tag_param];
        if (true != json_value_param.empty() && true == json_value_param.isObject()) {

          /// service_name
          if (
            true == json_value_param.isMember(g_platform_protocolimpl_content_param_tag_service_name) &&
            true == json_value_param[g_platform_protocolimpl_content_param_tag_service_name].isString()
          ) {
            message_pack_ptr->message_body.param.service_name = json_value_param[g_platform_protocolimpl_content_param_tag_service_name].asString();
          }

          /// service_param
          JsonValue json_value_service_param = json_value_param[g_platform_protocolimpl_request_content_param_tag_service_param];
          if (true != json_value_service_param.empty() && true == json_value_service_param.isObject()) {
            std::string json_string;
            if (YSOS_ERROR_SUCCESS != JsonValueToString(json_value_service_param, json_string)) {
              break;
            }

            message_pack_ptr->message_body.param.service_param = json_string;
          }
        }
      }
    }

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  return result;
}

int PlatformProtocolImpl::ParseResponseMessage(BufferInterfacePtr in_buffer, BufferInterfacePtr out_buffer, void *context_ptr) {
  int result = YSOS_ERROR_FAILED;

  do {
    if (NULL == in_buffer || NULL == context_ptr) {
      break;
    }


    UINT8* in_buffer_data_ptr = GetBufferUtility()->GetBufferData(in_buffer);
    if (NULL == in_buffer_data_ptr) {
      break;
    }

    std::string json_string = (char*)in_buffer_data_ptr;

    JsonFeatures json_features = JSON_FEATURES_STRICT_MODE;
    JsonReader json_reader(json_features);
    JsonValue json_value;

    if (true != json_reader.parse(json_string, json_value, true)) {
      break;
    }

    if (true == json_value.empty()) {
      break;
    }

    ResponseMessagePackPtr message_pack_ptr = static_cast<ResponseMessagePackPtr>(context_ptr);
    if (NULL == message_pack_ptr) {
      break;
    }

    /// Header
    if (true == json_value.isMember(g_platform_protocolimpl_header_tag_header)) {
      JsonValue json_value_header = json_value[g_platform_protocolimpl_header_tag_header];
      if (true != json_value_header.empty() && true == json_value_header.isObject()) {
        /// tag
        if (
          true == json_value_header.isMember(g_platform_protocolimpl_header_tag_tag) &&
          true == json_value_header[g_platform_protocolimpl_header_tag_tag].isString()
        ) {
          message_pack_ptr->message_header.tag = json_value_header[g_platform_protocolimpl_header_tag_tag].asString();
        }

        /// version
        if (
          true == json_value_header.isMember(g_platform_protocolimpl_header_tag_version) &&
          true == json_value_header[g_platform_protocolimpl_header_tag_version].isString()
        ) {
          message_pack_ptr->message_header.version = json_value_header[g_platform_protocolimpl_header_tag_version].asString();
        }

        /// type
        if (
          true == json_value_header.isMember(g_platform_protocolimpl_header_tag_type) &&
          true == json_value_header[g_platform_protocolimpl_header_tag_type].isString()
        ) {
          message_pack_ptr->message_header.type = json_value_header[g_platform_protocolimpl_header_tag_type].asString();
        }

        /// time_stamp
        if (
          true == json_value_header.isMember(g_platform_protocolimpl_header_tag_time_stamp) &&
          true == json_value_header[g_platform_protocolimpl_header_tag_time_stamp].isString()
        ) {
          message_pack_ptr->message_header.time_stamp = json_value_header[g_platform_protocolimpl_header_tag_time_stamp].asString();
        }

        /// session_id
        if (
          true == json_value_header.isMember(g_platform_protocolimpl_header_tag_session_id) &&
          true == json_value_header[g_platform_protocolimpl_header_tag_session_id].isString()
        ) {
          message_pack_ptr->message_header.session_id = json_value_header[g_platform_protocolimpl_header_tag_session_id].asString();
        }

        /// serial_number
        if (
          true == json_value_header.isMember(g_platform_protocolimpl_header_tag_serial_number) &&
          true == json_value_header[g_platform_protocolimpl_header_tag_serial_number].isString()
        ) {
          message_pack_ptr->message_header.serial_number = json_value_header[g_platform_protocolimpl_header_tag_serial_number].asString();
        }
      }
    }

    /// Content
    if (true == json_value.isMember(g_platform_protocolimpl_content_tag_content)) {
      JsonValue json_value_content = json_value[g_platform_protocolimpl_content_tag_content];
      if (true != json_value_content.empty() && true == json_value_content.isObject()) {

        /// from
        if (
          true == json_value_content.isMember(g_platform_protocolimpl_content_tag_from) &&
          true == json_value_content[g_platform_protocolimpl_content_tag_from].isString()
        ) {
          message_pack_ptr->message_body.from = json_value_content[g_platform_protocolimpl_content_tag_from].asString();
        }

        /// to
        if (
          true == json_value_content.isMember(g_platform_protocolimpl_content_tag_to) &&
          true == json_value_content[g_platform_protocolimpl_content_tag_to].isString()
        ) {
          message_pack_ptr->message_body.to = json_value_content[g_platform_protocolimpl_content_tag_to].asString();
        }

        /// verb
        if (
          true == json_value_content.isMember(g_platform_protocolimpl_content_tag_verb) &&
          true == json_value_content[g_platform_protocolimpl_content_tag_verb].isString()
        ) {
          message_pack_ptr->message_body.verb = json_value_content[g_platform_protocolimpl_content_tag_verb].asString();
        }

        /// param
        JsonValue json_value_param = json_value_content[g_platform_protocolimpl_content_tag_param];
        if (true != json_value_param.empty() && true == json_value_param.isObject()) {

          /// service_name
          if (
            true == json_value_param.isMember(g_platform_protocolimpl_content_param_tag_service_name) &&
            true == json_value_param[g_platform_protocolimpl_content_param_tag_service_name].isString()
          ) {
            message_pack_ptr->message_body.param.service_name = json_value_param[g_platform_protocolimpl_content_param_tag_service_name].asString();
          }

          /// result
          JsonValue json_value_result = json_value_param[g_platform_protocolimpl_response_content_param_tag_result];
          if (true != json_value_result.empty() && true == json_value_result.isObject()) {

            /// result_string
            std::string json_result_string;
            if (YSOS_ERROR_SUCCESS != JsonValueToString(json_value_result, json_result_string)) {
              break;
            }

            message_pack_ptr->message_body.param.result_string = json_result_string;

            /// status_code
            if (
              true == json_value_result.isMember(g_platform_protocolimpl_response_content_param_result_tag_status_code) &&
              true == json_value_result[g_platform_protocolimpl_response_content_param_result_tag_status_code].isString()
            ) {
              message_pack_ptr->message_body.param.result.status_code = json_value_result[g_platform_protocolimpl_response_content_param_result_tag_status_code].asString();
            }

            /// description
            if (
              true == json_value_result.isMember(g_platform_protocolimpl_response_content_param_result_tag_description) &&
              true == json_value_result[g_platform_protocolimpl_response_content_param_result_tag_description].isString()
            ) {
              message_pack_ptr->message_body.param.result.description = json_value_result[g_platform_protocolimpl_response_content_param_result_tag_description].asString();
            }

            /// detail
            JsonValue json_value_result_detail = json_value_result[g_platform_protocolimpl_response_content_param_result_tag_detail];
            if (true != json_value_result_detail.empty() && true == json_value_result_detail.isObject()) {

              std::string json_string;
              if (YSOS_ERROR_SUCCESS != JsonValueToString(json_value_result_detail, json_string)) {
                break;
              }

              message_pack_ptr->message_body.param.result.detail = json_string;
            }

          }
        }
      }
    }

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  return result;
}

int PlatformProtocolImpl::JsonObjectFromString(const std::string& json_string, JsonValue& json_object) {
  int result = YSOS_ERROR_FAILED;

  do {
    if (true == json_string.empty()) {
      break;
    }

    JsonFeatures json_features = JSON_FEATURES_STRICT_MODE;
    JsonReader json_reader(json_features);

    if (true != json_reader.parse(json_string, json_object, true)) {
      break;
    }

    if (true == json_object.empty()) {
      break;
    }

    result = YSOS_ERROR_SUCCESS;

  } while (0);

  return result;
}

void PlatformProtocolImpl::JsonObjectFromStringPair(const PlatformProtocolParamPair& key_value, JsonValue& json_object) {
  json_object[key_value.first] = key_value.second;
  return;
}

void PlatformProtocolImpl::JsonArrayFromJsonObject(const JsonValue& json_object, JsonValue& json_array) {
  json_array.append(json_object);
  return;
}

int PlatformProtocolImpl::JsonValueToString(const JsonValue& json_value, std::string& json_string) {
  int result = YSOS_ERROR_FAILED;

  do {
    JsonFastWriter json_writer;
    json_string = json_writer.write(json_value);
    json_string = GetUtility()->ReplaceAllDistinct ( json_string, "\\r\\n", "" );

    if (true == json_string.empty()) {
      break;
    }

    result  = YSOS_ERROR_SUCCESS;
  } while (0);

  return result;
}

int PlatformProtocolImpl::JsonArrayToVector(const JsonValue& json_array, PlatformProtocolJsonArrayUnitVector& platform_protocol_json_Array_unit_vector) {
  int result = YSOS_ERROR_FAILED;

  do {
    if (true == json_array.empty()) {
      break;
    }

    platform_protocol_json_Array_unit_vector.clear();

    if (true != json_array.isArray()) {
      break;
    }

    if (0 < json_array.size()) {
      JsonValue json_value_array_unit;
      for (unsigned ii = 0; ii < json_array.size(); ++ii) {
        json_value_array_unit = json_array[ii];
        JsonValue json_value_object;
        for (
          JsonValueIterator iterator = json_value_array_unit.begin();
          iterator != json_value_array_unit.end();
          ++iterator
        ) {
          json_value_object = json_value_array_unit[iterator.key().asString().c_str()];
          if (true == json_value_object.isString()) {
            PlatformProtocolParamPair platform_protocol_param_pair;
            platform_protocol_param_pair = std::make_pair(iterator.key().asString(), json_value_object.asString());

            JsonArrayUnit json_array_unit;
            json_array_unit.section = ii;
            json_array_unit.vector.push_back(platform_protocol_param_pair);

            platform_protocol_json_Array_unit_vector.push_back(json_array_unit);

            std::cout << iterator.key().asString().c_str() << ": " << json_value_object.asString().c_str() << std::endl;
          }
        }
      }
    }

    result  = YSOS_ERROR_SUCCESS;
  } while (0);

  return result;
}

void PlatformProtocolImpl::PrintRequestMessagePack(const RequestMessagePack& message_pack) {

  std::cout << "===" << "Request Message Pack" << "===" << std::endl;

  std::cout << "===" << g_platform_protocolimpl_header_tag_header << "===" << std::endl;

  /// tag
  if (true == message_pack.message_header.tag.empty()) {
    std::cout << g_platform_protocolimpl_header_tag_tag << ": " << "null" << std::endl;
  } else {
    std::cout << g_platform_protocolimpl_header_tag_tag << ": " << message_pack.message_header.tag.c_str() << std::endl;
  }

  /// version
  if (true == message_pack.message_header.version.empty()) {
    std::cout << g_platform_protocolimpl_header_tag_version << ": " << "null" << std::endl;
  } else {
    std::cout << g_platform_protocolimpl_header_tag_version << ": " << message_pack.message_header.version.c_str() << std::endl;
  }

  /// type
  if (true == message_pack.message_header.type.empty()) {
    std::cout << g_platform_protocolimpl_header_tag_type << ": " << "null" << std::endl;
  } else {
    std::cout << g_platform_protocolimpl_header_tag_type << ": " << message_pack.message_header.type.c_str() << std::endl;
  }

  /// time_stamp
  if (true == message_pack.message_header.time_stamp.empty()) {
    std::cout << g_platform_protocolimpl_header_tag_time_stamp << ": " << "null" << std::endl;
  } else {
    std::cout << g_platform_protocolimpl_header_tag_time_stamp << ": " << message_pack.message_header.time_stamp.c_str() << std::endl;
  }

  /// session_id
  if (true == message_pack.message_header.session_id.empty()) {
    std::cout << g_platform_protocolimpl_header_tag_session_id << ": " << "null" << std::endl;
  } else {
    std::cout << g_platform_protocolimpl_header_tag_session_id << ": " << message_pack.message_header.session_id.c_str() << std::endl;
  }

  /// serial_number
  if (true == message_pack.message_header.serial_number.empty()) {
    std::cout << g_platform_protocolimpl_header_tag_serial_number << ": " << "null" << std::endl;
  } else {
    std::cout << g_platform_protocolimpl_header_tag_serial_number << ": " << message_pack.message_header.serial_number.c_str() << std::endl;
  }

  std::cout << "===" << g_platform_protocolimpl_content_tag_content << "===" << std::endl;

  /// from
  if (true == message_pack.message_body.from.empty()) {
    std::cout << g_platform_protocolimpl_content_tag_from << ": " << "null" << std::endl;
  } else {
    std::cout << g_platform_protocolimpl_content_tag_from << ": " << message_pack.message_body.from.c_str() << std::endl;
  }

  /// to
  if (true == message_pack.message_body.to.empty()) {
    std::cout << g_platform_protocolimpl_content_tag_to << ": " << "null" << std::endl;
  } else {
    std::cout << g_platform_protocolimpl_content_tag_to << ": " << message_pack.message_body.to.c_str() << std::endl;
  }

  /// verb
  if (true == message_pack.message_body.verb.empty()) {
    std::cout << g_platform_protocolimpl_content_tag_verb << ": " << "null" << std::endl;
  } else {
    std::cout << g_platform_protocolimpl_content_tag_verb << ": " << message_pack.message_body.verb.c_str() << std::endl;
  }

  /// param
  std::cout << "==" << g_platform_protocolimpl_content_tag_param << "==" << std::endl;

  /// service_name
  if (true == message_pack.message_body.param.service_name.empty()) {
    std::cout << g_platform_protocolimpl_content_param_tag_service_name << ": " << "null" << std::endl;
  } else {
    std::cout << g_platform_protocolimpl_content_param_tag_service_name << ": " << message_pack.message_body.param.service_name.c_str() << std::endl;
  }

  /// service_param
  if (true == message_pack.message_body.param.service_param.empty()) {
    std::cout << g_platform_protocolimpl_request_content_param_tag_service_param << ": " << "null" << std::endl;
  } else {
    std::cout << g_platform_protocolimpl_request_content_param_tag_service_param << ": " << message_pack.message_body.param.service_param.c_str() << std::endl;
  }

  std::cout << "=========" << std::endl;

  return;
}

void PlatformProtocolImpl::PrintResponseMessagePack(const ResponseMessagePack& message_pack) {

  std::cout << "===" << "Response Message Pack" << "===" << std::endl;

  std::cout << "===" << g_platform_protocolimpl_header_tag_header << "===" << std::endl;

  /// tag
  if (true == message_pack.message_header.tag.empty()) {
    std::cout << g_platform_protocolimpl_header_tag_tag << ": " << "null" << std::endl;
  } else {
    std::cout << g_platform_protocolimpl_header_tag_tag << ": " << message_pack.message_header.tag.c_str() << std::endl;
  }

  /// version
  if (true == message_pack.message_header.version.empty()) {
    std::cout << g_platform_protocolimpl_header_tag_version << ": " << "null" << std::endl;
  } else {
    std::cout << g_platform_protocolimpl_header_tag_version << ": " << message_pack.message_header.version.c_str() << std::endl;
  }

  /// type
  if (true == message_pack.message_header.type.empty()) {
    std::cout << g_platform_protocolimpl_header_tag_type << ": " << "null" << std::endl;
  } else {
    std::cout << g_platform_protocolimpl_header_tag_type << ": " << message_pack.message_header.type.c_str() << std::endl;
  }

  /// time_stamp
  if (true == message_pack.message_header.time_stamp.empty()) {
    std::cout << g_platform_protocolimpl_header_tag_time_stamp << ": " << "null" << std::endl;
  } else {
    std::cout << g_platform_protocolimpl_header_tag_time_stamp << ": " << message_pack.message_header.time_stamp.c_str() << std::endl;
  }

  /// session_id
  if (true == message_pack.message_header.session_id.empty()) {
    std::cout << g_platform_protocolimpl_header_tag_session_id << ": " << "null" << std::endl;
  } else {
    std::cout << g_platform_protocolimpl_header_tag_session_id << ": " << message_pack.message_header.session_id.c_str() << std::endl;
  }

  /// serial_number
  if (true == message_pack.message_header.serial_number.empty()) {
    std::cout << g_platform_protocolimpl_header_tag_serial_number << ": " << "null" << std::endl;
  } else {
    std::cout << g_platform_protocolimpl_header_tag_serial_number << ": " << message_pack.message_header.serial_number.c_str() << std::endl;
  }

  std::cout << "===" << g_platform_protocolimpl_content_tag_content << "===" << std::endl;

  /// from
  if (true == message_pack.message_body.from.empty()) {
    std::cout << g_platform_protocolimpl_content_tag_from << ": " << "null" << std::endl;
  } else {
    std::cout << g_platform_protocolimpl_content_tag_from << ": " << message_pack.message_body.from.c_str() << std::endl;
  }

  /// to
  if (true == message_pack.message_body.to.empty()) {
    std::cout << g_platform_protocolimpl_content_tag_to << ": " << "null" << std::endl;
  } else {
    std::cout << g_platform_protocolimpl_content_tag_to << ": " << message_pack.message_body.to.c_str() << std::endl;
  }

  /// verb
  if (true == message_pack.message_body.verb.empty()) {
    std::cout << g_platform_protocolimpl_content_tag_verb << ": " << "null" << std::endl;
  } else {
    std::cout << g_platform_protocolimpl_content_tag_verb << ": " << message_pack.message_body.verb.c_str() << std::endl;
  }

  /// param
  std::cout << "==" << g_platform_protocolimpl_content_tag_param << "==" << std::endl;

  /// service_name
  if (true == message_pack.message_body.param.service_name.empty()) {
    std::cout << g_platform_protocolimpl_content_param_tag_service_name << ": " << "null" << std::endl;
  } else {
    std::cout << g_platform_protocolimpl_content_param_tag_service_name << ": " << message_pack.message_body.param.service_name.c_str() << std::endl;
  }

  /// result
  std::cout << "==" << g_platform_protocolimpl_response_content_param_tag_result << "==" << std::endl;

  /// status_code
  if (true == message_pack.message_body.param.result.status_code.empty()) {
    std::cout << g_platform_protocolimpl_response_content_param_result_tag_status_code << ": " << "null" << std::endl;
  } else {
    std::cout << g_platform_protocolimpl_response_content_param_result_tag_status_code << ": " << message_pack.message_body.param.result.status_code.c_str() << std::endl;
  }

  /// description
  if (true == message_pack.message_body.param.result.description.empty()) {
    std::cout << g_platform_protocolimpl_response_content_param_result_tag_description << ": " << "null" << std::endl;
  } else {
    std::cout << g_platform_protocolimpl_response_content_param_result_tag_description << ": " << message_pack.message_body.param.result.description.c_str() << std::endl;
  }

  /// detail
  if (true == message_pack.message_body.param.result.detail.empty()) {
    std::cout << g_platform_protocolimpl_response_content_param_result_tag_detail << ": " << "null" << std::endl;
  } else {
    std::cout << g_platform_protocolimpl_response_content_param_result_tag_detail << ": " << message_pack.message_body.param.result.detail.c_str() << std::endl;
  }

  std::cout << "=========" << std::endl;

  return;
}

void PlatformProtocolImpl::PrintJsonValue(const JsonValue& json_value) {

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

void PlatformProtocolImpl::GetMessageInfoByMessageID(const unsigned int message_id, std::string& message_id_string, std::string& service_name) {

  for (int ii = 0;
       ii < sizeof(g_message_id_service_name_info_req_direction) / sizeof(MessageIDnServiceNameInfo);
       ++ii
      ) {
    if (message_id == g_message_id_service_name_info_req_direction[ii].message_id) {
      message_id_string = g_message_id_service_name_info_req_direction[ii].message_id_string;
      service_name = g_message_id_service_name_info_req_direction[ii].service_name;
      return;
    }
  }

  for (int ii = 0;
       ii < sizeof(g_message_id_service_name_info_rsp_direction) / sizeof(MessageIDnServiceNameInfo);
       ++ii
      ) {
    if (message_id == g_message_id_service_name_info_rsp_direction[ii].message_id) {
      message_id_string = g_message_id_service_name_info_rsp_direction[ii].message_id_string;
      service_name = g_message_id_service_name_info_rsp_direction[ii].service_name;
      return;
    }
  }

  return;
}

void PlatformProtocolImpl::GetMessageInfoByServiceName(const std::string& service_name, ProtocolFormatId format_id, unsigned int& message_id, std::string& message_id_string) {
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger(), "PlatformProtocolImpl::GetMessageInfoByServiceName[Enter]");

  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger(), "PlatformProtocolImpl::GetMessageInfoByServiceName[Check Point][service_name]" << service_name);
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger(), "PlatformProtocolImpl::GetMessageInfoByServiceName[Check Point][format_id]" << format_id);
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger(), "PlatformProtocolImpl::GetMessageInfoByServiceName[Check Point][message_id]" << message_id);
  YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger(), "PlatformProtocolImpl::GetMessageInfoByServiceName[Check Point][message_id_string]" << message_id_string);

  if (PROP_REQUEST == format_id) {
    for (int ii = 0;
         ii < sizeof(g_message_id_service_name_info_req_direction) / sizeof(MessageIDnServiceNameInfo);
         ++ii
        ) {
      if (service_name == g_message_id_service_name_info_req_direction[ii].service_name) {
        message_id = g_message_id_service_name_info_req_direction[ii].message_id;
        message_id_string = g_message_id_service_name_info_req_direction[ii].message_id_string;
        break;
      }
    }
  } else if (PROP_RESPONSE == format_id) {
    for (int ii = 0;
         ii < sizeof(g_message_id_service_name_info_rsp_direction) / sizeof(MessageIDnServiceNameInfo);
         ++ii
        ) {
      if (service_name == g_message_id_service_name_info_rsp_direction[ii].service_name) {
        message_id = g_message_id_service_name_info_rsp_direction[ii].message_id;
        message_id_string = g_message_id_service_name_info_rsp_direction[ii].message_id_string;
        break;
      }
    }
  } else {
  }

  return;
}

}

/*
  Sample Json String

  Request
  //{"content":{"from":"127.0.0.1:6001","param":{"service_name":"RegDoServiceEvent","service_param":{"event_spec":[{"callback_name":"callbackName1","event_key":"event_key_1"},{"callback_name":"callbackName2","event_key":"event_key_2"}],"flag":"0","id":"123","name":"jinchengzhe","service_name":"default@ASRModuleByiFly"}},"to":"127.0.0.1:6002","verb":"DoService"},"header":{"serial_number":"0000000000","session_id":"cccccccccc","tag":"ysos","time_stamp":"2016-09-24 14:57:28 682254","type":"31006","version":"1.0.0.1"}}
  //{\"content\":{\"from\":\"127.0.0.1:6001\",\"param\":{\"service_name\":\"RegDoServiceEvent\",\"service_param\":{\"event_spec\":[{\"callback_name\":\"callbackName1\",\"event_key\":\"event_key_1\"},{\"callback_name\":\"callbackName2\",\"event_key\":\"event_key_2\"}],\"flag\":\"0\",\"id\":\"123\",\"name\":\"jinchengzhe\",\"service_name\":\"default@ASRModuleByiFly\"}},\"to\":\"127.0.0.1:6002\",\"verb\":\"DoService\"},\"header\":{\"serial_number\":\"0000000000\",\"session_id\":\"cccccccccc\",\"tag\":\"ysos\",\"time_stamp\":\"2016-09-24 14:57:28 682254\",\"type\":\"31006\",\"version\":\"1.0.0.1\"}}


  Response
  //{"content":{"from":"127.0.0.1:6002","param":{"result":{"description":"succeeded","detail":{"service_list":[{"alias":"alias_of_servicexxx","description":"description_of_servicexxx","name":"name_of_servicexxx"},{"alias":"alias_of_serviceyyy","description":"description_of_serviceyyy","name":"name_of_serviceyyy"}]},"status_code":"0"},"service_name":YSOS_AGENT_SERVICE_NAME_SERVICE_LIST},"to":"127.0.0.1:6001","verb":"OnDispatchMessage"},"header":{"serial_number":"0000000000","session_id":"cccccccccc","tag":"ysos","time_stamp":"2016-09-24 16:35:13 750419","type":"31003","version":"1.0.0.1"}}
  //{\"content\":{\"from\":\"127.0.0.1:6002\",\"param\":{\"result\":{\"description\":\"succeeded\",\"detail\":{\"service_list\":[{\"alias\":\"alias_of_servicexxx\",\"description\":\"description_of_servicexxx\",\"name\":\"name_of_servicexxx\"},{\"alias\":\"alias_of_serviceyyy\",\"description\":\"description_of_serviceyyy\",\"name\":\"name_of_serviceyyy\"}]},\"status_code\":\"0\"},\"service_name\":\"GetServiceList\"},\"to\":\"127.0.0.1:6001\",\"verb\":\"OnDispatchMessage\"},\"header\":{\"serial_number\":\"0000000000\",\"session_id\":\"cccccccccc\",\"tag\":\"ysos\",\"time_stamp\":\"2016-09-24 16:35:13 750419\",\"type\":\"31003\",\"version\":\"1.0.0.1\"}}

*/