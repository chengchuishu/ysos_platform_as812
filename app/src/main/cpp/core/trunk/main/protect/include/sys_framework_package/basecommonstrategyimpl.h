/**
 *@file BaseCommmonStrategyImpl.h
 *@brief Definition of BaseCommmonStrategyImpl
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-21 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef SFP_BASE_COMMON_STRATEGY_IMPL_H_  // NOLINT
#define SFP_BASE_COMMON_STRATEGY_IMPL_H_  // NOLINT

/// STL Headers
#include <map>
#include <list>
/// ThirdParty Headers
#include <tinyxml2/tinyxml2.h>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
/// Private Ysos Headers
#include "../../../public/include/os_hal_package/lock.h"
#include "../../../public/include/sys_interface_package/strategyinterface.h"
#include "../../../public/include/sys_interface_package/msginterface.h"
#include "../../../protect/include/sys_framework_package/serviceinfotable.h"

namespace ysos {
/*************************************************************************************************************
 *                      BaseCommonStrategyImpl                                                                     *
 *************************************************************************************************************/
/**
  *@brief BaseCommonStrategyImpl是独立于BaseStrategyImpl的又一个更通用的Strategy基类    //  NOLINT
     它首先用于状态收集器和Daemon与YSOS的通信逻辑处理           //  NOLINT
  */
class YSOS_EXPORT BaseCommonStrategyImpl : public StrategyInterface,
  public BaseInterfaceImpl,
  public boost::enable_shared_from_this<BaseCommonStrategyImpl> {
  DECLARE_CREATEINSTANCE(BaseCommonStrategyImpl);
  DISALLOW_COPY_AND_ASSIGN(BaseCommonStrategyImpl);
  DECLARE_PROTECT_CONSTRUCTOR(BaseCommonStrategyImpl);

 public:
  virtual ~BaseCommonStrategyImpl(void);

  //  FackLock
  typedef MutexLock                                          StrategyLock;
  typedef std::list<std::string>                             EventList;

  /**
   *@brief 基本初始化  // NOLINT
   *@param param： 初始化的参数 // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Initialize(void *param=NULL);
  /**
   *@brief 清除资源,Initialize的逆操作  // NOLINT
   *@param param： 初始化的参数 // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int UnInitialize(void *param=NULL);
  /**
  *@brief 加载策略逻辑  // NOLINT              @howto
  *@param strategy_name[Input]： 策略名称  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int LoadStrategy(const std::string &strategy_name);
  /**
   *@brief 创建策略逻辑  // NOLINT
   *@param strategy_name[Input]： 策略名称  // NOLINT
   *@param params[Input]： 策略所需的参数  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int CreateStrategy(const std::string &strategy_name, void *params);
  /**
   *@brief 销毁策略逻辑  // NOLINT
   *@param ： 无  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int DestroyStrategy();
  /**
   *@brief 保存策略逻辑  // NOLINT
   *@param ： 无  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int SaveStrategy();
  /**
   *@brief 配置函数，可对该接口进行详细配置  // NOLINT
   *@param ctrl_id[Input]： 详细配置ID号  // NOLINT
   *@param param[Input]：详细配置的参数  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Ioctl(INT32 ctrl_id, LPVOID param = NULL);
  /**
   *@brief 停止  // NOLINT
   *@param ： 无  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Stop(LPVOID param = NULL);
  /**
   *@brief 暂停  // NOLINT
   *@param ： 无  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Pause(LPVOID param = NULL);
  /**
   *@brief 运行  // NOLINT
   *@param ： 无  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Run(LPVOID param = NULL);
  /**
   *@brief 冲洗，快速将内部缓冲中剩余数据处理完  // NOLINT
   *@param ： 无  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Flush(LPVOID param = NULL);
  /**
     *@brief 获取接口的属性值  // NOLINT
     *@param type_id[Input]： 属性的ID  // NOLINT
     *@param type[Input/Output]：属性值的值  // NOLINT
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
   *@brief 获得被控制模块的状态  // NOLINT
   *@param timeout[Input]： 超时值  // NOLINT
   *@param state[Output]：状态  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int GetState(UINT32 timeout, INT32 *state, LPVOID param = NULL);

 protected:
  ////////////////////////////////////////////////////////////////////////
  /////////      internal interface    //////////////////////////////////
  //////////////////////////////////////////////////////////////////////
  /**
    *@brief  从msg_queue_里取得数据 //  NOLINT
    *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
    */
  MsgInterfacePtr GetMessage(void);
  /**
   *@brief  解析strategy_xml，判断是哪种机制，而后调用具体的机制解析 //  NOLINT
   *@param strategy_xml 待解析的消息  //  NOLINT
   *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
   */
  int ParseStrategyXml(const std::string &strategy_xml);
  /**
   *@brief  处理消息 //  NOLINT
   *@param  msg_ptr[Input]: 待处理的消息  //  NOLINT
   *@return 成功返回0，否则失败  //  NOLINT
   */
  int HandleMesssage(MsgInterfacePtr msg_ptr);
  /**
   *@brief  处理消息 //  NOLINT
   *@param  event_name[Input]: 待处理的事件名  //  NOLINT
   *@param  input_buffer[Input]: 待处理的数据  //  NOLINT
   *@param  context_ptr[Input]: 上下文指针  //  NOLINT
   *@return 成功返回0，否则失败  //  NOLINT
   */
  int HandleMesssage(const char *event_name, const BufferInterfacePtr input_buffer, void *context_ptr);
  /**
     *@brief  处理消息 //  NOLINT
     *@param  event_name[Input]: 待处理的事件名  //  NOLINT
     *@param  event_list[Input]: 事件链表  //  NOLINT
     *@return 成功返回true，失败返回false  //  NOLINT
     */
  bool IfMatchedMesssage(const char *event_name, EventList &event_list);
  /**
   *@brief  获取待处理的事件 //  NOLINT
   *@param  service_name[Input]: 待处理的服务名  //  NOLINT
   *@param  event_child 事件的XML参数[Input]: 待处理的数据  //  NOLINT
   *@param  事件的存储链表[Input]: 上下文指针  //  NOLINT
   *@return 无  //  NOLINT
   */
  void GetServiceEvent(const std::string &service_name, tinyxml2::XMLElement *event_child, EventList &event_list);
  /**
   *@brief Strategy正式启动  // NOLINT
   *@return： 无  // NOLINT
   */
  virtual void HandleStrategy(void);

 protected:
  bool                          is_run_;                        ///<  当前Strategy是否在运行 //  NOLINT
  bool                          is_loaded_;                     ///<  标识Strategy是否已Load完成  //  NOLINT
  StrategyLock                  *strategy_lock_;
  StrategyLock                  *msg_queue_lock_;
  MsgQueueInterfacePtr          msg_queue_;
  std::string                   local_service_name_;            ///< local service的名称
  CallbackInterfacePtr          local_service_strategy_;        ///< local service的实际处理逻辑
  std::string                   client_service_name_;           ///< client service的名称
  CallbackInterfacePtr          client_service_strategy_;       ///< client service的实际处理逻辑
  std::string                   internal_service_name_;         ///< internal service的名称
  CallbackInterfacePtr          internal_service_strategy_;     ///< internal service的实际处理逻辑
  EventList                     local_event_list_;              ///< local service处理的事件
  EventList                     client_event_list_;             ///< client service处理的事件
  EventList                     internal_event_list_;           ///< local service处理的事件
  std::string                   ip_;                            ///< 服务器或客户端的IP
  uint32_t                      port_;                          ///< 服务器或客户端的IP端口
  std::string                   state_;                         ///< 初始状态，暂时不使用

 private:
  boost::thread                 *cur_thread_;                   ///< 当前活动的线程
};

typedef boost::shared_ptr<BaseCommonStrategyImpl>   BaseCommonStrategyImplPtr;

}

#endif  // SFP_BASE_COMMON_STRATEGY_IMPL_H_  // NOLINT
