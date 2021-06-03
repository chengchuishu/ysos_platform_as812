/*
# socket.h
# Definition of socket
# Created on: 2016-05-11 19:46:58
# Original author: jinchengzhe
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
# version 0.1, 20160912, created by JinChengZhe
*/
#ifndef OHP_SOCKET_H        //NOLINT
#define OHP_SOCKET_H        //NOLINT

#ifdef _MSC_VER
#define _WIN32_WINNT 0x0501
#endif
#define BOOST_REGEX_NO_LIB

/* BOOST_ALL_DYN_LINK */
#if !defined(BOOST_ALL_DYN_LINK)
#define BOOST_DATE_TIME_SOURCE
#endif

/// C++ Standard Headers
#include <set>
#include <vector>
/// ThirdParty Headers
#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <boost/asio/error.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/functional/factory.hpp>
/// Platform Headers
#include "../../../public/include/sys_interface_package/common.h"
#include "../../../public/include/core_help_package/baseinterfaceimpl.h"
#include "../../../public/include/sys_interface_package/transportinterface.h"
#include "../../../public/include/sys_interface_package/transportcallbackinterface.h"
#include "../../../public/include/sys_interface_package/ysos_enum.h"

namespace ysos {

/**
*@brief 重新定义asion的io_service相关类  // NOLINT
*/
typedef boost::asio::io_service IOService;
typedef boost::asio::io_service::work IOServiceWork;
typedef boost::shared_ptr<IOService> IOServicePtr;
typedef boost::shared_ptr<IOServiceWork> IOServiceWorkPtr;

/**
*@brief 重新定义asio的tcp相关类  // NOLINT
*/
typedef boost::asio::ip::tcp TCP;
typedef boost::asio::ip::tcp::socket TCPSocket;
typedef boost::asio::ip::tcp::acceptor TCPAcceptor;
typedef boost::asio::ip::tcp::endpoint TCPEndPoint;
typedef boost::asio::ip::tcp::resolver TCPResolver;
typedef boost::asio::ip::tcp::resolver::query TCPQuery;
typedef boost::asio::ip::tcp::acceptor::non_blocking_io NonBlockingIO;
typedef boost::shared_ptr<TCPSocket> TCPSocketPtr;
typedef boost::shared_ptr<TCPAcceptor> TCPAcceptorPtr;

/**
*@brief 重新定义asio的address和system的error相关类  // NOLINT
*/
typedef boost::asio::ip::address TCPAddress;
typedef boost::system::error_code SystemErrorCode;

/**
*@brief 重新定义asio的signal_set相关类  // NOLINT
*/
typedef boost::asio::signal_set SignalSet;

/**
*@brief 重新定义asio的deadline_timer相关类  // NOLINT
*/
typedef boost::asio::deadline_timer DeadlineTimer;
typedef boost::shared_ptr<DeadlineTimer> DeadlineTimerPtr;

/**
*@brief 重新定义读写共享锁  // NOLINT
*/
typedef boost::shared_mutex BoostSharedMutex;
typedef boost::shared_lock<BoostSharedMutex> BoostSharedLock;
typedef boost::lock_guard<BoostSharedMutex> BoostLockGuard;

/**
*@brief 定义默认io service 个数  // NOLINT
*/
#define DEFALT_IO_SERVICE_COUNT_FOR_TCP_SOCKET_CLIENT 1
#define DEFALT_IO_SERVICE_COUNT_FOR_TCP_SOCKET_SERVER 1
#define DEFALT_IO_SERVICE_COUNT_FOR_TCP_SOCKET_SERVER_TERMINAL 1

/**
 *@brief IOServicePool的具体实现  // NOLINT
 */
class YSOS_EXPORT IOServicePool {
  DISALLOW_COPY_AND_ASSIGN(IOServicePool);
 public:
  explicit IOServicePool(std::size_t pool_size = 4);
  ~IOServicePool();

  void Run();
  void Stop();

  IOService& GetIOService();

 private:
  std::vector<IOServicePtr> io_services_;
  std::vector<IOServiceWorkPtr> io_service_works_;

  std::size_t next_io_service_index_;
};


/**
 *@brief BaseTCPTransportImpl的具体实现  // NOLINT
 */
class LightLock;
class BaseTCPTransportImpl;
typedef boost::shared_ptr<BaseTCPTransportImpl> TCPTransportPtr;
class BaseTCPTransportImpl : public TransportInterface, public BaseInterfaceImpl, public boost::enable_shared_from_this<BaseTCPTransportImpl> {
  DISALLOW_COPY_AND_ASSIGN(BaseTCPTransportImpl);

 public:
  /**
  *@brief 模块运行状态  // NOLINT
  */
  enum {
    IS_INITIAL = 0,  ///< 初始状态  // NOLINT
    IS_RUNNING = 1,  ///< 运行状态  // NOLINT
    IS_STOPPED = 2,  ///< 停止状态  // NOLINT
  };
  /**
  *@brief BaseTCPTransportImpl的消息头信息  // NOLINT
  */
  typedef struct HeaderInfo {
    UINT32 message_total_count;  ///< 消息总个数  // NOLINT
    UINT32 current_message_index;  ///< 当前消息索引  // NOLINT
    UINT32 message_body_length;  ///< 消息体长度  // NOLINT
    UINT32 version_info;  ///< 版本号  // NOLINT
  } HI, *HeaderInfoPtr;

  /**
  *@brief 显性构造函数  // NOLINT
  *@param mode[Input]： 模式，包括客户端/服务器/服务器终端三种模式  // NOLINT
  *@param io_service_pool_size[Input]： io service pool大小  // NOLINT
  *@param running_status[Input]： 运行状态  // NOLINT
  *@param strClassName[Input]： 初始化类名  // NOLINT
  *@return： 无  // NOLINT
  */
  explicit BaseTCPTransportImpl(const UINT32 mode = PROP_SERVER_TERMINAL,
                                const std::size_t io_service_pool_size = DEFALT_IO_SERVICE_COUNT_FOR_TCP_SOCKET_SERVER_TERMINAL,
                                const UINT32 running_status = IS_INITIAL,
                                const std::string strClassName = "BaseTCPTransportImpl");
  /**
  *@brief 析构函数  // NOLINT
  *@param 无  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual ~BaseTCPTransportImpl();
  /**
  *@brief 打开并初始化传输模块  // NOLINT
  *@param param[Input]： 输入的配置参数  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  int Open(void *params);
  /**
   *@brief 关闭类  // NOLINT
   *@return： 无  // NOLINT
   */
  void Close();
  /**
   *@brief 读取数据函数  // NOLINT
   *@param buffer_ptr[Input]： 写入数据的缓冲，内部包含实际写入数据长度  // NOLINT
   *@param length[Input]： 缓冲长度，暂时无用  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  int Read(BufferInterfacePtr buffer_ptr, int length);
  /**
   *@brief 写入数据函数  // NOLINT
   *@param buffer_ptr[Input]： 写入数据的缓冲，内部包含实际写入数据长度  // NOLINT
   *@param length[Input]： 缓冲长度，暂时无用  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  int Write(BufferInterfacePtr buffer_ptr, int length);
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
  *@brief 获取接口的属性值  // NOLINT
  *@param type_id[Input]： 属性的ID  // NOLINT
  *@param type[input]： 属性值的值  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  int GetProperty(int type_id, void *type);
  /**
   *@brief 设置接口的属性值  // NOLINT
   *@param type_id[Input]： 属性的ID  // NOLINT
   *@param type[input]： 属性值的值  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  int SetProperty(int type_id, void *type);
  /**
  *@brief 获取scocket  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  TCPSocket& GetTCPSocket();

 protected:
  /**
  *@brief 读取消息头数据回调函数，异步控制读取操作  // NOLINT
  *@param error[IN]:  系统错误  // NOLINT
  *@param bytes_transferred[IN]:  读取的字节数  // NOLINT
  *@param buffer_ptr[IN]:  读取数据的缓冲，内部包含实际写入数据长度  // NOLINT
  *@param transport_ptr[Input]：  建立起连接的transport指针  // NOLINT
  *@return:  无  // NOLINT
  */
  virtual void HandleReadHeaderInfo(const boost::system::error_code& error_code, const size_t bytes_transferred, const BufferInterfacePtr buffer_ptr);
  /**
  *@brief 读取数据回调函数，异步控制读取操作  // NOLINT
  *@param error[IN]:  系统错误  // NOLINT
  *@param bytes_transferred[IN]:  读取的字节数  // NOLINT
  *@param buffer_ptr[IN]:  读取数据的缓冲，内部包含实际写入数据长度  // NOLINT
  *@param transport_ptr[Input]：  建立起连接的transport指针  // NOLINT
  *@return:  无  // NOLINT
  */
  virtual void HandleRead(const boost::system::error_code& error_code, const size_t bytes_transferred, const BufferInterfacePtr buffer_ptr);
  /**
  *@brief 写入数据回调函数，异步控制写入操作  // NOLINT
  *@param error[IN]:  系统错误  // NOLINT
  *@param bytes_transferred[IN]:  写入的字节数  // NOLINT
  *@param buffer_ptr[IN]:  写入数据的缓冲，内部包含实际写入数据长度  // NOLINT
  *@param socket_ptr[IN]:  建立起连接的transport指针  // NOLINT
  *@return:  无  // NOLINT
  */
  virtual void HandleWrite(const boost::system::error_code& error_code, const size_t bytes_transferred, const BufferInterfacePtr buffer_ptr);
  /**
  *@brief Connect的回调函数  // NOLINT
  *@param error[IN]:  系统错误  // NOLINT
  *@return:  无  // NOLINT
  */
  virtual void HandleConnect(const SystemErrorCode& error_code);
  /**
  *@brief Accept回调函数  // NOLINT
  *@param error[Input]： 系统错误代码  // NOLINT
  *@param transport_ptr[Input]： 建立起连接的transport指针  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void HandleAccept(const SystemErrorCode& error_code, TransportInterfacePtr transport_ptr);
  /**
  *@brief 错误处理回调函数，异步控制错误处理操作  // NOLINT
  *@param callback_type[IN]:  回调函数类型  // NOLINT
  *@param error[IN]:  系统错误  // NOLINT
  *@return:  无  // NOLINT
  */
  virtual void HandleError(const UINT32 callback_type, const SystemErrorCode& error_code);
  /**
  *@brief 关闭连接回调函数，异步控制关闭连接操作  // NOLINT
  *@param error[IN]:  系统错误  // NOLINT
  *@return:  无  // NOLINT
  */
  virtual void HandleDisconnect(const SystemErrorCode& error_code);
  /**
  *@brief 初始化参数  // NOLINT
  *@param param[Input]： 输入的配置参数  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int InitOpenParam(void *params);
  /**
  *@brief 初始化信号量  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int InitSignal(void);
  /**
  *@brief 运行io service pool  // NOLINT
  *@param 无  // NOLINT
  *@return:  无  // NOLINT
  */
  virtual void RunIOServicePool();
  /**
  *@brief 获取模块运行状态值  // NOLINT
  *@return： 模块运行状态值  // NOLINT
  */
  UINT32 GetRunningStatus() {
    BoostSharedLock lock(shared_mutex_);
    return running_status_;
  }
  /**
  *@brief 设置模块运行状态值  // NOLINT
  *@param status[Input]： 模块运行状态值  // NOLINT
  *@return： 无  // NOLINT
  */
  void SetRunningStatus(const UINT32 status) {
    BoostLockGuard lock(shared_mutex_);
    running_status_ = status;
  }
  /**
  *@brief 向指定的目的连接  // NOLINT
  *@param address[IN]:  目的IP地址  // NOLINT
  *@param port[IN]:     目的端口号  // NOLINT
  *@return:  成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int Connect(const std::string& address, const UINT32 port);
  /**
  *@brief 开始接受连接  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int Accept();
  /**
  *@brief 实际的打开操作  // NOLINT
  *@return:  成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int RealOpen(void);
  /**
   *@brief 实际的关闭操作  // NOLINT
   *@return:  成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int RealClose(void);

 protected:
  std::size_t io_service_pool_size_;  ///< io_service_pool的个数  // NOLINT
  IOServicePool io_service_pool_;  ///< io_service_pool  // NOLINT
  TCPSocket tcp_socket_;  ///< 建立起连接的socket  // NOLINT
  SignalSet signal_set_;  ///< 关闭信号量  // NOLINT
  boost::thread run_io_service_pool_thread_;  ///< 打开io_service的线程  // NOLINT
  std::string address_;  ///< 地址  // NOLINT
  UINT32 port_;  ///< 端口  // NOLINT
  bool enable_read_;  ///< 可读标记符  // NOLINT
  bool enable_write_;  ///< 可写标记符  // NOLINT

  LightLock ll_lock_;  ///< 锁，设置属性值用  // NOLINT
  mutable BoostSharedMutex shared_mutex_;  ///< 读写共享锁  // NOLINT
  TransportCallbackInterfacePtr transport_callback_ptr_;  ///< 回调函数指针  // NOLINT
  boost::asio::io_service::strand strand_;  ///< 异步读写线程  // NOLINT
  volatile UINT32 running_status_;  ///< 运行状态标记符  // NOLINT
  UINT32 mode_;  ///< 模式，客户端/服务器模式  // NOLINT
  static UINT32 header_info_size_;  ///< 消息头大小  // NOLINT
};


/**
 *@brief TCPSocketClient的具体实现  // NOLINT
 */
class TCPSocketClient;  ///< 客户端类声明
typedef boost::shared_ptr<TCPSocketClient> TCPSocketClientPtr;  ///< 客户端类智能指针
class YSOS_EXPORT TCPSocketClient : public BaseTCPTransportImpl {
  DECLARE_CREATEINSTANCE(TCPSocketClient);
  DISALLOW_COPY_AND_ASSIGN(TCPSocketClient);
  DECLARE_PROTECT_CONSTRUCTOR(TCPSocketClient);

 public:
  typedef boost::unique_lock<boost::mutex> UniqueLock;
  /**
   *@brief 析构函数  // NOLINT
   *@param ： 无  // NOLINT
   *@return： 无  // NOLINT
   */
  virtual ~TCPSocketClient();

 protected:
  /**
   *@brief 实际的打开操作  // NOLINT
   *@return:  成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int RealOpen(void);
  /**
   *@brief 实际的关闭操作  // NOLINT
   *@return:  成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int RealClose(void);

 private:
  /**
  *@brief 向指定的目的连接  // NOLINT
  *@param address[IN]:  地址  // NOLINT
  *@param port[IN]:  端口  // NOLINT
  *@return:  成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  int Connect(const std::string& address, const std::string& port);
  /**
  *@brief 向指定的目的连接  // NOLINT
  *@param error[IN]:  系统错误  // NOLINT
  *@param endpoint_iterator[IN]:  解析器的迭代器  // NOLINT
  *@return:  成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  void HandleResolve(const SystemErrorCode& error_code, TCPResolver::iterator endpoint_iterator);
  /**
  *@brief Connect的回调函数  // NOLINT
  *@param error[IN]:  系统错误  // NOLINT
  *@return:  无  // NOLINT
  */
  void HandleConnect(const SystemErrorCode& error_code);
  /**
  *@brief 获取模块连接状态值  // NOLINT
  *@return： 模块连接状态值  // NOLINT
  */
  bool GetConnectStatus() {
    BoostSharedLock lock(shared_mutex_);
    return is_connected_;
  }
  /**
  *@brief 设置模块连接状态值  // NOLINT
  *@param status[Input]： 模块连接状态值  // NOLINT
  *@return： 无  // NOLINT
  */
  void SetConnectStatus(bool status) {
    BoostLockGuard lock(shared_mutex_);
    is_connected_ = status;
  }

 private:
  volatile bool is_connected_;  ///< 连接状态  // NOLINT
  TCPResolver resolver_;  ///< 解析器  // NOLINT
  boost::mutex mutex_lock_;  ///< 互斥锁  // NOLINT
  boost::condition_variable_any condition_variable_any_connect_;  ///< 打开模块用信号量，用于同步操作  // NOLINT
};


/**
 *@brief TCPSocketServer的具体实现  // NOLINT
 */
class TCPSocketServer;
typedef boost::shared_ptr<TCPSocketServer> TCPSocketServerPtr;  ///< 服务器类智能指针  // NOLINT
class YSOS_EXPORT TCPSocketServer : public BaseTCPTransportImpl {
  DECLARE_CREATEINSTANCE(TCPSocketServer);
  DISALLOW_COPY_AND_ASSIGN(TCPSocketServer);
  DECLARE_PROTECT_CONSTRUCTOR(TCPSocketServer);

 public:
  /**
   *@brief 析构函数  // NOLINT
   *@return： 无  // NOLINT
   */
  virtual ~TCPSocketServer();

 protected:
  /**
  *@brief 实际的打开操作  // NOLINT
  *@return:  成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int RealOpen(void);
  /**
   *@brief 实际的关闭操作  // NOLINT
   *@return:  成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int RealClose(void);
  /**
   *@brief 开始接受连接  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Accept();

 protected:
  TCPAcceptor tcp_acceptor_;  ///< acceptor  // NOLINT
};

}

#endif //  OHP_SOCKET_H  //NOLINT
