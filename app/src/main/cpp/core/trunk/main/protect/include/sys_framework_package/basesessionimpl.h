/**
*@file basesessionimpl.h
*@brief Definition of basesessionimpl
*@version 0.1
*@author dhongqian
*@date Created on: 2016-06-08 15:05:58
*@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
* 
*/

#ifndef SFP_BASE_SESSION_IMPL_H
#define SFP_BASE_SESSION_IMPL_H

/// Boost Headers
#include <boost/thread.hpp>
/// Private Ysos Headers
#include "../../../protect/include/sys_framework_package/platformrpc.h"
#include "../../../public/include/sys_interface_package/sessioninterface.h"
#include "../../../public/include/core_help_package/baseinterfaceimpl.h"
#include "../../../public/include/core_help_package/utility.h"

namespace ysos {
/**
*@brief BaseSessionImpl的具体实现  // NOLINT
*/
class AgentInterface;
class BaseSessionImpl;
class AppManagerInterface;
typedef boost::shared_ptr<BaseSessionImpl> BaseSessionImplPtr;

class BaseSessionImpl : public SessionInterface, public BaseInterfaceImpl {
  DECLARE_CREATEINSTANCE(BaseSessionImpl);
  DISALLOW_COPY_AND_ASSIGN(BaseSessionImpl);
  DECLARE_PROTECT_CONSTRUCTOR(BaseSessionImpl);

public:
  struct SessionAgentInfo {
    std::string  service_name;
    std::string  service_param;

    SessionAgentInfo(const std::string &name, const std::string &param): service_name(name), service_param(param){}
  };

protected:
  BaseSessionImpl(const std::string &session_id, const PlatformRPCPtr &platform_sdk_ptr, const std::string &str="BaseSessionImpl");
  /**
  *@brief 析构函数  // NOLINT
  *@param ： 无  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual ~BaseSessionImpl(void);

//  friend class Singleton<BaseSessionImpl>;

 public:

   enum SessionProperty {
     MANAGER,     ///< 设置所属的AppManager
     REDIRECT,    ///<  本地重定向 //  NOLINT
     DOSERVICE,  ///<  DoServiceCallback //  NOLINT
     DISPATCHSERVICE,  ///<  DispatchServiceCallback //  NOLINT
     ERROR,      ///<  platform sdk error //  NOLINT
     SessionPropertyEnd
   };

  /**
   *@brief 生成一个新的Session  // NOLINT
   *@param connection_ptr[Input]： 一个请求连接  // NOLINT
   *@return： 成功返回session指针，失败返回NULL  // NOLINT
   */
  virtual SessionInterface *Clone(const std::string& session_id, const PlatformRPCPtr& platform_sdk_ptr, void* param);

  /**
   *@brief 返回该Session的SessionId  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual std::string GetSessionId(void);

  /**
   *@brief 销毁一个指定的Session  // NOLINT
   *@param connection_ptr[Input]： 一个请求连接  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Destroy(SessionInterface *session_ptr);

  /**
   *@brief 接受某个应用的会话，并处理会话的初始化请求  // NOLINT
   *@param connection_ptr[Input]： 一个请求连接  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Accept(const std::string& session_id, const PlatformRPCPtr& platform_sdk_ptr, void* param);

  /**
   *@brief 检查保活信息  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int CheckAlive(void);

  /**
   *@brief 处理保活返回信息  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int HandleAliveResponse(const std::string &response="");

  /**
   *@brief 检查Agent的生命周期，如果生命周期结束，销毁Agent  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int CheckLifeCycle(void);

  /**
   *@brief 权限检查，App发送请求时，要先通过Checkjurisdiction，才能向Strategy发送请求  // NOLINT
   *@param service[Input]： 请求的服务  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int CheckJurisdiction(const std::string &service);

  virtual int GetProperty(int type_id, void *type);
  virtual int SetProperty(int type_id, void *type);
  virtual int Initialize(void *param=NULL);
  virtual int UnInitialize(void *param=NULL);

 protected:
  std::string  session_id_;    ///< 每个Session都有一个唯一的Session_Id
  AgentInterface   *agent_ptr_;   ///<  属于这个Session的Agent //  NOLINT
  PlatformRPCPtr    platform_rpc_ptr_; ///<  platform_sdk指针 //  NOLINT
  AppManagerInterface   *manager_ptr_;    ///<  所属的Manager //  NOLINT
  int                    disconnect_count_;  ///<  心跳失败的次数 //  NOLINT
  bool                   is_alive_;          ///<  Session是否还在生命周期内 //  NOLINT
  int                    alive_check_second_;  ///<  心跳检测的间隔，单位秒 //  NOLINT
  int                    max_disconnect_count_;  ///<  心跳断开的最大次数 //  NOLINT
  bool                   is_connecting_;         ///<  Session有两种状态：1 在连接中，心跳保持  2 心跳超过最大次数，连接断开，但仍存活，等待重边 //  NOLINT
  int                    wait_for_reconnect_count_;  ///<  等待重连的次数 //  NOLINT
  int                    max_reconnect_count_;  ///<  等待重连的次数 //  NOLINT
  boost::thread_group    check_alive_thread_;   ///<  检查状态的线程 //  NOLINT

  DECLARE_SINGLETON_VARIABLE(BaseSessionImpl);
};
}  // end of namespace
#define GetBaseSessionImpl  ysos::BaseSessionImpl::Instance
#endif
