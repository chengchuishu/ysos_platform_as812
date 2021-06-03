/**
 *@file baseagentimpl.h
 *@brief Definition of base agent implement
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-06-21 08:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef SFP_BASE_AGENT_IMPL_H       //NOLINT
#define SFP_BASE_AGENT_IMPL_H       //NOLINT

/// ThirdParty Headers
#include <boost/enable_shared_from_this.hpp>  // NOLINT
/// Platform Headers
#include "../../../public/include/sys_interface_package/common.h"
#include "../../../public/include/core_help_package/baseinterfaceimpl.h"
#include "../../../protect/include/sys_framework_package/platformrpc.h"
#include "../../../public/include/sys_interface_package/agentinterface.h"
#include "../../../public/include/sys_interface_package/datainterface.h"
#include "../../../public/include/sys_interface_package/sessioninterface.h"
#include "../../../public/include/sys_interface_package/msginterface.h"
#include "../../../public/include/sys_interface_package/strategyinterface.h"
#include "../../../public/include/core_help_package/utility.h"
#include "../../../protect/include/sys_framework_package/platformrpcentity.h"

namespace ysos {
class LightLock;

/**
  *@brief 代理接口，实现上层应用与核心平台之间的通讯机制  //NOLINT
  */
class BaseAgentImpl;
typedef boost::shared_ptr<BaseAgentImpl> BaseAgentImplPtr;
class YSOS_EXPORT BaseAgentImpl : public AgentInterface, public BaseInterfaceImpl, public boost::enable_shared_from_this<BaseAgentImpl> {
  DECLARE_CREATEINSTANCE(BaseAgentImpl);
  DISALLOW_COPY_AND_ASSIGN(BaseAgentImpl);
  DECLARE_PROTECT_CONSTRUCTOR(BaseAgentImpl);

 protected:
  BaseAgentImpl(const PlatformRPCPtr &connection_ptr, const SessionInterface *session_ptr, const std::string &strClassName = "BaseAgentImpl");
  virtual ~BaseAgentImpl(void);
//  friend class Singleton<BaseAgentImpl>;

 public:
  /**
  *@brief 初始化  // NOLINT
  *@param param[Input]： 初始化参数  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int Initialize(void *param=NULL);
  /**
  *@brief 反初始化  // NOLINT
  *@param param[Input]： 反初始化参数  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int UnInitialize(void *param=NULL);
  /**
   *@brief 生成一个新的Agent  // NOLINT
   *@param platform_sdk_ptr[Input]： 一个请求连接  // NOLINT
   *@param session_ptr[Input]： 管理该Agent的session  // NOLINT
   *@param param[Input]： 创建Agent的参数，是一个json字符串，包括应用名和状态机文件名  // NOLINT
   *@return： 成功返回agent指针，失败返回NULL  // NOLINT
   */
  virtual AgentInterface *Clone(const PlatformRPCPtr &platform_sdk_ptr, const SessionInterface *session_ptr, void* param);
  /**
   *@brief 销毁一个指定的Agent  // NOLINT
   *@param agent_ptr[Input]： 待删除的Agent  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Destroy(AgentInterface *agent_ptr);
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
 public:
  /**
  *@brief 处理上层App请求或者响应函数，供basesessionimpl调用公有接口
  */

  /**
   *@brief 处理上层App请求
   *@param service_name[Input]： 服务名  // NOLINT
   *@param service_param[Input]： 服务参数  // NOLINT
   *@return 无
   */
  virtual void HandleDoService(const std::string& service_name, const std::string& service_param);
  /**
   *@brief 处理上层App响应
   *@param service_name[Input]： 服务名  // NOLINT
   *@param result[Input]： 处理结果参数  // NOLINT
   *@return 无
   */
  virtual void HandleOnDispatchMessage(const std::string& service_name, const std::string& result);
  /**
   *@brief 处理错误信息
   *@param error_param[Input]： 错误信息  // NOLINT
   *@return 无
   */
  virtual void HandleError(const std::string& error_param);

 public:
  /**
  *@brief 向上层App发送请求或者响应函数，供basestatemachineimpl调用公有接口
  */

  // 平台向上层App发送服务请求

  /**
   *@brief 请求切换通知服务
   *@param service_param[Input]： 结构化服务请求参数  // NOLINT
   *@return 若成功则返回YSOS_ERROR_SUCCESS，否则返回相应错误值  //NOLINT
   */
  virtual int DoSwitchNotifyService(ReqSwitchNotifyServiceParam& service_param);
  /**
   *@brief 请求事件通知服务
   *@param service_param[Input]： 结构化服务请求参数  // NOLINT
   *@return 若成功则返回YSOS_ERROR_SUCCESS，否则返回相应错误值  //NOLINT
   */
  virtual int DoEventNotifyService(ReqEventNotifyServiceParam& service_param);
  /**
  *@brief 请求自定义服务
  *@param service_param[Input]： 结构化服务请求参数  // NOLINT
  *@return 若成功则返回YSOS_ERROR_SUCCESS，否则返回相应错误值  //NOLINT
  */
  virtual int DoCustomEventService(ReqCustomEventServiceParam& service_param);

  // 平台向上层App发送服务响应

  /**
   *@brief 响应切换服务
   *@param result[Input]： 处理结果结构化参数  // NOLINT
   *@return 若成功则返回YSOS_ERROR_SUCCESS，否则返回相应错误值  //NOLINT
   */
  virtual int DoSwitchDispatch(ResSwitchServiceParam& result);
  /**
   *@brief 响应注册服务
   *@param result[Input]： 处理结果结构化参数  // NOLINT
   *@return 若成功则返回YSOS_ERROR_SUCCESS，否则返回相应错误值  //NOLINT
   */
  virtual int DoRegServiceEventDispatch(ResRegServiceEventServiceParam& result);
  /**
   *@brief 响应反注册服务
   *@param result[Input]： 处理结果结构化参数  // NOLINT
   *@return 若成功则返回YSOS_ERROR_SUCCESS，否则返回相应错误值  //NOLINT
   */
  virtual int DoUnregServiceEventDispatch(ResUnregServiceEventServiceParam& result);
  /**
   *@brief 响应页面准备完毕服务
   *@param result[Input]： 处理结果结构化参数  // NOLINT
   *@return 若成功则返回YSOS_ERROR_SUCCESS，否则返回相应错误值  //NOLINT
   */
  virtual int DoReadyDispatch(ResReadyServiceParam& result);
  /**
   *@brief 响应IOCtl服务
   *@param result[Input]： 处理结果结构化参数  // NOLINT
   *@return 若成功则返回YSOS_ERROR_SUCCESS，否则返回相应错误值  //NOLINT
   */
  virtual int DoIOCtlDispatch(ResIOCtlServiceParam& result);
  /**
   *@brief 响应获取服务状态服务
   *@param result[Input]： 处理结果结构化参数  // NOLINT
   *@return 若成功则返回YSOS_ERROR_SUCCESS，否则返回相应错误值  //NOLINT
   */
  virtual int DoGetServiceStateDispatch(ResGetServiceStateServiceParam& result);
  /**
   *@brief 响应页面自定义事件服务
   *@param result[Input]： 处理结果结构化参数  // NOLINT
   *@return 若成功则返回YSOS_ERROR_SUCCESS，否则返回相应错误值  //NOLINT
   */
  virtual int DoCustomEventDispatch(ResCustomEventServiceParam& result);

 protected:
  /**
  *@brief 按具体服务类型细化的服务处理函数
  */

  // 服务处理函数，上层App发送请求，平台响应

  /**
   *@brief 处理心跳服务请求
   *@param service_param[Input]： 结构化服务请求参数  // NOLINT
   *@return 若成功则返回YSOS_ERROR_SUCCESS，否则返回相应错误值  //NOLINT
   */
  virtual int HandleHeartbeatService(const ReqHeartbeatServiceParam& service_param);
  /**
   *@brief 处理日志服务请求
   *@param service_param[Input]： 结构化服务请求参数  // NOLINT
   *@return 若成功则返回YSOS_ERROR_SUCCESS，否则返回相应错误值  //NOLINT
   */
  virtual int HandleWriteLogService(const ReqWriteLogServiceParam& service_param);
  /**
   *@brief 处理获取服务列表服务请求
   *@param service_param[Input]： 结构化服务请求参数  // NOLINT
   *@return 若成功则返回YSOS_ERROR_SUCCESS，否则返回相应错误值  //NOLINT
   */
  virtual int HandleGetServiceListService(const ReqGetServiceListServiceParam& service_param);
  /**
   *@brief 处理加载配置文件服务请求
   *@param service_param[Input]： 结构化服务请求参数  // NOLINT
   *@return 若成功则返回YSOS_ERROR_SUCCESS，否则返回相应错误值  //NOLINT
   */
  virtual int HandleLoadConfigService(const ReqLoadConfigServiceParam& service_param);
  /**
  *@brief 处理读取数据服务请求
  *@param service_param[Input]： 结构化服务请求参数  // NOLINT
  *@return 若成功则返回YSOS_ERROR_SUCCESS，否则返回相应错误值  //NOLINT
  */
  virtual int HandleGetDataService(const ReqGetDataServiceParam& service_param);
  /**
  *@brief 处理写入数据服务请求
  *@param service_param[Input]： 结构化服务请求参数  // NOLINT
  *@return 若成功则返回YSOS_ERROR_SUCCESS，否则返回相应错误值  //NOLINT
  */
  virtual int HandleSetDataService(const ReqSetDataServiceParam& service_param);

  // 处理上层App响应平台请求服务函数，平台发送请求，上层App响应

  /**
   *@brief 处理切换通知服务返回消息
   *@param result[Input]： 处理结果结构化参数  // NOLINT
   *@return 若成功则返回YSOS_ERROR_SUCCESS，否则返回相应错误值  //NOLINT
   */
  virtual int HandleSwitchNotifyDispatch(ResSwitchNotifyServiceParam& result);

  /**
   *@brief 处理事件通知服务返回消息
   *@param result[Input]： 处理结果结构化参数  // NOLINT
   *@return 若成功则返回YSOS_ERROR_SUCCESS，否则返回相应错误值  //NOLINT
   */
  virtual int HandleEventNotifyDispatch(ResEventNotifyServiceParam& result);

 protected:
  /**
  *@brief 注册应用给状态机  // NOLINT
  *@param app_name[Input]： 应用名  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int RegisterAppToStrategy(const std::string& app_name);
  /**
   *@brief 注册消息给状态机  // NOLINT
   *@param message_ptr[Input]： 消息  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int RegisterMessageToStrategy(MsgInterfacePtr message_ptr);
  /**
  *@brief 获取Strategy  //NOLINT
  *@param strategy_name[Input]： 状态机名  // NOLINT
  *@return： 若成功则返回strategy指针，否则返回NULL  //NOLINT
  */
  virtual StrategyInterfacePtr GetStrategy(const std::string &strategy_name);
  /**
   *@brief 生成消息  //NOLINT
   *@param message_id[Input]： 消息ID  // NOLINT
   *@param service_param[Input]： 服务参数  // NOLINT
   *@return： 若成功则返回MessageInterface指针，否则返回NULL  //NOLINT
   */
  virtual MsgInterfacePtr PackMessage(const UINT32 message_id, const std::string& service_param);
  /**
  *@brief 平台向上层APP发送服务请求  //NOLINT
  *@param service_name[Input]： 服务名  // NOLINT
  *@param service_param[Input]： 服务参数  // NOLINT
  *@return： 若成功则返回YSOS_ERROR_SUCCESS，否则返回相应错误值  //NOLINT
  */
  virtual int DoService(const std::string &service_name, const std::string &service_param);
  /**
   *@brief 平台向上层APP发送服务响应  //NOLINT
   *@param service_name[Input]： 服务名  // NOLINT
   *@param result[Input]： 处理结果参数  // NOLINT
   *@return： 若成功则返回YSOS_ERROR_SUCCESS，否则返回相应错误值  //NOLINT
   */
  virtual int OnDispatchMessage(const std::string &service_name, const std::string &result);

 protected:
  PlatformRPCPtr platform_rpc_ptr_;  ///< 通信指针 //  NOLINT
  const SessionInterface *session_ptr_;  ///< 匹配的Session指针 //  NOLINT
  StrategyInterfacePtr strategy_ptr_;  ///< 策略指针 //  NOLINT
  DataInterfacePtr data_ptr_;  ///< 数据指针 //  NOLINT
  std::string default_state_;  ///< 状态机默认状态名称 //  NOLINT

  DECLARE_SINGLETON_VARIABLE(BaseAgentImpl);
};
}
#define GetBaseAgentImpl ysos::BaseAgentImpl::Instance
#endif  //  SIP_BASE_AGENT_IMPL_H   //NOLINT
