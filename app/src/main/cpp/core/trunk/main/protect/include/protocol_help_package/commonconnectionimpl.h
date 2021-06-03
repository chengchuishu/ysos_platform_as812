/*
# commonconnectionimpl.h
# Definition of commonconnectionimpl
# Created on: 2016-05-26 09:59:58
# Original author: jinchengzhe
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
# version 0.1, 20160912, created by JinChengZhe
*/
#ifndef OHP_COMMON_CONNECTION_IMPL_H
#define OHP_COMMON_CONNECTION_IMPL_H

/// C++ Standard Headers
#include <list>
/// ThirdParty Headers
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/noncopyable.hpp>
#include <boost/assert.hpp>
#include <boost/concept_check.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/pool/object_pool.hpp>
/// Platform Headers
#include "../../../public/include/sys_interface_package/common.h"
#include "../../../public/include/core_help_package/baseinterfaceimpl.h"
#include "../../../public/include/sys_interface_package/connectioninterface.h"
#include "../../../public/include/sys_interface_package/connectioncallbackinterface.h"
#include "../../../public/include/sys_interface_package/transportinterface.h"
#include "../../../public/include/sys_interface_package/transportcallbackinterface.h"
#include "../../../public/include/sys_interface_package/bufferpool.h"
#include "../../../public/include/sys_interface_package/protocolinterface.h"

namespace ysos {

/**
*@brief JobQueue的具体实现  // NOLINT
*/
template<typename Job>
class JobQueue : boost::noncopyable {
 public:
  typedef Job JobType;
  typedef std::deque<JobType> QueueType;

  typedef boost::mutex MutexType;
  typedef boost::unique_lock<MutexType> LockType;
  typedef boost::condition_variable_any ConditionType;

  BOOST_CONCEPT_ASSERT((boost::SGIAssignable<JobType>));
  BOOST_CONCEPT_ASSERT((boost::DefaultConstructible<JobType>));

  JobQueue(void) : stop_flag_(false) {}
  ~JobQueue(void) {}

  void Push(const JobType& job) {
    LockType lock(mutex_);
    job_queue_.push_back(job);
    condition_.notify_one();
    return;
  }

  JobType Pop(void) {
    LockType lock(mutex_);
    while (true == job_queue_.empty() && false == stop_flag_) {
      condition_.wait(mutex_);
    }

    if (true == stop_flag_) {
      return JobType();
    }

    BOOST_ASSERT(true != job_queue_.empty());

    JobType temp = job_queue_.front();
    job_queue_.pop_front();

    return temp;
  }

  void Stop(void) {
    stop_flag_ = true;
    condition_.notify_all();
    return;
  }

  int GetSize(void) {
    LockType lock(mutex_);
    return job_queue_.size();
  }

  void Clear(void) {
    LockType lock(mutex_);
    job_queue_.clear();
  }

 private:
  QueueType job_queue_;
  MutexType mutex_;
  ConditionType condition_;
  bool stop_flag_;
};

/**
*@brief Worker的具体实现  // NOLINT
*/
template<typename Queue>
class Worker {
 public:
  typedef Queue QueueType;
  typedef typename Queue::JobType JobType;
  typedef boost::function<bool(JobType&)> FunctionType;

  template<typename Func>
  Worker(QueueType& queue, Func func, int thread_count = 1)
    : queue_(queue),
      function_(func),
      thread_count_(thread_count) {
    BOOST_ASSERT(0 < thread_count_);
  }

  Worker(QueueType& queue, int thread_count = 1)
    : queue_(queue),
      thread_count_(thread_count) {
    BOOST_ASSERT(0 < thread_count_);
  }

  void Start() {
    BOOST_ASSERT(function_);
    if (0 < thread_goup_.size()) {
      return;
    }

    for (int ii = 0; ii < thread_count_; ++ii) {
      thread_goup_.create_thread(boost::bind(&Worker::DoWork, this));
    }

    return;
  }

  template<typename Func>
  void Start(Func func) {
    function_ = func;  //need update for linux
    Start();
    return;
  }

  void Run() {
    Start();
    thread_goup_.join_all();
    return;
  }

  void Stop() {
    function_ = 0;
    queue_.Stop();
    return;
  }

 private:
  void DoWork() {
    for (;;) {
      JobType job = queue_.Pop();
      if (!function_ || !function_(job)) {
        break;
      }
    }
    return;
  }

  QueueType& queue_;
  FunctionType function_;
  int thread_count_;
  boost::thread_group thread_goup_;
};


/**
*@brief CommonConnectionImpl的具体实现  // NOLINT
*/
class LightLock;
class CommonConnectionImpl;
typedef boost::shared_ptr<CommonConnectionImpl> CommonConnectionImplPtr;
class YSOS_EXPORT CommonConnectionImpl : public ConnectionInterface, public BaseInterfaceImpl, public boost::enable_shared_from_this<CommonConnectionImpl> {
  DECLARE_CREATEINSTANCE(CommonConnectionImpl);
  DISALLOW_COPY_AND_ASSIGN(CommonConnectionImpl);
  DECLARE_PROTECT_CONSTRUCTOR(CommonConnectionImpl);

 protected:
  /**
  *@brief 定制构造函数，服务器模式下，生成每个子连接时使用  // NOLINT
  *@param root_connection_ptr[Input]： 顶层基类connection指针  // NOLINT
  *@param transport_ptr[Input]： transport指针  // NOLINT
  *@return： 无  // NOLINT
  */
  CommonConnectionImpl(const ConnectionInterfacePtr& root_connection_ptr, const TransportInterfacePtr& transport_ptr);

 public:
  /**
  *@brief 读操作相关队列和工作线程相关重定义  // NOLINT
  */
  typedef JobQueue<BufferInterfacePtr> ReadJobQueue;  ///< 重新定义读取消息队列  // NOLINT
  typedef boost::shared_ptr<Worker<ReadJobQueue>> ReadWorkerPtr;  ///< 重新定义工作线程指针  // NOLINT
  /**
  *@brief 写操作操作相关队列和工作线程相关重定义  // NOLINT
  */
  typedef std::tuple<BufferInterfacePtr, int, std::string> WriteJobParams;  ///< 重新定义写操作所需参数  // NOLINT
  typedef WriteJobParams* WriteJobParamsPtr;  ///< 重新定义写操作所需参数指针  // NOLINT
  typedef JobQueue<WriteJobParamsPtr> WriteJobQueue;  ///< 重定义写操作消息队列  // NOLINT
  typedef boost::shared_ptr<Worker<WriteJobQueue>> WriteWorkerPtr;  ///< 重新定义写操作工作线程指针  // NOLINT
  /**
  *@brief 无效连接相关队列和工作线程相关重定义  // NOLINT
  */
  typedef JobQueue<ConnectionInterfacePtr> InvalidConnectionJobQueue;  ///< 重新定义无效连接队列  // NOLINT
  typedef boost::shared_ptr<Worker<InvalidConnectionJobQueue>> InvalidConnectionWorkerPtr;  ///< 重新定义无效连接处理线程指针  // NOLINT
  /**
  *@brief 心跳相关队列和工作线程相关重定义  // NOLINT
  */
  typedef JobQueue<std::string> HeartbeatJobQueue;  ///< 重新定义心跳消息队列  // NOLINT
  typedef boost::shared_ptr<Worker<HeartbeatJobQueue>> HeartbeatWorkerPtr;  ///< 重新定义工作线程指针  // NOLINT
  /**
  *@brief session id和connection指针匹配表相关数据重定义  // NOLINT
  */
  typedef std::pair<std::string, ConnectionInterfacePtr> SessionIDMatchingTableUnit;  ///< 重定义session id和connection指针匹配表的保存单元  // NOLINT
  typedef std::map<std::string, ConnectionInterfacePtr> SessionIDMatchingTable;  ///< 重定义session id和connection指针匹配表  // NOLINT
  typedef std::map<std::string, ConnectionInterfacePtr>::iterator SessionIDMatchingTableIterator;  ///< 重定义session id和connection指针匹配表的迭代器  // NOLINT
  /**
  *@brief boost期限timer相关类重定义  // NOLINT
  */
  typedef boost::asio::deadline_timer DeadlineTimer;  ///< 重新定义boost期限timer  // NOLINT
  /**
  *@brief 重新定义读写共享锁  // NOLINT
  */
  typedef boost::shared_mutex BoostSharedMutex;
  typedef boost::shared_lock<BoostSharedMutex> BoostSharedLock;
  typedef boost::lock_guard<BoostSharedMutex> BoostLockGuard;
  /**
  *@brief 协议类相关重定义  // NOLINT
  */
  typedef std::map<UINT32, ProtocolInterfacePtr> ProtocolMap;  ///< 重新定义保存协议类智能指针Map  // NOLINT
  typedef std::map<UINT32, ProtocolInterfacePtr>::iterator ProtocolMapIterator;  ///< 重新定义保存协议类智能指针Map的Iterator  // NOLINT

  /**
  *@brief 心跳操作事件  // NOLINT
  */
  enum {
    IS_INITIAL = 0,  ///< 初始状态  // NOLINT
    IS_RUNNING = 1,  ///< 运行状态  // NOLINT
    IS_STOPPED = 2,  ///< 停止状态  // NOLINT
    LEGITIMATE_INCONNECTION_TIMES = 3,  ///< 允许最大失联测试，20秒读一次心跳，所以允许失联最大时间为60秒  // NOLINT
    HEARTBEAT_SERIAL_NUMBER_LENGTH = 32,  ///< 心跳操作流水号  // NOLINT
    READ_HEARTBEAT_DEADLINE = 20,  ///< 读取心跳操作执行时间  // NOLINT
    WRITE_HEARTBEAT_DEADLINE = 20,  ///< 写入心跳操作执行时间  // NOLINT
    WRITE_BUFFER_PREFIX = 1024,  ///< 缓存prefix大小  // NOLINT
  };

  /**
  *@brief 析构函数  // NOLINT
  *@param ： 无  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual ~CommonConnectionImpl();
  /**
  *@brief 打开并初始化类  // NOLINT
  *@param params[Input]： 初始化参数，函数内部以TeamParamsImpl格式强制转换使用  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int Open(void *params);
  /**
   *@brief 关闭类  // NOLINT
   *@param params[Input]： 当为Server时，param若不为NULL，则是自己建立的一个连接端点，要关闭了  // NOLINT
   *@return： 无  // NOLINT
   */
  virtual void Close(void *param = NULL);
  /**
   *@brief 读取数据函数  // NOLINT
   *@param input_buffer_ptr[Input]： 读取数据的缓冲，内部包含实际读取数据长度  // NOLINT
   *@param input_length[Input]： 缓冲长度  // NOLINT
   *@param output_buffer_ptr[Out]： 读取数据的缓冲，内部包含实际读取数据长度  // NOLINT
   *@param out_length[Out]： 缓冲长度  // NOLINT
   *@param context_ptr[Input]： 传输接口的上下文，与具体实现密切相关  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Read(BufferInterfacePtr input_buffer_ptr, int input_length, void* context_ptr = NULL);
  /**
  *@brief 写入数据函数  // NOLINT
  *@param input_buffer_ptr[Input]： 读取数据的缓冲，内部包含实际读取数据长度  // NOLINT
  *@param input_length[Input]： 缓冲长度  // NOLINT
  *@param output_buffer_ptr[Out]： 读取数据的缓冲，内部包含实际读取数据长度  // NOLINT
  *@param out_length[Out]： 缓冲长度  // NOLINT
  *@param context_ptr[Input]： 传输接口的上下文，与具体实现密切相关  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int Write(BufferInterfacePtr input_buffer_ptr, int input_length, void* context_ptr = NULL);
  /**
    *@brief 是否允许写入  // NOLINT
    *@param is_enable[Input]： 允许写入开关量  // NOLINT
    *@return： 无  // NOLINT
  */
  virtual void EnableWrite(bool is_enable = true);
  /**
    *@brief 是否允许读出  // NOLINT
    *@param is_enable[Input]： 允许读出开关量  // NOLINT
    *@return： 无  // NOLINT
  */
  virtual void EnableRead(bool is_enable = true);
  /**
  *@brief 是否封装数据  // NOLINT
  *@param is_enable[Input]： 是否封装开关量  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void EnableWrap(bool is_enable = true);
  /**
  *@brief 获取接口的属性值  // NOLINT
  *@param type_id[Input]： 属性的ID  // NOLINT
  *@param type[Output]： 属性值的值  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int GetProperty(int type_id, void *type);
  /**
   *@brief 设置接口的属性值  // NOLINT
   *@param type_id[Input]： 属性的ID  // NOLINT
   *@param type[input]： 属性值的值  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int SetProperty(int type_id, void *type);
  /**
  *@brief 封装数据  // NOLINT
  *@param input_buffer_ptr[Input]： 读取数据的缓冲，内部包含实际读取数据长度  // NOLINT
  *@param input_length[Input]： 缓冲长度  // NOLINT
  *@param output_buffer_ptr[Out]： 读取数据的缓冲，内部包含实际读取数据长度  // NOLINT
  *@param out_length[Out]： 缓冲长度  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int Wrap(BufferInterfacePtr input_buffer_ptr, int input_length, BufferInterfacePtr output_buffer_ptr, int *out_length, void* context_ptr = NULL);
  /**
  *@brief 解封数据  // NOLINT
  *@param input_buffer_ptr[Input]： 读取数据的缓冲，内部包含实际读取数据长度  // NOLINT
  *@param input_length[Input]： 缓冲长度  // NOLINT
  *@param output_buffer_ptr[Out]： 读取数据的缓冲，内部包含实际读取数据长度  // NOLINT
  *@param out_length[Out]： 缓冲长度  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int Unwrap(BufferInterfacePtr input_buffer_ptr, int input_length, BufferInterfacePtr output_buffer_ptr, int *out_length, void* context_ptr = NULL);

 public:
  /**
  *@brief 接收连接回调函数  // NOLINT
  *@param error_code[Input]： 系统错误码  // NOLINT
  *@param transport_ptr[Input]： transport指针  // NOLINT
  *@return： 无  // NOLINT
  */
  void HandleAccept(const boost::system::error_code& error_code, const TransportInterfacePtr& transport_ptr);
  /**
  *@brief 连接回调函数  // NOLINT
  *@param error_code[Input]： 系统错误码  // NOLINT
  *@param transport_ptr[Input]： transport指针  // NOLINT
  *@return： 无  // NOLINT
  */
  void HandleConnect(const boost::system::error_code& error_code, const TransportInterfacePtr& transport_ptr);
  /**
  *@brief 读取回调函数  // NOLINT
  *@param error_code[Input]： 系统错误码  // NOLINT
  *@param bytes_transferred[Input]： 读取字节数  // NOLINT
  *@param buffer_ptr[Input]： 读取buffer指针  // NOLINT
  *@param transport_ptr[Input]： transport指针  // NOLINT
  *@return： 无  // NOLINT
  */
  void HandleRead(const boost::system::error_code& error_code, const std::size_t bytes_transferred, const BufferInterfacePtr& buffer_ptr, const TransportInterfacePtr& transport_ptr);
  /**
  *@brief 写入回调函数  // NOLINT
  *@param error_code[Input]： 系统错误码  // NOLINT
  *@param bytes_transferred[Input]： 写入字节数  // NOLINT
  *@param buffer_ptr[Input]： 写入buffer指针  // NOLINT
  *@param transport_ptr[Input]： transport指针  // NOLINT
  *@return： 无  // NOLINT
  */
  void HandleWrite(const boost::system::error_code& error_code, const std::size_t bytes_transferred, const BufferInterfacePtr& buffer_ptr, const TransportInterfacePtr& transport_ptr);
  /**
  *@brief 断开回调函数  // NOLINT
  *@param error_code[Input]： 系统错误码  // NOLINT
  *@param transport_ptr[Input]： transport指针  // NOLINT
  *@return： 无  // NOLINT
  */
  void HandleDisconnect(const boost::system::error_code& error_code, const TransportInterfacePtr& transport_ptr);
  /**
  *@brief 错误处理回调函数  // NOLINT
  *@param error_code[Input]： 系统错误码  // NOLINT
  *@param transport_ptr[Input]： transport指针  // NOLINT
  *@return： 无  // NOLINT
  */
  void HandleError(const boost::system::error_code& error_code, const UINT32 callback_type, const TransportInterfacePtr& transport_ptr);

 protected:
  /**
  *@brief 关闭子连接  // NOLINT
  *@param child_connection_ptr[Input]： 自连接智能指针  // NOLINT
  *@return： 无  // NOLINT
  */
  void CloseChildConnection(ConnectionInterfacePtr child_connection_ptr);
  /**
  *@brief 读操作消息处理线程，客户端模式下对应整个连接，服务器模式下对应个别子连接  // NOLINT
  *@param buffer_ptr[Input]： 读取到的buffer指针  // NOLINT
  *@return： 成功返回true，失败返回false  // NOLINT
  */
  bool ProcessReadMessage(const BufferInterfacePtr& buffer_ptr);
  /**
  *@brief 写操作消息处理线程，客户端模式下对应整个连接，服务器模式下对应个别子连接  // NOLINT
  *@param WriteJobParamsPtr[Input]： 通过write接口传入值的封装对象指针  // NOLINT
  *@return： 成功返回true，失败返回false  // NOLINT
  */
  bool ProcessWriteMessage(const WriteJobParamsPtr& write_job_params_ptr);
  /**
  *@brief 无效连接处理线程，客户端模式下无效，服务器模式下只对应顶层基类  // NOLINT
  *@param invalid_connection_ptr[Input]： 无效连接指针  // NOLINT
  *@return： 成功返回true，失败返回false  // NOLINT
  */
  bool ProcessInvalidConnection(ConnectionInterfacePtr& invalid_connection_ptr);
  /**
  *@brief 通过connection指针获取session id函数  // NOLINT
  *@param connection_ptr[Input]： connection指针  // NOLINT
  *@return： 成功返回session id，失败返回空字符串  // NOLINT
  */
  virtual std::string GetSessionIDWithConnectionPtr(const ConnectionInterfacePtr& connection_ptr);
  /**
  *@brief 通过session id获取connection指针函数  // NOLINT
  *@param session_id[Input]： session id  // NOLINT
  *@return： 成功返回connection指针，失败返回空  // NOLINT
  */
  virtual ConnectionInterfacePtr GetConnectionPtrWithSessionID(const std::string& session_id);
  /**
  *@brief 通过connection指针生成新session id  // NOLINT
  *@param connection_ptr[Input]： connection指针  // NOLINT
  *@return： 成功返回session id，失败返回空字符串  // NOLINT
  */
  virtual std::string GenerateSessionIDWithConnectionPtr(const ConnectionInterfacePtr& connection_ptr);
  /**
  *@brief 保存session id和connection指针  // NOLINT
  *@param session_id[Input]： session id  // NOLINT
  *@param connection_ptr[Input]： connection指针  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  virtual int SaveSessionIDnConnectionPtr(const std::string& session_id, const ConnectionInterfacePtr& connection_ptr);
  /**
  *@brief 通过session id设置connection指针  // NOLINT
  *@param session_id[Input]： session id  // NOLINT
  *@param connection_ptr[Input]： connection指针  // NOLINT
  *@param is_restored[Input]： 如果没有查找到session_id，判断是否重新保存该session_id和connection指针  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  virtual int SetConnectionPtrWithSessionID(const std::string& session_id, const ConnectionInterfacePtr& connection_ptr, bool is_restored = false);
  /**
  *@brief 通过connection指针设置session id  // NOLINT
  *@param connection_ptr[Input]： connection指针  // NOLINT
  *@param session_id[Input]： session id  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  virtual int SetSessionIDWithConnectionPtr(const ConnectionInterfacePtr& connection_ptr, const std::string& session_id);
  /**
  *@brief 重置connection指针  // NOLINT
  *@param connection_ptr[Input]： 原connection指针  // NOLINT
  *@param new_connection_ptr[Input]： 新connection指针  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  virtual int ResetConnectionPtr(const ConnectionInterfacePtr& connection_ptr, const ConnectionInterfacePtr& new_connection_ptr);
  /**
  *@brief 重置session id  // NOLINT
  *@param session_id[Input]： 原session id  // NOLINT
  *@param new_session_id[Input]： 新session id  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  virtual int ResetSessionID(const std::string& session_id, const std::string& new_session_id);
  /**
  *@brief 打印所有session id和connection指针  // NOLINT
  *@param 无  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void DumpSessionIDMatchingTable();
  /**
  *@brief 读取心跳函数  // NOLINT
  *@param 无  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int ReadHeartbeat();
  /**
  *@brief 写入心跳函数  // NOLINT
  *@param 无  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int WriteHeartbeat();
  /**
  *@brief 读取心跳回调函数  // NOLINT
  *@param 无  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void HandleReadHeartbeat();
  /**
  *@brief 写入心跳回调函数  // NOLINT
  *@param time_stamp[Input]： 写入心跳的时间戳   // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void HandleWriteHeartbeat(const std::string& time_stamp);
  /**
  *@brief 写入心跳操作消息处理线程，客户端模式下对应整个连接，服务器模式下对应个别子连接  // NOLINT
  *@param time_stamp[Input]： 读取到的时间戳信息   // NOLINT
  *@return： 成功返回true，失败返回false  // NOLINT
  */
  bool ProcessWriteHeartbeatMessage(const std::string& time_stamp);
  /**
  *@brief 获取流水号  // NOLINT
  *@param serial_number[Input]： 流水号  // NOLINT
  *@return： 无  // NOLINT
  */
  void GetSerialNumber(std::string& serial_number);
  /**
  *@brief 运行io service  // NOLINT
  *@param 无  // NOLINT
  *@return： 无  // NOLINT
  */
  void RunIOService();

 private:
  /**
  *@brief 初始化参数函数  // NOLINT
  *@param params[Input]： 初始化参数  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  int InitOpenParam(void *params);
  /**
  *@brief 获取模块运行状态值
  *@return： 模块运行状态值
  */
  UINT32 GetRunningStatus() {
    BoostSharedLock lock(shared_mutex_);
    return running_status_;
  }
  /**
  *@brief 设置模块运行状态值
  *@param status[Input]： 模块运行状态值
  *@return： 无
  */
  void SetRunningStatus(const UINT32 status) {
    BoostLockGuard lock(shared_mutex_);
    running_status_ = status;
  }

 private:
  /**
  *@brief 顶层基类和子类共同使用成员变量(客户端模式下将不分顶层基类与子类，将全部使用)  // NOLINT
  */
  TransportInterfacePtr transport_ptr_;  ///< Transport指针(客户端模式下，Open接口内创建/服务器模式下，Open接口内创建/服务器端点模式下，由socket层Accet接口创建，通过回调函数传给connection层)  // NOLINT
  ConnectionCallbackInterfacePtr connection_callback_impl_ptr_;  ///< connection层回调类指针  // NOLINT
  LightLock ll_lock_;  //锁，设置属性值时使用  // NOLINT

  /**
  *@brief 顶层基类使用成员变量  // NOLINT
  */
  TransportCallbackInterfacePtr transport_callback_ptr_;  ///< transport层回调类指针  // NOLINT

  std::list<ConnectionInterfacePtr> connection_ptr_list_;  ///< 保存所有连接的链表  // NOLINT
  InvalidConnectionJobQueue invalid_connection_job_queue_;  ///< 无效连接队列  // NOLINT
  InvalidConnectionWorkerPtr invalid_connection_worker_ptr_;  ///< 处理无效连线程  // NOLINT

  ProtocolMap protocol_map_;  ///< 保存协议类智能指针的Map，暂时不使用  // NOLINT

  boost::object_pool<WriteJobParams> write_job_object_pool_;  ///< 写入数据消息对象池  // NOLINT
  WriteJobQueue write_job_queue_;  ///< 写入数据消息队列  // NOLINT
  WriteWorkerPtr write_worker_ptr_;  ///< 写入消息处理线程  // NOLINT

  SessionIDMatchingTable session_id_matching_table_;  ///< session id和connection指针匹配表  // NOLINT

  mutable BoostSharedMutex shared_mutex_;  ///< 读写共享锁  // NOLINT
  volatile UINT32 running_status_;  ///< 运行状态标记符  // NOLINT

  UINT32 mode_;  ///< 模式，服务器模式/客户端模式/服务器节点模式  // NOLINT
  std::string address_;  ///< 地址  // NOLINT
  UINT32 port_;  ///< 端口  // NOLINT

  bool enable_wrap_;  ///< 解/封标记符  // NOLINT
  bool enable_write_;  ///< 写入标记符  // NOLINT
  bool enable_read_;  ///< 读取标记符  // NOLINT

  /**
  *@brief 子类使用成员变量  // NOLINT
  */
  ConnectionInterfacePtr root_connection_ptr_;  ///< 顶层基类connection指针  // NOLINT
  ReadJobQueue read_job_queue_;  ///< 保存读取数据消息队列  // NOLINT
  ReadWorkerPtr read_worker_ptr_;  ///< 读取消息处理线程  // NOLINT

  BufferPoolInterfacePtr buffer_pool_ptr_;  ///< buffer pool指针  // NOLINT

  ProtocolInterfacePtr protocol_ptr_;  ///< 协议接口指针  // NOLINT

  boost::asio::io_service io_service_;  ///< io service  // NOLINT
  boost::thread run_io_service_thread_;  ///< 运行io_service的线程  // NOLINT
  DeadlineTimer read_heartbeat_timer_;  ///< 读取心跳timer  // NOLINT
  DeadlineTimer write_heartbeat_timer_;  ///< 写入心跳timer  // NOLINT

  UINT64 serial_number_;  ///< 心跳操作流水号  // NOLINT
  UINT32 read_heartbeat_deadline_time_;  ///< 读取心跳操作时间  // NOLINT
  UINT32 write_heartbeat_deadline_time_;  ///< 写入心跳操作时间  // NOLINT
  UINT32 legitimate_inconnection_times_counter_;  ///< 最大失联次数计数器  // NOLINT

  HeartbeatJobQueue read_heartbeat_job_queue_;  ///< 保存读取心跳数据消息队列  // NOLINT

  HeartbeatJobQueue write_heartbeat_job_queue_;  ///< 保存写入心跳数据消息队列  // NOLINT
  HeartbeatWorkerPtr write_heartbeat_worker_ptr_;  ///< 写入心跳消息处理线程  // NOLINT
};

//////////////////////////////////////////////////////////////////////////
/**
*@brief ProtocolMapValueFinder的具体实现，辅助类  // NOLINT
*/
class ProtocolMapValueFinder {
 public:
  ProtocolMapValueFinder(const UINT32 protocol_type) : protocol_type_(protocol_type) {}

  bool operator()(const CommonConnectionImpl::ProtocolMap::value_type &pair) {
    return pair.first == protocol_type_;
  }

 private:
  const UINT32 protocol_type_;
};

//////////////////////////////////////////////////////////////////////////
/**
*@brief SessionIDMatchingTableValueFinder的具体实现，辅助类  // NOLINT
*/
class SessionIDMatchingTableValueFinder {
 public:
  SessionIDMatchingTableValueFinder(const ConnectionInterfacePtr& connection_ptr) : connection_ptr_(connection_ptr) {}

  bool operator()(const std::map<std::string, ConnectionInterfacePtr>::value_type &pair) {
    return pair.second == connection_ptr_;
  }

 private:
  const ConnectionInterfacePtr& connection_ptr_;
};

}  //  end of namespace ysos

#endif
