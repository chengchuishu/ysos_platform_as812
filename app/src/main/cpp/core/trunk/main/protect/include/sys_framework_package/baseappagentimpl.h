/**
 *@file baseappagentimpl.h
 *@brief Implement of base app agent implement
 *@version 0.1
 *@author JinChengzhe
 *@date Created on: 2016-07-12 16:12:58
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef SIP_BASE_APP_AGENT_IMPL_H       //NOLINT
#define SIP_BASE_APP_AGENT_IMPL_H       //NOLINT

#include "../../../Public/include/sys_interface_package/baseinterface.h"
#include "../../../Public/include/sys_interface_package/agentinterface.h"
#include "../../../Public/include/sys_interface_package/bufferpool.h"
#include "../../../Public/include/sys_interface_package/connectioninterface.h"
#include "../../../Public/include/sys_interface_package/transportinterface.h"
#include "../../../protect/include/protocol_help_package/httpprotocolimpl.h"
#include "../../../Public/include/os_hal_package/basecallbackimpl.h"  // NOLINT
#include "../../../ThirdParty/boost_1_59_0/boost/shared_ptr.hpp"
#include "../../../ThirdParty/boost_1_59_0/boost/system/system_error.hpp"
#include "../../../ThirdParty/boost_1_59_0/boost/enable_shared_from_this.hpp"  // NOLINT
#include "../../../ThirdParty/boost_1_59_0/boost/thread.hpp"
#include "../../../ThirdParty/boost_1_59_0/boost/bind.hpp"
#include "../../../ThirdParty/boost_1_59_0/boost/function.hpp"  // NOLINT


namespace ysos {
#define CONNECT_OPERATION_ERROR "Connect_Operation_Error"
#define READ_OPERATION_ERROR "Read_Operation_Error"
#define WRITE_OPERATION_ERROR "Write_Operation_Error"

class LightLock;
class MessageImpl;
typedef boost::shared_ptr<MessageImpl> MessageImplPtr;
class StrategyInterface;
typedef boost::shared_ptr<StrategyInterface> StrategyInterfacePtr;
class MessageInterface;
typedef boost::shared_ptr<MessageInterface> MessageInterfacePtr;


class BaseAppAgentCallbackImpl;
typedef boost::shared_ptr<BaseAppAgentCallbackImpl> BaseAppAgentCallbackImplPtr;
class YSOS_EXPORT BaseAppAgentCallbackImpl : public BaseCallbackImpl {
  DECLARE_CREATEINSTANCE(BaseAppAgentCallbackImpl);
  DISALLOW_COPY_AND_ASSIGN(BaseAppAgentCallbackImpl);
  DECLARE_PROTECT_CONSTRUCTOR(BaseAppAgentCallbackImpl);

 public:
  /**
  *@brief 设置BaseAppAgentCallbackInfo回调函数信息  // NOLINT
  */
  struct BaseAppAgentCallbackInfo {
    UINT32 type;
    boost::system::error_code error_code;
    std::string message;
    BaseAppAgentCallbackInfo() : message("") {}
  };

  /**
  *@brief 析构函数  // NOLINT
  *@param ： 无  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual ~BaseAppAgentCallbackImpl();

  /**
  *@brief 回调接口的实现函数  // NOLINT
  *@param input_buffer[Input]： 输入数据缓冲  // NOLINT
  *@param output_buffer[Output]：回调处理后输出的数据缓冲  // NOLINT
  *@param context[Input]： 回调处理的上下文数据,是ModuleInterface* 类型  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  int Callback(BufferInterfacePtr input_buffer, BufferInterfacePtr output_buffer, void *context = NULL);

  boost::function<void(const boost::system::error_code&, const std::string&, const UINT32)> ProcessMessageHandler;
};

class BaseAppAgentImpl;
typedef boost::shared_ptr<BaseAppAgentImpl> BaseAppAgentImplPtr;
class YSOS_EXPORT BaseAppAgentImpl : public AgentInterface, public BaseInterfaceImpl, public boost::enable_shared_from_this<BaseAppAgentImpl> {
  DECLARE_CREATEINSTANCE(BaseAppAgentImpl);
  DECLARE_PROTECT_CONSTRUCTOR(BaseAppAgentImpl);

 public:

  enum BaseAppAgentImplProperties {
    BUFFER_POOL_INFO,
    CLIENT_IP,
    CLIENT_PORT,
    SERVER_IP,
    SERVER_PORT,
    SYNC_MODE,
    ASYNC_MODE,
    ON_PROCESS_MESSAGE,
    ON_SENT_MESSAGE,
    ON_CONNECT_PLATFROM,
    BASE_APP_AGENT_IMPL_PROPERTY_END,
  };

  virtual ~BaseAppAgentImpl(void);

  /**
  *@brief  //NOLINT
  *@param [Input]： //NOLINT
  *@param [Output]： //NOLINT
  *@return： 若成功则返回YSOS_ERROR_SUCCESS，否则返回相应错误值//NOLINT
  */
  virtual int GetServiceList(std::string& service_list);

  /**
  *@brief  //NOLINT
  *@param [Input]： //NOLINT
  *@param [Output]： //NOLINT
  *@return： 若成功则返回YSOS_ERROR_SUCCESS，否则返回相应错误值//NOLINT
  */
  virtual int RegDoServiceEvent(
    const std::string &service_name,
#if ENABLE_SERVICE_WITH_PARAM
    const std::string &service_param,
#endif
    const std::string &event_param);

  /**
  *@brief  //NOLINT
  *@param [Input]： //NOLINT
  *@param [Output]： //NOLINT
  *@return： 若成功则返回YSOS_ERROR_SUCCESS，否则返回相应错误值//NOLINT
  */
  virtual int UnregDoServiceEvent(
    const std::string &service_name,
#if ENABLE_SERVICE_WITH_PARAM
    const std::string &service_param,
#endif
    const std::string &event_param);

  /**
  *@brief  //NOLINT
  *@param [Input]： //NOLINT
  *@param [Output]： //NOLINT
  *@return： 若成功则返回YSOS_ERROR_SUCCESS，否则返回相应错误值//NOLINT
  */
  virtual int DoService(
    const std::string &service_name,
#if ENABLE_SERVICE_WITH_PARAM
    const std::string &service_param,
#endif
    const std::string &event_param);

  /**
   *@brief  //NOLINT
   *@param [Input]： //NOLINT
   *@param [Output]： //NOLINT
   *@return： 若成功则返回YSOS_ERROR_SUCCESS，否则返回相应错误值//NOLINT
   */
  virtual int GetServiceState(
    const std::string &service_name,
    std::string &state_param);

  /**
  *@brief  //NOLINT
  *@param [Input]： //NOLINT
  *@param [Output]： //NOLINT
  *@return： 若成功则返回YSOS_ERROR_SUCCESS，否则返回相应错误值//NOLINT
  */
  virtual int OnDispatchMessage(
    const std::string &service_name,
    const std::string &event_key,
    const std::string &callback_name,
    const std::string &event_param);

  /**
  *@brief  //NOLINT
  *@param [Input]： //NOLINT
  *@param [Output]： //NOLINT
  *@return： 若成功则返回YSOS_ERROR_SUCCESS，否则返回相应错误值//NOLINT
  */
  virtual int LoadConfig(
    const std::string &config_file_name,
    const std::string &config_param,
    int config_type);

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

  int Open();
  void Close();

  void HandleAccept(const boost::system::error_code& error_code, const TransportInterfacePtr transport_interface_ptr);
  void HandleRead4Server(const boost::system::error_code& error_code, const std::size_t bytes_transferred, const BufferInterfacePtr buffer_interface_ptr, const TransportInterfacePtr transport_interface_ptr);
  void HandleWrite4Server(const boost::system::error_code& error_code, const std::size_t bytes_transferred, const BufferInterfacePtr buffer_interface_ptr, const TransportInterfacePtr transport_interface_ptr);

  void HandleConnect(const boost::system::error_code& error_code, const ConnectionInterfacePtr connection_interface_ptr);
  void HandleDisconnect(const ConnectionInterfacePtr connection_interface_ptr);
  void HandleRead(const boost::system::error_code& error_code, const std::size_t bytes_transferred, const BufferInterfacePtr buffer_interface_ptr, const ConnectionInterfacePtr connection_interface_ptr);
  void HandleWrite(const boost::system::error_code& error_code, const std::size_t bytes_transferred, const BufferInterfacePtr buffer_interface_ptr, const ConnectionInterfacePtr connection_interface_ptr);

 private:
  int InitBufferPool(const UINT32 buffer_size, const UINT32 buffer_count, BufferPoolInterfacePtr& buffer_pool_interface_ptr);
  int InitBuffer(const BufferPoolInterfacePtr& buffer_pool_interface_ptr, BufferInterfacePtr& buffer_interface_ptr, UINT32& buffer_length);
  int RequestMessage(std::string& message);

 private:
  //  ConnectionInterfacePtr connection_interface_ptr_;

  BufferPoolInterfacePtr buffer_pool_interface_ptr_;
  BufferInterfacePtr read_buffer_interface_ptr_;
  BufferInterfacePtr write_buffer_interface_ptr_;

  std::list<std::string> message_queue_;
  typedef std::list<std::string>::iterator message_queue_iterator;

  LightLock ll_lock_;  //锁，设置属性值时使用

  static int serial_number_;

  BaseAppAgentCallbackImplPtr base_app_agent_callback_impl_ptr_;

  std::string client_IP_;
  UINT32 client_port_;

  std::string server_IP_;
  UINT32 server_port_;

  bool is_sync_;
};
}

#endif  //  SIP_BASE_APP_AGENT_IMPL_H   //NOLINT
