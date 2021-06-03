/*
# jsonrpc.cpp
# Definition of JsonRPC
# Created on: 2016-09-19 16:01:03
# Original author: jinchengzhe
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
# version 0.1, 20160919, created by JinChengZhe
*/

/// Private Headers
#include "../../../protect/include/protocol_help_package/jsonrpc.h"
/// ThirdParty Headers
#include <boost/thread/mutex.hpp>
/// Platform Headers
#include "../../../protect/include/protocol_help_package/commonconnectionimpl.h"
#include "../../../protect/include/protocol_help_package/commonconnectioncallbackimpl.h"
#include "../../../protect/include/protocol_help_package/jsonrpcconnectioncallbackimpl.h"
#include "../../../protect/include/protocol_help_package/platformprotocolimpl.h"
#include "../../../public/include/sys_interface_package/messagetype.h"
#include "../../../public/include/os_hal_package/bufferutility.h"
#include "../../../protect/include/protocol_help_package/baserpccallbackimpl.h"
#include "../../../public/include/core_help_package/utility.h"
#include "../../../public/include/sys_interface_package/ysos_enum.h"

namespace ysos {
/**
*@brief Loger的静态声明  // NOLINT
*/
namespace ysos_json_rpc_loger {
static log4cplus::Logger logger_;
}

using namespace ysos_json_rpc_loger;

#define CONNECTION_BUFFER_LENGTH 9216
#define CONNECTION_BUFFER_NUMBER 6
#define DUPLICATED_SESSION_ID_LENGTH 64
#define INIT_WAITING_TIME 60
#define SERIAL_NUMBER_LENGTH 11
#define BUFFER_NUMBER 6
#define BUFFER_PREFIX 1024

static const char* g_request_service_param_handshake_app_name = "app_name";
static const char* g_request_service_param_handshake_strategy_name = "strategy_name";

#if _CERTUSNET
static const char* g_request_service_param_handshake_app_name_value = "robotterminalapp";
static const char* g_request_service_param_handshake_strategy_name_value = "robotterminalapp";
#endif

UINT64 JsonRPC::serial_number_ = 0;
JsonRPC::JsonRPC(const std::string &strClassName /* =JsonRPC */)
  : BaseInterfaceImpl(strClassName),
    base_rpc_callback_impl_ptr_(NULL),
    connection_ptr_(NULL),
    json_rpc_connection_callback_impl_ptr_(NULL),
    init_status_(false),
    running_status_(IS_INITIAL),
    buffer_pool_ptr_(NULL),
    protocol_ptr_(NULL),
    session_id_("0") {
  logger_ = GetUtility()->GetLogger("ysos.sdk");
}

JsonRPC::~JsonRPC() {
  if (NULL != base_rpc_callback_impl_ptr_) {
    YSOS_LOG_DEBUG("JsonRPC::~JsonRPC[Check Point][base_rpc_callback_impl_ptr_ is not NULL.]");
    base_rpc_callback_impl_ptr_ = NULL;
  }

  connection_ptr_ = NULL;

  json_rpc_connection_callback_impl_ptr_ = NULL;

  buffer_pool_ptr_ = NULL;

  protocol_ptr_ = NULL;

  init_status_ = false;

  running_status_ = IS_INITIAL;

  session_id_.clear();

  serial_number_ = 0;
}

int JsonRPC::DoService(const std::string& session_id, const std::string& service_name, const std::string& service_param) {
  YSOS_LOG_DEBUG("JsonRPC::DoService[Enter]");

  if (true != session_id.empty()) {
    YSOS_LOG_DEBUG("session_id: " << session_id);
  }

  if (true != service_name.empty()) {
    YSOS_LOG_DEBUG("service_name: " << service_name);
  }

  if (true != service_param.empty()) {
    YSOS_LOG_DEBUG("service_param: " << service_param);
  }

  int result = YSOS_ERROR_FAILED;

  do {
    /// 判断模块是否已开启
    if (IS_RUNNING != GetRunningStatus()) {
      YSOS_LOG_DEBUG("JsonRPC::DoService[It is not running]");
      result = YSOS_ERROR_INVALID_OPERATION;
      break;
    }

    YSOS_LOG_DEBUG("JsonRPC::DoService[Check Point][0]");

    if (true == session_id.empty() || true == service_name.empty()) {
      YSOS_LOG_DEBUG("JsonRPC::DoService[Fail][session_id is empaty or service_name is empty]");
      break;
    }

    YSOS_LOG_DEBUG("JsonRPC::DoService[Check Point][1]");

    /// 不是初始化服务，session id不可为0
#if _CERTUSNET
    if (YSOS_AGENT_SERVICE_NAME_INIT_CONNECTION != service_name && "0" == session_id) {
#else
    if (YSOS_AGENT_SERVICE_NAME_INIT != service_name && "0" == session_id) {
#endif
      YSOS_LOG_DEBUG("JsonRPC::DoService[Fail][service_name is not init and session_id is 0.]");
      break;
    }

    YSOS_LOG_DEBUG("JsonRPC::DoService[Check Point][2]");

    /// 通过service_name字符串获得消息id
    unsigned int message_id = 0;
    std::string message_id_string;
    PlatformProtocolImpl::GetMessageInfoByServiceName(service_name, PROP_REQUEST, message_id, message_id_string);
    if (true == message_id_string.empty() || 0 == message_id) {
      YSOS_LOG_DEBUG("JsonRPC::DoService[Fail][message_id_string is empty.]");
      break;
    }

    YSOS_LOG_DEBUG("JsonRPC::DoService[Check Point][3]");

    /// 封装成消息内容结构体
    PlatformProtocolImpl::RequestMessagePack message_pack;

    /// Header
    message_pack.message_header.type = message_id_string;
    message_pack.message_header.session_id = session_id;

    std::string serial_number;
    GetSerialNumber(serial_number);
    message_pack.message_header.serial_number = serial_number;

    YSOS_LOG_DEBUG("JsonRPC::DoService[Check Point][4]");

    if (NULL == base_rpc_callback_impl_ptr_) {
      YSOS_LOG_DEBUG("JsonRPC::DoService[Fail][base_rpc_callback_impl_ptr_ is NULL.]");
      break;
    }

    /// Body
    if (PROP_CLIENT == base_rpc_callback_impl_ptr_->GetMode()) {
      YSOS_LOG_DEBUG("JsonRPC::DoService[Check Point][client mode]");
      message_pack.message_body.to = base_rpc_callback_impl_ptr_->GetAddress() + ":" + boost::lexical_cast<std::string>(base_rpc_callback_impl_ptr_->GetPort());
    } else {
      YSOS_LOG_DEBUG("JsonRPC::DoService[Check Point][server mode]");
      message_pack.message_body.from = base_rpc_callback_impl_ptr_->GetAddress() + ":" + boost::lexical_cast<std::string>(base_rpc_callback_impl_ptr_->GetPort());
    }

    message_pack.message_body.param.service_name = service_name;
    if (true != service_param.empty()) {
      YSOS_LOG_DEBUG("JsonRPC::DoService[Check Point][service_param is not empty.]");
      message_pack.message_body.param.service_param = service_param;
    }

    YSOS_LOG_DEBUG("JsonRPC::DoService[Check Point][5]");

    /// 从buffer pool里获取buffer
    if (NULL == buffer_pool_ptr_) {
      YSOS_LOG_DEBUG("JsonRPC::DoService[Fail][buffer_pool_ptr_ is NULL.]");
      break;
    }

    YSOS_LOG_DEBUG("JsonRPC::DoService[Check Point][6]");

    BufferInterfacePtr buffer_ptr = GetBufferUtility()->GetBufferFromBufferPool(buffer_pool_ptr_);
    while (NULL == buffer_ptr) {
      YSOS_LOG_DEBUG("JsonRPC::DoService[Get Buffer Ptr][Fail]");
      boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
      buffer_ptr = GetBufferUtility()->GetBufferFromBufferPool(buffer_pool_ptr_);
      if (NULL != buffer_ptr) {
        YSOS_LOG_DEBUG("JsonRPC::DoService[Get Buffer Ptr][Sucess]");
        break;
      }
    }

    YSOS_LOG_DEBUG("JsonRPC::DoService[Check Point][7]");

    YSOS_LOG_DEBUG("JsonRPC::DoService[Start][buffer_ptr][" << reinterpret_cast<UINT64>(buffer_ptr.get()) << "]");
    YSOS_LOG_DEBUG("JsonRPC::DoService[Start][buffer_ptr use count][" << buffer_ptr.use_count() << "]");

    if (YSOS_ERROR_SUCCESS != GetBufferUtility()->SetBufferLength(buffer_ptr, (GetBufferUtility()->GetBufferMaxLength(buffer_ptr) - BUFFER_PREFIX))) {
      YSOS_LOG_DEBUG("JsonRPC::DoService[Fail][The function[GetBufferUtility()->SetBufferLength()] is failed.]");
      break;
    }

    if (YSOS_ERROR_SUCCESS != GetBufferUtility()->SetBufferPrefixLength(buffer_ptr, BUFFER_PREFIX)) {
      YSOS_LOG_DEBUG("JsonRPC::DoService[Fail][The function[GetBufferUtility()->SetBufferPrefixLength()] is failed.]");
      break;
    }

    /// 初始化buffer
    if (YSOS_ERROR_SUCCESS != GetBufferUtility()->InitialBuffer(buffer_ptr)) {
      YSOS_LOG_DEBUG("JsonRPC::DoService[Fail][The function[GetBufferUtility()->InitialBuffer()] is failed.]");
      break;
    }

    YSOS_LOG_DEBUG("JsonRPC::DoService[Check Point][8]");

    /// 封装消息为标准通讯json字符串格式
    if (NULL == protocol_ptr_) {
      YSOS_LOG_DEBUG("JsonRPC::DoService[Fail][protocol_ptr_ is NULL.]");
      break;
    }

    if (protocol_ptr_->FormMessage(NULL, buffer_ptr, PROP_REQUEST, &message_pack)) {
      YSOS_LOG_DEBUG("JsonRPC::DoService[Fail][The function[protocol_ptr_->FormMessage()] is failed.]");
      break;
    }

    YSOS_LOG_DEBUG("JsonRPC::DoService[Check Point][9]");

    /// 发送消息
    if (NULL == connection_ptr_) {
      YSOS_LOG_DEBUG("JsonRPC::DoService[Fail][connection_ptr_ is NULL.]");
      break;
    }

    YSOS_LOG_DEBUG("JsonRPC::DoService[Check Point][10]");

    /// 客户端模式下，直接使用connection指针发送
    if (PROP_CLIENT == base_rpc_callback_impl_ptr_->GetMode()) {
      YSOS_LOG_DEBUG("JsonRPC::DoService[Check Point][client mode]");

      if (YSOS_ERROR_SUCCESS != connection_ptr_->Write(buffer_ptr, GetBufferUtility()->GetBufferLength(buffer_ptr))) {
        YSOS_LOG_DEBUG("JsonRPC::DoService[Fail][The function[connection_ptr_->Write()] is failed.]");
        break;
      }
    }
    /// 服务器模式下，将session id传给connection指针，由connection层使用session id查找指定的connection，再做发送
    else if (PROP_SERVER == base_rpc_callback_impl_ptr_->GetMode()) {
      YSOS_LOG_DEBUG("JsonRPC::DoService[Check Point][server mode]");

      char duplicated_session_id[DUPLICATED_SESSION_ID_LENGTH] = {0,};
      #ifdef _WIN32
        strcpy_s(duplicated_session_id, session_id.c_str());
      #else
        strcpy(duplicated_session_id, session_id.c_str());
      #endif
      if (YSOS_ERROR_SUCCESS != connection_ptr_->Write(buffer_ptr, GetBufferUtility()->GetBufferLength(buffer_ptr), duplicated_session_id)) {
        YSOS_LOG_DEBUG("JsonRPC::DoService[Fail][The function[connection_ptr_->Write()] is failed.]");
        break;
      }
    } else {
      YSOS_LOG_DEBUG("JsonRPC::DoService[Check Point][unknown mode]");
      break;
    }

    YSOS_LOG_DEBUG("JsonRPC::DoService[End][buffer_ptr][" << reinterpret_cast<UINT64>(buffer_ptr.get()) << "]");
    YSOS_LOG_DEBUG("JsonRPC::DoService[End][buffer_ptr use count][" << buffer_ptr.use_count() << "]");

    YSOS_LOG_DEBUG("JsonRPC::DoService[Check Point][End]");

    result = YSOS_ERROR_SUCCESS;
  }
  while (0);

  YSOS_LOG_DEBUG("JsonRPC::DoService[Exit]");

  return result;
}

int JsonRPC::OnDispatchMessage(const std::string& session_id, const std::string& service_name, const std::string& result) {
  YSOS_LOG_DEBUG("JsonRPC::OnDispatchMessage[Enter]");

  if (true != session_id.empty()) {
    YSOS_LOG_DEBUG("session_id: " << session_id);
  }

  if (true != service_name.empty()) {
    YSOS_LOG_DEBUG("service_name: " << service_name);
  }

  if (true != result.empty()) {
    YSOS_LOG_DEBUG("result: " << result);
  }

  int result_value = YSOS_ERROR_FAILED;

  do {
    /// 判断模块是否已开启
    if (IS_RUNNING != GetRunningStatus()) {
      YSOS_LOG_DEBUG("JsonRPC::OnDispatchMessage[It is not running]");
      result_value = YSOS_ERROR_INVALID_OPERATION;
      break;
    }

    YSOS_LOG_DEBUG("JsonRPC::OnDispatchMessage[Check Point][0]");

    if (true == session_id.empty() || true == service_name.empty()) {
      YSOS_LOG_DEBUG("JsonRPC::OnDispatchMessage[Fail][session_id is empty or service_name is empty]");
      break;
    }

    YSOS_LOG_DEBUG("JsonRPC::OnDispatchMessage[Check Point][1]");

    /// 通过service_name字符串获得消息id
    unsigned int message_id = 0;
    std::string message_id_string;
    PlatformProtocolImpl::GetMessageInfoByServiceName(service_name, PROP_RESPONSE, message_id, message_id_string);
    if (true == message_id_string.empty() || 0 == message_id) {
      YSOS_LOG_DEBUG("JsonRPC::OnDispatchMessage[Fail][message_id_string is empty.]");
      break;
    }

    YSOS_LOG_DEBUG("JsonRPC::OnDispatchMessage[Check Point][2]");

    /// 封装消息内容结构体
    PlatformProtocolImpl::ResponseMessagePack message_pack;

    /// Header
    message_pack.message_header.type = message_id_string;
    message_pack.message_header.session_id = session_id;

    std::string serial_number;
    GetSerialNumber(serial_number);
    message_pack.message_header.serial_number = serial_number;

    YSOS_LOG_DEBUG("JsonRPC::OnDispatchMessage[Check Point][3]");

    if (NULL == base_rpc_callback_impl_ptr_) {
      YSOS_LOG_DEBUG("JsonRPC::OnDispatchMessage[Fail][base_rpc_callback_impl_ptr_ is NULL.]");
      break;
    }

    /// Body
    if (PROP_CLIENT == base_rpc_callback_impl_ptr_->GetMode()) {
      YSOS_LOG_DEBUG("JsonRPC::OnDispatchMessage[Check Point][client mode]");
      message_pack.message_body.to = base_rpc_callback_impl_ptr_->GetAddress() + ":" + boost::lexical_cast<std::string>(base_rpc_callback_impl_ptr_->GetPort());
    } else {
      YSOS_LOG_DEBUG("JsonRPC::OnDispatchMessage[Check Point][server mode]");
      message_pack.message_body.from = base_rpc_callback_impl_ptr_->GetAddress() + ":" + boost::lexical_cast<std::string>(base_rpc_callback_impl_ptr_->GetPort());
    }

    message_pack.message_body.param.service_name = service_name;
    if (true != result.empty()) {
      YSOS_LOG_DEBUG("JsonRPC::OnDispatchMessage[Check Point][result is not empty.]");
      message_pack.message_body.param.result_string = result;
    }

    YSOS_LOG_DEBUG("JsonRPC::OnDispatchMessage[Check Point][4]");

    /// 从boffuer pool里获取buffer
    if (NULL == buffer_pool_ptr_) {
      YSOS_LOG_DEBUG("JsonRPC::DoService[Fail][buffer_pool_ptr_ is NULL.]");
      break;
    }

    YSOS_LOG_DEBUG("JsonRPC::OnDispatchMessage[Check Point][5]");

    BufferInterfacePtr buffer_ptr = GetBufferUtility()->GetBufferFromBufferPool(buffer_pool_ptr_);
    while (NULL == buffer_ptr) {
      YSOS_LOG_DEBUG("JsonRPC::OnDispatchMessage[Get Buffer Ptr][Fail]");
      boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
      buffer_ptr = GetBufferUtility()->GetBufferFromBufferPool(buffer_pool_ptr_);
      if (NULL != buffer_ptr) {
        YSOS_LOG_DEBUG("JsonRPC::OnDispatchMessage[Get Buffer Ptr][Sucess]");
        break;
      }
    }

    YSOS_LOG_DEBUG("JsonRPC::OnDispatchMessage[Check Point][6]");

    YSOS_LOG_DEBUG("JsonRPC::OnDispatchMessage[Start][buffer_ptr][" << reinterpret_cast<UINT64>(buffer_ptr.get()) << "]");
    YSOS_LOG_DEBUG("JsonRPC::OnDispatchMessage[Start][buffer_ptr use count][" << buffer_ptr.use_count() << "]");

    if (YSOS_ERROR_SUCCESS != GetBufferUtility()->SetBufferLength(buffer_ptr, (GetBufferUtility()->GetBufferMaxLength(buffer_ptr) - BUFFER_PREFIX))) {
      YSOS_LOG_DEBUG("JsonRPC::OnDispatchMessage[Fail][The function[GetBufferUtility()->SetBufferLength()] is failed.]");
      break;
    }

    if (YSOS_ERROR_SUCCESS != GetBufferUtility()->SetBufferPrefixLength(buffer_ptr, BUFFER_PREFIX)) {
      YSOS_LOG_DEBUG("JsonRPC::OnDispatchMessage[Fail][The function[GetBufferUtility()->SetBufferPrefixLength()] is failed.]");
      break;
    }

    /// 初始化buffer
    if (YSOS_ERROR_SUCCESS != GetBufferUtility()->InitialBuffer(buffer_ptr)) {
      YSOS_LOG_DEBUG("JsonRPC::OnDispatchMessage[Fail][The function[GetBufferUtility()->InitialBuffer(] is failed.]");
      break;
    }

    YSOS_LOG_DEBUG("JsonRPC::OnDispatchMessage[Check Point][7]");

    /// 封装消息为标准通讯json字符串格式
    if (NULL == protocol_ptr_) {
      YSOS_LOG_DEBUG("JsonRPC::OnDispatchMessage[Fail][protocol_ptr_ is NULL.]");
      break;
    }

    YSOS_LOG_DEBUG("JsonRPC::OnDispatchMessage[Check Point][8]");

    if (protocol_ptr_->FormMessage(NULL, buffer_ptr, PROP_RESPONSE, &message_pack)) {
      YSOS_LOG_DEBUG("JsonRPC::OnDispatchMessage[Fail][The function[protocol_ptr_->FormMessage()] is failed.]");
      break;
    }

    YSOS_LOG_DEBUG("JsonRPC::OnDispatchMessage[Check Point][9]");

    /// 发送消息
    if (NULL == connection_ptr_) {
      YSOS_LOG_DEBUG("JsonRPC::OnDispatchMessage[Fail][connection_ptr_ is NULL.]");
      break;
    }

    YSOS_LOG_DEBUG("JsonRPC::OnDispatchMessage[Check Point][10]");

    /// 客户端模式下，直接发送消息
    if (PROP_CLIENT == base_rpc_callback_impl_ptr_->GetMode()) {
      YSOS_LOG_DEBUG("JsonRPC::DoService[Check Point][client mode]");

      if (YSOS_ERROR_SUCCESS != connection_ptr_->Write(buffer_ptr, GetBufferUtility()->GetBufferLength(buffer_ptr))) {
        YSOS_LOG_DEBUG("JsonRPC::DoService[Fail][The function[connection_ptr_->Write()] is failed.]");
        break;
      }
    }
    /// 服务器模式下，将session id传给connection指针，由connection层使用session id查找指定的connection，再做发送
    else if (PROP_SERVER == base_rpc_callback_impl_ptr_->GetMode()) {
      YSOS_LOG_DEBUG("JsonRPC::DoService[Check Point][server mode]");

      char duplicated_session_id[DUPLICATED_SESSION_ID_LENGTH] = {0,};
      #ifdef _WIN32
        strcpy_s(duplicated_session_id, session_id.c_str());
      #else
        strcpy(duplicated_session_id, session_id.c_str());
      #endif
      if (YSOS_ERROR_SUCCESS != connection_ptr_->Write(buffer_ptr, GetBufferUtility()->GetBufferLength(buffer_ptr), duplicated_session_id)) {
        YSOS_LOG_DEBUG("JsonRPC::DoService[Fail][The function[connection_ptr_->Write()] is failed.]");
        break;
      }
    } else {
      YSOS_LOG_DEBUG("JsonRPC::DoService[Check Point][unknown mode]");
      break;
    }

    YSOS_LOG_DEBUG("JsonRPC::OnDispatchMessage[End][buffer_ptr][" << reinterpret_cast<UINT64>(buffer_ptr.get()) << "]");
    YSOS_LOG_DEBUG("JsonRPC::OnDispatchMessage[End][buffer_ptr use count][" << buffer_ptr.use_count() << "]");

    YSOS_LOG_DEBUG("JsonRPC::OnDispatchMessage[Check Point][End]");

    result_value = YSOS_ERROR_SUCCESS;
  } while (0);

  YSOS_LOG_DEBUG("JsonRPC::OnDispatchMessage[Exit]");

  return result_value;
}

void JsonRPC::HandleDoService(const std::string& session_id, const std::string& service_name, const std::string& service_param) {
  YSOS_LOG_DEBUG("JsonRPC::HandleDoService[Enter]");

  if (true != session_id.empty()) {
    YSOS_LOG_DEBUG("session_id: " << session_id);
  }

  if (true != service_name.empty()) {
    YSOS_LOG_DEBUG("service_name: " << service_name);
  }

  if (true != service_param.empty()) {
    YSOS_LOG_DEBUG("service_param: " << service_param);
  }

  int result = YSOS_ERROR_FAILED;

  do {
    /// 判断模块是否已关闭
    if (IS_STOPPED == GetRunningStatus()) {
      YSOS_LOG_DEBUG("JsonRPC::HandleDoService[It is already stopped]");
      result = YSOS_ERROR_SUCCESS;
      break;
    }

    YSOS_LOG_DEBUG("JsonRPC::HandleDoService[Check Point][0]");

    if (NULL == base_rpc_callback_impl_ptr_) {
      YSOS_LOG_DEBUG("JsonRPC::HandleDoService[Fail][base_rpc_callback_impl_ptr_ is NULL.]");
      break;
    }

    YSOS_LOG_DEBUG("JsonRPC::HandleDoService[Check Point][1]");

    base_rpc_callback_impl_ptr_->HandleDoService(session_id, service_name, service_param);

    YSOS_LOG_DEBUG("JsonRPC::HandleDoService[Check Point][End]");

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  YSOS_LOG_DEBUG("JsonRPC::HandleDoService[Exit]");

  return;
}

void JsonRPC::HandleOnDispatchMessage(const std::string& session_id, const std::string& service_name, const std::string& result) {
  YSOS_LOG_DEBUG("JsonRPC::HandleOnDispatchMessage[Enter]");

  if (true != session_id.empty()) {
    YSOS_LOG_DEBUG("session_id: " << session_id);
  }

  if (true != service_name.empty()) {
    YSOS_LOG_DEBUG("service_name: " << service_name);
  }

  if (true != result.empty()) {
    YSOS_LOG_DEBUG("result: " << result);
  }

  int result_value = YSOS_ERROR_FAILED;

  do {
    /// 判断模块是否已关闭
    if (IS_STOPPED == GetRunningStatus()) {
      YSOS_LOG_DEBUG("JsonRPC::HandleOnDispatchMessage[It is already stopped]");
      result_value = YSOS_ERROR_SUCCESS;
      break;
    }

    YSOS_LOG_DEBUG("JsonRPC::HandleOnDispatchMessage[Check Point][0]");

    if (NULL == base_rpc_callback_impl_ptr_) {
      YSOS_LOG_DEBUG("JsonRPC::HandleOnDispatchMessage[Fail][base_rpc_callback_impl_ptr_ is NULL.]");
      break;
    }

    YSOS_LOG_DEBUG("JsonRPC::HandleOnDispatchMessage[Check Point][1]");

    /// 客户端模式下，进行初始化协议
    if (PROP_CLIENT == base_rpc_callback_impl_ptr_->GetMode()) {
      YSOS_LOG_DEBUG("JsonRPC::HandleOnDispatchMessage[Check Point][client mode]");

      /// 初始化服务下，将获取的session id赋给成员变量，通知Open操作的等待线程
#if _CERTUSNET
      if (YSOS_AGENT_SERVICE_NAME_INIT_CONNECTION == service_name) {
#else
      if (YSOS_AGENT_SERVICE_NAME_INIT == service_name) {
#endif
        YSOS_LOG_DEBUG("JsonRPC::HandleOnDispatchMessage[Check Point][init service]");

        /// 设置session id
        session_id_ = session_id;

        /// 服务器返回的session id
        YSOS_LOG_DEBUG("JsonRPC::HandleOnDispatchMessage[Check Point][session_id_]" << session_id_);

        /// 设置初始化协议状态
        SetInitStatus(true);

        YSOS_LOG_DEBUG("JsonRPC::HandleOnDispatchMessage[Check Point][init service][0]");

        /// 通知所有等待线程
        condition_vraiable_any_init_.notify_all();

        YSOS_LOG_DEBUG("JsonRPC::HandleOnDispatchMessage[Check Point][init service][End]");
      }
      /// 其他服务下，直接回调上层函数
      else {
        YSOS_LOG_DEBUG("JsonRPC::HandleOnDispatchMessage[Check Point][normal service]");

        base_rpc_callback_impl_ptr_->HandleOnDispatchMessage(session_id, service_name, result);

        YSOS_LOG_DEBUG("JsonRPC::HandleOnDispatchMessage[Check Point][normal service][End]");
      }
    }

    YSOS_LOG_DEBUG("JsonRPC::HandleOnDispatchMessage[Check Point][End]");

    result_value = YSOS_ERROR_SUCCESS;
  }
  while (0);

  YSOS_LOG_DEBUG("JsonRPC::HandleOnDispatchMessage[Exit]");

  return;
}

void JsonRPC::HandleError(const std::string& session_id, const std::string& error_param) {
  YSOS_LOG_DEBUG("JsonRPC::HandleError[Enter]");

  if (true != session_id.empty()) {
    YSOS_LOG_DEBUG("session_id: " << session_id);
  }

  if (true != error_param.empty()) {
    YSOS_LOG_DEBUG("error_param: " << error_param);
  }

  int result = YSOS_ERROR_FAILED;

  do {
    if (NULL == base_rpc_callback_impl_ptr_) {
      YSOS_LOG_DEBUG("JsonRPC::HandleError[Fail][base_rpc_callback_impl_ptr_ is NULL]");
      break;
    }

    YSOS_LOG_DEBUG("JsonRPC::HandleError[Check Point][0]");

    base_rpc_callback_impl_ptr_->HandleError(session_id, error_param);

    YSOS_LOG_DEBUG("JsonRPC::HandleError[Check Point][End]");

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  YSOS_LOG_DEBUG("JsonRPC::HandleError[Exit]");

  return;
}

int JsonRPC::Open(RPCCallbackInterface* params, std::string* session_id_ptr) {
  YSOS_LOG_DEBUG("JsonRPC::Open[Enter]");

  int result = YSOS_ERROR_FAILED;

  do {
    /// 判断模块是否已开启
    if (IS_RUNNING == GetRunningStatus()) {
      YSOS_LOG_DEBUG("JsonRPC::Open[It is already running]");
      result = YSOS_ERROR_SUCCESS;
      break;
    }

    YSOS_LOG_DEBUG("JsonRPC::Open[Check Point][0]");

    if (NULL == params) {
      YSOS_LOG_DEBUG("JsonRPC::Open[Fail][params is NULL.]");
      break;
    }

    YSOS_LOG_DEBUG("JsonRPC::Open[Check Point][1]");

    /// 初始化open参数
    if (YSOS_ERROR_SUCCESS != InitOpenParam(params)) {
      YSOS_LOG_DEBUG("JsonRPC::Open[Fail][The funticon[InitOpenParam] is failed.]");
      break;
    }

    YSOS_LOG_DEBUG("JsonRPC::Open[Check Point][2]");

    /// 打开connection模块
    connection_ptr_ = ConnectionInterfacePtr(CommonConnectionImpl::CreateInstance());
    if (YSOS_ERROR_SUCCESS != connection_ptr_->Open(&json_rpc_connection_callback_impl_ptr_)) {
      YSOS_LOG_DEBUG("JsonRPC::Open[Fail][The function[connection_ptr_->Open()] is failed.]");
      break;
    }

    YSOS_LOG_DEBUG("JsonRPC::Open[Check Point][3]");

    /// 设置运行状态
    SetRunningStatus(IS_RUNNING);

    /// 客户端模式下，Open成功后，进行初始化协议
    if (PROP_CLIENT == base_rpc_callback_impl_ptr_->GetMode()) {
      YSOS_LOG_DEBUG("JsonRPC::Open[Check Point][Client Mode][Enter]");

      /// 客户端模式下，调用HandleConnect，让读取循环跑起来
      if (YSOS_ERROR_SUCCESS != connection_ptr_->SetProperty(PROP_HANDLECONNECT, NULL)) {
        YSOS_LOG_DEBUG("JsonRPC::Open[Fail][The function[connection_ptr_->SetProperty()] is failed.]");
        break;
      }

      /// platform rpc模式下，发送应用名和策略名
      std::string duplicated_service_param;
#if _CERTUSNET
      if (true == base_rpc_callback_impl_ptr_->GetApplicationName().empty() ||
          true == base_rpc_callback_impl_ptr_->GetStrategyName().empty()) {
        YSOS_LOG_DEBUG("JsonRPC::Open[Check Point][Client Mode][PLATFORM_RPC]");

        JsonValue json_value_object;
        PlatformProtocolImpl::PlatformProtocolParamPair platform_protocol_param_pair;

        platform_protocol_param_pair.first = g_request_service_param_handshake_app_name;
        platform_protocol_param_pair.second = g_request_service_param_handshake_app_name_value;
        PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_object);

        YSOS_LOG_DEBUG("JsonRPC::Open[Check Point][Client Mode][0]");

        platform_protocol_param_pair.first = g_request_service_param_handshake_strategy_name;
        platform_protocol_param_pair.second = g_request_service_param_handshake_strategy_name_value;
        PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_object);

        YSOS_LOG_DEBUG("JsonRPC::Open[Check Point][Client Mode][1]");

        if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonValueToString(json_value_object, duplicated_service_param)) {
          YSOS_LOG_DEBUG("JsonRPC::Open[Fail][The function[PlatformProtocolImpl::JsonValueToString()] is failed.]");
          break;
        }
      }
#else
      if (true != base_rpc_callback_impl_ptr_->GetApplicationName().empty() ||
          true != base_rpc_callback_impl_ptr_->GetStrategyName().empty()) {
        YSOS_LOG_DEBUG("JsonRPC::Open[Check Point][Client Mode][PLATFORM_RPC]");

        JsonValue json_value_object;
        PlatformProtocolImpl::PlatformProtocolParamPair platform_protocol_param_pair;

        platform_protocol_param_pair.first = g_request_service_param_handshake_app_name;
        platform_protocol_param_pair.second = base_rpc_callback_impl_ptr_->GetApplicationName();
        PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_object);

        YSOS_LOG_DEBUG("JsonRPC::Open[Check Point][Client Mode][0]");

        platform_protocol_param_pair.first = g_request_service_param_handshake_strategy_name;
        platform_protocol_param_pair.second = base_rpc_callback_impl_ptr_->GetStrategyName();
        PlatformProtocolImpl::JsonObjectFromStringPair(platform_protocol_param_pair, json_value_object);

        YSOS_LOG_DEBUG("JsonRPC::Open[Check Point][Client Mode][1]");

        if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonValueToString(json_value_object, duplicated_service_param)) {
          YSOS_LOG_DEBUG("JsonRPC::Open[Fail][The function[PlatformProtocolImpl::JsonValueToString()] is failed.]");
          break;
        }
      }
#endif

      YSOS_LOG_DEBUG("JsonRPC::Open[Check Point][4]");

      /// 判断session id是否是空或者为0，不为空或者不为0，可能是重连操作
      if (NULL != session_id_ptr &&
          true != session_id_ptr->empty() &&
          "0" != *session_id_ptr) {
        YSOS_LOG_DEBUG("JsonRPC::Open[Check Point][session_id_ptr][" << *session_id_ptr << "]");
        /// 设置初始化session_id
        session_id_ = *session_id_ptr;
      }

      /// 发送初始化协议
#if _CERTUSNET
      std::string duplicated_service_name = YSOS_AGENT_SERVICE_NAME_INIT_CONNECTION;
#else
      std::string duplicated_service_name = YSOS_AGENT_SERVICE_NAME_INIT;
#endif
      if (YSOS_ERROR_SUCCESS != DoService(session_id_, duplicated_service_name, duplicated_service_param)) {
        YSOS_LOG_DEBUG("JsonRPC::Open[Fail][The function[DoService()] is failed.]");
        break;
      }

      YSOS_LOG_DEBUG("JsonRPC::Open[Check Point][5]");

      /// 线程同步等待初始化成功状态，等待60秒
      bool time_out = false;
      UniqueLock unique_lock(mutex_lock_);
      while (false == GetInitStatus()) {
        if (condition_vraiable_any_init_.wait_for(unique_lock, boost::chrono::seconds(INIT_WAITING_TIME)) == boost::cv_status::timeout) {
          YSOS_LOG_DEBUG("JsonRPC::Open[Fail][time out]");
          time_out = true;
          break;
        }
      }

      YSOS_LOG_DEBUG("JsonRPC::Open[Check Point][6]");

      /// 初始化超时
      if (true == time_out) {
        YSOS_LOG_DEBUG("JsonRPC::Open[Fail][time_out is true.]");
        break;
      }

      YSOS_LOG_DEBUG("JsonRPC::Open[Check Point][7]");

      /// 设置session_id，通过初始化服务HandleOnDispatchMessage回掉函数重置
      if (true == session_id_.empty() ||
          "0" == session_id_) {
        /// 为空或者为零，初始化失败
        YSOS_LOG_DEBUG("JsonRPC::Open[Fail][session_id_ is empty or 0]");
        break;
      } else {
        if (NULL != session_id_ptr) {
          /// 客户端模式下，将服务器返回的session id以参数形式返回给上层调用者
          YSOS_LOG_DEBUG("JsonRPC::Open[Check Point][Connect to server success][session_id][" << session_id_ << "]");
          *session_id_ptr = session_id_;
        } else {
          /// 客户端模式下，不传session_id_ptr，模块打开失败
          YSOS_LOG_DEBUG("JsonRPC::Open[Fail][session_id_ptr is NULL]");
          break;
        }
      }

      YSOS_LOG_DEBUG("JsonRPC::Open[Check Point][Client Mode][End]");
    }

    YSOS_LOG_DEBUG("JsonRPC::Open[Check Point][End]");

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  if (YSOS_ERROR_SUCCESS != result) {
    YSOS_LOG_DEBUG("JsonRPC::Open[Error]");

    /// 设置运行状态
    SetRunningStatus(IS_INITIAL);
  }

  YSOS_LOG_DEBUG("JsonRPC::Open[Exit]");

  return result;
}

int JsonRPC::Close(const std::string* session_id_ptr) {
  YSOS_LOG_DEBUG("JsonRPC::Close[Enter]");

  /// 判断模块是否已关闭
  if (IS_STOPPED == GetRunningStatus()) {
    YSOS_LOG_DEBUG("JsonRPC::Close[It is already stopped]");
    return YSOS_ERROR_SUCCESS;
  }

  YSOS_LOG_DEBUG("JsonRPC::Close[Check Point][0]");

  /// 设置运行状态
  SetRunningStatus(IS_STOPPED);

  if (NULL != connection_ptr_) {
    YSOS_LOG_DEBUG("JsonRPC::Close[Check Point][connection_ptr_ is not NULL.]");
    // 在jsonrpc层暂时屏蔽传递参数
//    connection_ptr_->Close(const_cast<std::string*>(session_id_ptr));
    connection_ptr_->Close();
  }

  YSOS_LOG_DEBUG("JsonRPC::Close[Check Point][1]");

  if (NULL != base_rpc_callback_impl_ptr_) {
    YSOS_LOG_DEBUG("JsonRPC::Close[Check Point][base_rpc_callback_impl_ptr_ is not NULL.]");
    base_rpc_callback_impl_ptr_ = NULL;
  }

  YSOS_LOG_DEBUG("JsonRPC::Close[Check Point][2]");

  /// 重置标识位
  SetInitStatus(false);

  /// 清空session id
  session_id_.clear();

  /// 重置流水号
  serial_number_ = 0;

  YSOS_LOG_DEBUG("JsonRPC::Close[Exit]");

  return YSOS_ERROR_SUCCESS;
}

int JsonRPC::InitOpenParam(void* params) {
  YSOS_LOG_DEBUG("JsonRPC::InitOpenParam[Enter]");

  int result = YSOS_ERROR_FAILED;

  do {
    /// 设置运行状态
    SetRunningStatus(IS_INITIAL);

    if (NULL == params) {
      YSOS_LOG_DEBUG("JsonRPC::InitOpenParam[params is NULL.]");
      break;
    }

    YSOS_LOG_DEBUG("JsonRPC::InitOpenParam[Check Point][0]");

    /// 强转成rpccallback
    base_rpc_callback_impl_ptr_ = static_cast<BaseRPCCallbackImpl*>(params);

    YSOS_LOG_DEBUG("JsonRPC::InitOpenParam[Check Point][JsonRPC buffer length][" << base_rpc_callback_impl_ptr_->GetBufferLength() << "]");

    /// 初始化buffer pool
    buffer_pool_ptr_ = GetBufferUtility()->CreateBufferPool(base_rpc_callback_impl_ptr_->GetBufferLength(), BUFFER_NUMBER, BUFFER_PREFIX);
    if (NULL == buffer_pool_ptr_) {
      YSOS_LOG_DEBUG("JsonRPC::InitOpenParam[Fail][buffer_pool_ptr_ is NULL.]");
      break;
    }

    YSOS_LOG_DEBUG("JsonRPC::InitOpenParam[Check Point][1]");

    /// 初始化connectioncallback
    json_rpc_connection_callback_impl_ptr_ = JsonRPCConnectionCallbackImplPtr(JsonRPCConnectionCallbackImpl::CreateInstance());
    if (NULL == json_rpc_connection_callback_impl_ptr_) {
      YSOS_LOG_DEBUG("JsonRPC::InitOpenParam[Fail][json_rpc_connection_callback_impl_ptr_ is NULL.]");
      break;
    }

    YSOS_LOG_DEBUG("JsonRPC::InitOpenParam[Check Point][2]");

    if (PROP_CLIENT == base_rpc_callback_impl_ptr_->GetMode()) {
      YSOS_LOG_DEBUG("JsonRPC::InitOpenParam[Check Point][client mode]");
      json_rpc_connection_callback_impl_ptr_->SetMode(PROP_CLIENT);
    } else {
      YSOS_LOG_DEBUG("JsonRPC::InitOpenParam[Check Point][server mode]");
      json_rpc_connection_callback_impl_ptr_->SetMode(PROP_SERVER);
    }

    YSOS_LOG_DEBUG("JsonRPC::InitOpenParam[Check Point][3]");

    json_rpc_connection_callback_impl_ptr_->SetAddress(base_rpc_callback_impl_ptr_->GetAddress());
    json_rpc_connection_callback_impl_ptr_->SetPort(base_rpc_callback_impl_ptr_->GetPort());
    json_rpc_connection_callback_impl_ptr_->SetBufferLength(CONNECTION_BUFFER_LENGTH);
    json_rpc_connection_callback_impl_ptr_->SetBufferNumber(CONNECTION_BUFFER_NUMBER);
    json_rpc_connection_callback_impl_ptr_->SetEnableWrap(true);
    json_rpc_connection_callback_impl_ptr_->SetEnableRead(true);
    json_rpc_connection_callback_impl_ptr_->SetEnableWrite(true);
    json_rpc_connection_callback_impl_ptr_->SetErrorHandler(boost::bind(&JsonRPC::HandleError, this, _1, _2));
    json_rpc_connection_callback_impl_ptr_->SetDoServiceHandler(boost::bind(&JsonRPC::HandleDoService, this, _1, _2, _3));
    json_rpc_connection_callback_impl_ptr_->SetOnDispatchMessageHandler(boost::bind(&JsonRPC::HandleOnDispatchMessage, this, _1, _2, _3));

    YSOS_LOG_DEBUG("JsonRPC::InitOpenParam[Check Point][4]");

    /// 初始化protocol
    protocol_ptr_ = ProtocolInterfacePtr(PlatformProtocolImpl::CreateInstance());
    if (NULL == protocol_ptr_) {
      YSOS_LOG_DEBUG("JsonRPC::InitOpenParam[Fail][protocol_ptr_ is NULL.]");
      break;
    }

    YSOS_LOG_DEBUG("JsonRPC::InitOpenParam[Check Point][End]");

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  YSOS_LOG_DEBUG("JsonRPC::InitOpenParam[Exit]");

  return result;
}

void JsonRPC::GetSerialNumber(std::string& serial_number) {
  char temp_serial_number[SERIAL_NUMBER_LENGTH] = {0,};
  #ifdef _WIN32
    sprintf_s(temp_serial_number, SERIAL_NUMBER_LENGTH, "%010d", serial_number_);   //need update for linux
  #else
    sprintf(temp_serial_number, "%010d", (int)serial_number_);    //need update for linux
  #endif 
  ++serial_number_;
  serial_number = temp_serial_number;
  return;
}

}
