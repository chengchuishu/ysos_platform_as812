/*
# commonconnectionimpl.cpp
# Definition of commonconnectionimpl
# Created on: 2016-05-26 09:59:58
# Original author: jinchengzhe
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
# version 0.1, 20160912, created by JinChengZhe
*/

/// Private Headers
#include "../../../protect/include/protocol_help_package/commonconnectionimpl.h"
/// ThirdParty Headers
#include <boost/bind.hpp>
#include <boost/make_shared.hpp>
/// Platform Headers
#include "../../../public/include/sys_interface_package/messagetype.h"
#include "../../../public/include/sys_interface_package/ysos_enum.h"
#include "../../../public/include/os_hal_package/socket.h"
#include "../../../protect/include/core_help_package/bufferpoolimpl.h"
#include "../../../protect/include/core_help_package/bufferimpl.h"
#include "../../../public/include/os_hal_package/lock.h"
#include "../../../public/include/os_hal_package/bufferutility.h"
#include "../../../public/include/core_help_package/utility.h"
#include "../../../protect/include/protocol_help_package/platformprotocolimpl.h"

namespace ysos {

/**
*@brief 拼装日志报文使用全局变量  // NOLINT
*/
const char* const g_request_service_param_log_content = "log_content";

#if _CERTUSNET
const char* const g_response_result_status_code = "status";
#else
const char* const g_response_result_status_code = "status_code";
#endif

const char* const g_response_result_description = "description";
const char* const g_response_result_detail = "detail";

/**
*@brief CommonConnectionImpl的具体实现  // NOLINT
*/
CommonConnectionImpl::CommonConnectionImpl(const std::string &strClassName /* =CommonConnectionImpl */)
  : BaseInterfaceImpl(strClassName),
    transport_ptr_(NULL),
    connection_callback_impl_ptr_(NULL),
    transport_callback_ptr_(NULL),
    invalid_connection_worker_ptr_(NULL),
    write_worker_ptr_(NULL),
    running_status_(IS_INITIAL),
    mode_(0),
    port_(0),
    address_(""),
    enable_wrap_(true),
    enable_write_(true),
    enable_read_(true),
    root_connection_ptr_(NULL),
    read_worker_ptr_(NULL),
    buffer_pool_ptr_(NULL),
    protocol_ptr_(NULL),
    read_heartbeat_timer_(io_service_),
    write_heartbeat_timer_(io_service_),
    serial_number_(0),
    legitimate_inconnection_times_counter_(0),
    read_heartbeat_deadline_time_(READ_HEARTBEAT_DEADLINE),
    write_heartbeat_deadline_time_(WRITE_HEARTBEAT_DEADLINE),
    write_heartbeat_worker_ptr_(NULL) {
  logger_ = GetUtility()->GetLogger("ysos.sdk");
}

CommonConnectionImpl::CommonConnectionImpl(const ConnectionInterfacePtr& root_connection_ptr, const TransportInterfacePtr& transport_ptr)
  : transport_ptr_(transport_ptr),
    connection_callback_impl_ptr_(NULL),
    transport_callback_ptr_(NULL),
    invalid_connection_worker_ptr_(NULL),
    write_worker_ptr_(NULL),
    running_status_(IS_RUNNING),
    mode_(PROP_SERVER_TERMINAL),
    port_(0),
    address_(""),
    enable_wrap_(true),
    enable_write_(true),
    enable_read_(true),
    root_connection_ptr_(root_connection_ptr),
    read_worker_ptr_(NULL),
    buffer_pool_ptr_(NULL),
    protocol_ptr_(NULL),
    read_heartbeat_timer_(io_service_),
    write_heartbeat_timer_(io_service_),
    serial_number_(0),
    legitimate_inconnection_times_counter_(0),
    read_heartbeat_deadline_time_(READ_HEARTBEAT_DEADLINE),
    write_heartbeat_deadline_time_(WRITE_HEARTBEAT_DEADLINE),
    write_heartbeat_worker_ptr_(NULL) {
  logger_ = GetUtility()->GetLogger("ysos.sdk");

  /// 服务器端点模式下，直接初始化读取消息处理线程并运行
  read_worker_ptr_ = boost::make_shared<Worker<ReadJobQueue>>(read_job_queue_, boost::bind(&CommonConnectionImpl::ProcessReadMessage, this, _1));
  read_worker_ptr_->Start();

  /// 服务器端点模式下，直接初始化写入心跳消息处理线程并运行
  write_heartbeat_worker_ptr_ = boost::make_shared<Worker<HeartbeatJobQueue>>(write_heartbeat_job_queue_, boost::bind(&CommonConnectionImpl::ProcessWriteHeartbeatMessage, this, _1));
  write_heartbeat_worker_ptr_->Start();

  /// 服务器端点模式下，直接初始化协议类，用于心跳操作
  protocol_ptr_ = ProtocolInterfacePtr(PlatformProtocolImpl::CreateInstance());
}

CommonConnectionImpl::~CommonConnectionImpl() {
  /// 顶层基类和子类共同使用成员变量
  transport_ptr_ = NULL;
  connection_callback_impl_ptr_ = NULL;

  /// 顶层基类使用成员变量
  transport_callback_ptr_ = NULL;

  connection_ptr_list_.clear();
  invalid_connection_job_queue_.Clear();
  invalid_connection_worker_ptr_ = NULL;

  protocol_map_.clear();

  write_job_queue_.Clear();
  write_worker_ptr_ = NULL;

  session_id_matching_table_.clear();

  running_status_ = IS_INITIAL;

  mode_ = 0;
  port_ = 0;
  address_.clear();

  enable_wrap_ = false;
  enable_read_ = false;
  enable_write_ = false;

  /// 子类使用成员变量
  root_connection_ptr_ = NULL;

  read_job_queue_.Clear();
  read_worker_ptr_ = NULL;

  connection_ptr_list_.clear();

  protocol_ptr_ = NULL;

  buffer_pool_ptr_ = NULL;

  serial_number_ = 0;
  read_heartbeat_deadline_time_ = 0;
  write_heartbeat_deadline_time_ = 0;
  legitimate_inconnection_times_counter_ = 0;

  read_heartbeat_job_queue_.Clear();

  write_heartbeat_job_queue_.Clear();
  write_heartbeat_worker_ptr_ = NULL;
}

int CommonConnectionImpl::InitOpenParam(void *params) {
  YSOS_LOG_DEBUG("CommonConnectionImpl::InitOpenParam[Enter]");

  int result = YSOS_ERROR_FAILED;

  do {
    /// 重置运行状态标识符
    SetRunningStatus(IS_INITIAL);

    if (NULL == params) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::InitOpenParam[Fail][params is NULL]");
      result = YSOS_ERROR_INVALID_ARGUMENTS;
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::InitOpenParam[Check Point][0]");

    /// 强转为connectioncallback，复制给成员变量
    ConnectionCallbackInterfacePtr* connection_callback_ptr_ptr = static_cast<ConnectionCallbackInterfacePtr*>(params);
    connection_callback_impl_ptr_ = *connection_callback_ptr_ptr;

    YSOS_LOG_DEBUG("CommonConnectionImpl::InitOpenParam[Check Point][1]");

    /// mode
    mode_ = connection_callback_impl_ptr_->GetMode();

    /// 客户端模式下初始化
    if (PROP_CLIENT == mode_) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::InitOpenParam[Client Mode]");

      /// 创建及初始化内存池
      if (0 == connection_callback_impl_ptr_->GetBufferLength() ||
          0 == connection_callback_impl_ptr_->GetBufferNumber()) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::InitOpenParam[Fail][The parameters of buffer is invalied]");
        result = YSOS_ERROR_INVALID_ARGUMENTS;
        break;
      }

      YSOS_LOG_DEBUG("CommonConnectionImpl::InitOpenParam[Client Mode][Check Point][0]");

      buffer_pool_ptr_ = GetBufferUtility()->CreateBufferPool(connection_callback_impl_ptr_->GetBufferLength(), connection_callback_impl_ptr_->GetBufferNumber());
      if (NULL == buffer_pool_ptr_) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::InitOpenParam[Fail]The function[GetBufferUtility()->CreateBufferPool()] is failed");
        break;
      }

      YSOS_LOG_DEBUG("CommonConnectionImpl::InitOpenParam[Client Mode][Check Point][1]");

      /// 创建工作线程
      read_worker_ptr_ = boost::make_shared<Worker<ReadJobQueue>>(read_job_queue_, boost::bind(&CommonConnectionImpl::ProcessReadMessage, this, _1));
      read_worker_ptr_->Start();
      write_worker_ptr_ = boost::make_shared<Worker<WriteJobQueue>>(write_job_queue_, boost::bind(&CommonConnectionImpl::ProcessWriteMessage, this, _1));
      write_worker_ptr_->Start();
      write_heartbeat_worker_ptr_ = boost::make_shared<Worker<HeartbeatJobQueue>>(write_heartbeat_job_queue_, boost::bind(&CommonConnectionImpl::ProcessWriteHeartbeatMessage, this, _1));
      write_heartbeat_worker_ptr_->Start();

      YSOS_LOG_DEBUG("CommonConnectionImpl::InitOpenParam[Client Mode][Check Point][2]");

      /// 初始化protocol
      protocol_ptr_ = ProtocolInterfacePtr(PlatformProtocolImpl::CreateInstance());
      if (NULL == protocol_ptr_) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::InitOpenParam[Fail][protocol_ptr_ is NULL.]");
        break;
      }

      YSOS_LOG_DEBUG("CommonConnectionImpl::InitOpenParam[Client Mode][Check Point][3]");

      /// 创建transport
      transport_ptr_ = TransportInterfacePtr(TCPSocketClient::CreateInstance());
      if (NULL == transport_ptr_) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::InitOpenParam[Fail][transport_ptr_ is NULL.]");
        break;
      }

      YSOS_LOG_DEBUG("CommonConnectionImpl::InitOpenParam[Client Mode][Check Point][4]");

      /// 初始化transportcallback，实例是由connectioncallback构造函数中生成
      transport_callback_ptr_ = connection_callback_impl_ptr_->GetTransportCallbackPtr();
      if (NULL == transport_callback_ptr_) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::InitOpenParam[Fail][transport_callback_ptr_ is NULL]");
        break;
      }

      YSOS_LOG_DEBUG("CommonConnectionImpl::InitOpenParam[Client Mode][Check Point][5]");

      /// 将自身绑定掉transportcallback
      transport_callback_ptr_->SetConnectionPtr(this);

      YSOS_LOG_DEBUG("CommonConnectionImpl::InitOpenParam[Client Mode][Check Point][End]");

    }
    /// 服务器模式下初始化
    else if (PROP_SERVER == mode_) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::InitOpenParam[Server Mode]");

      /// 服务器模式下，创建写操作工作线程
      write_worker_ptr_ = boost::make_shared<Worker<WriteJobQueue>>(write_job_queue_, boost::bind(&CommonConnectionImpl::ProcessWriteMessage, this, _1));
      write_worker_ptr_->Start();

      YSOS_LOG_DEBUG("CommonConnectionImpl::InitOpenParam[Server Mode][Check Point][0]");

      /// 创建处理无效连接处理工作线程
      invalid_connection_worker_ptr_ = boost::make_shared<Worker<InvalidConnectionJobQueue>>(invalid_connection_job_queue_, boost::bind(&CommonConnectionImpl::ProcessInvalidConnection, this, _1));
      invalid_connection_worker_ptr_->Start();

      YSOS_LOG_DEBUG("CommonConnectionImpl::InitOpenParam[Server Mode][Check Point][1]");

      /// 创建transport
      transport_ptr_ = TransportInterfacePtr(TCPSocketServer::CreateInstance());
      if (NULL == transport_ptr_) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::InitOpenParam[Fail][transport_ptr_ is NULL.]");
        break;
      }

      YSOS_LOG_DEBUG("CommonConnectionImpl::InitOpenParam[Server Mode][Check Point][2]");

      /// 初始化transportcallback，实例是由connectioncallback构造函数中生成
      transport_callback_ptr_ = connection_callback_impl_ptr_->GetTransportCallbackPtr();
      if (NULL == transport_callback_ptr_) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::InitOpenParam[Fail][transport_callback_ptr_ is NULL]");
        break;
      }

      YSOS_LOG_DEBUG("CommonConnectionImpl::InitOpenParam[Server Mode][Check Point][3]");

      /// 将自身绑定掉transportcallback
      transport_callback_ptr_->SetConnectionPtr(this);

      YSOS_LOG_DEBUG("CommonConnectionImpl::InitOpenParam[Server Mode][Check Point][End]");

    } else {
      YSOS_LOG_DEBUG("CommonConnectionImpl::InitOpenParam[Unknown]");
      result = YSOS_ERROR_NOT_SUPPORTED;
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::InitOpenParam[Check Point][2]");

    /// port
    port_ = connection_callback_impl_ptr_->GetPort();
    if (0 == port_) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::InitOpenParam[Fail][port_ is 0]");
      result = YSOS_ERROR_INVALID_ARGUMENTS;
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::InitOpenParam[Check Point][3]");

    /// address
    address_ = connection_callback_impl_ptr_->GetAddress();
    if (true == address_.empty()) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::InitOpenParam[Fail][address_ is empty]");
      result = YSOS_ERROR_INVALID_ARGUMENTS;
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::InitOpenParam[Check Point][4]");

    /// enable
    EnableRead(connection_callback_impl_ptr_->GetEnableRead());
    EnableWrite(connection_callback_impl_ptr_->GetEnableWrite());
    EnableWrap(connection_callback_impl_ptr_->GetEnableWrap());

    YSOS_LOG_DEBUG("CommonConnectionImpl::InitOpenParam[Check Point][End]");

    result = YSOS_ERROR_SUCCESS;

  } while (0);

  YSOS_LOG_DEBUG("CommonConnectionImpl::InitOpenParam[Exit]");

  return result;
}

int CommonConnectionImpl::Open(void *params) {
  YSOS_LOG_DEBUG("CommonConnectionImpl::Open[Enter]");

  int result = YSOS_ERROR_FAILED;

  do {
    /// 判断模块是否已打开
    if (IS_RUNNING == GetRunningStatus()) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[It is already running]");
      result = YSOS_ERROR_SUCCESS;
      break;
    }

    if (NULL == params) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::Open[Fail][params is NULL]");
      result = YSOS_ERROR_INVALID_ARGUMENTS;
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::Open[Check Point][0]");

    /// 初始化参数
    if (YSOS_ERROR_SUCCESS != InitOpenParam(params)) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::Open[Fail]The function[InitOpenParam()] is failed");
      result = YSOS_ERROR_INVALID_ARGUMENTS;
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::Open[Check Point][1]");

    if (NULL == transport_callback_ptr_) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::Open[Fail][transport_callback_ptr_ is NULL]");
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::Open[Check Point][2]");

    if (NULL == transport_ptr_) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::Open[Fail][transport_ptr_ is NULL]");
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::Open[Check Point][3]");

    /// 打开transport
    if (YSOS_ERROR_SUCCESS != transport_ptr_->Open(&transport_callback_ptr_)) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::Open[Fail]The funtion[transport_ptr_->Open()] is failed");
      break;
    }

    /// 设置模块运行状态
    SetRunningStatus(IS_RUNNING);

    YSOS_LOG_DEBUG("CommonConnectionImpl::Open[Check Point][End]");

    result = YSOS_ERROR_SUCCESS;

  } while (0);

  YSOS_LOG_DEBUG("CommonConnectionImpl::Open[Exit]");

  return result;
}

void CommonConnectionImpl::Close(void *param) {
  YSOS_LOG_DEBUG("CommonConnectionImpl::Close[Enter]");

  /// 判断模块是否已关闭
  if (IS_STOPPED == GetRunningStatus()) {
    YSOS_LOG_DEBUG("CommonConnectionImpl::Close[It is already stopped]");
    return;
  }

  /// 关闭服务器模式
  if (PROP_SERVER == mode_) {
    YSOS_LOG_DEBUG("CommonConnectionImpl::Close[SERVER][Start]");

    /// 关闭某个节点
    if (NULL != param) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::Close[SERVER][SpecifiedConection][Start]");

      ConnectionInterfacePtr *child_connection_ptr = static_cast<ConnectionInterfacePtr*>(param);

      YSOS_LOG_DEBUG("CommonConnectionImpl::Close[SERVER][SpecifiedConection][Check Point][0]");

      /// 调用关闭一个子连接函数
      CloseChildConnection(*child_connection_ptr);

      YSOS_LOG_DEBUG("CommonConnectionImpl::Close[SERVER][SpecifiedConection][End]");
    }
    /// 关闭整个连接
    else {
      YSOS_LOG_DEBUG("CommonConnectionImpl::Close[SERVER][AllConection][Start]");

      /// 设置模块已停止
      SetRunningStatus(IS_STOPPED);

      /// 关闭无效连接消息处理线程
      if (NULL != invalid_connection_worker_ptr_) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::Close[SERVER][AllConection][Close Invalid Connection Worker][Enter]");
        invalid_connection_worker_ptr_->Stop();
        boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
        YSOS_LOG_DEBUG("CommonConnectionImpl::Close[SERVER][AllConection][Close Invalid Connection Worker][End]");
      }

      YSOS_LOG_DEBUG("CommonConnectionImpl::Close[SERVER][AllConection][Check Point][0]");

      /// 关闭写入消息处理线程
      if (NULL != write_worker_ptr_) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::Close[SERVER][AllConection][Close Write Worker][Enter]");
        write_worker_ptr_->Stop();
        boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
        YSOS_LOG_DEBUG("CommonConnectionImpl::Close[SERVER][AllConection][Close Write Worker][End]");
      }

      YSOS_LOG_DEBUG("CommonConnectionImpl::Close[SERVER][AllConection][Check Point][1]");

      /// 清除 write job queue
      if (0 != write_job_queue_.GetSize()) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::Close[SERVER][AllConection][write_job_queue_ size][" << write_job_queue_.GetSize() << "]");
        write_job_queue_.Clear();
        boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
        YSOS_LOG_DEBUG("CommonConnectionImpl::Close[SERVER][AllConection][write_job_queue_ size][" << write_job_queue_.GetSize() << "]");
      }

      YSOS_LOG_DEBUG("CommonConnectionImpl::Close[SERVER][AllConection][Check Point][2]");

      /// 关闭transport
      if (NULL != transport_ptr_) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::Close[SERVER][AllConection][Close Transport]");
        transport_ptr_->Close();
      }

      YSOS_LOG_DEBUG("CommonConnectionImpl::Close[SERVER][AllConection][Check Point][3]");

      /// 从连接链表里查找所有子连接并关闭
      {
        YSOS_LOG_DEBUG("CommonConnectionImpl::Close[SERVER][AllConection][Close and Remove Child Connections]");

        AutoLockOper lock(&ll_lock_);

        /// 关闭所有子连接
        if (0 < connection_ptr_list_.size()) {
          for (std::list<ConnectionInterfacePtr>::iterator it = connection_ptr_list_.begin();
               it != connection_ptr_list_.end();
               ++it) {
            YSOS_LOG_DEBUG("CommonConnectionImpl::Close[SERVER][AllConection][ChildConnection]");
            (*it)->Close();
          }

          YSOS_LOG_DEBUG("CommonConnectionImpl::Close[SERVER][AllConection][Check Point][4]");

          /// 如果是顶层基类，就清空连接链表
          connection_ptr_list_.clear();

          YSOS_LOG_DEBUG("CommonConnectionImpl::Close[SERVER][AllConection][Check Point][5]");
        }

      }

      YSOS_LOG_DEBUG("CommonConnectionImpl::Close[SERVER][AllConection][Check Point][6]");

      /// 清除session id和connection指针匹配表
      {
        AutoLockOper lock(&ll_lock_);
        session_id_matching_table_.clear();
      }

      YSOS_LOG_DEBUG("CommonConnectionImpl::Close[SERVER][AllConection][End]");
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::Close[SERVER][End]");
  }
  /// 关闭客户端模式
  else if (PROP_CLIENT == mode_ || PROP_SERVER_TERMINAL == mode_) {
    YSOS_LOG_DEBUG("CommonConnectionImpl::Close[CLIENT/PROP_SERVER_TERMINAL][Start]");

    /// 设置模块已停止
    SetRunningStatus(IS_STOPPED);

    YSOS_LOG_DEBUG("CommonConnectionImpl::Close[mode_][" << (mode_ == PROP_CLIENT ? "CLIENT" : "SERVERTERMINAL")  << "]");

    /// 停止心跳io service
    io_service_.stop();

    YSOS_LOG_DEBUG("CommonConnectionImpl::Close[CLIENT/PROP_SERVER_TERMINAL][Check Point][0]");

    /// 等待心跳io service运行结束
    if (true == run_io_service_thread_.joinable()) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::Close[Check Point][join run_io_service_thread_][Enter]");
      run_io_service_thread_.join();
      YSOS_LOG_DEBUG("CommonConnectionImpl::Close[Check Point][join run_io_service_thread_][End]");
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::Close[CLIENT/PROP_SERVER_TERMINAL][Check Point][1]");

    /// 清除timer
    read_heartbeat_timer_.cancel();
    write_heartbeat_timer_.cancel();

    YSOS_LOG_DEBUG("CommonConnectionImpl::Close[CLIENT/PROP_SERVER_TERMINAL][Check Point][2]");

    /// 清除读取心跳消息工作队列
    if (0 != read_heartbeat_job_queue_.GetSize()) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::Close[Check Point][read_heartbeat_job_queue_ size][" << read_heartbeat_job_queue_.GetSize() << "]");
      read_heartbeat_job_queue_.Clear();
      boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
      YSOS_LOG_DEBUG("CommonConnectionImpl::Close[Check Point][read_heartbeat_job_queue_ size][" << read_heartbeat_job_queue_.GetSize() << "]");
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::Close[CLIENT/PROP_SERVER_TERMINAL][Check Point][3]");

    /// 关闭写入心跳消息处理线程
    if (NULL != write_heartbeat_worker_ptr_) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::Close[Check Point][Close Connection write_heartbeat_worker_ptr_]");
      write_heartbeat_worker_ptr_->Stop();
      boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::Close[CLIENT/PROP_SERVER_TERMINAL][Check Point][4]");

    /// 清除写入心跳消息工作队列
    if (0 != write_heartbeat_job_queue_.GetSize()) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::Close[Check Point][write_heartbeat_job_queue_ size][" << write_heartbeat_job_queue_.GetSize() << "]");
      write_heartbeat_job_queue_.Clear();
      boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
      YSOS_LOG_DEBUG("CommonConnectionImpl::Close[Check Point][write_heartbeat_job_queue_ size][" << write_heartbeat_job_queue_.GetSize() << "]");
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::Close[CLIENT/PROP_SERVER_TERMINAL][Check Point][5]");

    /// 关闭读取消息处理线程
    if (NULL != read_worker_ptr_) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::Close[Check Point][Close Connection read_worker_ptr_]");
      read_worker_ptr_->Stop();
      boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::Close[CLIENT/PROP_SERVER_TERMINAL][Check Point][6]");

    /// 清除读取消息工作队列
    if (0 != read_job_queue_.GetSize()) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::Close[Check Point][read_job_queue_ size][" << read_job_queue_.GetSize() << "]");
      read_job_queue_.Clear();
      boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
      YSOS_LOG_DEBUG("CommonConnectionImpl::Close[Check Point][read_job_queue_ size][" << read_job_queue_.GetSize() << "]");
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::Close[CLIENT/PROP_SERVER_TERMINAL][Check Point][7]");

    if (PROP_CLIENT == mode_) {
      /// 关闭写入消息处理线程
      if (NULL != write_worker_ptr_) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::Close[Check Point][Close Connection write_worker_ptr_]");
        write_worker_ptr_->Stop();
        boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
      }

      YSOS_LOG_DEBUG("CommonConnectionImpl::Close[CLIENT/PROP_SERVER_TERMINAL][Check Point][8]");

      /// 清除job queue
      if (0 != write_job_queue_.GetSize()) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::Close[Check Point][write_job_queue_ size][" << write_job_queue_.GetSize() << "]");
        write_job_queue_.Clear();
        boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
        YSOS_LOG_DEBUG("CommonConnectionImpl::Close[Check Point][write_job_queue_ size][" << write_job_queue_.GetSize() << "]");
      }
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::Close[CLIENT/PROP_SERVER_TERMINAL][Check Point][9]");

    /// 关闭transport
    if (NULL != transport_ptr_) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::Close[Check Point][Close Transport]");
      transport_ptr_->Close();
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::Close[CLIENT/PROP_SERVER_TERMINAL][Check Point][10]");

    /// 释放Buffer Pool
    if (NULL != buffer_pool_ptr_) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::Close[Check Point][Decommit Buffer Pool]");
      buffer_pool_ptr_->Decommit();
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::Close[CLIENT/PROP_SERVER_TERMINAL][End]");
  } else {
    YSOS_LOG_DEBUG("CommonConnectionImpl::Close[Unknown mode][mode_][" << mode_ << "]");
  }

  YSOS_LOG_DEBUG("CommonConnectionImpl::Close[Exit]");

  return;
}

void CommonConnectionImpl::CloseChildConnection(ConnectionInterfacePtr child_ptr) {
  YSOS_LOG_DEBUG("CommonConnectionImpl::CloseChildConnection[Enter]");

  if (NULL == child_ptr) {
    YSOS_LOG_DEBUG("CommonConnectionImpl::CloseChildConnection[Fail][child_ptr is NULL]");
    return;
  }

  YSOS_LOG_DEBUG("CommonConnectionImpl::CloseChildConnection[Check Point][0]");

  /// 如果是顶层基类，就从连接链表里查找该子连接并关闭/删除它
  {
    YSOS_LOG_DEBUG("CommonConnectionImpl::CloseChildConnection[Check Point][Close and Remove Child Connections][Enter]");

    AutoLockOper lock(&ll_lock_);

    /// 查找sesseion id和connection指针匹配表，保留该子连接的session id重置该子连接connection ptr为NULL
    if (YSOS_ERROR_SUCCESS != ResetConnectionPtr(child_ptr, NULL)) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::CloseChildConnection[Fail]The function[ResetConnectionPtr] is failed");
    }

    /// 关闭指定的子连接
    if (0 < connection_ptr_list_.size()) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::CloseChildConnection[Check Point][1]");
      std::list<ConnectionInterfacePtr>::iterator it = std::find(connection_ptr_list_.begin(), connection_ptr_list_.end(), child_ptr);
      if (it != connection_ptr_list_.end()) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::CloseChildConnection[Check Point][2]");
        (*it)->Close();
        connection_ptr_list_.erase(it);
      }
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::CloseChildConnection[Check Point][Close and Remove Child Connections][End]");
  }

  YSOS_LOG_DEBUG("CommonConnectionImpl::CloseChildConnection[Exit]");

  return;
}

int CommonConnectionImpl::Read(BufferInterfacePtr input_buffer_ptr, int input_length, void* context_ptr) {
  YSOS_LOG_DEBUG("CommonConnectionImpl::Read[Enter]");

  /// 判断模块是否已开启
  if (IS_RUNNING != GetRunningStatus()) {
    YSOS_LOG_DEBUG("CommonConnectionImpl::Read[It is not running]");
    return YSOS_ERROR_INVALID_OPERATION;
  }

  YSOS_LOG_DEBUG("CommonConnectionImpl::Read[Check Point][0]");

  if (false == enable_read_) {
    YSOS_LOG_DEBUG("CommonConnectionImpl::Read[Fail][enable_read_ is false]");
    return YSOS_ERROR_INVALID_OPERATION;
  }

  YSOS_LOG_DEBUG("CommonConnectionImpl::Read[Check Point][1]");

  if (NULL == transport_ptr_) {
    YSOS_LOG_DEBUG("CommonConnectionImpl::Read[Fail][transport_ptr_ is NULL]");
    return YSOS_ERROR_FAILED;
  }

  YSOS_LOG_DEBUG("CommonConnectionImpl::Read[Check Point][2]");

  if (YSOS_ERROR_SUCCESS != transport_ptr_->Read(input_buffer_ptr, input_length)) {
    YSOS_LOG_DEBUG("CommonConnectionImpl::Read[Fail]The function[transport_ptr_->Read()] is failed");
    return YSOS_ERROR_FAILED;
  }

  YSOS_LOG_DEBUG("CommonConnectionImpl::Read[Exit]");

  return YSOS_ERROR_SUCCESS;
}

int CommonConnectionImpl::Write(BufferInterfacePtr input_buffer_ptr, int input_length, void* context_ptr) {
  YSOS_LOG_DEBUG("CommonConnectionImpl::Write[Enter]");

  YSOS_LOG_DEBUG("CommonConnectionImpl::Write[Start][buffer_ptr][" << reinterpret_cast<UINT64>(input_buffer_ptr.get()) << "]");
  YSOS_LOG_DEBUG("CommonConnectionImpl::Write[Start][buffer_ptr use count][" << input_buffer_ptr.use_count() << "]");

  int result = YSOS_ERROR_FAILED;

  do {
    /// 判断模块是否已开启
    if (IS_RUNNING != GetRunningStatus()) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::Write[It is not running]");
      result = YSOS_ERROR_INVALID_OPERATION;
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::Write[Check Point][0]");

    if (false == enable_write_) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::Write[Fail][enable_write_ is false]");
      result = YSOS_ERROR_INVALID_OPERATION;
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::Write[Check Point][1]");

    /// 服务器端点模式下，直接调用transport层写操作
    if (PROP_SERVER_TERMINAL == mode_) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::Write[SERVERTERMINAL mode][Enter]");

      if (NULL != context_ptr) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::Write[Fail][context_ptr is not NULL]");
        break;
      }

      YSOS_LOG_DEBUG("CommonConnectionImpl::Write[Check Point][2]");

      if (NULL == transport_ptr_) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::Write[Fail][transport_ptr_ is NULL]");
        break;
      }

      YSOS_LOG_DEBUG("CommonConnectionImpl::Write[Check Point][3]");

      if (YSOS_ERROR_SUCCESS != transport_ptr_->Write(input_buffer_ptr, input_length)) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::ProcessWriteMessage[Fail]The function[transport_ptr_->Write()] is failed");
        break;
      }

      YSOS_LOG_DEBUG("CommonConnectionImpl::Write[SERVERTERMINAL mode][End]");
    }
    /// 服务器/客户端模式下，将内容写入消息队列
    else if (PROP_CLIENT == mode_ || PROP_SERVER == mode_) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::Write[CLIENT/SERVER mode][Enter]");

      std::string session_id;
      if (PROP_SERVER == mode_) {
        session_id = (char*)context_ptr;
        YSOS_LOG_DEBUG("CommonConnectionImpl::Write[Check Point][session_id][" << session_id << "]");
      }

      YSOS_LOG_DEBUG("CommonConnectionImpl::Write[Check Point][4]");

      /// 从对象池中获取新的对象
      WriteJobParams* write_job_params_ptr = NULL;
      {
        AutoLockOper lock(&ll_lock_);
        write_job_params_ptr = write_job_object_pool_.construct(input_buffer_ptr, input_length, session_id);
        if (NULL == write_job_params_ptr && true != write_job_object_pool_.is_from(write_job_params_ptr)) {
          YSOS_LOG_DEBUG("CommonConnectionImpl::Write[Fail][write_job_params_ptr is NULL]");
          break;
        }
      }

      YSOS_LOG_DEBUG("CommonConnectionImpl::Write[Check Point][write_job_params_ptr][" << (UINT64)(write_job_params_ptr) << "]");

      YSOS_LOG_DEBUG("CommonConnectionImpl::Write[Check Point][buffer_ptr][" << reinterpret_cast<UINT64>(input_buffer_ptr.get()) << "]");
      YSOS_LOG_DEBUG("CommonConnectionImpl::Write[Check Point][buffer_ptr use count][" << input_buffer_ptr.use_count() << "]");

      YSOS_LOG_DEBUG("CommonConnectionImpl::Write[Check Point][5]");

      /// 添加到写操作工作队列里
      write_job_queue_.Push(write_job_params_ptr);

      YSOS_LOG_DEBUG("CommonConnectionImpl::Write[Check Point][buffer_ptr][" << reinterpret_cast<UINT64>(input_buffer_ptr.get()) << "]");
      YSOS_LOG_DEBUG("CommonConnectionImpl::Write[Check Point][buffer_ptr use count][" << input_buffer_ptr.use_count() << "]");

      YSOS_LOG_DEBUG("CommonConnectionImpl::Write[CLIENT/SERVER mode][End]");
    } else {
      YSOS_LOG_DEBUG("CommonConnectionImpl::Write[Fail][Unknown mode. It is not supported]");
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::Write[Check Point][End]");

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  YSOS_LOG_DEBUG("CommonConnectionImpl::Write[End][buffer_ptr][" << reinterpret_cast<UINT64>(input_buffer_ptr.get()) << "]");
  YSOS_LOG_DEBUG("CommonConnectionImpl::Write[End][buffer_ptr use count][" << input_buffer_ptr.use_count() << "]");

  YSOS_LOG_DEBUG("CommonConnectionImpl::Write[Exit]");

  return result;
}

void CommonConnectionImpl::EnableWrite(bool is_enable) {
  AutoLockOper lock(&ll_lock_);
  enable_write_ = is_enable;

  if (NULL != transport_ptr_) {
    transport_ptr_->EnableWrite(enable_write_);
  }

  return;
}

void CommonConnectionImpl::EnableRead(bool is_enable) {
  AutoLockOper lock(&ll_lock_);
  enable_read_ = is_enable;

  if (NULL != transport_ptr_) {
    transport_ptr_->EnableRead(enable_read_);
  }

  return;
}

void CommonConnectionImpl::EnableWrap(bool is_enable) {
  AutoLockOper lock(&ll_lock_);
  enable_wrap_ = is_enable;
  return;
}

int CommonConnectionImpl::GetProperty(int type_id, void *type) {
  YSOS_LOG_DEBUG("CommonConnectionImpl::GetProperty[Enter]");

  AutoLockOper lock(&ll_lock_);

  int result = YSOS_ERROR_FAILED;

  switch (type_id) {
  case PROP_ERROR_CODE: {
    /// 获取错误码信息
    YSOS_LOG_DEBUG("CommonConnectionImpl::GetProperty[PROP_ERROR_CODE][Check Point][Enter]");

    if (NULL == type) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::GetProperty[Fail][type is NULL]");
      result = YSOS_ERROR_INVALID_ARGUMENTS;
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::GetProperty[Check Point][0]");

    CommonConnectionImpl::ConnectionErrorCodePtr connection_error_code_ptr = static_cast<CommonConnectionImpl::ConnectionErrorCodePtr>(type);

    YSOS_LOG_DEBUG("CommonConnectionImpl::GetProperty[Check Point][1]");

    if (connection_error_code_ptr->boost_asio_error_code == boost::asio::error::operation_aborted) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::GetProperty[operation_aborted]");
      connection_error_code_ptr->ysos_error_code = YSOS_ERROR_TRANSPORT_ERROR_OPERATION_ABORTED;
    } else if (connection_error_code_ptr->boost_asio_error_code == boost::asio::error::eof) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::GetProperty[eof]");
      connection_error_code_ptr->ysos_error_code = YSOS_ERROR_TRANSPORT_ERROR_EOF;
    } else if (connection_error_code_ptr->boost_asio_error_code == boost::asio::error::bad_descriptor) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::GetProperty[bad_descriptor]");
      connection_error_code_ptr->ysos_error_code = YSOS_ERROR_TRANSPORT_ERROR_BAD_DESCRIPTOR;
    } else if (connection_error_code_ptr->boost_asio_error_code == boost::asio::error::connection_reset) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::GetProperty[connection_reset]");
      connection_error_code_ptr->ysos_error_code = YSOS_ERROR_TRANSPORT_ERROR_CONNECTION_RESET;
    } else {
      YSOS_LOG_DEBUG("CommonConnectionImpl::GetProperty[other]");
      connection_error_code_ptr->ysos_error_code = YSOS_ERROR_TRANSPORT_ERROR_OTHER;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::GetProperty[PROP_ERROR_CODE][Check Point][End]");

    result = YSOS_ERROR_SUCCESS;
  }
  break;
  case PROP_SPECIFIED_SESSIONID: {
    /// 获取connection指针对应的session id
    YSOS_LOG_DEBUG("CommonConnectionImpl::GetProperty[PROP_SPECIFIED_SESSIONID][Enter]");

    if (PROP_SERVER == mode_) {
      /// 服务器模式下，顶层基类负责管理connneciton指针和session id匹配表
      /// 服务器端点模式下，以自身指针为参数，调用顶层基类的方法
      YSOS_LOG_DEBUG("CommonConnectionImpl::GetProperty[PROP_SPECIFIED_SESSIONID][Check Point][SERVERTERMINAL][Enter]");

      if (NULL == type) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::GetProperty[PROP_SPECIFIED_SESSIONID][Fail][type is NULL]");
        result = YSOS_ERROR_INVALID_ARGUMENTS;
        break;
      }

      YSOS_LOG_DEBUG("CommonConnectionImpl::GetProperty[PROP_SPECIFIED_SESSIONID][Check Point][0]");

      SessionIDMatchingTableUnit* session_id_matching_table_unit_ptr = static_cast<SessionIDMatchingTableUnit*>(type);

      YSOS_LOG_DEBUG("CommonConnectionImpl::GetProperty[PROP_SPECIFIED_SESSIONID][Check Point][1]");

      ConnectionInterfacePtr connection_ptr = (*session_id_matching_table_unit_ptr).second;

      YSOS_LOG_DEBUG("CommonConnectionImpl::GetProperty[PROP_SPECIFIED_SESSIONID][Check Point][2]");

      /// 查找connneciton指针和session id匹配表
      std::string session_id = GetSessionIDWithConnectionPtr(connection_ptr);
      if (true == session_id.empty()) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::GetProperty[PROP_SPECIFIED_SESSIONID][session_id is empty]");
        break;
      }

      YSOS_LOG_DEBUG("CommonConnectionImpl::GetProperty[PROP_SPECIFIED_SESSIONID][Check Point][session_id][" << session_id << "]");

      /// 赋值给服务器端点模式下传过来的参数
      (*session_id_matching_table_unit_ptr).first = session_id;

      YSOS_LOG_DEBUG("CommonConnectionImpl::GetProperty[PROP_SPECIFIED_SESSIONID][Check Point][SERVERTERMINAL][End]");
    } else {
      YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_SPECIFIED_SESSIONID][Check Point][Unknown]");
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::GetProperty[PROP_SPECIFIED_SESSIONID][End]");

    result = YSOS_ERROR_SUCCESS;
  }
  break;
  case PROP_SESSIONID: {
    /// 获取指定自身对应的session id
    YSOS_LOG_DEBUG("CommonConnectionImpl::GetProperty[PROP_SESSIONID][Enter]");

    if (PROP_SERVER_TERMINAL == mode_) {
      /// 服务器端点模式，重组数据结构，回调基类的GetProperty
      YSOS_LOG_DEBUG("CommonConnectionImpl::GetProperty[PROP_SESSIONID][Check Point][SERVERTERMINAL][Enter]");

      SessionIDMatchingTableUnit session_id_matching_table_unit("", shared_from_this());

      YSOS_LOG_DEBUG("CommonConnectionImpl::GetProperty[PROP_SESSIONID][Check Point][0]");

      if (NULL == root_connection_ptr_) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::GetProperty[PROP_SESSIONID][Fail][root_connection_ptr_ is NULL]");
        break;
      }

      YSOS_LOG_DEBUG("CommonConnectionImpl::GetProperty[PROP_SESSIONID][Check Point][1]");

      if (YSOS_ERROR_SUCCESS != root_connection_ptr_->GetProperty(PROP_SPECIFIED_SESSIONID, &session_id_matching_table_unit)) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::GetProperty[PROP_SESSIONID][Fail]The function[root_connection_ptr_->GetProperty()] is faild");
        break;
      }

      YSOS_LOG_DEBUG("CommonConnectionImpl::GetProperty[PROP_SESSIONID][Check Point][2]");

      if (NULL == type) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::GetProperty[PROP_SESSIONID][Fail][type is NULL]");
        break;
      }

      YSOS_LOG_DEBUG("CommonConnectionImpl::GetProperty[PROP_SESSIONID][Check Point][3]");

      std::string* session_id_ptr = static_cast<std::string*>(type);
      *session_id_ptr = session_id_matching_table_unit.first;

      YSOS_LOG_DEBUG("CommonConnectionImpl::GetProperty[PROP_SESSIONID][Check Point][*session_id_ptr][" << *session_id_ptr << "]");

      YSOS_LOG_DEBUG("CommonConnectionImpl::GetProperty[PROP_SESSIONID][Check Point][SERVERTERMINAL][End]");
    } else if (PROP_CLIENT == mode_) {
      /// 客户端模式下，直接获取session id
      YSOS_LOG_DEBUG("CommonConnectionImpl::GetProperty[PROP_SESSIONID][Check Point][CLIENT][Enter]");

      if (NULL == type) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::GetProperty[PROP_SESSIONID][Fail][type is NULL]");
        result = YSOS_ERROR_INVALID_ARGUMENTS;
        break;
      }

      YSOS_LOG_DEBUG("CommonConnectionImpl::GetProperty[PROP_SESSIONID][Check Point][4]");

      std::string* session_id_ptr = static_cast<std::string*>(type);

      *session_id_ptr = GetSessionIDWithConnectionPtr(shared_from_this());
      if (true == (*session_id_ptr).empty()) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::GetProperty[PROP_SESSIONID][Fail][*session_id_ptr is empty]");
        break;
      }

      YSOS_LOG_DEBUG("CommonConnectionImpl::GetProperty[PROP_SESSIONID][Check Point][CLIENT][End]");
    } else {
      YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_SESSIONID][Check Point][Unknown]");
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::GetProperty[PROP_SESSIONID][End]");

    result = YSOS_ERROR_SUCCESS;
  }
  break;
  default: {
    YSOS_LOG_DEBUG("CommonConnectionImpl::GetProperty[Unknwon]");
    result = YSOS_ERROR_NOT_SUPPORTED;
  }
  break;
  }

  YSOS_LOG_DEBUG("CommonConnectionImpl::GetProperty[Exit]");

  return result;
}

int CommonConnectionImpl::SetProperty(int type_id, void *type) {
  YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[Enter]");

  AutoLockOper lock(&ll_lock_);

  int result = YSOS_ERROR_FAILED;

  switch (type_id) {
  case PROP_CALLBACK: {
    /// 服务器端点模式下，设置新创建connection指针的connectioncallback
    YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_CALLBACK][Enter]");

    if (PROP_SERVER_TERMINAL != mode_) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_CALLBACK][Fail][mode_ is not PROP_SERVER_TERMINAL]");
      result = YSOS_ERROR_INVALID_OPERATION;
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_CALLBACK][Check Point][0]");

    if (NULL == type) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_CALLBACK][Fail][type is NULL]");
      result = YSOS_ERROR_INVALID_ARGUMENTS;
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_CALLBACK][Check Point][1]");

    ConnectionCallbackInterfacePtr *connection_callback_ptr_ptr = static_cast<ConnectionCallbackInterfacePtr*>(type);

    YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_CALLBACK][Check Point][2]");

    connection_callback_impl_ptr_ = *connection_callback_ptr_ptr;

    YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_CALLBACK][Check Point][3]");

    /// 获取Buffer信息
    if (0 == connection_callback_impl_ptr_->GetBufferLength() ||
        0 == connection_callback_impl_ptr_->GetBufferNumber()) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_CALLBACK][Fail][The parameters of buffer is invalid]");
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_CALLBACK][Check Point][4]");

    /// 创建buffer pool
    buffer_pool_ptr_ = GetBufferUtility()->CreateBufferPool(connection_callback_impl_ptr_->GetBufferLength(), connection_callback_impl_ptr_->GetBufferNumber());
    if (NULL == buffer_pool_ptr_) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_CALLBACK][Fail][buffer_pool_ptr_ is NULL]");
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_CALLBACK][Check Point][5]");

    /// 从传进来的ConnectionCallback里获得TransportCallback指针
    TransportCallbackInterfacePtr transport_callback_ptr = connection_callback_impl_ptr_->GetTransportCallbackPtr();
    if (NULL == transport_callback_ptr) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_CALLBACK][Fail][transport_callback_ptr is NULL]");
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_CALLBACK][Check Point][6]");

    /// 把自身设置给TransportCallback，建立一一对应关系
    transport_callback_ptr->SetConnectionPtr(this);

    YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_CALLBACK][Check Point][7]");

    if (NULL == transport_ptr_) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_CALLBACK][Fail][transport_ptr_ is NULL]");
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_CALLBACK][Check Point][8]");

    /// 设置TranportCallback给新Transport
    if (YSOS_ERROR_SUCCESS != transport_ptr_->SetProperty(PROP_CALLBACK, &transport_callback_ptr)) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_CALLBACK][Fail]The function[transport_ptr_->SetProperty()] is failed");
      break;
    }

    result = YSOS_ERROR_SUCCESS;

    YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_CALLBACK][Exit]");
  }
  break;
  case PROP_READ: {
    /// 服务器端点模式下，调用Read，达到循环读取操作
    YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_READ][Enter]");

    if (PROP_SERVER_TERMINAL != mode_) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_READ][Fail][mode_ is not PROP_SERVER_TERMINAL]");
      result = YSOS_ERROR_INVALID_OPERATION;
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_READ][Check Point][0]");

    /// 获取新buffer
    if (NULL == buffer_pool_ptr_) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_READ][Fail][buffer_pool_ptr_ is NULL]");
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_READ][Check Point][1]");

    BufferInterfacePtr new_buffer_ptr = GetBufferUtility()->GetBufferFromBufferPool(buffer_pool_ptr_);
    while (NULL == new_buffer_ptr) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[Get Buffer Ptr][Fail]");

      /// 判断模块是否已关闭
      if (IS_STOPPED == GetRunningStatus()) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[It is already stopped]");
        return YSOS_ERROR_FAILED;
      }

      boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
      new_buffer_ptr = GetBufferUtility()->GetBufferFromBufferPool(buffer_pool_ptr_);
      if (NULL != new_buffer_ptr) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[Get Buffer Ptr][Sucess]");
        break;
      }
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_READ][Check Point][2]");

    /// 设置buffer的prefix长度0
    if (YSOS_ERROR_SUCCESS != GetBufferUtility()->SetBufferPrefixLength(new_buffer_ptr, 0)) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_READ][Fail][The function[GetBufferUtility()->SetBufferPrefixLength()] is failed.]");
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_READ][Check Point][3]");

    /// 设置buffer长度为最大
    if (YSOS_ERROR_SUCCESS != GetBufferUtility()->SetBufferLength(new_buffer_ptr, (GetBufferUtility()->GetBufferMaxLength(new_buffer_ptr)))) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_READ][Fail][The function[GetBufferUtility()->SetBufferLength()] is failed.]");
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_READ][Check Point][4]");

    /// 初始化buffer
    if (YSOS_ERROR_SUCCESS != GetBufferUtility()->InitialBuffer(new_buffer_ptr)) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_READ][Fail]The function[GetBufferUtility()->InitialBuffer()] is failed");
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_READ][Check Point][5]");

    /// 调用Read，达到读操作循环
    if (YSOS_ERROR_SUCCESS != Read(new_buffer_ptr, GetBufferUtility()->GetBufferUsableLength(new_buffer_ptr))) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_READ][Fail]The function[Read()] is failed");
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_READ][End]");

    result = YSOS_ERROR_SUCCESS;
  }
  break;
  case PROP_INVALID_CONNECTION: {
    /// 将无效连接即出错的连接添加到工作队列里，服务器模式下由顶层基类统一关闭
    YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_INVALID_CONNECTION][Enter]");

    if (NULL == type) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_INVALID_CONNECTION][Fail][type is NULL]");
      result = YSOS_ERROR_INVALID_ARGUMENTS;
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_INVALID_CONNECTION][Check Point][0]");

    ConnectionInterfacePtr* connection_ptr_ptr = static_cast<ConnectionInterfacePtr*>(type);

    YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_INVALID_CONNECTION][Check Point][1]");

    if (PROP_SERVER == mode_) {
      /// 服务器模式下，如果是顶层基类，则把错误连接消息添加到顶层基类的无效连接队列
      YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_INVALID_CONNECTION][Check Point][SERVER]");

      /// 添加到顶层基类的无效连接队列
      invalid_connection_job_queue_.Push(*connection_ptr_ptr);
    } else if (PROP_SERVER_TERMINAL == mode_) {
      /// 服务器模式下，如果是端点，则把错误连接消息添加到顶层基类的无效连接队列
      YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_INVALID_CONNECTION][Check Point][SERVERTERMINAL]");

      if (NULL == root_connection_ptr_) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_INVALID_CONNECTION][Fail][root_connection_ptr_ is NULL]");
        break;
      }

      YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_INVALID_CONNECTION][Check Point][2]");

      /// 回调基类的SetProperty
      if (YSOS_ERROR_SUCCESS != root_connection_ptr_->SetProperty(PROP_INVALID_CONNECTION, type)) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_INVALID_CONNECTION][Fail]The function[root_connection_ptr_->SetProperty()] is failed");
        break;
      }

      YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_INVALID_CONNECTION][Check Point][3]");

    } else {
      YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_INVALID_CONNECTION][Check Point][Unknown]");
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_INVALID_CONNECTION][End]");

    result = YSOS_ERROR_SUCCESS;
  }
  break;
  case PROP_NEW_SESSIONID: {
    /// 生成新的session id, session id和connection指针匹配表，服务器模式下，由顶层基类统一管理
    YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_NEW_SESSIONID][Enter]");

    if (PROP_SERVER == mode_) {
      /// 服务器模式下，生成新的session id，插入到session id和connection指针匹配表
      YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_NEW_SESSIONID][Check Point][SERVER][Enter]");

      if (NULL == type) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_NEW_SESSIONID][Fail][type is NULL]");
        result = YSOS_ERROR_INVALID_ARGUMENTS;
        break;
      }

      YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_NEW_SESSIONID][Check Point][0]");

      ConnectionInterfacePtr* connection_ptr_ptr = static_cast<ConnectionInterfacePtr*>(type);

      YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_NEW_SESSIONID][Check Point][1]");

      /// 生成新的session id并将新生成的session id和connection指针添加到表里
      std::string new_session_id = GenerateSessionIDWithConnectionPtr(*connection_ptr_ptr);
      if (true == new_session_id.empty()) {
        YSOS_LOG_DEBUG("CommonConnetionImpl::SetProperty[PROP_NEW_SESSIONID][Fail][new_session_id is empty]");
        break;
      }

      YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_NEW_SESSIONID][Check Point][SERVER][End]");

    } else if (PROP_SERVER_TERMINAL == mode_) {
      /// 服务器端点模式下，回调基类的SetProperty
      YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_NEW_SESSIONID][Check Point][SERVERTERMINAL][Enter]");

      if (NULL == root_connection_ptr_) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_NEW_SESSIONID][Fail][root_connection_ptr_ is NULL]");
        break;
      }

      YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_NEW_SESSIONID][Check Point][2]");

      /// 回调基类的SetProperty
      //need update for linux
      #ifdef _WIN32
        if (YSOS_ERROR_SUCCESS != root_connection_ptr_->SetProperty(PROP_NEW_SESSIONID, &shared_from_this())) {
          YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_NEW_SESSIONID][Fail]The function[root_connection_ptr_->SetProperty()] is failed");
          break;
        }
      #else
        boost::shared_ptr<ysos::CommonConnectionImpl> temp = shared_from_this();
        if (YSOS_ERROR_SUCCESS != root_connection_ptr_->SetProperty(PROP_NEW_SESSIONID, &temp)) {
          YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_NEW_SESSIONID][Fail]The function[root_connection_ptr_->SetProperty()] is failed");
          break;
        }
      #endif
      
      YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_NEW_SESSIONID][Check Point][SERVERTERMINAL][End]");
    } else {
      YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_NEW_SESSIONID][Check Point][Unknown]");
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_NEW_SESSIONID][End]");

    result = YSOS_ERROR_SUCCESS;
  }
  break;
  case PROP_SESSIONID_CONNECTIONPTR: {
    /// 更新session id和connection指针匹配表中session id对应的connection指针
    /// 发生在服务器模式下断开的客户端以以前发放的session id请求连接的时候
    YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_SESSIONID_CONNECTIONPTR][Enter]");

    if (PROP_SERVER == mode_) {
      /// 服务器模式下，由顶层基类负责管理session id和connection指针匹配表
      YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_SESSIONID_CONNECTIONPTR][Check Point][SERVER][Enter]");

      if (NULL == type) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_SESSIONID_CONNECTIONPTR][Fail][type is NULL]");
        result = YSOS_ERROR_INVALID_ARGUMENTS;
        break;
      }

      YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_SESSIONID_CONNECTIONPTR][Check Point][0]");

      SessionIDMatchingTableUnit* session_id_matching_table_unit_ptr = static_cast<SessionIDMatchingTableUnit*>(type);

      YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_SESSIONID_CONNECTIONPTR][Check Point][1]");

      std::string session_id = (*session_id_matching_table_unit_ptr).first;
      ConnectionInterfacePtr connection_ptr = (*session_id_matching_table_unit_ptr).second;

      if (true == session_id.empty()) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_SESSIONID_CONNECTIONPTR][Fail][session_id is empty]");
        break;
      }

      YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_SESSIONID_CONNECTIONPTR][Check Point][session_id][" << session_id << "]");

      /// 更新connection指针
      if (YSOS_ERROR_SUCCESS != SetConnectionPtrWithSessionID(session_id, connection_ptr, true)) {
        YSOS_LOG_DEBUG("CommonConnetionImpl::SetProperty[PROP_SESSIONID_CONNECTIONPTR][Fail]The function[SetConnectionPtrWithSessionID()] is failed");
        break;
      }

      YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_SESSIONID_CONNECTIONPTR][Check Point][SERVER][End]");
    } else {
      YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_SESSIONID_CONNECTIONPTR][Check Point][Unknown]");
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_SESSIONID_CONNECTIONPTR][End]");

    result = YSOS_ERROR_SUCCESS;
  }
  break;
  case PROP_SESSIONID: {
    /// 保存session id
    YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_SESSIONID][Enter]");

    if (PROP_CLIENT == mode_) {
      /// 客户端模式下，保存session id
      YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_SESSIONID][Check Point][CLIENT][Enter]");

      if (NULL == type) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_SESSIONID][Fail][type is NULL]");
        result = YSOS_ERROR_INVALID_ARGUMENTS;
        break;
      }

      std::string* session_id_ptr = static_cast<std::string*>(type);
      if (true == (*session_id_ptr).empty()) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_SESSIONID][Fail][*session_id_ptr is empty]");
        result = YSOS_ERROR_INVALID_ARGUMENTS;
        break;
      }

      YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_SESSIONID][Check Point][*session_id_ptr][" << *session_id_ptr << "]");

      if (YSOS_ERROR_SUCCESS != SaveSessionIDnConnectionPtr(*session_id_ptr, shared_from_this())) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_SESSIONID][Fail]The function[SaveSessionIDnConnectionPtr()] is failed");
        break;
      }

      YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_SESSIONID][Check Point][CLIENT][End]");
    } else if (PROP_SERVER_TERMINAL == mode_) {
      /// 服务器端点模式下，重新组成数据结构，回调基类的SetProperty
      YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_SESSIONID][Check Point][SERVERTERMINAL][Enter]");

      if (NULL == type) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_SESSIONID][Fail][type is NULL]");
        result = YSOS_ERROR_INVALID_ARGUMENTS;
        break;
      }

      YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_SESSIONID][Check Point][0]");

      std::string* session_id_ptr = static_cast<std::string*>(type);
      if (true == (*session_id_ptr).empty()) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_SESSIONID][Fail][*session_id_ptr is empty]");
        result = YSOS_ERROR_INVALID_ARGUMENTS;
        break;
      }

      YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_SESSIONID][Check Point][*session_id_ptr][" << *session_id_ptr << "]");

      SessionIDMatchingTableUnit session_id_matching_table_unit(*session_id_ptr, shared_from_this());

      if (NULL == root_connection_ptr_) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_SESSIONID][Fail][root_connection_ptr_ is NULL]");
        break;
      }

      YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_SESSIONID][Check Point][1]");

      /// 回调顶层基类的SetProperty
      if (YSOS_ERROR_SUCCESS != root_connection_ptr_->SetProperty(PROP_SESSIONID_CONNECTIONPTR, &session_id_matching_table_unit)) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_SESSIONID][Fail]The function[root_connection_ptr_->SetProperty()] is failed");
        break;
      }

      YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_SESSIONID][Check Point][SERVERTERMINAL][End]");

    } else {
      YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_SESSIONID][Check Point][Unknown]");
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_SESSIONID][End]");

    result = YSOS_ERROR_SUCCESS;
  }
  break;
  case PROP_HANDLECONNECT: {
    /// 客户端模式下，触发ConnectionHandler，达到读取操作循环
    YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_HANDLECONNECT][Enter]");

    if (PROP_CLIENT == mode_) {
      /// 客户端模式下，触发ConnectionHandler
      YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_HANDLECONNECT][Check Point][PROP_CLIENT][Enter]");

      if (NULL == transport_ptr_) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_HANDLECONNECT][Fail][transport_ptr_ is NULL]");
        break;
      }

      YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_HANDLECONNECT][Check Point][1]");

      if (YSOS_ERROR_SUCCESS != transport_ptr_->SetProperty(PROP_HANDLECONNECT, type)) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[SaveSessionID][Fail]The function[transport_ptr_->SetProperty()] is failed");
        break;
      }

      YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_HANDLECONNECT][Check Point][PROP_CLIENT][End]");
    } else {
      YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_HANDLECONNECT][Check Point][Unknown]");
      result = YSOS_ERROR_NOT_SUPPORTED;
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_HANDLECONNECT][End]");

    result = YSOS_ERROR_SUCCESS;
  }
  break;
  case PROP_READ_HEARTBEAT: {
    /// 将读取到的心跳请求或者响应添加到读取心跳工作队列里，将只抽取发送的事件
    YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_READ_HEARTBEAT][Enter]");

    if (PROP_SERVER_TERMINAL == mode_ || PROP_CLIENT == mode_) {
      /// 服务器模式/客户端模式下，都进行保存心跳
      YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_READ_HEARTBEAT][mode_][" << (PROP_SERVER_TERMINAL == mode_ ? "PROP_SERVER_TERMINAL" : "PROP_CLIENT") << "]");

      if (NULL == type) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_READ_HEARTBEAT][Fail][type is NULL]");
        result = YSOS_ERROR_INVALID_ARGUMENTS;
        break;
      }

      YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_READ_HEARTBEAT][Check Point][0]");

      std::string* time_stamp_ptr = static_cast<std::string*>(type);

      YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_READ_HEARTBEAT][Check Point][1]");

      /// 添加到读取心跳工作队列
      read_heartbeat_job_queue_.Push(*time_stamp_ptr);

      YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_READ_HEARTBEAT][Check Point][2]");

    } else {
      YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_READ_HEARTBEAT][Fail][It's not PROP_SERVER_TERMINAL/PROP_CLIENT]");
      result = YSOS_ERROR_NOT_SUPPORTED;
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_READ_HEARTBEAT][End]");

    result = YSOS_ERROR_SUCCESS;
  }
  break;
  case PROP_WRITE_HEARTBEAT: {
    /// 将写入的心跳请求或者响应添加到写入心跳工作队列里，将只抽取发送时间
    YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_WRITE_HEARTBEAT][Enter]");

    if (PROP_SERVER_TERMINAL == mode_ || PROP_CLIENT == mode_) {
      /// 服务器模式/客户端模式下，都进行保存心跳
      YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_WRITE_HEARTBEAT][mode_][" << (PROP_SERVER_TERMINAL == mode_ ? "PROP_SERVER_TERMINAL" : "PROP_CLIENT") << "]");

      if (NULL == type) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_WRITE_HEARTBEAT][Fail][type is NULL]");
        result = YSOS_ERROR_INVALID_ARGUMENTS;
        break;
      }

      YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_WRITE_HEARTBEAT][Check Point][0]");

      std::string* time_stamp_ptr = static_cast<std::string*>(type);

      YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_WRITE_HEARTBEAT][Check Point][1]");

      /// 添加到写入心跳工作队列
      write_heartbeat_job_queue_.Push(*time_stamp_ptr);

      YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_WRITE_HEARTBEAT][Check Point][2]");

    } else {
      YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_WRITE_HEARTBEAT][Fail][It's not PROP_SERVER_TERMINAL/PROP_CLIENT]");
      result = YSOS_ERROR_NOT_SUPPORTED;
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_WRITE_HEARTBEAT][End]");

    result = YSOS_ERROR_SUCCESS;
  }
  break;
  case PROP_HEARTBEAT: {
    /// 触发心跳操作，服务器端点/客户端模式下，正常建立连接后进行此操作
    YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_HEARTBEAT][Enter]");

    if (PROP_SERVER_TERMINAL == mode_ || PROP_CLIENT == mode_) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_HEARTBEAT][mode_][" << (PROP_SERVER_TERMINAL == mode_ ? "PROP_SERVER_TERMINAL" : "PROP_CLIENT") << "]");

      if (NULL != type) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_WRITE_HEARTBEAT][Fail][type is not NULL]");
        result = YSOS_ERROR_INVALID_ARGUMENTS;
        break;
      }

      /// 执行读取心跳操作
      if (YSOS_ERROR_SUCCESS != ReadHeartbeat()) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_HEARTBEAT][Fail]The funtion[ReadHeartbeat()] is failed");
        break;
      }

      YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_HEARTBEAT][Check Point][0]");

      /// 执行写入心跳操作
      if (YSOS_ERROR_SUCCESS != WriteHeartbeat()) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_HEARTBEAT][Fail]The funtion[WriteHeartbeat()] is failed");
        break;
      }

      YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_HEARTBEAT][Check Point][1]");

      /// 心跳操作循环开始
      RunIOService();

      YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_HEARTBEAT][Check Point][2]");

    } else {
      YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_HEARTBEAT][Fail][It's not PROP_SERVER_TERMINAL/PROP_CLIENT]");
      result = YSOS_ERROR_NOT_SUPPORTED;
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[PROP_HEARTBEAT][End]");

    result = YSOS_ERROR_SUCCESS;
  }
  break;
  default: {
    YSOS_LOG_DEBUG("CommonConnetionImpl::SetProperty[Check Point][Unknown]");
    result = YSOS_ERROR_NOT_SUPPORTED;
  }
  break;
  }

  YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[Exit]");

  return result;
}

int CommonConnectionImpl::Wrap(BufferInterfacePtr input_buffer_ptr, int input_length, BufferInterfacePtr output_buffer_ptr, int *out_length, void* context_ptr) {
  YSOS_LOG_DEBUG("CommonConnectionImpl::Wrap()");
  return YSOS_ERROR_SUCCESS;
}

int CommonConnectionImpl::Unwrap(BufferInterfacePtr input_buffer_ptr, int input_length, BufferInterfacePtr output_buffer_ptr, int *out_length, void* context_ptr) {
  YSOS_LOG_DEBUG("CommonConnectionImpl::Unwrap()");
  return YSOS_ERROR_SUCCESS;
}

void CommonConnectionImpl::HandleError(const boost::system::error_code& error_code, const UINT32 callback_type, const TransportInterfacePtr& transport_ptr) {
  YSOS_LOG_DEBUG("CommonConnectionImpl::HandlerError[Enter]");

  /// 调用上层HandleError
  if (NULL != connection_callback_impl_ptr_) {
    YSOS_LOG_DEBUG("CommonConnectionImpl::HandlerError[Call connection_callback_impl_ptr_->HandleError()]");
    connection_callback_impl_ptr_->HandleError(error_code, callback_type, shared_from_this());
  }

  YSOS_LOG_DEBUG("CommonConnectionImpl::HandlerError[Check Point][0]");

  /// 把错误信息添加到顶层基类的无效队列中
  if (NULL != root_connection_ptr_) {
    YSOS_LOG_DEBUG("CommonConnectionImpl::HandlerError[Add invalid connectio ptr to invalid connection queue of root connection]");
    //need update for linux
    #ifdef _WIN32
      if (YSOS_ERROR_SUCCESS != root_connection_ptr_->SetProperty(PROP_INVALID_CONNECTION, &shared_from_this())) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::HandlerError[Fail]The function[root_connection_ptr_->SetProperty()] is failed");
      }
    #else
      boost::shared_ptr<ysos::CommonConnectionImpl> temp2 = shared_from_this();
      if (YSOS_ERROR_SUCCESS != root_connection_ptr_->SetProperty(PROP_INVALID_CONNECTION, &temp2)) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::HandlerError[Fail]The function[root_connection_ptr_->SetProperty()] is failed");
      }
    #endif
  }

  YSOS_LOG_DEBUG("CommonConnectionImpl::HandlerError[Exit]");

  return;
}

void CommonConnectionImpl::HandleAccept(const boost::system::error_code& error_code, const TransportInterfacePtr& transport_ptr) {
  YSOS_LOG_DEBUG("CommonConnectionImpl::HandleAccept[Enter]");

  int result = YSOS_ERROR_FAILED;

  do {
    /// 判断模块是否已关闭
    if (IS_STOPPED == GetRunningStatus()) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::HandleAccept[It is already stopped]");
      result = YSOS_ERROR_SUCCESS;
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::HandleAccept[Check Point][0]");

    if (0 != error_code) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::HandleAccept[Error Message]: " << error_code.message());
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::HandleAccept[Check Point][1]");

    /// 创建新的连接对象
    ConnectionInterfacePtr connection_ptr = ConnectionInterfacePtr(new CommonConnectionImpl(shared_from_this(), transport_ptr));
    if (NULL == connection_ptr) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::HandleAccept[Fail][0]");
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::HandleAccept[Check Point][2]");

    /// 添加到连接队列里
    connection_ptr_list_.push_back(connection_ptr);

    YSOS_LOG_DEBUG("CommonConnectionImpl::HandleAccept[Check Point][3]");

    /// 调用回调函数
    if (NULL != connection_callback_impl_ptr_) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::HandleAccept[Call connection_callback_impl_ptr_->HandleAccept()]");
      connection_callback_impl_ptr_->HandleAccept(error_code, connection_ptr);
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::HandleAccept[Check Point][End]");

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  if (YSOS_ERROR_SUCCESS != result) {
    YSOS_LOG_DEBUG("CommonConnectionImpl::HandleAccept[Error]");
    HandleError(error_code, PROP_ON_ACCEPT, transport_ptr);
  }

  YSOS_LOG_DEBUG("CommonConnectionImpl::HandleAccept[Exit]");

  return;
}

void CommonConnectionImpl::HandleConnect(const boost::system::error_code& error_code, const TransportInterfacePtr& transport_ptr) {
  YSOS_LOG_DEBUG("CommonConnectionImpl::HandleConnect[Enter]");

  int result = YSOS_ERROR_FAILED;

  do {
    /// 判断模块是否已关闭
    if (IS_STOPPED == GetRunningStatus()) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::HandleConnect[It is already stopped]");
      result = YSOS_ERROR_SUCCESS;
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::HandleConnect[Check Point][0]");

    if (0 != error_code) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::HandleConnect[Error Message]: " << error_code.message());
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::HandleConnect[Check Point][1]");

    /// 调用回调函数
    if (NULL != connection_callback_impl_ptr_) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::HandleConnect[Check Point][Call connection_callback_impl_ptr_->HandleConnect()]");
      connection_callback_impl_ptr_->HandleConnect(error_code, shared_from_this());
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::HandleConnect[Check Point][2]");

    /// 获取新buffer
    if (NULL == buffer_pool_ptr_) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::HandleConnect[Fail][buffer_pool_ptr_ is NULL]");
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::HandleConnect[Check Point][3]");

    BufferInterfacePtr buffer_ptr = GetBufferUtility()->GetBufferFromBufferPool(buffer_pool_ptr_);
    while (NULL == buffer_ptr) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::HandleConnect[Get Buffer Ptr][Fail]");

      /// 判断模块是否已关闭
      if (IS_STOPPED == GetRunningStatus()) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::HandleConnect[It is already stopped]");
        return;
      }

      boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
      buffer_ptr = GetBufferUtility()->GetBufferFromBufferPool(buffer_pool_ptr_);
      if (NULL != buffer_ptr) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::HandleConnect[Get Buffer Ptr][Sucess]");
        break;
      }
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::HandleConnect[Check Point][4]");

    /// 设置buffer prefix为0
    if (YSOS_ERROR_SUCCESS != GetBufferUtility()->SetBufferPrefixLength(buffer_ptr, 0)) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::HandleConnect[Fail][The function[GetBufferUtility()->SetBufferPrefixLength()] is failed.]");
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::HandleConnect[Check Point][5]");

    //// 设置buffer最大度
    if (YSOS_ERROR_SUCCESS != GetBufferUtility()->SetBufferLength(buffer_ptr, (GetBufferUtility()->GetBufferMaxLength(buffer_ptr)))) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::HandleRead[Fail][The function[GetBufferUtility()->SetBufferLength()] is failed.]");
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::HandleConnect[Check Point][6]");

    if (YSOS_ERROR_SUCCESS != GetBufferUtility()->InitialBuffer(buffer_ptr)) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::HandleConnect[Fail]The function[GetBufferUtility()->InitialBuffer()] is failed");
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::HandleConnect[Check Point][7]");

    /// 调用Read函数执行读取循环调用
    if (YSOS_ERROR_SUCCESS != Read(buffer_ptr, GetBufferUtility()->GetBufferMaxLength(buffer_ptr))) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::HandleConnect[Fail]The function[Read()] is failed");
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::HandleConnect[Check Point][End]");

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  if (YSOS_ERROR_SUCCESS != result) {
    YSOS_LOG_DEBUG("CommonConnectionImpl::HandleConnect[Error]");
    HandleError(error_code, PROP_ON_CONNECT, transport_ptr);
  }

  YSOS_LOG_DEBUG("CommonConnectionImpl::HandleConnect[Exit]");

  return;
}

void CommonConnectionImpl::HandleDisconnect(const boost::system::error_code& error_code, const TransportInterfacePtr& transport_ptr) {
  YSOS_LOG_DEBUG("CommonConnectionImpl::HandleDisconnect[Enter]");

  /// 判断模块是否已关闭
  if (IS_STOPPED == GetRunningStatus()) {
    YSOS_LOG_DEBUG("CommonConnectionImpl::HandleDisconnect[It is already stopped]");
    return;
  }

  YSOS_LOG_DEBUG("CommonConnectionImpl::HandleDisconnect[Check Point][0]");

  /// 调用回调函数
  if (NULL != connection_callback_impl_ptr_) {
    YSOS_LOG_DEBUG("CommonConnectionImpl::HandleDisconnect[Call connection_callback_impl_ptr_->HandleDisconnect()]");
    connection_callback_impl_ptr_->HandleDisconnect(error_code, shared_from_this());
  }

  YSOS_LOG_DEBUG("CommonConnectionImpl::HandleDisconnect[Check Point][1]");

  /// 把错误信息添加到顶层基类的无效队列中
  if (NULL != root_connection_ptr_) {
    YSOS_LOG_DEBUG("CommonConnectionImpl::HandleDisconnect[Add invalid connectio ptr to invalid connection queue of root connection]");
    //need update for linux
    #ifdef _WIN32
      if (YSOS_ERROR_SUCCESS != root_connection_ptr_->SetProperty(PROP_INVALID_CONNECTION, &shared_from_this())) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::HandleDisconnect[Fail]The function[root_connection_ptr_->SetProperty()] is failed");
      }
    #else
      boost::shared_ptr<ysos::CommonConnectionImpl> temp3 = shared_from_this();
      if (YSOS_ERROR_SUCCESS != root_connection_ptr_->SetProperty(PROP_INVALID_CONNECTION, &temp3)) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::HandleDisconnect[Fail]The function[root_connection_ptr_->SetProperty()] is failed");
      }
    #endif
  }

  YSOS_LOG_DEBUG("CommonConnectionImpl::HandleDisconnect[Exit]");

  return;
}

void CommonConnectionImpl::HandleRead(const boost::system::error_code& error_code, const std::size_t bytes_transferred, const BufferInterfacePtr& buffer_ptr, const TransportInterfacePtr& transport_ptr) {
  YSOS_LOG_DEBUG("CommonConnectionImpl::HandleRead[Enter]");

  int result = YSOS_ERROR_FAILED;

  do {
    /// 判断模块是否已关闭
    if (IS_STOPPED == GetRunningStatus()) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::HandleRead[It is already stopped]");
      result = YSOS_ERROR_SUCCESS;
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::HandleRead[Check Point][0]");

    if (0 != error_code) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::HandleRead[Error Message]: " << error_code.message());
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::HandleRead[Check Point][1]");

    /// 把消息添加到工作队列
    read_job_queue_.Push(buffer_ptr);

    YSOS_LOG_DEBUG("CommonConnectionImpl::HandleRead[Check Point][2]");

    /// 获取新Buffer
    if (NULL == buffer_pool_ptr_) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::HandleRead[Fail][buffer_pool_ptr_ is NULL]");
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::HandleRead[Check Point][3]");

    BufferInterfacePtr new_buffer_ptr = GetBufferUtility()->GetBufferFromBufferPool(buffer_pool_ptr_);
    while (NULL == new_buffer_ptr) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::HandleRead[Get Buffer Ptr][Fail]");

      /// 判断模块是否已关闭
      if (IS_STOPPED == GetRunningStatus()) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::HandleRead[It is already stopped]");
        return;
      }

      boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
      new_buffer_ptr = GetBufferUtility()->GetBufferFromBufferPool(buffer_pool_ptr_);
      if (NULL != new_buffer_ptr) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::HandleRead[Get Buffer Ptr][Sucess]");
        break;
      }
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::HandleRead[Check Point][4]");

    /// 设置buffer prefix为0
    if (YSOS_ERROR_SUCCESS != GetBufferUtility()->SetBufferPrefixLength(new_buffer_ptr, 0)) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::HandleRead[Fail][The function[GetBufferUtility()->SetBufferPrefixLength()] is failed.]");
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::HandleRead[Check Point][5]");

    //// 设置buffer最大度
    if (YSOS_ERROR_SUCCESS != GetBufferUtility()->SetBufferLength(new_buffer_ptr, (GetBufferUtility()->GetBufferMaxLength(new_buffer_ptr)))) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::HandleRead[Fail][The function[GetBufferUtility()->SetBufferLength()] is failed.]");
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::HandleRead[Check Point][6]");

    //// 初始化buffer
    if (YSOS_ERROR_SUCCESS != GetBufferUtility()->InitialBuffer(new_buffer_ptr)) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::HandleRead[Fail]The funtion[GetBufferUtility()->InitialBuffer()] is failed");
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::HandleRead[Check Point][7]");

    /// 再次执行读取操作达到循环读取
    if (YSOS_ERROR_SUCCESS != Read(new_buffer_ptr, GetBufferUtility()->GetBufferUsableLength(new_buffer_ptr))) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::HandleRead[Fail]The funtion[Read()] is failed");
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::HandleRead[Check Point][End]");

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  if (YSOS_ERROR_SUCCESS != result) {
    YSOS_LOG_DEBUG("CommonConnectionImpl::HandleRead[Error]");
    HandleError(error_code, PROP_ON_READ, transport_ptr);
  }

  YSOS_LOG_DEBUG("CommonConnectionImpl::HandleRead[Exit]");

  return;
}

void CommonConnectionImpl::HandleWrite(const boost::system::error_code& error_code, const std::size_t bytes_transferred, const BufferInterfacePtr& buffer_ptr, const TransportInterfacePtr& transport_ptr) {
  YSOS_LOG_DEBUG("CommonConnectionImpl::HandleWrite[Enter]");

  YSOS_LOG_DEBUG("CommonConnectionImpl::HandleWrite[Start][buffer_ptr][" << reinterpret_cast<UINT64>(buffer_ptr.get()) << "]");
  YSOS_LOG_DEBUG("CommonConnectionImpl::HandleWrite[Start][buffer_ptr use count][" << buffer_ptr.use_count() << "]");

  int result = YSOS_ERROR_FAILED;

  do {
    /// 判断模块是否已关闭
    if (IS_STOPPED == GetRunningStatus()) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::HandleWrite[It is already stopped]");
      result = YSOS_ERROR_SUCCESS;
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::HandleWrite[Check Point][0]");

    if (0 != error_code) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::HandleWrite[Error Message]: " << error_code.message());
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::HandleWrite[Check Point][1]");

    /// 回调connectioncallback处理数据
    if (NULL != connection_callback_impl_ptr_) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::HandleWrite[Call connection_callback_impl_ptr_->HandleWrite()]");
      connection_callback_impl_ptr_->HandleWrite(buffer_ptr, shared_from_this());
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::HandleWrite[Check Point][End]");

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  if (YSOS_ERROR_SUCCESS != result) {
    YSOS_LOG_DEBUG("CommonConnectionImpl::HandleWrite[Error]");
    HandleError(error_code, PROP_ON_WRITE, transport_ptr);
  }

  YSOS_LOG_DEBUG("CommonConnectionImpl::HandleWrite[End][buffer_ptr][" << reinterpret_cast<UINT64>(buffer_ptr.get()) << "]");
  YSOS_LOG_DEBUG("CommonConnectionImpl::HandleWrite[End][buffer_ptr use count][" << buffer_ptr.use_count() << "]");

  YSOS_LOG_DEBUG("CommonConnectionImpl::HandleWrite[Exit]");

  return;
}

bool CommonConnectionImpl::ProcessReadMessage(const BufferInterfacePtr& buffer_ptr) {
  YSOS_LOG_DEBUG("CommonConnectionImpl::ProcessReadMessage[Enter]");

  /// 回调connectioncallback处理数据
  if (NULL != connection_callback_impl_ptr_) {
    YSOS_LOG_DEBUG("CommonConnectionImpl::ProcessReadMessage[Call connection_callback_impl_ptr_->HandleRead()]");
    connection_callback_impl_ptr_->HandleRead(buffer_ptr, shared_from_this());
  }

  YSOS_LOG_DEBUG("CommonConnectionImpl::ProcessReadMessage[Exit]");

  return true;
}

bool CommonConnectionImpl::ProcessWriteMessage(const WriteJobParamsPtr& write_job_params_ptr) {
  YSOS_LOG_DEBUG("CommonConnectionImpl::ProcessWriteMessage[Enter]");

  int result = YSOS_ERROR_FAILED;

  YSOS_LOG_DEBUG("CommonConnectionImpl::ProcessWriteMessage[Check Point][write_job_params_ptr][" << (UINT64)(write_job_params_ptr) << "]");

  BufferInterfacePtr input_buffer_ptr = std::get<0>(*write_job_params_ptr);
  int input_length = std::get<1>(*write_job_params_ptr);
  std::string session_id = std::get<2>(*write_job_params_ptr);

  do {
    YSOS_LOG_DEBUG("CommonConnectionImpl::ProcessWriteMessage[Check Point][input_buffer_ptr][" << (UINT64)(input_buffer_ptr.get()) << "]" << "use count[" << input_buffer_ptr.use_count() << "]");
    YSOS_LOG_DEBUG("CommonConnectionImpl::ProcessWriteMessage[Check Point][input_length][" << input_length << "]");

    /// 客户端模式下，直接调用transport层写操作
    if (PROP_CLIENT == mode_) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::ProcessWriteMessage[CLIENT mode][Enter]");

      if (true != session_id.empty()) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::ProcessWriteMessage[Fail][session_id is not empty]");
        break;
      }

      YSOS_LOG_DEBUG("CommonConnectionImpl::ProcessWriteMessage[Check Point][0]");

      if (NULL == transport_ptr_) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::ProcessWriteMessage[Fail][transport_ptr_ is NULL]");
        break;
      }

      YSOS_LOG_DEBUG("CommonConnectionImpl::ProcessWriteMessage[Check Point][1]");

      if (YSOS_ERROR_SUCCESS != transport_ptr_->Write(input_buffer_ptr, input_length)) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::ProcessWriteMessage[Fail]The function[transport_ptr_->Write()] is failed");
        break;
      }

      YSOS_LOG_DEBUG("CommonConnectionImpl::ProcessWriteMessage[CLIENT mode][End]");
    }
    /// 服务器模式下，先用session_id转换为connection指针，然后在连接链表里查找，如果存在再调用对应的connectiont的Write函数
    else if (PROP_SERVER == mode_) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::ProcessWriteMessage[SERVER mode][Enter]");

      YSOS_LOG_DEBUG("CommonConnectionImpl::ProcessWriteMessage[Check Point][session id][" << session_id << "]");

      /// 查找session id与connection ptr的匹配表
      ConnectionInterfacePtr connection_ptr = GetConnectionPtrWithSessionID(session_id);
      if (NULL == connection_ptr) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::ProcessWriteMessage[Fail][connection_ptr is NULL]");
        break;
      }

      YSOS_LOG_DEBUG("CommonConnectionImpl::ProcessWriteMessage[Check Point][session id][" << session_id << "]");

      YSOS_LOG_DEBUG("CommonConnectionImpl::ProcessWriteMessage[Check Point][2]");

      /// 查找连接链表
      std::list<ConnectionInterfacePtr>::iterator iterator = std::find(connection_ptr_list_.begin(), connection_ptr_list_.end(), connection_ptr);
      if (iterator != connection_ptr_list_.end()) {
        /// 找到
        YSOS_LOG_DEBUG("CommonConnectionImpl::ProcessWriteMessage[To find session_id[" << session_id << "]");
        YSOS_LOG_DEBUG("CommonConnectionImpl::ProcessWriteMessage[Check Point][connection_ptr][" << (UINT64)(connection_ptr.get()) << "]" << "use count[" << connection_ptr.use_count() << "]");

        /// 调用子连接的write
        if (YSOS_ERROR_SUCCESS != connection_ptr->Write(input_buffer_ptr, input_length)) {
          YSOS_LOG_DEBUG("CommonConnectionImpl::ProcessWriteMessage[Fail]The function[connection_ptr->Write()] is failed");
          break;
        }

        YSOS_LOG_DEBUG("CommonConnectionImpl::ProcessWriteMessage[Check Point][3]");
      } else {
        /// 没找到
        YSOS_LOG_DEBUG("CommonConnectionImpl::ProcessWriteMessage[Fail]To find session_id[" << session_id << "] ");
        break;
      }

      YSOS_LOG_DEBUG("CommonConnectionImpl::ProcessWriteMessage[SERVER mode][End]");
    } else {
      YSOS_LOG_DEBUG("CommonConnectionImpl::Write[Fail][The mode is not supported][" << mode_ << "]");
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::ProcessWriteMessage[Check Point][End]");

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  if (result != YSOS_ERROR_SUCCESS) {
    YSOS_LOG_DEBUG("CommonConnectionImpl::ProcessWriteMessage[Fail][Write operation is failed]");
  }

  /// 释放对象
  {
    AutoLockOper lock(&ll_lock_);
    write_job_object_pool_.destroy(write_job_params_ptr);
  }

  YSOS_LOG_DEBUG("CommonConnectionImpl::ProcessWriteMessage[Check Point][input_buffer_ptr][" << (UINT64)(input_buffer_ptr.get()) << "]" << "use count[" << input_buffer_ptr.use_count() << "]");
  YSOS_LOG_DEBUG("CommonConnectionImpl::ProcessWriteMessage[Check Point][input_length][" << input_length << "]");

  YSOS_LOG_DEBUG("CommonConnectionImpl::ProcessWriteMessage[Exit]");

  return true;
}

bool CommonConnectionImpl::ProcessInvalidConnection(ConnectionInterfacePtr& invalid_connection_ptr) {
  YSOS_LOG_DEBUG("CommonConnectionImpl::ProcessInvalidConnection[Enter]");

  /// 服务器模式下，各个服务器端点将无效端点添加到顶层基类的无效队列中，由顶层基类负责删除无效连接
  if (PROP_SERVER != mode_) {
    YSOS_LOG_DEBUG("CommonConnectionImpl::SetProperty[ProcessInvalidConnection][Fail][mode_ is not PROP_SERVER]");
    return false;
  }

  /// 关闭无效连接
  Close(&invalid_connection_ptr);

  YSOS_LOG_DEBUG("CommonConnectionImpl::ProcessInvalidConnection[Exit]");

  return true;
}

std::string CommonConnectionImpl::GetSessionIDWithConnectionPtr(const ConnectionInterfacePtr& connection_ptr) {
  YSOS_LOG_DEBUG("CommonConnectionImpl::GetSessionIDWithConnectionPtr[Enter]");

  std::string result;

  do {
    if (NULL == connection_ptr) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::GetSessionIDWithConnectionPtr[Fail][connection_ptr is NULL]");
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::GetSessionIDWithConnectionPtr[Check Point][0]");

    {
      AutoLockOper lock(&ll_lock_);

      SessionIDMatchingTableIterator itr = std::find_if(session_id_matching_table_.begin(), session_id_matching_table_.end(), SessionIDMatchingTableValueFinder(connection_ptr));
      if (itr != session_id_matching_table_.end()) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::GetSessionIDWithConnectionPtr[Find session id][" << itr->first << "]");
        result = itr->first;
      }
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::GetSessionIDWithConnectionPtr[Check Point][End]");

  } while (0);

  YSOS_LOG_DEBUG("CommonConnectionImpl::GetSessionIDWithConnectionPtr[Exit]");

  return std::string(result);
}

ConnectionInterfacePtr CommonConnectionImpl::GetConnectionPtrWithSessionID(const std::string& session_id) {
  YSOS_LOG_DEBUG("CommonConnectionImpl::GetConnectionPtrWithSessionID[Enter]");

  ConnectionInterfacePtr result = NULL;

  do {
    if (PROP_CLIENT == mode_) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::GetConnectionPtrWithSessionID[Fail][GetMode() is PROP_CLIENT][" << mode_ << "]");
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::GetConnectionPtrWithSessionID[Check Point][0]");

    if (true == session_id.empty()) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::GetConnectionPtrWithSessionID[Fail][session_id is empty]");
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::GetConnectionPtrWithSessionID[Check Point][1]");

    {
      AutoLockOper lock(&ll_lock_);

      SessionIDMatchingTableIterator itr = session_id_matching_table_.find(session_id);
      if (itr != session_id_matching_table_.end()) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::GetConnectionPtrWithSessionID[Find connection ptr][" << (UINT64)(itr->second.get()) << "]");
        result = itr->second;
        break;
      }
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::GetConnectionPtrWithSessionID[Check Point][End]");

  } while (0);

  YSOS_LOG_DEBUG("CommonConnectionImpl::GetConnectionPtrWithSessionID[Exit]");

  return result;
}

std::string CommonConnectionImpl::GenerateSessionIDWithConnectionPtr(const ConnectionInterfacePtr& connection_ptr) {
  YSOS_LOG_DEBUG("CommonConnectionImpl::GenerateSessionIDWithConnectionPtr[Enter]");

  std::string result;

  do {
    if (PROP_CLIENT == mode_) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::GenerateSessionIDWithConnectionPtr[Fail][GetMode() is PROP_CLIENT][" << mode_ << "]");
      break;
    }

    if (NULL == connection_ptr) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::GenerateSessionIDWithConnectionPtr[Fail][connection_ptr is NULL]");
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::GenerateSessionIDWithConnectionPtr[Check Point][0]");

    /// 查找是否已保存
    result = GetSessionIDWithConnectionPtr(connection_ptr);

    /// 已保存，则返回空字符串
    if (true != result.empty()) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::GenerateSessionIDWithConnectionPtr[Fail][result is not empty][" << result << "]");
      result.clear();
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::GenerateSessionIDWithConnectionPtr[Check Point][1]");

    /// 没保存，则生成新session id
    boost::uuids::uuid new_uuid = boost::uuids::random_generator()();
    result = to_string(new_uuid);

    YSOS_LOG_DEBUG("CommonConnectionImpl::GenerateSessionIDWithConnectionPtr[Check Point][2][new session id][" << result << "]");

    {
      AutoLockOper lock(&ll_lock_);

      /// 保存session id和connection指针
      session_id_matching_table_.insert(SessionIDMatchingTableUnit(result, connection_ptr));
    }

  } while (0);

  YSOS_LOG_DEBUG("CommonConnectionImpl::GenerateSessionIDWithConnectionPtr[Exit]");

  return std::string(result);
}

int CommonConnectionImpl::SaveSessionIDnConnectionPtr(const std::string& session_id, const ConnectionInterfacePtr& connection_ptr) {
  YSOS_LOG_DEBUG("CommonConnectionImpl::SaveSessionIDnConnectionPtr[Enter]");

  int result = YSOS_ERROR_FAILED;

  do {
    if (PROP_CLIENT != mode_) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::SaveSessionIDnConnectionPtr[Fail][GetMode() is not PROP_CLIENT][" << mode_ << "]");
      break;
    }

    if (true == session_id.empty()) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::SaveSessionIDnConnectionPtr[Fail][session_id is empty]");
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::SaveSessionIDnConnectionPtr[Check Point][0][session_id][" << session_id << "]");

    {
      AutoLockOper lock(&ll_lock_);
      session_id_matching_table_.insert(SessionIDMatchingTableUnit(session_id, connection_ptr));
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::SaveSessionIDnConnectionPtr[Check Point][End]");

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  YSOS_LOG_DEBUG("CommonConnectionImpl::SaveSessionIDnConnectionPtr[Exit]");

  return result;
}

int CommonConnectionImpl::SetConnectionPtrWithSessionID(const std::string& session_id, const ConnectionInterfacePtr& connection_ptr, bool is_restored) {
  YSOS_LOG_DEBUG("CommonConnectionImpl::SetConnectionPtrWithSessionID[Enter]");

  int result = YSOS_ERROR_FAILED;

  do {
    if (PROP_CLIENT == mode_) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::SetConnectionPtrWithSessionID[Fail][GetMode() is PROP_CLIENT][" << mode_ << "]");
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::SetConnectionPtrWithSessionID[Check Point][0]");

    if (true == session_id.empty() ||
        NULL == connection_ptr) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::SetConnectionPtrWithSessionID[Fail][session_id is empty or connection_ptr is empty]");
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::SetConnectionPtrWithSessionID[session_id][" << session_id << "]");
    YSOS_LOG_DEBUG("CommonConnectionImpl::SetConnectionPtrWithSessionID[connection_ptr][" << (UINT64)(connection_ptr.get()) << "]");

    /// for debug
    DumpSessionIDMatchingTable();

    {
      AutoLockOper lock(&ll_lock_);

      /// 通过session_id查找connection指针
      SessionIDMatchingTableIterator itr = session_id_matching_table_.find(session_id);
      if (itr != session_id_matching_table_.end()) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::SetConnectionPtrWithSessionID[Before][Find connection ptr][" << (UINT64)(itr->second.get()) << "]");
        /// 更新原有指针
        itr->second = connection_ptr;
        YSOS_LOG_DEBUG("CommonConnectionImpl::SetConnectionPtrWithSessionID[After][Find connection ptr][" << (UINT64)(itr->second.get()) << "]");
      } else {
        YSOS_LOG_DEBUG("CommonConnectionImpl::SetConnectionPtrWithSessionID[Can't find session_id]");
        if (true == is_restored) {
          YSOS_LOG_DEBUG("CommonConnectionImpl::SetConnectionPtrWithSessionID[Restore session_id and connection ptr]");

          /// 重新添加一个新的对象
          session_id_matching_table_.insert(SessionIDMatchingTableUnit(session_id, connection_ptr));
          result = YSOS_ERROR_SUCCESS;
        }

        break;
      }
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::SetConnectionPtrWithSessionID[Check Point][End]");

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  YSOS_LOG_DEBUG("CommonConnectionImpl::SetConnectionPtrWithSessionID[Exit]");

  return result;
}

int CommonConnectionImpl::SetSessionIDWithConnectionPtr(const ConnectionInterfacePtr& connection_ptr, const std::string& session_id) {
  YSOS_LOG_DEBUG("CommonConnectionImpl::SetSessionIDWithConnectionPtr[Enter]");

  int result = YSOS_ERROR_FAILED;

  do {
    if (PROP_CLIENT == mode_) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::SetSessionIDWithConnectionPtr[Fail][GetMode() is PROP_CLIENT][" << mode_ << "]");
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::SetSessionIDWithConnectionPtr[Check Point][0]");

    if (NULL == connection_ptr ||
        true == session_id.empty()) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::SetSessionIDWithConnectionPtr[Fail][connection_ptr is empty or session_id is empty]");
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::SetSessionIDWithConnectionPtr[session_id][" << session_id << "]");
    YSOS_LOG_DEBUG("CommonConnectionImpl::SetSessionIDWithConnectionPtr[connection_ptr][" << (UINT64)(connection_ptr.get()) << "]");

    /// for debug
    DumpSessionIDMatchingTable();

    {
      AutoLockOper lock(&ll_lock_);

      SessionIDMatchingTableIterator itr = std::find_if(session_id_matching_table_.begin(), session_id_matching_table_.end(), SessionIDMatchingTableValueFinder(connection_ptr));
      if (itr != session_id_matching_table_.end()) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::SetSessionIDWithConnectionPtr[Find session id][" << itr->first << "]");
        /// 将找到的connection指针对应的对象删掉
        session_id_matching_table_.erase(itr);

        /// 重新添加一个新的对象
        if (YSOS_ERROR_SUCCESS != SaveSessionIDnConnectionPtr(session_id, connection_ptr)) {
          YSOS_LOG_DEBUG("CommonConnectionImpl::SetSessionIDWithConnectionPtr[Fail]The function[SaveSessionIDnConnectionPtr()] is failed");
          break;
        }
      } else {
        YSOS_LOG_DEBUG("CommonConnectionImpl::SetSessionIDWithConnectionPtr[Can't find connection_ptr]");
        break;
      }
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::SetSessionIDWithConnectionPtr[Check Point][End]");

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  YSOS_LOG_DEBUG("CommonConnectionImpl::SetSessionIDWithConnectionPtr[Exit]");

  return result;
}

int CommonConnectionImpl::ResetConnectionPtr(const ConnectionInterfacePtr& connection_ptr, const ConnectionInterfacePtr& new_connection_ptr) {
  YSOS_LOG_DEBUG("CommonConnectionImpl::ResetConnectionPtr[Enter]");

  int result = YSOS_ERROR_FAILED;

  do {
    if (PROP_CLIENT == mode_) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::ResetConnectionPtr[Fail][GetMode() is PROP_CLIENT][" << mode_ << "]");
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::ResetConnectionPtr[Check Point][0]");

    if (NULL == connection_ptr) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::ResetConnectionPtr[Fail][connection_ptr is empty]");
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::ResetConnectionPtr[connection_ptr][" << (UINT64)(connection_ptr.get()) << "]");

    /// for debug
    DumpSessionIDMatchingTable();

    {
      AutoLockOper lock(&ll_lock_);

      SessionIDMatchingTableIterator itr = std::find_if(session_id_matching_table_.begin(), session_id_matching_table_.end(), SessionIDMatchingTableValueFinder(connection_ptr));
      if (itr != session_id_matching_table_.end()) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::ResetConnectionPtr[Find session id][" << itr->first << "]");
        /// 将找到的connection指针重置
        itr->second = new_connection_ptr;
      } else {
        YSOS_LOG_DEBUG("CommonConnectionImpl::ResetConnectionPtr[Can't find connection_ptr]");
        break;
      }
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::ResetConnectionPtr[Check Point][End]");

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  YSOS_LOG_DEBUG("CommonConnectionImpl::ResetConnectionPtr[Exit]");

  return result;
}

int CommonConnectionImpl::ResetSessionID(const std::string& session_id, const std::string& new_session_id) {
  YSOS_LOG_DEBUG("CommonConnectionImpl::ResetSessionID[Enter]");

  int result = YSOS_ERROR_FAILED;

  do {
    if (PROP_CLIENT == mode_) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::ResetSessionID[Fail][GetMode() is PROP_CLIENT][" << mode_ << "]");
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::ResetSessionID[Check Point][0]");

    if (true == session_id.empty() ||
        true == new_session_id.empty()) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::ResetSessionID[Fail][session_id is empty or new_session_id is empty]");
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::ResetSessionID[session_id][" << session_id << "]");
    YSOS_LOG_DEBUG("CommonConnectionImpl::ResetSessionID[new_session_id][" << new_session_id << "]");

    /// for debug
    DumpSessionIDMatchingTable();

    {
      AutoLockOper lock(&ll_lock_);

      /// 通过session_id查找connection指针
      SessionIDMatchingTableIterator itr = session_id_matching_table_.find(session_id);
      if (itr != session_id_matching_table_.end()) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::ResetSessionID[Before][Find connection ptr][" << (UINT64)(itr->second.get()) << "]");
        /// 保存原有指针
        ConnectionInterfacePtr connection_ptr = itr->second;

        /// 删除session_id对应的对象
        session_id_matching_table_.erase(itr);

        /// 重新添加一个新的对象
        if (YSOS_ERROR_SUCCESS != SaveSessionIDnConnectionPtr(new_session_id, connection_ptr)) {
          YSOS_LOG_DEBUG("CommonConnectionImpl::ResetSessionID[Fail]The function[SaveSessionIDnConnectionPtr()] is failed");
          break;
        }
      } else {
        YSOS_LOG_DEBUG("CommonConnectionImpl::ResetSessionID[Can't find session_id]");
        break;
      }
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::ResetSessionID[Check Point][End]");

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  YSOS_LOG_DEBUG("CommonConnectionImpl::ResetSessionID[Exit]");

  return result;
}

void CommonConnectionImpl::DumpSessionIDMatchingTable() {
  YSOS_LOG_DEBUG("CommonConnectionImpl::DumpSessionIDMatchingTable[Enter]");

  if (PROP_CLIENT == mode_) {
    YSOS_LOG_DEBUG("CommonConnectionImpl::DumpSessionIDMatchingTable[Fail][GetMode() is PROP_CLIENT][" << mode_ << "]");
    return;
  }

  {
    AutoLockOper lock(&ll_lock_);

    if (0 < session_id_matching_table_.size()) {
      for (SessionIDMatchingTableIterator itr = session_id_matching_table_.begin();
           itr != session_id_matching_table_.end();
           ++itr) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::DumpSessionIDMatchingTable[session id][" << itr->first << "]" << "[connection ptr][" << (UINT64)(itr->second.get()) << "]");
      }
    }
  }

  YSOS_LOG_DEBUG("CommonConnectionImpl::DumpSessionIDMatchingTable[Exit]");

  return;
}


int CommonConnectionImpl::ReadHeartbeat() {
  YSOS_LOG_DEBUG("CommonConnectionImpl::ReadHeartbeat[Enter]");

  int result = YSOS_ERROR_FAILED;

  do {
    /// 判断模块是否已开启
    if (IS_RUNNING != GetRunningStatus()) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::ReadHeartbeat[It is not running]");
      result = YSOS_ERROR_INVALID_OPERATION;
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::ReadHeartbeat[Check Point][0]");

    /// 设置读取心跳操作超时时间
    read_heartbeat_timer_.expires_from_now(boost::posix_time::seconds(read_heartbeat_deadline_time_));

    YSOS_LOG_DEBUG("CommonConnectionImpl::ReadHeartbeat[Check Point][1]");

    /// 异步等待超时后，计数器线程调用HandleReadHeartbeat
    read_heartbeat_timer_.async_wait(boost::bind(&CommonConnectionImpl::HandleReadHeartbeat, this));

    YSOS_LOG_DEBUG("CommonConnectionImpl::ReadHeartbeat[Check Point][End]");

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  YSOS_LOG_DEBUG("CommonConnectionImpl::ReadHeartbeat[Exit]");

  return result;
}

int CommonConnectionImpl::WriteHeartbeat() {
  YSOS_LOG_DEBUG("CommonConnectionImpl::WriteHeartbeat[Enter]");

  int result = YSOS_ERROR_FAILED;

  do {
    /// 判断模块是否已开启
    if (IS_RUNNING != GetRunningStatus()) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::WriteHeartbeat[It is not running]");
      result = YSOS_ERROR_INVALID_OPERATION;
      break;
    }

    /// 服务器端点/客户端模式下，将内容写入消息队列
    if (PROP_CLIENT == mode_ || PROP_SERVER_TERMINAL == mode_) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::WriteHeartbeat[CLIENT/SERVERTERMINAL mode][Enter]");

      /// 获取新buffer
      if (NULL == buffer_pool_ptr_) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::WriteHeartbeat[Fail][buffer_pool_ptr_ is NULL]");
        break;
      }

      YSOS_LOG_DEBUG("CommonConnectionImpl::WriteHeartbeat[Check Point][0]");

      BufferInterfacePtr new_buffer_ptr = GetBufferUtility()->GetBufferFromBufferPool(buffer_pool_ptr_);
      while (NULL == new_buffer_ptr) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::WriteHeartbeat[Get Buffer Ptr][Fail]");

        /// 判断模块是否已关闭
        if (IS_STOPPED == GetRunningStatus()) {
          YSOS_LOG_DEBUG("CommonConnectionImpl::WriteHeartbeat[It is already stopped]");
          return YSOS_ERROR_INVALID_OPERATION;
        }

        boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
        new_buffer_ptr = GetBufferUtility()->GetBufferFromBufferPool(buffer_pool_ptr_);
        if (NULL != new_buffer_ptr) {
          YSOS_LOG_DEBUG("CommonConnectionImpl::WriteHeartbeat[Get Buffer Ptr][Sucess]");
          break;
        }
      }

      YSOS_LOG_DEBUG("CommonConnectionImpl::WriteHeartbeat[Start][buffer_ptr][" << reinterpret_cast<UINT64>(new_buffer_ptr.get()) << "]");
      YSOS_LOG_DEBUG("CommonConnectionImpl::WriteHeartbeat[Start][buffer_ptr use count][" << new_buffer_ptr.use_count() << "]");

      /// 设置buffer的长度
      if (YSOS_ERROR_SUCCESS != GetBufferUtility()->SetBufferLength(new_buffer_ptr, (GetBufferUtility()->GetBufferMaxLength(new_buffer_ptr) - WRITE_BUFFER_PREFIX))) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::WriteHeartbeat[Fail][The function[GetBufferUtility()->SetBufferLength()] is failed.]");
        break;
      }

      YSOS_LOG_DEBUG("CommonConnectionImpl::WriteHeartbeat[Check Point][1]");

      /// 设置buffer的prefix长度
      if (YSOS_ERROR_SUCCESS != GetBufferUtility()->SetBufferPrefixLength(new_buffer_ptr, WRITE_BUFFER_PREFIX)) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::WriteHeartbeat[Fail][The function[GetBufferUtility()->SetBufferPrefixLength()] is failed.]");
        break;
      }

      YSOS_LOG_DEBUG("CommonConnectionImpl::WriteHeartbeat[Check Point][2]");

      /// 初始化buffer
      if (YSOS_ERROR_SUCCESS != GetBufferUtility()->InitialBuffer(new_buffer_ptr)) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::WriteHeartbeat[Fail]The funtion[GetBufferUtility()->InitialBuffer()] is failed");
        break;
      }

      YSOS_LOG_DEBUG("CommonConnectionImpl::WriteHeartbeat[Check Point][3]");

      /// 获取session id
      std::string session_id;
      if (YSOS_ERROR_SUCCESS != GetProperty(PROP_SESSIONID, &session_id)) {
        YSOS_LOG_DEBUG("JsonRPCConnectionCallbackImpl::WriteHeartbeat[Fail]The function[connection_ptr->SetProperty()] is failed");
        break;
      }

      YSOS_LOG_DEBUG("CommonConnectionImpl::WriteHeartbeat[Check Point][4]");

      if (true == session_id.empty()) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::WriteHeartbeat[Fail][session_id is empty]");
        break;
      }

      /// 客户端模式下，写入心跳请求
      if (PROP_CLIENT == mode_) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::WriteHeartbeat[CLIENT mode][Enter]");

        PlatformProtocolImpl::RequestMessagePack message_pack;

        /// message header
        message_pack.message_header.type = boost::lexical_cast<std::string>(YSOS_AGENT_MESSAGE_HEART_BEAT_REQ);
        message_pack.message_header.session_id = session_id;

        std::string serial_number;
        GetSerialNumber(serial_number);
        message_pack.message_header.serial_number = serial_number;

        /// message body
        message_pack.message_body.to = connection_callback_impl_ptr_->GetAddress();
        message_pack.message_body.param.service_name = YSOS_AGENT_SERVICE_NAME_HEART_BEAT;

        YSOS_LOG_DEBUG("CommonConnectionImpl::WriteHeartbeat[CLIENT mode][Check Point][0]");

        /// 填写日志内容
        JsonValue json_value;
        json_value[g_request_service_param_log_content] = "log";

        std::string json_value_string;
        if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonValueToString(json_value, json_value_string)) {
          YSOS_LOG_DEBUG("CommonConnectionImpl::WriteHeartbeat[Fail]The funtion[PlatformProtocolImpl::JsonValueToString()] is failed");
          break;
        }

        YSOS_LOG_DEBUG("CommonConnectionImpl::WriteHeartbeat[CLIENT mode][Check Point][1]");

        if (true == json_value_string.empty()) {
          YSOS_LOG_DEBUG("CommonConnectionImpl::WriteHeartbeat[Fail][json_value_string is empty]");
          break;
        }

        message_pack.message_body.param.service_param = json_value_string;

        YSOS_LOG_DEBUG("CommonConnectionImpl::WriteHeartbeat[CLIENT mode][Check Point][2]");

        /// 封装消息为标准通讯json字符串格式
        if (NULL == protocol_ptr_) {
          YSOS_LOG_DEBUG("CommonConnectionImpl::WriteHeartbeat[Fail][protocol_ptr_ is NULL.]");
          break;
        }

        YSOS_LOG_DEBUG("CommonConnectionImpl::WriteHeartbeat[CLIENT mode][Check Point][3]");

        if (protocol_ptr_->FormMessage(NULL, new_buffer_ptr, PROP_REQUEST, &message_pack)) {
          YSOS_LOG_DEBUG("CommonConnectionImpl::WriteHeartbeat[Fail][The function[protocol_ptr_->FormMessage()] is failed.]");
          break;
        }

        YSOS_LOG_DEBUG("CommonConnectionImpl::WriteHeartbeat[CLIENT mode][Check Point][4]");

        /// 调用Write写入心跳数据
        if (YSOS_ERROR_SUCCESS != Write(new_buffer_ptr, GetBufferUtility()->GetBufferLength(new_buffer_ptr))) {
          YSOS_LOG_DEBUG("CommonConnectionImpl::WriteHeartbeat[Fail][The function[Write()] is failed.]");
          break;
        }

        YSOS_LOG_DEBUG("CommonConnectionImpl::WriteHeartbeat[CLIENT mode][End]");
      }
      /// 服务器端点模式下，写入心跳响应
      else {
        YSOS_LOG_DEBUG("CommonConnectionImpl::WriteHeartbeat[SERVERTERMINAL mode][Enter]");

        PlatformProtocolImpl::ResponseMessagePack message_pack;

        /// message header
        message_pack.message_header.type = boost::lexical_cast<std::string>(YSOS_AGENT_MESSAGE_HEART_BEAT_RSP);
        message_pack.message_header.session_id = session_id;

        std::string serial_number;
        GetSerialNumber(serial_number);
        message_pack.message_header.serial_number = serial_number;

        /// message body
        message_pack.message_body.from = connection_callback_impl_ptr_->GetAddress();
        message_pack.message_body.param.service_name = YSOS_AGENT_SERVICE_NAME_HEART_BEAT;

        YSOS_LOG_DEBUG("CommonConnectionImpl::WriteHeartbeat[SERVERTERMINAL mode][Check Point][0]");

        /// 填写日志返回内容
        JsonValue json_value;
        json_value[g_response_result_status_code] = "0";
        json_value[g_response_result_description] = "succeeded";
        json_value[g_response_result_detail] = JsonNullValue;

        std::string json_value_string;
        if (YSOS_ERROR_SUCCESS != PlatformProtocolImpl::JsonValueToString(json_value, json_value_string)) {
          YSOS_LOG_DEBUG("CommonConnectionImpl::WriteHeartbeat[Fail]The funtion[PlatformProtocolImpl::JsonValueToString()] is failed");
          break;
        }

        YSOS_LOG_DEBUG("CommonConnectionImpl::WriteHeartbeat[SERVERTERMINAL mode][Check Point][1]");

        if (true == json_value_string.empty()) {
          YSOS_LOG_DEBUG("CommonConnectionImpl::WriteHeartbeat[Fail][json_value_string is empty]");
          break;
        }

        message_pack.message_body.param.result_string = json_value_string;

        YSOS_LOG_DEBUG("CommonConnectionImpl::WriteHeartbeat[SERVERTERMINAL mode][Check Point][2]");

        /// 封装消息为标准通讯json字符串格式
        if (NULL == protocol_ptr_) {
          YSOS_LOG_DEBUG("CommonConnectionImpl::WriteHeartbeat[Fail][protocol_ptr_ is NULL.]");
          break;
        }

        YSOS_LOG_DEBUG("CommonConnectionImpl::WriteHeartbeat[SERVERTERMINAL mode][Check Point][3]");

        if (protocol_ptr_->FormMessage(NULL, new_buffer_ptr, PROP_RESPONSE, &message_pack)) {
          YSOS_LOG_DEBUG("CommonConnectionImpl::WriteHeartbeat[Fail][The function[protocol_ptr_->FormMessage()] is failed.]");
          break;
        }

        YSOS_LOG_DEBUG("CommonConnectionImpl::WriteHeartbeat[SERVERTERMINAL mode][Check Point][4]");

        /// 调用基类的Write函数，写入心跳数据
        if (NULL == root_connection_ptr_) {
          YSOS_LOG_DEBUG("CommonConnectionImpl::WriteHeartbeat[Fail][root_connection_ptr_ is NULL]");
          break;
        }

        YSOS_LOG_DEBUG("CommonConnectionImpl::WriteHeartbeat[SERVERTERMINAL mode][Check Point][5]");

        if (YSOS_ERROR_SUCCESS != root_connection_ptr_->Write(new_buffer_ptr, GetBufferUtility()->GetBufferLength(new_buffer_ptr), const_cast<char*>(session_id.c_str()))) {
          YSOS_LOG_DEBUG("CommonConnectionImpl::WriteHeartbeat[Fail][The function[root_connection_ptr_->Write()] is failed.]");
          break;
        }

        YSOS_LOG_DEBUG("CommonConnectionImpl::WriteHeartbeat[SERVERTERMINAL mode][Check Point][End]");
      }

      YSOS_LOG_DEBUG("CommonConnectionImpl::WriteHeartbeat[End][buffer_ptr][" << reinterpret_cast<UINT64>(new_buffer_ptr.get()) << "]");
      YSOS_LOG_DEBUG("CommonConnectionImpl::WriteHeartbeat[End][buffer_ptr use count][" << new_buffer_ptr.use_count() << "]");

      YSOS_LOG_DEBUG("CommonConnectionImpl::WriteHeartbeat[CLIENT/SERVERTERMINAL mode][End]");
    } else {
      YSOS_LOG_DEBUG("CommonConnectionImpl::WriteHeartbeat[Fail][Unknown mode. It is not supported]");
      break;
    }

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  YSOS_LOG_DEBUG("CommonConnectionImpl::WriteHeartbeat[Exit]");

  return result;
}

void CommonConnectionImpl::HandleReadHeartbeat() {
  YSOS_LOG_DEBUG("CommonConnectionImpl::HandleReadHeartbeat[Enter]");

  int result = YSOS_ERROR_FAILED;

  do {
    /// 判断模块是否已关闭
    if (IS_STOPPED == GetRunningStatus()) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::HandleReadHeartbeat[It is already stopped]");
      result = YSOS_ERROR_SUCCESS;
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::HandleReadHeartbeat[Check Point][0]");

    /// 判断读取心跳工作队列为空，启动最大失联次数机制
    if (0 == read_heartbeat_job_queue_.GetSize()) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::HandleReadHeartbeat[Check Point][The size of read_heartbeat_job_queue_ is 0][Start]");

      /// 增加最大失联次数计数器
      ++legitimate_inconnection_times_counter_;

      YSOS_LOG_DEBUG("CommonConnectionImpl::HandleReadHeartbeat[Check Point][legitimate_inconnection_times_counter_][" << legitimate_inconnection_times_counter_ << "]");

      /// 判断是超出最大失联次数
      if (LEGITIMATE_INCONNECTION_TIMES <= legitimate_inconnection_times_counter_) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::HandleReadHeartbeat[Check Point][legitimate_inconnection_times_counter_ is greater than LEGITIMATE_INCONNECTION_TIMES]");

        /// 判断是失联，调用错误处理
        boost::system::error_code error_code;
        HandleError(error_code, PROP_ON_DISCONNECT, NULL);
      }

      YSOS_LOG_DEBUG("CommonConnectionImpl::HandleReadHeartbeat[Check Point][The size of read_heartbeat_job_queue_ is 0][End]");
    }
    /// 判断读取心跳工作队列不为空，继续回调ReadHeartbeat
    else {
      YSOS_LOG_DEBUG("CommonConnectionImpl::HandleReadHeartbeat[Check Point][The size of read_heartbeat_job_queue_ is [" << read_heartbeat_job_queue_.GetSize() << "]");

      /// 重置最大失联计数器
      legitimate_inconnection_times_counter_ = 0;

      /// 销毁读取心跳工作队列中最前面的
      read_heartbeat_job_queue_.Pop();

      YSOS_LOG_DEBUG("CommonConnectionImpl::HandleReadHeartbeat[Check Point][The size of read_heartbeat_job_queue_ is [" << read_heartbeat_job_queue_.GetSize() << "]");

      /// 继续调用ReadHeartbeat达到心跳读取操作循环
      if (YSOS_ERROR_SUCCESS != ReadHeartbeat()) {
        YSOS_LOG_DEBUG("CommonConnectionImpl::HandleReadHeartbeat[Fail]The funtion[ReadHeartbeat()] is failed");
        break;
      }

      YSOS_LOG_DEBUG("CommonConnectionImpl::HandleReadHeartbeat[Check Point][1]");
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::HandleReadHeartbeat[Check Point][End]");

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  YSOS_LOG_DEBUG("CommonConnectionImpl::HandleReadHeartbeat[Exit]");

  return;
}

void CommonConnectionImpl::HandleWriteHeartbeat(const std::string& time_stamp) {
  YSOS_LOG_DEBUG("CommonConnectionImpl::HandleWriteHeartbeat[Enter]");

  int result = YSOS_ERROR_FAILED;

  do {
    /// 判断模块是否已关闭
    if (IS_STOPPED == GetRunningStatus()) {
      YSOS_LOG_DEBUG("CommonConnectionImpl::HandleWriteHeartbeat[It is already stopped]");
      result = YSOS_ERROR_SUCCESS;
      break;
    }

    YSOS_LOG_DEBUG("CommonConnectionImpl::HandleWriteHeartbeat[Check Point][0]");

    /// 设置写入心跳操作超时时间
    write_heartbeat_timer_.expires_from_now(boost::posix_time::seconds(write_heartbeat_deadline_time_));

    YSOS_LOG_DEBUG("CommonConnectionImpl::HandleWriteHeartbeat[Check Point][1]");

    /// 异步等待超时后，计数器线程调用WriteHeartbeat
    write_heartbeat_timer_.async_wait(boost::bind(&CommonConnectionImpl::WriteHeartbeat, this));

    YSOS_LOG_DEBUG("CommonConnectionImpl::HandleWriteHeartbeat[Check Point][End]");

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  YSOS_LOG_DEBUG("CommonConnectionImpl::HandleWriteHeartbeat[Exit]");

  return;
}

bool CommonConnectionImpl::ProcessWriteHeartbeatMessage(const std::string& time_stamp) {
  YSOS_LOG_DEBUG("CommonConnectionImpl::ProcessWriteHeartbeatMessage[Enter]");

  /// 调用HandleWriteHeartbeat处理写入心跳
  HandleWriteHeartbeat(time_stamp);

  YSOS_LOG_DEBUG("CommonConnectionImpl::ProcessWriteHeartbeatMessage[Exit]");

  return true;
}

void CommonConnectionImpl::GetSerialNumber(std::string& serial_number) {
  char temp_serial_number[HEARTBEAT_SERIAL_NUMBER_LENGTH] = {0,};
  //need update for linux
  #ifdef _WIN32 
    sprintf_s(temp_serial_number, HEARTBEAT_SERIAL_NUMBER_LENGTH, "%010d", serial_number_);  //need update for linux
  #else
    sprintf(temp_serial_number, "%010d", (int)serial_number_);   //need update for linux
  #endif
  ++serial_number_;
  serial_number = temp_serial_number;
  return;
}

void CommonConnectionImpl::RunIOService() {
  YSOS_LOG_DEBUG("CommonConnectionImpl::RunIOService[Enter]");
  run_io_service_thread_ = boost::thread(boost::bind(&boost::asio::io_service::run, &io_service_));
  YSOS_LOG_DEBUG("CommonConnectionImpl::RunIOService[Exit]");
  return;
}

}
