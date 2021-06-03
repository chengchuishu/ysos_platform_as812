/**
*@file YsosDaemonConnnectionCalllback.h
*@brief Definition of YsosDaemonConnnectionCalllback
*@version 0.1
*@author dhongqian
*@date Created on: 2016-06-08 15:05:58
*@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
* 
*/

#ifndef SFP_YSOS_DAEMON_CONNECTION_CALLLBACK_H
#define SFP_YSOS_DAEMON_CONNECTION_CALLLBACK_H

/// C++ Standard Headers
#include <map>
/// Private Ysos Headers
#include "../../../protect/include/protocol_help_package/jsonrpc.h"
#include "../../../public/include/sys_interface_package/appmanagerinterface.h"
#include "../../../public/include/sys_interface_package/sessioninterface.h"
#include "../../../public/include/core_help_package/baseinterfaceimpl.h"
#include "../../../protect/include/protocol_help_package/baserpccallbackimpl.h"
#include "../../../public/include/sys_interface_package/strategyinterface.h"

namespace ysos {
/**
*@brief BaseAppManagerImpl的具体实现  // NOLINT
*/
class YSOS_EXPORT YsosDaemonConnectionCalllback : public BaseRPCCallbackImpl {
  DECLARE_CREATEINSTANCE(YsosDaemonConnectionCalllback);
  DISALLOW_COPY_AND_ASSIGN(YsosDaemonConnectionCalllback);
  DECLARE_PROTECT_CONSTRUCTOR(YsosDaemonConnectionCalllback);

 public:
  /**
   *@brief 析构函数  // NOLINT
   *@param ： 无  // NOLINT
   *@return： 无  // NOLINT
   */
  virtual ~YsosDaemonConnectionCalllback();
  /**
   *@brief 设置接口的属性值  // NOLINT
   *@param type_id[Input]： 属性的ID  // NOLINT
   *@param type[input]： 属性值的值  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int SetProperty(int type_id, void *type);

 public:
  /**
   *@brief 设置SDK的DoService回调函数
   *@param session_id 该连接的SessionID
   *@param service_name 服务名
   *@param service_param 服务的参数
   *@return 无
   */
  virtual void HandleDoService(const std::string& session_id, const std::string& service_name, const std::string& service_param);
  /**
   *@brief 设置SDK的OnDispatchMessage回调函数
   *@param session_id 该连接的SessionID
   *@param service_name 服务名
   *@param service_param 服务的参数
   *@return 无
   */
  virtual void HandleOnDispatchMessage(const std::string& session_id, const std::string& service_name, const std::string& result);
  /**
   *@brief 设置SDK的错误处理回调函数
   *@param session_id 该连接的SessionID
   *@param error_param 错误参数
   *@return 无
   */
  virtual void HandleError(const std::string& session_id, const std::string& error_param);

 protected:
  SessionInterface *session_ptr_;  ///< 所属的Session指针
};

typedef boost::shared_ptr<YsosDaemonConnectionCalllback>   YsosDaemonConnectionCalllbackPtr;

}  // end of namespace

#endif

