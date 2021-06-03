/**
  *@file jsonrpcconnectioncallbackimpl.cpp
  *@brief Implement of jsonrpcconnectioncallbackimpl
  *@version 0.1
  *@author jinchengzhe
  *@date Created on: 2016-09-24 10:30:00
  *@copyright Copyright © 2016 YunShen Technology. All rights reserved.
  * 
  */

/// Private Headers
#include "../../../protect/include/protocol_help_package/jsonrpcconnectioncallbackimpl.h"
/// C++ Standard Headers
#include <string>
/// ThirdParty Headers
#include <boost/thread.hpp>
#include <boost/make_shared.hpp>
/// Platform Headers
#include "../../../public/include/sys_interface_package/error.h"
#include "../../../public/include/os_hal_package/bufferutility.h"
#include "../../../protect/include/protocol_help_package/platformprotocolimpl.h"
#include "../../../public/include/sys_interface_package/messagetype.h"
#include "../../../public/include/core_help_package/utility.h"
#include "../../../public/include/sys_interface_package/ysos_enum.h"

namespace ysos {

JsonRPCConnectionCallbackImpl::JsonRPCConnectionCallbackImpl(const std::string &strClassName /* =JsonRPCConnectionCallbackImpl */)
  : CommonConnectionCallbackImpl(strClassName),
    error_handler_(NULL),
    do_service_handler_(NULL),
    on_dispatch_message_handler_(NULL) {
  if (YSOS_ERROR_SUCCESS != Initialize()) {
    YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::Initialize[Fail]The function[Initialize()] is failed");
  }
}

JsonRPCConnectionCallbackImpl::~JsonRPCConnectionCallbackImpl() {
  if (NULL != protocol_ptr_) {
    protocol_ptr_ = NULL;
  }
}

int JsonRPCConnectionCallbackImpl::Initialize(void *param) {
  YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::Initialize[Enter]");

  if (NULL == protocol_ptr_) {
    YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::Initialize[Check Point][protocol_ptr_ is NULL]");
    protocol_ptr_ = ProtocolInterfacePtr(PlatformProtocolImpl::CreateInstance());
    if (NULL == protocol_ptr_) {
      YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::Initialize[Fail][protocol_ptr_ is NULL]");
      return YSOS_ERROR_FAILED;
    }
  }

  YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::Initialize[Exit]");

  return YSOS_ERROR_SUCCESS;
}

void JsonRPCConnectionCallbackImpl::HandleAccept(const boost::system::error_code& error_code, const ConnectionInterfacePtr& connection_ptr) {
  YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleAccept[Enter]");

  int result = YSOS_ERROR_FAILED;

  do {
    if (0 != error_code) {
      YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleAccept[Error Message]: " << error_code.message());
      break;
    }

    YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleAccept[Check Point][0]");

    /// 创建新的ConnectionCallbackImpl实例
    JsonRPCConnectionCallbackImplPtr json_rpc_connection_callback_impl_ptr = JsonRPCConnectionCallbackImplPtr(JsonRPCConnectionCallbackImpl::CreateInstance());
    if (NULL == json_rpc_connection_callback_impl_ptr) {
      YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleAccept[Fail][json_rpc_connection_callback_impl_ptr is NULL]");
      break;
    }

    YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleAccept[Check Point][1]");

    /// 初始化新的ConnectionCallbackImpl实例
    json_rpc_connection_callback_impl_ptr->SetMode(PROP_SERVER_TERMINAL);
    json_rpc_connection_callback_impl_ptr->SetErrorHandler(GetErrorHandler());
    json_rpc_connection_callback_impl_ptr->SetDoServiceHandler(GetDoServiceHandler());
    json_rpc_connection_callback_impl_ptr->SetOnDispatchMessageHandler(GetOnDispatchMessageHandler());
    json_rpc_connection_callback_impl_ptr->SetBufferLength(this->GetBufferLength());
    json_rpc_connection_callback_impl_ptr->SetBufferNumber(this->GetBufferNumber());

    YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleAccept[Check Point][2]");

    /// 调用Connection层SetProperty，初始化Connection的Callback
    if (YSOS_ERROR_SUCCESS != connection_ptr->SetProperty(PROP_CALLBACK, &json_rpc_connection_callback_impl_ptr)) {
      YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleAccept[Fail]The function[connection_ptr->SetProperty()] is failed");
      break;
    }

    YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleAccept[Check Point][3]");

    /// 调用Connection层SetProperty，触发Connection层读循环
    if (YSOS_ERROR_SUCCESS != connection_ptr->SetProperty(PROP_READ, NULL)) {
      YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleAccept[Fail]The function[connection_ptr->SetProperty()] is failed");
      break;
    }

    YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleAccept[Check Point][4]");

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  if (YSOS_ERROR_SUCCESS != result) {
    YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleAccept[Fail]");
    HandleError(error_code, PROP_ON_ACCEPT, connection_ptr);
  }

  YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleAccept[Exit]");

  return;
}

void JsonRPCConnectionCallbackImpl::HandleConnect(const boost::system::error_code& error_code, const ConnectionInterfacePtr& connection_ptr) {
  YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleConnect[Enter]");

  int result = YSOS_ERROR_FAILED;

  do {
    if (0 != error_code) {
      YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleConnect[Error Message]: " << error_code.message());
      break;
    }

    YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleConnect[Check Point][0]");

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  if (YSOS_ERROR_SUCCESS != result) {
    YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleConnect[Fail]");
    HandleError(error_code, PROP_ON_CONNECT, connection_ptr);
  }

  YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleConnect[Exit]");

  return;
}

void JsonRPCConnectionCallbackImpl::HandleDisconnect(const boost::system::error_code& error_code, const ConnectionInterfacePtr& connection_ptr) {
  YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleDisconnect[Enter]");
  HandleError(error_code, PROP_ON_DISCONNECT, connection_ptr);
  YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleDisconnect[Exit]");
  return;
}

void JsonRPCConnectionCallbackImpl::HandleRead(const BufferInterfacePtr& buffer_ptr, const ConnectionInterfacePtr& connection_ptr) {
  YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleRead[Enter]");

  int result = YSOS_ERROR_FAILED;

  do {
    /// 获取读取消息内容
    uint8_t* buffer_data_ptr = GetBufferUtility()->GetBufferData(buffer_ptr);
    if (NULL == buffer_data_ptr) {
      YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleRead[Fail][buffer_data_ptr is NULL]");
      break;
    }

    YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleRead[Check Point][0]");

    /// 判断读取消息是否为空
    if (0 == strlen((char*)buffer_data_ptr)) {
      YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleRead[Fail][The size of buffer_data_ptr is 0]");
      result = YSOS_ERROR_SUCCESS;
      break;
    }

    /// for debug
    YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleRead()[Read Data]: " << (char*)buffer_data_ptr);

    /// 一次解析读取消息数据，判断服务类型
    if (NULL == protocol_ptr_) {
      YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleRead[Fail][protocol_ptr_ is NULL]");
      break;
    }

    YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleRead[Check Point][1]");

    PlatformProtocolImpl::NormalMessagePack message_pack;
    if (YSOS_ERROR_SUCCESS != protocol_ptr_->ParseMessage(buffer_ptr, NULL, PROP_NORMAL, &message_pack)) {
      YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleRead[Fail]The function[protocol_ptr_->ParseMessage()] is failed");
      break;
    }

    /// 获取读取消息服务类型
    UINT32 message_type = boost::lexical_cast<unsigned int>(message_pack.message_header.type);

    YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleRead[message_type][" << message_type << "]");

    /// 判断是请求消息
    if (g_platform_protocolimpl_content_tag_request_verb == message_pack.message_body.verb) {
      YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleRead[Request][Check Point][Enter]");
      switch (message_type) {
      case YSOS_AGENT_MESSAGE_HEART_BEAT_REQ: {
        /// 心跳请求服务，由客户端发送，服务器端点读取
        YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleRead[Request][YSOS_AGENT_MESSAGE_HEART_BEAT_REQ][Enter]");

        /// 服务器端点读取
        if (PROP_SERVER_TERMINAL == GetMode()) {
          YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleRead[Request][PROP_SERVER_TERMINAL][Enter]");

          /// 将心跳数据添加到该connection的心跳读取工作队列，当前仅取时间戳
          if (YSOS_ERROR_SUCCESS != connection_ptr->SetProperty(PROP_READ_HEARTBEAT, &(message_pack.message_header.time_stamp))) {
            YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleRead[Request][Fail]The function[connection_ptr->SetProperty()] is failed");
            break;
          }

          YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleRead[Request][PROP_SERVER_TERMINAL][End]");
        }

        YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleRead[Request][YSOS_AGENT_MESSAGE_HEART_BEAT_REQ][End]");

        result = YSOS_ERROR_SUCCESS;
      }
      break;
      case YSOS_AGENT_MESSAGE_INIT_CONNECTION_REQ:  ///< _CERTUSNET
      case YSOS_AGENT_MESSAGE_INIT_REQ: {
        /// 初始化请求服务
        YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleRead[Request][YSOS_AGENT_MESSAGE_INIT_CONNECTION_REQ/YSOS_AGENT_MESSAGE_INIT_REQ][Enter]");

        /// 服务器端点模式下，生成session id或者重定向
        if (PROP_SERVER_TERMINAL == GetMode()) {
          YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleRead[Request][PROP_SERVER_TERMINAL][Enter]");

          /// 进一步解析数据
          PlatformProtocolImpl::RequestMessagePack request_message_pack;
          if (YSOS_ERROR_SUCCESS != protocol_ptr_->ParseMessage(buffer_ptr, NULL, PROP_REQUEST, &request_message_pack)) {
            YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleRead[Request][Fail]The function[protocol_ptr_->ParseMessage()] is failed");
            break;
          }

          YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleRead[Request][session_id][" << request_message_pack.message_header.session_id << "]");

          /// 判断session id是空或者为0，则是客户端请求建立新的连接
          if (true == request_message_pack.message_header.session_id.empty() ||
              "0" == request_message_pack.message_header.session_id) {
            YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleRead[Request][session_id is empty][Generate new session id]");

            /// 生成新的sessin id，并插入session id和connection指针匹配表
            if (YSOS_ERROR_SUCCESS != connection_ptr->SetProperty(PROP_NEW_SESSIONID, NULL)) {
              YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleRead[Request][Fail]The function[connection_ptr->SetProperty()] is failed");
              break;
            }

            YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleRead[Request][Check Point][PROP_SERVER_TERMINAL][0]");

            /// 获取新的session id
            std::string new_session_id;
            if (YSOS_ERROR_SUCCESS != connection_ptr->GetProperty(PROP_SESSIONID, &new_session_id)) {
              YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleRead[Request][Fail]The function[connection_ptr->SetProperty()] is failed");
              break;
            }

            YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleRead[Request][Check Point][PROP_SERVER_TERMINAL][1]");

            if (true == new_session_id.empty()) {
              YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleRead[Request][Fail][new_session_id is empty]");
              break;
            }

            YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleRead[Request][new_session_id][" << new_session_id << "]");

            /// 设置返回给客户端的session id
            request_message_pack.message_header.session_id = new_session_id;
          }
          /// 判断session id不为空，则是客户端请求重新建立连接
          else {
            YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleRead[Request][session_id is note empty][Update connection ptr]");

            /// 根据客户端发送的session id查找是否有匹配，如果有，则更新connection ptr
            if (YSOS_ERROR_SUCCESS != connection_ptr->SetProperty(PROP_SESSIONID, &(request_message_pack.message_header.session_id))) {
              YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleRead[Request][Fail]The function[connection_ptr->SetProperty()] is failed");
              break;
            }
          }

          YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleRead[Request][Check Point][PROP_SERVER_TERMINAL][2]");

          /// 调用jsonrpc层HandleDoService
          if (NULL != do_service_handler_) {
            YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleRead[Request][Check Point][do_service_handler_ is not NULL]");
            do_service_handler_(request_message_pack.message_header.session_id, request_message_pack.message_body.param.service_name, request_message_pack.message_body.param.service_param);
          }

          YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleRead[Request][PROP_SERVER_TERMINAL][End]");
        }

        YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleRead[Request][YSOS_AGENT_MESSAGE_INIT_CONNECTION_REQ/YSOS_AGENT_MESSAGE_INIT_REQ][End]");

        result = YSOS_ERROR_SUCCESS;
      }
      break;
      default: {
        YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleRead[Check Point][2]");

        /// 进一步解析数据
        PlatformProtocolImpl::RequestMessagePack request_message_pack;
        if (YSOS_ERROR_SUCCESS != protocol_ptr_->ParseMessage(buffer_ptr, NULL, PROP_REQUEST, &request_message_pack)) {
          YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleRead[Request][Fail]The function[protocol_ptr_->ParseMessage()] is failed");
          break;
        }

        YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleRead[Check Point][3]");

        /// 调用jsonrpc层HandleDoService
        if (NULL != do_service_handler_) {
          YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleRead[Request][Check Point][do_service_handler_ is not NULL]");
          do_service_handler_(request_message_pack.message_header.session_id, request_message_pack.message_body.param.service_name, request_message_pack.message_body.param.service_param);
        }

        YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleRead[Check Point][4]");

        result = YSOS_ERROR_SUCCESS;
      }
      break;
      }

      YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleRead[Request][Check Point][End]");
    }
    /// 判断是响应消息
    else if (g_platform_protocolimpl_content_tag_response_verb == message_pack.message_body.verb) {
      YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleRead[Response][Check Point][Enter]");
      switch (message_type) {
      case YSOS_AGENT_MESSAGE_HEART_BEAT_RSP: {
        /// 心跳响应服务，由服务器端点发送，客户端读取
        YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleRead[Response][YSOS_AGENT_MESSAGE_HEART_BEAT_RSP][Enter]");

        /// 客户端读取
        if (PROP_CLIENT == GetMode()) {
          YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleRead[Response][PROP_CLIENT][Enter]");

          /// 将心跳数据添加到该connection的读取心跳工作队列
          if (YSOS_ERROR_SUCCESS != connection_ptr->SetProperty(PROP_READ_HEARTBEAT, &(message_pack.message_header.time_stamp))) {
            YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleRead[Response][Fail]The function[connection_ptr->SetProperty()] is failed");
            break;
          }
          YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleRead[Response][PROP_CLIENT][End]");
        }

        YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleRead[Response][YSOS_AGENT_MESSAGE_HEART_BEAT_RSP][End]");

        result = YSOS_ERROR_SUCCESS;
      }
      break;
      case YSOS_AGENT_MESSAGE_INIT_CONNECTION_RSP:  ///< _CERTUSNET
      case YSOS_AGENT_MESSAGE_INIT_RSP: {
        /// 初始化响应服务
        YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleRead[Response][Check Point][YSOS_AGENT_MESSAGE_INIT_CONNECTION_RSP/YSOS_AGENT_MESSAGE_INIT_RSP][Enter]");

        /// 客户端模式下，保存session id
        if (PROP_CLIENT == GetMode()) {
          YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleRead[Response][PROP_CLIENT][Enter]");

          if (YSOS_ERROR_SUCCESS != connection_ptr->SetProperty(PROP_SESSIONID, &(message_pack.message_header.session_id))) {
            YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleRead[Response][Fail]The function[connection_ptr->SetProperty()] is failed");
            break;
          }

          YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleRead[Response][YSOS_AGENT_MESSAGE_INIT_CONNECTION_RSP/YSOS_AGENT_MESSAGE_INIT_RSP][Check Point][0]");

          /// 客户端模式下，收到初始化响应请求后，启动心跳机制
          if (YSOS_ERROR_SUCCESS != connection_ptr->SetProperty(PROP_HEARTBEAT, NULL)) {
            YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleRead[Fail]The function[connection_ptr->SetProperty()] is failed");
            break;
          }

          YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleRead[Response][PROP_CLIENT][End]");
        }

        YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleRead[Response][YSOS_AGENT_MESSAGE_INIT_CONNECTION_RSP/YSOS_AGENT_MESSAGE_INIT_RSP][Check Point][1]");

        /// 进一步解析数据
        PlatformProtocolImpl::ResponseMessagePack response_message_pack;
        if (YSOS_ERROR_SUCCESS != protocol_ptr_->ParseMessage(buffer_ptr, NULL, PROP_RESPONSE, &response_message_pack)) {
          YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleRead[Response][Fail]The function[protocol_ptr_->ParseMessage()] is failed");
          break;
        }

        YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleRead[Response][YSOS_AGENT_MESSAGE_INIT_CONNECTION_RSP/YSOS_AGENT_MESSAGE_INIT_RSP][Check Point][2]");

        /// 调用jsonrpc层HandleOnDispatchMessage
        if (NULL != on_dispatch_message_handler_) {
          YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleRead[Response][Check Point][on_dispatch_message_handler_ is not NULL]");
          on_dispatch_message_handler_(response_message_pack.message_header.session_id, response_message_pack.message_body.param.service_name, response_message_pack.message_body.param.result_string);
        }

        YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleRead[Response][YSOS_AGENT_MESSAGE_INIT_CONNECTION_RSP/YSOS_AGENT_MESSAGE_INIT_RSP][End]");

        result = YSOS_ERROR_SUCCESS;
      }
      break;
      default: {
        YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleRead[Check Point][5]");

        /// 进一步解析数据
        PlatformProtocolImpl::ResponseMessagePack response_message_pack;
        if (YSOS_ERROR_SUCCESS != protocol_ptr_->ParseMessage(buffer_ptr, NULL, PROP_RESPONSE, &response_message_pack)) {
          YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleRead[Response][Fail]The function[protocol_ptr_->ParseMessage()] is failed");
          break;
        }

        YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleRead[Check Point][6]");

        /// 调用jsonrpc层HandleOnDispatchMessage
        if (NULL != on_dispatch_message_handler_) {
          YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleRead[Response][Check Point][on_dispatch_message_handler_ is not NULL]");
          on_dispatch_message_handler_(response_message_pack.message_header.session_id, response_message_pack.message_body.param.service_name, response_message_pack.message_body.param.result_string);
        }

        YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleRead[Check Point][7]");

        result = YSOS_ERROR_SUCCESS;
      }
      break;
      }

      YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleRead[Response][Check Point][End]");
    }
    /// 判断是其他消息
    else {
      YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleRead[Fail][Unknown Verb]");
      result = YSOS_ERROR_NOT_SUPPORTED;
      break;
    }

    YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleRead[Check Point][End]");

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  if (YSOS_ERROR_SUCCESS != result) {
    YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleRead[Fail]");
    boost::system::error_code error_code;
    HandleError(error_code, PROP_ON_READ, connection_ptr);
  }

  YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleRead[Exit]");

  return;
}

void JsonRPCConnectionCallbackImpl::HandleWrite(const BufferInterfacePtr& buffer_ptr, const ConnectionInterfacePtr& connection_ptr) {
  YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleWrite[Enter]");

  int result = YSOS_ERROR_FAILED;

  do {
    uint8_t* buffer_data_ptr = GetBufferUtility()->GetBufferData(buffer_ptr);
    if (NULL == buffer_data_ptr) {
      YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleWrite[Fail][buffer_data_ptr is NULL]");
      break;
    }

    YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleWrite[Check Point][0]");

    if (0 == strlen((char*)buffer_data_ptr)) {
      YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleWrite[Fail][The size of buffer_data_ptr is 0]");
      result = YSOS_ERROR_SUCCESS;
      break;
    }

    /// for debug
    YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleWrite()[Write Data]: " << (char*)buffer_data_ptr);

    /// 解析数据
    if (NULL == protocol_ptr_) {
      YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleWrite[Fail][protocol_ptr_ is NULL]");
      break;
    }

    YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleWrite[Check Point][1]");

    PlatformProtocolImpl::NormalMessagePack message_pack;
    if (YSOS_ERROR_SUCCESS != protocol_ptr_->ParseMessage(buffer_ptr, NULL, PROP_NORMAL, &message_pack)) {
      YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleWrite[Fail]The function[protocol_ptr_->ParseMessage()] is failed");
      break;
    }

    /// 写入消息服务类型
    UINT32 message_type = boost::lexical_cast<unsigned int>(message_pack.message_header.type);

    YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleWrite[message_type][" << message_type << "]");

    /// 判断是请求消息
    if (g_platform_protocolimpl_content_tag_request_verb == message_pack.message_body.verb) {
      YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleWrite[Request][Check Point][Enter]");

      switch (message_type) {
      case YSOS_AGENT_MESSAGE_HEART_BEAT_REQ: {
        /// 心跳请求服务
        YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleWrite[Request][YSOS_AGENT_MESSAGE_HEART_BEAT_REQ][Enter]");

        if (PROP_CLIENT == GetMode()) {
          YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleWrite[Response][PROP_CLIENT][Enter]");

          /// 将心跳数据添加到该connection的写入心跳工作队列
          if (YSOS_ERROR_SUCCESS != connection_ptr->SetProperty(PROP_WRITE_HEARTBEAT, &(message_pack.message_header.time_stamp))) {
            YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleWrite[Fail]The function[connection_ptr->SetProperty()] is failed");
            break;
          }

          YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleWrite[Response][PROP_CLIENT][End]");
        }

        YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleWrite[Request][YSOS_AGENT_MESSAGE_HEART_BEAT_REQ][End]");

        result = YSOS_ERROR_SUCCESS;
      }
      break;
      default:
        break;
      }

      YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleWrite[Request][Check Point][End]");
    }
    /// 判断是响应消息
    else if (g_platform_protocolimpl_content_tag_response_verb == message_pack.message_body.verb) {
      YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleWrite[Response][Check Point][Enter]");

      switch (message_type) {
      case YSOS_AGENT_MESSAGE_INIT_CONNECTION_RSP:  ///< _CERTUSNET
      case YSOS_AGENT_MESSAGE_INIT_RSP: {
        /// 初始化响应服务
        YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleWrite[Response][YSOS_AGENT_MESSAGE_INIT_CONNECTION_RSP/YSOS_AGENT_MESSAGE_INIT_RSP][Enter]");

        /// 服务器模式端点模式下，响应客户端初始化服务请求后，启动心跳机制
        if (PROP_SERVER_TERMINAL == GetMode()) {
          YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleWrite[Response][PROP_SERVER_TERMINAL][Enter]");

          if (YSOS_ERROR_SUCCESS != connection_ptr->SetProperty(PROP_HEARTBEAT, NULL)) {
            YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleWrite[Fail]The function[connection_ptr->SetProperty()] is failed");
            break;
          }

          YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleWrite[Response][PROP_SERVER_TERMINAL][End]");
        }

        YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleWrite[Response][YSOS_AGENT_MESSAGE_INIT_CONNECTION_RSP/YSOS_AGENT_MESSAGE_INIT_RSP][End]");

        result = YSOS_ERROR_SUCCESS;
      }
      break;
      case YSOS_AGENT_MESSAGE_UNINIT_CONNECTION_RSP:  ///< _CERTUSNET
      case YSOS_AGENT_MESSAGE_UNINIT_RSP: {
        /// 注销响应服务，由服务器端点及客户端都执行
        YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleWrite[Response][YSOS_AGENT_MESSAGE_UNINIT_CONNECTION_RSP/YSOS_AGENT_MESSAGE_UNINIT_RSP][Enter]");

        /// 调用connection层SetProperty，主动把自己添加到无效连接队列里，等待顶层基类处理
        ConnectionInterfacePtr& temp_connection_ptr = const_cast<ConnectionInterfacePtr&>(connection_ptr);
        if (YSOS_ERROR_SUCCESS != connection_ptr->SetProperty(PROP_INVALID_CONNECTION, &temp_connection_ptr)) {
          YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleWrite[Fail]The function[connection_ptr->SetProperty()] is failed");
          break;
        }

        YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleWrite[Response][YSOS_AGENT_MESSAGE_UNINIT_CONNECTION_RSP/YSOS_AGENT_MESSAGE_UNINIT_RSP][End]");

        result = YSOS_ERROR_SUCCESS;
      }
      break;
      case YSOS_AGENT_MESSAGE_HEART_BEAT_RSP: {
        /// 心跳响应服务，由服务器端点发送，客户端读取
        YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleWrite[Response][YSOS_AGENT_MESSAGE_HEART_BEAT_RSP][Enter]");

        /// 服务器端点写入
        if (PROP_SERVER_TERMINAL == GetMode()) {
          YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleWrite[Response][PROP_SERVER_TERMINAL][Enter]");

          /// 将心跳数据添加到该connection的写入心跳工作队列
          if (YSOS_ERROR_SUCCESS != connection_ptr->SetProperty(PROP_WRITE_HEARTBEAT, &(message_pack.message_header.time_stamp))) {
            YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleWrite[Fail]The function[connection_ptr->SetProperty()] is failed");
            break;
          }

          YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleWrite[Response][PROP_SERVER_TERMINAL][End]");
        }

        YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleWrite[Response][YSOS_AGENT_MESSAGE_HEART_BEAT_RSP][End]");

        result = YSOS_ERROR_SUCCESS;
      }
      break;
      default:
        break;
      }

      YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleWrite[Response][Check Point][End]");
    }
    /// 判断是其他消息
    else {
      YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleWrite[Fail][Unknown Verb]");
      result = YSOS_ERROR_NOT_SUPPORTED;
      break;
    }

    YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleWrite[Check Point][End]");

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  if (YSOS_ERROR_SUCCESS != result) {
    YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleWrite[Fail]");
    boost::system::error_code error_code;
    HandleError(error_code, PROP_ON_WRITE, connection_ptr);
  }

  YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleWrite[Exit]");

  return;
}

void JsonRPCConnectionCallbackImpl::HandleError(const boost::system::error_code& error_code, const UINT32 callback_type, const ConnectionInterfacePtr& connection_ptr) {
  YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleError[Enter]");

  int result = YSOS_ERROR_FAILED;

  do {
    YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleError[Check Point][0]");

    /// 获取session id
    std::string session_id;
    if (YSOS_ERROR_SUCCESS != connection_ptr->GetProperty(PROP_SESSIONID, &session_id)) {
      YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleRead[Request][Fail]The function[connection_ptr->SetProperty()] is failed");
      break;
    }

    YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleError[Check Point][session_id][" << session_id << "]");

    YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleError[Check Point][1]");

    CommonConnectionCallbackImpl::ConnectionCallbackErrorParam error_param;
    switch (callback_type) {
    case PROP_ON_ACCEPT: {
      error_param.callback_type = "OnAccept";
    }
    break;
    case PROP_ON_CONNECT: {
      error_param.callback_type = "OnConnect";
    }
    break;
    case PROP_ON_READ: {
      error_param.callback_type = "OnRead";
    }
    break;
    case PROP_ON_WRITE: {
      error_param.callback_type = "OnWrite";
    }
    break;
    case PROP_ON_DISCONNECT: {
      error_param.callback_type = "OnDisconnect";
    }
    break;
    default: {
      error_param.callback_type = "Unknown";
    }
    break;
    }

    YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleError[Check Point][2]");

    ConnectionInterface::ConnectionErrorCode connection_error_code;
    connection_error_code.boost_asio_error_code = error_code.value();
    if (YSOS_ERROR_SUCCESS != connection_ptr->GetProperty(PROP_ERROR_CODE, &connection_error_code)) {
      YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleError[Fail]The function[connection_ptr->GetProperty()] is failed");
      break;
    }

    YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleError[Check Point][3]");

    error_param.error_code = connection_error_code.ysos_error_code;
    error_param.description = error_code.message();

    std::string error_param_string;
    if (true != error_param.ToString(error_param_string)) {
      YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleError[Fail]The function[error_param.ToString()] is failed");
    }

    YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleError[session_id] [" << session_id << "]" << " [error_param_string] [" << error_param_string << "]");

    if (NULL != error_handler_) {
      YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleError[Check Point][error_handler_ is not NULL]");
      error_handler_(session_id, error_param_string);
    }

    result = YSOS_ERROR_SUCCESS;

  } while (0);

  YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::HandleError[Exit]");

  return;
}

void JsonRPCConnectionCallbackImpl::SetErrorHandler(const ErrorHandler error_handler) {
  error_handler_ = error_handler;
  return;
}

JsonRPCConnectionCallbackImpl::ErrorHandler JsonRPCConnectionCallbackImpl::GetErrorHandler(void) {
  return error_handler_;
}

void JsonRPCConnectionCallbackImpl::SetDoServiceHandler(const DoServiceHandler do_service_handler) {
  do_service_handler_ = do_service_handler;
  return;
}

JsonRPCConnectionCallbackImpl::DoServiceHandler JsonRPCConnectionCallbackImpl::GetDoServiceHandler(void) {
  return do_service_handler_;
}

void JsonRPCConnectionCallbackImpl::SetOnDispatchMessageHandler(const OnDispatchMessageHandler on_dispatch_message_handler) {
  on_dispatch_message_handler_ = on_dispatch_message_handler;
  return;
}

JsonRPCConnectionCallbackImpl::OnDispatchMessageHandler JsonRPCConnectionCallbackImpl::GetOnDispatchMessageHandler(void) {
  return on_dispatch_message_handler_;
}

}
