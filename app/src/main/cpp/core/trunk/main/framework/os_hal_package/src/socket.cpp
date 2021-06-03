/*
# socket.cpp
# Definition of socket
# Created on: 2016-05-11 19:46:58
# Original author: jinchengzhe
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
# version 0.1, 20160912, created by JinChengZhe
*/


#include <iostream> // need add for linux
#include <cstring>  // need add for linux

/// Private Headers
#include "../../../public/include/os_hal_package/socket.h"
/// Platform Headers
#include "../../../public/include/sys_interface_package/transportcallbackinterface.h"
#include "../../../public/include/core_help_package/utility.h"
#include "../../../public/include/os_hal_package/bufferutility.h"

namespace ysos {

/**
*@brief IOServicePool的具体实现  // NOLINT
*/
IOServicePool::IOServicePool(std::size_t pool_size /*= 4*/) : next_io_service_index_(0) {
  /// Give all the io_services work to do so that their run() functions will not
  /// exit until they are explicitly stopped.
  for (std::size_t ii = 0; ii < pool_size; ++ii) {
    IOServicePtr io_service_ptr(new IOService);
    IOServiceWorkPtr io_service_work_ptr(new IOServiceWork(*io_service_ptr));
    io_services_.push_back(io_service_ptr);
    io_service_works_.push_back(io_service_work_ptr);
  }
}

IOServicePool::~IOServicePool() {
  io_service_works_.clear();
  io_services_.clear();
}

void IOServicePool::Run() {
  /// Create a pool of threads to run all of the io_services.
  std::vector<boost::shared_ptr<boost::thread>> threads;
  for (std::size_t ii = 0; ii < io_services_.size(); ++ii) {
    boost::shared_ptr<boost::thread> thread_ptr(new boost::thread(boost::bind(&IOService::run, io_services_[ii])));
    threads.push_back(thread_ptr);
  }

  /// Wait for all threads in the pool to exit.
  for (std::size_t ii = 0; ii < threads.size(); ++ii) {
    boost::this_thread::interruption_point();
    threads[ii]->join();
  }

  return;
}

void IOServicePool::Stop() {
  /// Explicitly stop all io_services.
  for (std::size_t ii = 0; ii < io_services_.size(); ++ii) {
    io_services_[ii]->stop();
  }

  return;
}

IOService& IOServicePool::GetIOService() {
  /// Use a round-robin scheme to choose the next io_service to use.
  IOService& io_service = *io_services_[next_io_service_index_];
  ++next_io_service_index_;
  if (next_io_service_index_ == io_services_.size()) {
    next_io_service_index_ = 0;
  }

  return io_service;
}


/**
 *@brief BaseTCPTransportImpl的具体实现  // NOLINT
 */
UINT32 BaseTCPTransportImpl::header_info_size_ = sizeof(HeaderInfo);
BaseTCPTransportImpl::BaseTCPTransportImpl(const UINT32 mode, const std::size_t io_service_pool_size, const UINT32 running_status, const std::string strClassName /* = "BaseTCPTransportImpl" */)
  : BaseInterfaceImpl(strClassName),
    mode_(mode),
    io_service_pool_size_(io_service_pool_size),
    io_service_pool_(io_service_pool_size_),
    tcp_socket_(io_service_pool_.GetIOService()),
    signal_set_(io_service_pool_.GetIOService()),
    address_(""),
    port_(0),
    enable_write_(true),
    enable_read_(true),
    strand_(io_service_pool_.GetIOService()),
    running_status_(running_status) {
  logger_ = GetUtility()->GetLogger("ysos.sdk");
}

BaseTCPTransportImpl::~BaseTCPTransportImpl() {
  if (NULL != transport_callback_ptr_) {
    transport_callback_ptr_ = NULL;
  }

  running_status_ = IS_INITIAL;
}

int BaseTCPTransportImpl::Open(void *params) {
  YSOS_LOG_DEBUG("BaseTCPTransportImpl::Open[Enter]");

  int result = YSOS_ERROR_FAILED;

  do {
    /// 判断模块是否已开启
    if (IS_RUNNING == GetRunningStatus()) {
      YSOS_LOG_DEBUG("BaseTCPTransportImpl::Open[It is already runing]");
      result = YSOS_ERROR_SUCCESS;
      break;
    }

    YSOS_LOG_DEBUG("BaseTCPTransportImpl::Open[Check Point][0]");

    if (NULL == params) {
      YSOS_LOG_DEBUG("BaseTCPTransportImpl::Open[Fail][params is NULL]");
      break;
    }

    YSOS_LOG_DEBUG("BaseTCPTransportImpl::Open[Check Point][1]");

    if (YSOS_ERROR_SUCCESS != InitOpenParam(params)) {
      YSOS_LOG_DEBUG("BaseTCPTransportImpl::Open[Fail]The function[InitOpenParam()] is failed");
      break;
    }

    YSOS_LOG_DEBUG("BaseTCPTransportImpl::Open[Check Point][2]");

    if (YSOS_ERROR_SUCCESS != RealOpen()) {
      YSOS_LOG_DEBUG("BaseTCPTransportImpl::Open[Fail]The function[RealOpen()] is failed");
      break;
    }

    /// 设置模块运行成功
    SetRunningStatus(IS_RUNNING);

    YSOS_LOG_DEBUG("BaseTCPTransportImpl::Open[Check Point][End]");

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  YSOS_LOG_DEBUG("BaseTCPTransportImpl::Open[Exit]");

  return result;
}

void BaseTCPTransportImpl::Close() {
  YSOS_LOG_DEBUG("BaseTCPTransportImpl::Close[Enter]");

  /// 判断模块是否已关闭
  if (IS_STOPPED == GetRunningStatus()) {
    YSOS_LOG_DEBUG("BaseTCPTransportImpl::SetProperty[It is already stopped]");
    return;
  }

  YSOS_LOG_DEBUG("BaseTCPTransportImpl::Close[Check Point][0]");

  /// 停止读取循环，不再回调上层HandleError
  SetRunningStatus(IS_STOPPED);

  /// 停止io service pool
  io_service_pool_.Stop();

  YSOS_LOG_DEBUG("BaseTCPTransportImpl::Close[Check Point][1]");

  if (true == run_io_service_pool_thread_.joinable()) {
    YSOS_LOG_DEBUG("BaseTCPTransportImpl::Close[Check Point][join run_io_service_pool_thread_][Enter]");
    run_io_service_pool_thread_.join();
    YSOS_LOG_DEBUG("BaseTCPTransportImpl::Close[Check Point][join run_io_service_pool_thread_][End]");
  }

  YSOS_LOG_DEBUG("BaseTCPTransportImpl::Close[Check Point][2]");

  RealClose();

  YSOS_LOG_DEBUG("BaseTCPTransportImpl::Close[Exit]");

  return;
}

int BaseTCPTransportImpl::Read(BufferInterfacePtr buffer_ptr, int length) {
  YSOS_LOG_DEBUG("BaseTCPTransportImpl::Read[Enter]");

  /// 判断模块是否已开启
  if (IS_RUNNING != GetRunningStatus()) {
    YSOS_LOG_DEBUG("BaseTCPTransportImpl::Read[It is not running]");
    return YSOS_ERROR_INVALID_OPERATION;
  }

  YSOS_LOG_DEBUG("BaseTCPTransportImpl::Read[Check Point][0]");

  if (false == enable_read_) {
    YSOS_LOG_DEBUG("BaseTCPTransportImpl::Read[enable_read_ is false]");
    return YSOS_ERROR_INVALID_OPERATION;
  }

  YSOS_LOG_DEBUG("BaseTCPTransportImpl::Read[Check Point][1]");

  if (NULL == buffer_ptr || 0 == length) {
    YSOS_LOG_DEBUG("BaseTCPTransportImpl::Read[Fail][The parameters is invalid]");
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  YSOS_LOG_DEBUG("BaseTCPTransportImpl::Read[Check Point][2]");

  uint8_t* buffer_data_ptr = BufferUtility::Instance()->GetBufferData(buffer_ptr);
  if (NULL == buffer_data_ptr) {
    YSOS_LOG_DEBUG("BaseTCPTransportImpl::Read[Fail][buffer_data_ptr is NULL]");
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  YSOS_LOG_DEBUG("BaseTCPTransportImpl::Read[Check Point][3]");

  tcp_socket_.async_read_some(
    boost::asio::buffer(buffer_data_ptr, header_info_size_),
    strand_.wrap(boost::bind(
                   &BaseTCPTransportImpl::HandleReadHeaderInfo,
                   shared_from_this(),
                   boost::asio::placeholders::error,
                   boost::asio::placeholders::bytes_transferred,
                   buffer_ptr
                 ))
  );

  YSOS_LOG_DEBUG("BaseTCPTransportImpl::Read[Exit]");

  return YSOS_ERROR_SUCCESS;
}

int BaseTCPTransportImpl::Write(BufferInterfacePtr buffer_ptr, int length) {
  YSOS_LOG_DEBUG("BaseTCPTransportImpl::Write[Enter]");

  /// 判断模块是否已开启
  if (IS_RUNNING != GetRunningStatus()) {
    YSOS_LOG_DEBUG("BaseTCPTransportImpl::Write[It is not running]");
    return YSOS_ERROR_INVALID_OPERATION;
  }

  YSOS_LOG_DEBUG("BaseTCPTransportImpl::Write[Check Point][0]");

  if (false == enable_write_) {
    YSOS_LOG_DEBUG("BaseTCPTransportImpl::Write[enable_write_ is false]");
    return YSOS_ERROR_INVALID_OPERATION;
  }

  YSOS_LOG_DEBUG("BaseTCPTransportImpl::Write[Check Point][1]");

  if (NULL == buffer_ptr || 0 == length) {
    YSOS_LOG_DEBUG("BaseTCPTransportImpl::Write[Fail][The parameters is invalid]");
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  YSOS_LOG_DEBUG("BaseTCPTransportImpl::Write[Check Point][2]");

  /// for debug
  uint8_t* buffer_data_ptr = GetBufferUtility()->GetBufferData(buffer_ptr);
  if (NULL == buffer_data_ptr) {
    YSOS_LOG_DEBUG("BaseTCPTransportImpl::Write[Fail][buffer_data_ptr is NULL]");
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  YSOS_LOG_DEBUG("BaseTCPTransportImpl::Write[Check Point][org buffer_data_ptr][" << (char*)buffer_data_ptr << "]");
  YSOS_LOG_DEBUG("BaseTCPTransportImpl::Write[Check Point][length][" << length << "]");

  /// 封装消息头
  HeaderInfo header_info = {0, 0, (UINT32)length, 0};

  /// 将消息头内容写入buffer
  UINT32 buffer_prefix_length = (UINT32)(GetBufferUtility()->GetBufferPrefixLength(buffer_ptr)) - header_info_size_;
  if (buffer_prefix_length > (UINT32)(GetBufferUtility()->GetBufferMaxLength(buffer_ptr))) {
    YSOS_LOG_DEBUG("BaseTCPTransportImpl::Write[Fail][buffer_prefix_length is too big]");
    return YSOS_ERROR_FAILED;
  }

  YSOS_LOG_DEBUG("BaseTCPTransportImpl::Write[Check Point][buffer_prefix_length][" << buffer_prefix_length << "]");

  if (YSOS_ERROR_SUCCESS != GetBufferUtility()->SetBufferPrefixLength(buffer_ptr, buffer_prefix_length)) {
    YSOS_LOG_DEBUG("BaseTCPTransportImpl::Open[Fail]The function[GetBufferUtility()->SetBufferPrefixLength()] is failed");
    return YSOS_ERROR_FAILED;
  }

  YSOS_LOG_DEBUG("BaseTCPTransportImpl::Write[Check Point][3]");

//  uint8_t* buffer_data_ptr = GetBufferUtility()->GetBufferData(buffer_ptr);
  buffer_data_ptr = GetBufferUtility()->GetBufferData(buffer_ptr);
  if (NULL == buffer_data_ptr) {
    YSOS_LOG_DEBUG("BaseTCPTransportImpl::Write[Fail][buffer_data_ptr is NULL]");
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  YSOS_LOG_DEBUG("BaseTCPTransportImpl::Write[Check Point][4]");

  //todo: for buffer safe
  //memcpy_s(buffer_data_ptr, header_info_size_, &header_info, header_info_size_);  //for win32
  memcpy(buffer_data_ptr, &header_info, header_info_size_); // need add fro linux

  YSOS_LOG_DEBUG("BaseTCPTransportImpl::Write[Check Point][new buffer_data_ptr][" << ((char*)buffer_data_ptr + header_info_size_) << "]");

  /// 重新调整buffer长度
  if (GetBufferUtility()->SetBufferLength(buffer_ptr, (header_info_size_ + length))) {
    YSOS_LOG_DEBUG("BaseTCPTransportImpl::Open[Fail]The function[GetBufferUtility()->SetBufferLength()] is failed");
    return YSOS_ERROR_FAILED;
  }

  YSOS_LOG_DEBUG("BaseTCPTransportImpl::Write[Check Point][5]");

  int new_length = GetBufferUtility()->GetBufferLength(buffer_ptr);

  YSOS_LOG_DEBUG("BaseTCPTransportImpl::Write[Check Point][new_length][" << new_length << "]");

  tcp_socket_.async_write_some(
    boost::asio::buffer(buffer_data_ptr, new_length),
    strand_.wrap(
      boost::bind(
        &BaseTCPTransportImpl::HandleWrite,
        shared_from_this(),
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred,
        buffer_ptr
      )
    )
  );

  YSOS_LOG_DEBUG("BaseTCPTransportImpl::Write[Exit]");

  return YSOS_ERROR_SUCCESS;
}

void BaseTCPTransportImpl::EnableRead(bool is_enable) {
  AutoLockOper lock(&ll_lock_);
  enable_read_ = is_enable;
  return;
}

void BaseTCPTransportImpl::EnableWrite(bool is_enable) {
  AutoLockOper lock(&ll_lock_);
  enable_write_ = is_enable;
  return;
}

int BaseTCPTransportImpl::GetProperty(int type_id, void *type) {
  YSOS_LOG_DEBUG("BaseTCPTransportImpl::GetProperty[Enter]");

  AutoLockOper lock(&ll_lock_);

  int result = YSOS_ERROR_FAILED;

  switch (type_id) {
  case PROP_MODE: {
    YSOS_LOG_DEBUG("BaseTCPTransportImpl::GetProperty[PROP_MODE][Enter]");

    if (NULL == type) {
      YSOS_LOG_DEBUG("BaseTCPTransportImpl::GetProperty[Fail][type is NULL]");
      result = YSOS_ERROR_INVALID_ARGUMENTS;
      break;
    }

    YSOS_LOG_DEBUG("BaseTCPTransportImpl::GetProperty[Check Point][0]");

    UINT32* mode_ptr = static_cast<UINT32*>(type);
    *mode_ptr = mode_;

    result = YSOS_ERROR_SUCCESS;

    YSOS_LOG_DEBUG("BaseTCPTransportImpl::GetProperty[PROP_MODE][Exit]");
  }
  break;
  default:
    YSOS_LOG_DEBUG("BaseTCPTransportImpl::GetProperty[Unknown]");
    result = YSOS_ERROR_NOT_SUPPORTED;
    break;
  }

  YSOS_LOG_DEBUG("BaseTCPTransportImpl::GetProperty[Exit]");

  return result;
}

int BaseTCPTransportImpl::SetProperty(int type_id, void *type) {
  YSOS_LOG_DEBUG("BaseTCPTransportImpl::SetProperty[Enter]");

  AutoLockOper lock(&ll_lock_);

  int result = YSOS_ERROR_FAILED;

  switch (type_id) {
  case PROP_CALLBACK: {
    YSOS_LOG_DEBUG("BaseTCPTransportImpl::SetProperty[PROP_CALLBACK][Enter]");

    if (PROP_SERVER_TERMINAL != mode_) {
      YSOS_LOG_DEBUG("BaseTCPTransportImpl::SetProperty[mode_ is not SERVERTERMINAL]");
      break;
    }

    YSOS_LOG_DEBUG("BaseTCPTransportImpl::SetProperty[PROP_CALLBACK][Check Point][0]");

    if (NULL == type) {
      YSOS_LOG_DEBUG("BaseTCPTransportImpl::SetProperty[PROP_CALLBACK][Fail][type is NULL]");
      break;
    }

    YSOS_LOG_DEBUG("BaseTCPTransportImpl::SetProperty[PROP_CALLBACK][Check Point][1]");

    TransportCallbackInterfacePtr* transport_callback_ptr_ptr = static_cast<TransportCallbackInterfacePtr*>(type);

    YSOS_LOG_DEBUG("BaseTCPTransportImpl::SetProperty[PROP_CALLBACK][Check Point][2]");

    transport_callback_ptr_ = *transport_callback_ptr_ptr;

    YSOS_LOG_DEBUG("BaseTCPTransportImpl::SetProperty[PROP_CALLBACK][Check Point][3]");

    run_io_service_pool_thread_ = boost::thread(boost::bind(&IOServicePool::Run, &io_service_pool_));

    result = YSOS_ERROR_SUCCESS;

    YSOS_LOG_DEBUG("BaseTCPTransportImpl::SetProperty[PROP_CALLBACK][Exit]");
  }
  break;
  case PROP_MODE: {
    YSOS_LOG_DEBUG("BaseTCPTransportImpl::SetProperty[PROP_MODE][Enter]");

    if (NULL == type) {
      YSOS_LOG_DEBUG("BaseTCPTransportImpl::SetProperty[PROP_MODE][Fail][type is NULL]");
      break;
    }

    YSOS_LOG_DEBUG("BaseTCPTransportImpl::SetProperty[PROP_MODE][Check Point][0]");

    UINT32* mode_ptr = static_cast<UINT32*>(type);
    mode_ = *mode_ptr;

    result = YSOS_ERROR_SUCCESS;

    YSOS_LOG_DEBUG("BaseTCPTransportImpl::SetProperty[PROP_MODE][Exit]");
  }
  break;
  case PROP_HANDLECONNECT: {
    YSOS_LOG_DEBUG("BaseTCPTransportImpl::SetProperty[PROP_HANDLECONNECT][Enter]");

    if (PROP_CLIENT != mode_) {
      YSOS_LOG_DEBUG("BaseTCPTransportImpl::SetProperty[mode_ is not CLIENT]");
      break;
    }

    YSOS_LOG_DEBUG("BaseTCPTransportImpl::SetProperty[PROP_HANDLECONNECT][Check Point][0]");

    if (NULL != transport_callback_ptr_) {
      YSOS_LOG_DEBUG("TCPSocketClient::SetProperty[PROP_HANDLECONNECT][Check Point][transport_callback_ptr_ is not NULL]");
      SystemErrorCode error_code;
      transport_callback_ptr_->HandleConnect(error_code, shared_from_this());
    }

    result = YSOS_ERROR_SUCCESS;

    YSOS_LOG_DEBUG("BaseTCPTransportImpl::SetProperty[PROP_HANDLECONNECT][Exit]");
  }
  break;
  default:
    YSOS_LOG_DEBUG("BaseTCPTransportImpl::SetProperty[Unknown]");
    result = YSOS_ERROR_NOT_SUPPORTED;
    break;
  }

  YSOS_LOG_DEBUG("BaseTCPTransportImpl::SetProperty[Exit]");

  return result;
}

void BaseTCPTransportImpl::HandleReadHeaderInfo(const boost::system::error_code& error_code, const size_t bytes_transferred, const BufferInterfacePtr buffer_ptr) {
  YSOS_LOG_DEBUG("BaseTCPTransportImpl::HandleReadHeaderInfo[Enter]");

  int result = YSOS_ERROR_FAILED;

  do {
    /// 判断模块是否已关闭
    if (IS_STOPPED == GetRunningStatus()) {
      YSOS_LOG_DEBUG("BaseTCPTransportImpl::HandleReadHeaderInfo[It is already stopped]");
      result = YSOS_ERROR_SUCCESS;
      break;
    }

    YSOS_LOG_DEBUG("BaseTCPTransportImpl::HandleReadHeaderInfo[Check Point][0]");

    if (0 != error_code) {
      YSOS_LOG_DEBUG("BaseTCPTransportImpl::HandleReadHeaderInfo[error_code] [" << error_code.value() << "]" << " [error_message] [" << error_code.message() << "]");
      break;
    }

    YSOS_LOG_DEBUG("BaseTCPTransportImpl::HandleReadHeaderInfo[Check Point][bytes_transferred][" << bytes_transferred << "]");

    uint8_t* buffer_data_ptr = GetBufferUtility()->GetBufferData(buffer_ptr);
    if (NULL == buffer_data_ptr) {
      YSOS_LOG_DEBUG("BaseTCPTransportImpl::HandleReadHeaderInfo[Fail][buffer_data_ptr is NULL]");
      break;
    }

    YSOS_LOG_DEBUG("BaseTCPTransportImpl::HandleReadHeaderInfo[Check Point][1]");

    /// 强转成消息头结构体
    HeaderInfoPtr header_info_ptr = reinterpret_cast<HeaderInfoPtr>(buffer_data_ptr);
    UINT32 message_total_count = header_info_ptr->message_total_count;
    UINT32 current_message_index = header_info_ptr->current_message_index;
    UINT32 message_body_length = header_info_ptr->message_body_length;
    UINT32 version_info = header_info_ptr->version_info;

    YSOS_LOG_DEBUG("BaseTCPTransportImpl::HandleReadHeaderInfo[Check Point][message_total_count][" << message_total_count << "]");
    YSOS_LOG_DEBUG("BaseTCPTransportImpl::HandleReadHeaderInfo[Check Point][current_message_index][" << current_message_index << "]");
    YSOS_LOG_DEBUG("BaseTCPTransportImpl::HandleReadHeaderInfo[Check Point][message_body_length][" << message_body_length << "]");
    YSOS_LOG_DEBUG("BaseTCPTransportImpl::HandleReadHeaderInfo[Check Point][version_info][" << version_info << "]");

    if (0 == message_body_length) {
      YSOS_LOG_DEBUG("BaseTCPTransportImpl::HandleReadHeaderInfo[Fail][message_body_length is 0]");
      break;
    }

    YSOS_LOG_DEBUG("BaseTCPTransportImpl::HandleReadHeaderInfo[Check Point][2]");

    tcp_socket_.async_read_some(
      boost::asio::buffer(buffer_data_ptr, message_body_length),
      strand_.wrap(boost::bind(
                     &BaseTCPTransportImpl::HandleRead,
                     shared_from_this(),
                     boost::asio::placeholders::error,
                     boost::asio::placeholders::bytes_transferred,
                     buffer_ptr
                   ))
    );

    YSOS_LOG_DEBUG("BaseTCPTransportImpl::HandleReadHeaderInfo[Check Point][End]");

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  if (YSOS_ERROR_SUCCESS != result) {
    YSOS_LOG_DEBUG("BaseTCPTransportImpl::HandleReadHeaderInfo[Error]");
    HandleError(PROP_ON_READ, error_code);
  }

  YSOS_LOG_DEBUG("BaseTCPTransportImpl::HandleReadHeaderInfo[Exit]");

  return;
}

void BaseTCPTransportImpl::HandleRead(const boost::system::error_code& error_code, const size_t bytes_transferred, const BufferInterfacePtr buffer_ptr) {
  YSOS_LOG_DEBUG("BaseTCPTransportImpl::HandleRead[Enter]");

  int result = YSOS_ERROR_FAILED;

  do {
    /// 判断模块是否已关闭
    if (IS_STOPPED == GetRunningStatus()) {
      YSOS_LOG_DEBUG("BaseTCPTransportImpl::HandleRead[It is already stopped]");
      result = YSOS_ERROR_SUCCESS;
      break;
    }

    YSOS_LOG_DEBUG("BaseTCPTransportImpl::HandleRead[Check Point][0]");

    if (0 != error_code) {
      YSOS_LOG_DEBUG("BaseTCPTransportImpl::HandleRead[error_code] [" << error_code.value() << "]" << " [error_message] [" << error_code.message() << "]");
      break;
    }

    YSOS_LOG_DEBUG("BaseTCPTransportImpl::HandleRead[Check Point][bytes_transferred][" << bytes_transferred << "]");

    /// for debug
    //////////////////////////////////////////////////////////////////////////
    uint8_t* buffer_data_ptr = GetBufferUtility()->GetBufferData(buffer_ptr);
    if (NULL == buffer_data_ptr) {
      YSOS_LOG_DEBUG("BaseTCPTransportImpl::HandleRead[Fail][buffer_data_ptr is NULL]");
      break;
    }
    YSOS_LOG_DEBUG("BaseTCPTransportImpl::HandleRead[buffer_data_ptr] [" << (char*)buffer_data_ptr << "]" << "buffer_data_ptr length [" << strlen((char*)buffer_data_ptr) << "]");
    //////////////////////////////////////////////////////////////////////////

    if (NULL != transport_callback_ptr_) {
      YSOS_LOG_DEBUG("BaseTCPTransportImpl::HandleRead[Check Point][transport_callback_ptr_ is not NULL]");
      transport_callback_ptr_->HandleRead(error_code, bytes_transferred, buffer_ptr, shared_from_this());
    }

    YSOS_LOG_DEBUG("BaseTCPTransportImpl::HandleRead[Check Point][End]");

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  if (YSOS_ERROR_SUCCESS != result) {
    YSOS_LOG_DEBUG("BaseTCPTransportImpl::HandleRead[Error]");
    HandleError(PROP_ON_READ, error_code);
  }

  YSOS_LOG_DEBUG("BaseTCPTransportImpl::HandleRead[Exit]");

  return;
}

void BaseTCPTransportImpl::HandleWrite(const boost::system::error_code& error_code, const size_t bytes_transferred, const BufferInterfacePtr buffer_ptr) {
  YSOS_LOG_DEBUG("BaseTCPTransportImpl::HandleWrite[Enter]");

//   YSOS_LOG_DEBUG("BaseTCPTransportImpl::HandleWrite[Start][buffer_ptr][" << reinterpret_cast<UINT64>(buffer_ptr.get()) << "]");
//   YSOS_LOG_DEBUG("BaseTCPTransportImpl::HandleWrite[Start][buffer_ptr use count][" << buffer_ptr.use_count() << "]");

  int result = YSOS_ERROR_FAILED;

  do {
    /// 判断模块是否已关闭
    if (IS_STOPPED == GetRunningStatus()) {
      YSOS_LOG_DEBUG("BaseTCPTransportImpl::HandleWrite[It is already stopped]");
      result = YSOS_ERROR_SUCCESS;
      break;
    }

    YSOS_LOG_DEBUG("BaseTCPTransportImpl::HandleWrite[Check Point][0]");

    if (0 != error_code) {
      YSOS_LOG_DEBUG("BaseTCPTransportImpl::HandleWrite[error_code] [" << error_code.value() << "]" << " [error_message] [" << error_code.message() << "]");
      break;
    }

    /// for debug
    //////////////////////////////////////////////////////////////////////////
    uint8_t* buffer_data_ptr = GetBufferUtility()->GetBufferData(buffer_ptr);
    if (NULL == buffer_data_ptr) {
      YSOS_LOG_DEBUG("BaseTCPTransportImpl::HandleWrite[Fail][buffer_data_ptr is NULL]");
      break;
    }
    YSOS_LOG_DEBUG("BaseTCPTransportImpl::HandleWrite[buffer_data_ptr] [" << ((char*)buffer_data_ptr + header_info_size_) << "]");
    //////////////////////////////////////////////////////////////////////////

    if (NULL != transport_callback_ptr_) {
      int buffer_prefix_length = GetBufferUtility()->GetBufferPrefixLength(buffer_ptr);
      int buffer_length = GetBufferUtility()->GetBufferLength(buffer_ptr);

      YSOS_LOG_DEBUG("BaseTCPTransportImpl::Write[Check Point][buffer_prefix_length][" << buffer_prefix_length << "]");
      YSOS_LOG_DEBUG("BaseTCPTransportImpl::Write[Check Point][buffer_length][" << buffer_length << "]");

      if (YSOS_ERROR_SUCCESS != buffer_ptr->SetLength((buffer_length - header_info_size_), (buffer_prefix_length + header_info_size_))) {
        YSOS_LOG_DEBUG("BaseTCPTransportImpl::Write[Fail]The function[buffer_ptr->SetLength()] is failed");
        break;
      }

      YSOS_LOG_DEBUG("BaseTCPTransportImpl::HandleWrite[Check Point][1]");

      YSOS_LOG_DEBUG("BaseTCPTransportImpl::HandleWrite[Check Point][transport_callback_ptr_ is not NULL]");
      transport_callback_ptr_->HandleWrite(error_code, bytes_transferred, buffer_ptr, shared_from_this());
    }

    YSOS_LOG_DEBUG("BaseTCPTransportImpl::HandleWrite[Check Point][End]");

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  if (YSOS_ERROR_SUCCESS != result) {
    YSOS_LOG_DEBUG("BaseTCPTransportImpl::HandleWrite[error]");
    HandleError(PROP_ON_WRITE, error_code);
  }

//   YSOS_LOG_DEBUG("BaseTCPTransportImpl::HandleWrite[End][buffer_ptr][" << reinterpret_cast<UINT64>(buffer_ptr.get()) << "]");
//   YSOS_LOG_DEBUG("BaseTCPTransportImpl::HandleWrite[End][buffer_ptr use count][" << buffer_ptr.use_count() << "]");

  YSOS_LOG_DEBUG("BaseTCPTransportImpl::HandleWrite[Exit]");

  return;
}

void BaseTCPTransportImpl::HandleConnect(const SystemErrorCode& error_code) {
  YSOS_LOG_DEBUG("BaseTCPTransportImpl::HandleConnect()");
  return;
}

void BaseTCPTransportImpl::HandleAccept(const SystemErrorCode& error_code, TransportInterfacePtr transport_ptr) {
  YSOS_LOG_DEBUG("BaseTCPTransportImpl::HandleAccept[Enter]");

  int result = YSOS_ERROR_FAILED;

  do {
    /// 判断模块是否已关闭
    if (IS_STOPPED == GetRunningStatus()) {
      YSOS_LOG_DEBUG("BaseTCPTransportImpl::HandleAccept[It is already stopped]");
      result = YSOS_ERROR_SUCCESS;
      break;
    }

    YSOS_LOG_DEBUG("BaseTCPTransportImpl::HandleAccept[Check Point][0]");

    if (0 != error_code) {
      YSOS_LOG_DEBUG("BaseTCPTransportImpl::HandleAccept[error_code] [" << error_code.value() << "]" << " [error_message] [" << error_code.message() << "]");
      break;
    }

    YSOS_LOG_DEBUG("BaseTCPTransportImpl::HandleAccept[Check Point][1]");

    if (NULL != transport_callback_ptr_) {
      YSOS_LOG_DEBUG("BaseTCPTransportImpl::HandleAccept[Check Point][transport_callback_ptr_ is not NULL]");
      transport_callback_ptr_->HandleAccept(error_code, transport_ptr);
    }

    YSOS_LOG_DEBUG("BaseTCPTransportImpl::HandleAccept[Check Point][2]");

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  if (YSOS_ERROR_SUCCESS != result) {
    YSOS_LOG_DEBUG("BaseTCPTransportImpl::HandleAccept[Error]");
    HandleError(PROP_ON_ACCEPT, error_code);
  }

  Accept();

  YSOS_LOG_DEBUG("BaseTCPTransportImpl::HandleAccept[Exit]");

  return;
}

void BaseTCPTransportImpl::HandleDisconnect(const SystemErrorCode& error_code) {
  YSOS_LOG_DEBUG("BaseTCPTransportImpl::HandleDisconnect[Enter]");

  /// 判断模块是否已关闭
  if (IS_STOPPED == GetRunningStatus()) {
    YSOS_LOG_DEBUG("BaseTCPTransportImpl::HandleDisconnect[It is already stopped]");
    return;
  }

  if (NULL != transport_callback_ptr_) {
    YSOS_LOG_DEBUG("BaseTCPTransportImpl::HandleDisconnect[Check Point][transport_callback_ptr_ is not NULL]");
    transport_callback_ptr_->HandleDisconnect(error_code, shared_from_this());
  }

  YSOS_LOG_DEBUG("BaseTCPTransportImpl::HandleDisconnect[Exit]");

  return;
}

void BaseTCPTransportImpl::HandleError(const UINT32 callback_type, const SystemErrorCode& error_code) {
  YSOS_LOG_DEBUG("BaseTCPTransportImpl::HandleError[Enter]");

  /// 远程主机强迫关闭了一个现有的连接
  if (10054 == error_code.value()) {
    YSOS_LOG_DEBUG("BaseTCPTransportImpl::HandleError[error_code][disconnect] [" << error_code.value() << "]" << " [error_message] [" << error_code.message() << "]");
    HandleDisconnect(error_code);
    YSOS_LOG_DEBUG("BaseTCPTransportImpl::HandleError[Check Point][0]");
  } else {
    YSOS_LOG_DEBUG("BaseTCPTransportImpl::HandleError[error_code][other error] [" << error_code.value() << "]" << " [error_message] [" << error_code.message() << "]");
    if (NULL != transport_callback_ptr_) {
      YSOS_LOG_DEBUG("BaseTCPTransportImpl::HandleError[Check Point][transport_callback_ptr_ is not NULL]");
      transport_callback_ptr_->HandleError(error_code, callback_type, shared_from_this());
    }
    YSOS_LOG_DEBUG("BaseTCPTransportImpl::HandleError[Check Point][1]");
  }

  YSOS_LOG_DEBUG("BaseTCPTransportImpl::HandleError[Exit]");

  return;
}

int BaseTCPTransportImpl::InitOpenParam(void *params) {
  YSOS_LOG_DEBUG("BaseTCPTransportImpl::InitOpenParam[Enter]");

  /// 重置运行状态标识符
  SetRunningStatus(IS_INITIAL);

  if (NULL == params) {
    YSOS_LOG_DEBUG("BaseTCPTransportImpl::InitOpenParam[Fail][params is NULL]");
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  YSOS_LOG_DEBUG("BaseTCPTransportImpl::InitOpenParam[Check Point][0]");

  TransportCallbackInterfacePtr *transport_callback_ptr_ptr = static_cast<TransportCallbackInterfacePtr*>(params);
  transport_callback_ptr_ = *transport_callback_ptr_ptr;

  YSOS_LOG_DEBUG("BaseTCPTransportImpl::InitOpenParam[Check Point][1]");

  if (NULL == transport_callback_ptr_) {
    YSOS_LOG_DEBUG("BaseTCPTransportImpl::InitOpenParam[Fail][transport_callback_ptr_ is NULL]");
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  YSOS_LOG_DEBUG("BaseTCPTransportImpl::InitOpenParam[Check Point][2]");

  address_ = transport_callback_ptr_->GetAddress();
  YSOS_LOG_DEBUG("BaseTCPTransportImpl::InitOpenParam[Check Point][3]");

  port_ = transport_callback_ptr_->GetPort();
  YSOS_LOG_DEBUG("BaseTCPTransportImpl::InitOpenParam[Check Point][4]");

  if (true == address_.empty() || 0 == port_) {
    YSOS_LOG_DEBUG("BaseTCPTransportImpl::InitOpenParam[Fail][The parameters is invalid]");
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  YSOS_LOG_DEBUG("BaseTCPTransportImpl::InitOpenParam[Exit]");

  return YSOS_ERROR_SUCCESS;
}

int BaseTCPTransportImpl::InitSignal(void) {
  YSOS_LOG_DEBUG("BaseTCPTransportImpl::InitSignal[Enter]");

  /*Register to handle the signals that indicate when the server should exit.
  It is safe to register for the same signal multiple times in a program,
  provided all registration for the specified signal is made through Asio.*/
  signal_set_.add(SIGINT);
  signal_set_.add(SIGTERM);
#if defined(SIGQUIT)
  signal_set_.add(SIGQUIT);
#endif ///< defined(SIGQUIT)
  signal_set_.async_wait(boost::bind(&BaseTCPTransportImpl::Close, this));

  YSOS_LOG_DEBUG("BaseTCPTransportImpl::InitSignal[Exit]");

  return YSOS_ERROR_SUCCESS;
}

TCPSocket& BaseTCPTransportImpl::GetTCPSocket() {
  return tcp_socket_;
}

void BaseTCPTransportImpl::RunIOServicePool() {
  YSOS_LOG_DEBUG("BaseTCPTransportImpl::RunIOServicePool[Enter]");

  run_io_service_pool_thread_ = boost::thread(boost::bind(&IOServicePool::Run, &io_service_pool_));

  YSOS_LOG_DEBUG("BaseTCPTransportImpl::RunIOServicePool[Exit]");

  return;
}

int BaseTCPTransportImpl::Connect(const std::string& address, const UINT32 port) {
  YSOS_LOG_DEBUG("BaseTCPTransportImpl::Connect()");
  return YSOS_ERROR_NOT_SUPPORTED;
}

int BaseTCPTransportImpl::Accept() {
  YSOS_LOG_DEBUG("BaseTCPTransportImpl::Accept()");
  return YSOS_ERROR_NOT_SUPPORTED;
}

int BaseTCPTransportImpl::RealOpen() {
  YSOS_LOG_DEBUG("BaseTCPTransportImpl::RealOpen()");
  return YSOS_ERROR_NOT_SUPPORTED;
}

int BaseTCPTransportImpl::RealClose() {
  YSOS_LOG_DEBUG("BaseTCPTransportImpl::RealClose()");
  return YSOS_ERROR_NOT_SUPPORTED;
}

/**
 *@brief TCPSocketClient的具体实现  // NOLINT
 */
TCPSocketClient::TCPSocketClient(const std::string &strClassName /* =TCPSocketClient */)
  : BaseTCPTransportImpl(PROP_CLIENT, DEFALT_IO_SERVICE_COUNT_FOR_TCP_SOCKET_CLIENT, IS_INITIAL, strClassName),
    resolver_(io_service_pool_.GetIOService()),
    is_connected_(false) {
  YSOS_LOG_DEBUG("TCPSocketClient::TCPSocketClient()");
}

TCPSocketClient::~TCPSocketClient() {
  YSOS_LOG_DEBUG("TCPSocketClient::~TCPSocketClient()");
}

int TCPSocketClient::RealOpen(void) {
  YSOS_LOG_DEBUG("TCPSocketClient::RealOpen[Enter]");

  /// 连接服务器
  if (YSOS_ERROR_SUCCESS != Connect(address_, boost::lexical_cast<std::string>(port_))) {
    YSOS_LOG_DEBUG("BaseTCPTransportImpl::RealOpen[Fail]The function[Connect()] is failed");
    return YSOS_ERROR_FAILED;
  }

  YSOS_LOG_DEBUG("TCPSocketClient::RealOpen[Check Point][0]");

  /// 启动io service pool
  RunIOServicePool();

  YSOS_LOG_DEBUG("TCPSocketClient::RealOpen[Check Point][1]");

  /// 线程同步等待连接成功失败信号量，同步等待30秒
  UniqueLock unique_lock(mutex_lock_);
  bool time_out = false;
  if (condition_variable_any_connect_.wait_for(unique_lock, boost::chrono::seconds(30)) == boost::cv_status::timeout) {
    YSOS_LOG_DEBUG("TCPSocketClient::RealOpen[Fail][time out]");
    return YSOS_ERROR_FAILED;
  }

  YSOS_LOG_DEBUG("TCPSocketClient::RealOpen[Check Point][2]");

  /// 判断连接状态
  if (false == GetConnectStatus()) {
    YSOS_LOG_DEBUG("TCPSocketClient::RealOpen[Fail][connect fail]");
    return YSOS_ERROR_FAILED;
  }

  YSOS_LOG_DEBUG("TCPSocketClient::RealOpen[Exit]");

  return YSOS_ERROR_SUCCESS;
}

int TCPSocketClient::RealClose(void) {
  YSOS_LOG_DEBUG("TCPSocketClient::RealClose[Enter]");

  boost::system::error_code ignored_error_code;
  tcp_socket_.close(ignored_error_code);
  tcp_socket_.shutdown(TCPSocket::shutdown_both, ignored_error_code);
  
  /// 重置连接状态
  SetConnectStatus(false);

  YSOS_LOG_DEBUG("TCPSocketClient::RealClose[Exit]");

  return YSOS_ERROR_SUCCESS;
}

int TCPSocketClient::Connect(const std::string& address, const std::string& port) {
  YSOS_LOG_DEBUG("TCPSocketClient::Connect[Enter]");

  if (true == GetConnectStatus()) {
    YSOS_LOG_DEBUG("TCPSocketClient::Connect[It is already connected]");
    return YSOS_ERROR_SUCCESS;
  }

  YSOS_LOG_DEBUG("TCPSocketClient::Connect[Check Point][0]");

  if (true == address.empty() ||
      true == port.empty()) {
    YSOS_LOG_DEBUG("TCPSocketClient::Connect[Fail][address is empty or port is empty.]");
    return YSOS_ERROR_FAILED;
  }

  YSOS_LOG_DEBUG("TCPSocketClient::Connect[address][" << address << "]");
  YSOS_LOG_DEBUG("TCPSocketClient::Connect[port][" << port << "]");

  TCPQuery query(address, port);

  YSOS_LOG_DEBUG("TCPSocketClient::Connect[Check Point][1]");

  resolver_.async_resolve(query,
                          boost::bind(&TCPSocketClient::HandleResolve,
                                      this,
                                      boost::asio::placeholders::error,
                                      boost::asio::placeholders::iterator));

  YSOS_LOG_DEBUG("TCPSocketClient::Connect[Exit]");

  return YSOS_ERROR_SUCCESS;
}

void TCPSocketClient::HandleResolve(const SystemErrorCode& error_code, TCPResolver::iterator endpoint_iterator) {
  YSOS_LOG_DEBUG("TCPSocketClient::HandleResolve[Enter]");

  /// 判断模块是否已关闭
  if (IS_STOPPED == GetRunningStatus()) {
    YSOS_LOG_DEBUG("TCPSocketClient::HandleResolve[It is already stopped]");

    /// 通知所有等待中的线程
    condition_variable_any_connect_.notify_all();
    return;
  }

  YSOS_LOG_DEBUG("TCPSocketClient::HandleResolve[Check Point][0]");

  if (0 != error_code) {
    YSOS_LOG_DEBUG("TCPSocketClient::HandleResolve[error_code] [" << error_code.value() << "]" << " [error_message] [" << error_code.message() << "]");
    return;
  }

  YSOS_LOG_DEBUG("TCPSocketClient::HandleResolve[Check Point][1]");

  /// Attempt a connection to each endpoint in the list until we
  /// successfully establish a connection.
  boost::asio::async_connect(tcp_socket_,
                             endpoint_iterator,
                             boost::bind(&TCPSocketClient::HandleConnect,
                                         this,
                                         boost::asio::placeholders::error));

//   tcp_socket_.async_connect(endpoint_iterator->endpoint(),
//                             boost::bind(&TCPSocketClient::HandleConnect,
//                                         this,
//                                         boost::asio::placeholders::error));

  YSOS_LOG_DEBUG("TCPSocketClient::HandleResolve[Exit]");

  return;
}

void TCPSocketClient::HandleConnect(const SystemErrorCode& error_code) {
  YSOS_LOG_DEBUG("TCPSocketClient::HandleConnect[Enter]");

  int result = YSOS_ERROR_FAILED;

  do {
    /// 判断模块是否已关闭
    if (IS_STOPPED == GetRunningStatus()) {
      YSOS_LOG_DEBUG("TCPSocketClient::HandleConnect[It is already stopped]");
      result = YSOS_ERROR_SUCCESS;
      break;
    }

    YSOS_LOG_DEBUG("TCPSocketClient::HandleConnect[Check Point][0]");

    if (0 != error_code) {
      YSOS_LOG_DEBUG("TCPSocketClient::HandleConnect[error_code] [" << error_code.value() << "]" << " [error_message] [" << error_code.message() << "]");
      break;
    }

    YSOS_LOG_DEBUG("TCPSocketClient::HandleConnect[Check Point][1]");

    /// 设置连接成功
    SetConnectStatus(true);

    YSOS_LOG_DEBUG("TCPSocketClient::HandleConnect[Check Point][End]");

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  YSOS_LOG_DEBUG("TCPSocketClient::HandleConnect[Check Point][Before Notify...]");

  /// 通知所有等待中的线程
  condition_variable_any_connect_.notify_all();

  YSOS_LOG_DEBUG("TCPSocketClient::HandleConnect[Check Point][After Notify...]");

  YSOS_LOG_DEBUG("TCPSocketClient::HandleConnect[Exit]");

  return;
}

/**
 *@brief TCPSocketServer的具体实现  // NOLINT
 */
TCPSocketServer::TCPSocketServer(const std::string &strClassName /* =TCPSocketServer */)
  : BaseTCPTransportImpl(PROP_SERVER, DEFALT_IO_SERVICE_COUNT_FOR_TCP_SOCKET_SERVER, IS_INITIAL, strClassName),
    tcp_acceptor_(io_service_pool_.GetIOService()) {
  YSOS_LOG_DEBUG("TCPSocketServer::TCPSocketServer()");
}

TCPSocketServer::~TCPSocketServer() {
  YSOS_LOG_DEBUG("TCPSocketServer::~TCPSocketServer()");
}

int TCPSocketServer::RealOpen(void) {
  YSOS_LOG_DEBUG("TCPSocketServer::RealOpen[Enter]");

  InitSignal();

  YSOS_LOG_DEBUG("TCPSocketServer::RealOpen[Check Point][0]");

  /// Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
  TCPResolver resolver(tcp_acceptor_.get_io_service());

  YSOS_LOG_DEBUG("TCPSocketServer::RealOpen[Check Point][1]");

  TCPQuery query(address_, boost::lexical_cast<std::string>(port_));

  YSOS_LOG_DEBUG("TCPSocketServer::RealOpen[Check Point][2]");

  boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);

  YSOS_LOG_DEBUG("TCPSocketServer::RealOpen[Check Point][3]");

  tcp_acceptor_.open(endpoint.protocol());

  YSOS_LOG_DEBUG("TCPSocketServer::RealOpen[Check Point][4]");

  tcp_acceptor_.set_option(TCPAcceptor::reuse_address(true));

  YSOS_LOG_DEBUG("TCPSocketServer::RealOpen[Check Point][5]");

  //boost::asio::socket_base::non_blocking_io command(true);
  NonBlockingIO command(true);  //need update for linux
  tcp_acceptor_.io_control(command);

  YSOS_LOG_DEBUG("TCPSocketServer::RealOpen[Check Point][6]");

  tcp_acceptor_.bind(endpoint);

  YSOS_LOG_DEBUG("TCPSocketServer::RealOpen[Check Point][7]");

  tcp_acceptor_.listen();

  YSOS_LOG_DEBUG("TCPSocketServer::RealOpen[Check Point][8]");

  if (YSOS_ERROR_SUCCESS != Accept()) {
    YSOS_LOG_DEBUG("TCPSocketServer::RealOpen[Fail]The function[Accept()] is failed");
    return YSOS_ERROR_FAILED;
  }

  YSOS_LOG_DEBUG("TCPSocketServer::RealOpen[Check Point][9]");

  /// 运行io service pool
  RunIOServicePool();

  YSOS_LOG_DEBUG("TCPSocketServer::RealOpen[Exit]");

  return YSOS_ERROR_SUCCESS;
}

int TCPSocketServer::RealClose(void) {
  YSOS_LOG_DEBUG("TCPSocketServer::RealClose[Enter]");

  tcp_acceptor_.close();

  YSOS_LOG_DEBUG("TCPSocketServer::RealClose[Exit]");

  return YSOS_ERROR_SUCCESS;
}

int TCPSocketServer::Accept() {
  YSOS_LOG_DEBUG("TCPSocketServer::Accept[Enter]");

  TCPTransportPtr tcp_transport_ptr = TCPTransportPtr(new BaseTCPTransportImpl(PROP_SERVER_TERMINAL, DEFALT_IO_SERVICE_COUNT_FOR_TCP_SOCKET_SERVER_TERMINAL, IS_RUNNING));
  if (NULL == tcp_transport_ptr) {
    YSOS_LOG_DEBUG("TCPSocketServer::Accept[Fail][tcp_transport_ptr is NULL]");
    return YSOS_ERROR_OUTOFMEMORY;
  }

  YSOS_LOG_DEBUG("TCPSocketServer::Accept[Check Point][0]");

  tcp_acceptor_.async_accept(tcp_transport_ptr->GetTCPSocket(),
                             boost::bind(&TCPSocketServer::HandleAccept,
                                         this,
                                         boost::asio::placeholders::error,
                                         tcp_transport_ptr));

  YSOS_LOG_DEBUG("###### TCPSocketServer Accept is ready ######");

  YSOS_LOG_DEBUG("TCPSocketServer::Accept[Exit]");

  return YSOS_ERROR_SUCCESS;
}

}  ///  namespace ysos
