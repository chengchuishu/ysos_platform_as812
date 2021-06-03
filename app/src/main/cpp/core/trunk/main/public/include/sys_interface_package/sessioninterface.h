/**
 *@file sessioninterface.h
 *@brief Definition of session
 *@version 0.1
 *@author Steven.Shi
 *@date Created on: 2016-04-13 08:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef SESSION_INTERFACE_H  //NOLINT
#define SESSION_INTERFACE_H  //NOLINT

#include "../../../public/include/sys_interface_package/baseinterface.h"
#include "../../../public/include/sys_interface_package/connectioninterface.h"
#include <boost/shared_ptr.hpp>

namespace ysos {
class PlatformRPC;
typedef  boost::shared_ptr<PlatformRPC>   PlatformRPCPtr;
/**
  *@brief Session接口，实现上层应用与核心平台之间的通讯机制  //NOLINT
  */
class SessionInterface;
typedef boost::shared_ptr<SessionInterface> SessionInterfacePtr;

/**
 *@brief 会话接口，用于管理上层应用实例，
   角色
   维持同一应用的使用权限，及其相关资源，
   该权限将映射到场景中，与Strategy_Interface有映射关系
 */
class YSOS_EXPORT SessionInterface: virtual public BaseInterface {
  IMPLEMENT_PUBLIC_EMPTY_DESTRUCTOR(SessionInterface)

 public:

  /**
   *@brief 生成一个新的Session  // NOLINT
   *@param connection_ptr[Input]： 一个请求连接  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual SessionInterface *Clone(const std::string& session_id, const PlatformRPCPtr& platform_sdk_ptr, void* param) = 0;

  /**
   *@brief 返回该Session的SessionId  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual std::string GetSessionId(void) = 0;

  /**
   *@brief 销毁一个指定的Session  // NOLINT
   *@param connection_ptr[Input]： 一个请求连接  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Destroy(SessionInterface *session_ptr) = 0;

  /**
   *@brief 接受某个应用的会话，并处理会话的握手请求  // NOLINT
   *@param connection_ptr[Input]： 一个请求连接  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Accept(const std::string& session_id, const PlatformRPCPtr& platform_sdk_ptr, void* param) = 0;

  /**
   *@brief 检查保活信息  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int CheckAlive(void) = 0;

  /**
   *@brief 处理保活返回信息  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int HandleAliveResponse(const std::string &response="") = 0;

  /**
   *@brief 检查Agent的生命周期，如果生命周期结束，销毁Agent  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int CheckLifeCycle(void) = 0;

  /**
   *@brief 权限检查，App发送请求时，要先通过Checkjurisdiction，才能向Strategy发送请求  // NOLINT
   *@param service[Input]： 请求的服务  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int CheckJurisdiction(const std::string &service) = 0;
};

} // namespace ysos

#endif  // SESSION_INTERFACE_H  //NOLINT
