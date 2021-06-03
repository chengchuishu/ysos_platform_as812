/**
 *@file agentinterface.h
 *@brief Definition of agent
 *@version 0.1
 *@author Steven.Shi
 *@date Created on: 2016-05-17 08:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef SIP_AGENT_INTERFACE_H       //NOLINT
#define SIP_AGENT_INTERFACE_H       //NOLINT

#include "../../../public/include/sys_interface_package/baseinterface.h"
#include "../../../public/include/sys_interface_package/connectioninterface.h"
#include "../../../public/include/sys_interface_package/sessioninterface.h"
#include <boost/shared_ptr.hpp>

namespace ysos {
class PlatformRPC;
typedef boost::shared_ptr<PlatformRPC> PlatformRPCPtr;
/**
  *@brief 代理接口，实现上层应用与核心平台之间的通讯机制  //NOLINT
  */
class YSOS_EXPORT AgentInterface: virtual public BaseInterface {
  IMPLEMENT_PUBLIC_EMPTY_DESTRUCTOR(AgentInterface)
  /**
   *@brief 设置SDK的DoService回调函数
   *@param session_id 该连接的SessionID
   *@param service_name 服务名
   *@param service_param 服务的参数
   *@return 无
   */
  virtual void HandleDoService(const std::string& service_name, const std::string& service_param) = 0;
  /**
   *@brief 设置SDK的OnDispatchMessage回调函数
   *@param session_id 该连接的SessionID
   *@param service_name 服务名
   *@param service_param 服务的参数
   *@return 无
   */
  virtual void HandleOnDispatchMessage(const std::string& service_name, const std::string& result) = 0;
  /**
   *@brief 设置SDK的错误处理回调函数
   *@param session_id 该连接的SessionID
   *@param error_param 错误参数
   *@return 无
   */
  virtual void HandleError(const std::string& error_param) = 0;
};

typedef boost::shared_ptr<AgentInterface> AgentInterfacePtr;

} // namespace ysos

#endif  //  SIP_AGENT_INTERFACE_H   //NOLINT
