/**
*@file BaseAppManagerImpl.h
*@brief Definition of BaseAppManagerImpl
*@version 0.1
*@author dhongqian
*@date Created on: 2016-06-08 15:05:58
*@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
* 
*/

#ifndef SFP_BASE_APP_MANAGER_IMPL_H
#define SFP_BASE_APP_MANAGER_IMPL_H

/// C++ Standard Headers
#include <list>
#include <map>
#include <algorithm>
/// 3rd Party Headers
#include <tinyxml2/tinyxml2.h>
/// Private Ysos Headers
#include "../../../protect/include/sys_framework_package/platformrpc.h"
#include "../../../public/include/sys_interface_package/appmanagerinterface.h"
#include "../../../public/include/sys_interface_package/sessioninterface.h"
#include "../../../public/include/core_help_package/baseinterfaceimpl.h"
#include "../../../protect/include/protocol_help_package/baserpccallbackimpl.h"

namespace ysos {
/**
*@brief BaseAppManagerImpl的具体实现  // NOLINT
*/
class BaseAppManagerImpl;
typedef boost::shared_ptr<BaseAppManagerImpl> BaseAppManagerImplPtr;

class YSOS_EXPORT BaseAppManagerImpl : public AppManagerInterface, public BaseRPCCallbackImpl {
  DECLARE_CREATEINSTANCE(BaseAppManagerImpl);
  DISALLOW_COPY_AND_ASSIGN(BaseAppManagerImpl);
  DECLARE_PROTECT_CONSTRUCTOR(BaseAppManagerImpl);

 public:

  struct ClientInfo {
    std::string name;                   ///< client的名称
    std::string strategy_name;          ///< client对应的Strategy名称
    std::string strategy_callback;      /// client对应的Strategy对应的callback
    std::string ip_address;             ///< client要连接Server的IP地址
    uint16_t    ip_port;                ///< client要连接Server的Port地址
  };
  typedef boost::shared_ptr<ClientInfo> ClientInfoPtr;

  /// Session的创建与删除，由AppManger严格控制，可以不使用智能指针    //  NOLINT
  typedef std::map<std::string, SessionInterface*>      SessionMapType;
  typedef std::list<ClientInfoPtr>                      ClientInfoList;
  typedef std::list<PlatformRPCPtr>                     ClientPlatformRPCList;

  /**
  *@brief 析构函数  // NOLINT
  *@param ： 无  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual ~BaseAppManagerImpl();

  /**
   *@brief  启动AppManager：创建Server和对应的SessionImpl //NOLINT
   *@param app_param_ptr appmanager的Start参数
   *@param conf_dir 配置文件目录
   *@return： 若成功则返回YSOS_ERROR_SUCCESS，否则返回相应错误值  //NOLINT
   */
  virtual int Start(const AppManagerParamPtr &app_param_ptr, const std::string &conf_dir="");

  /**
   *@brief  添加一个Session //NOLINT
   *@param session_ptr 待添加的Session
   *@return： 若成功则返回YSOS_ERROR_SUCCESS，否则返回相应错误值  //NOLINT
   */
  virtual int AddSession(SessionInterface *session_ptr);

  /**
   *@brief  通过SessionId查找Session //NOLINT
   *@param session_id 待查找的Session的ID
   *@return： 若成功则返回session指针，否则返回NULL  //NOLINT
   */
  virtual SessionInterface *FindSession(const std::string &session_id);

  /**
   *@brief  删除指定的Session //NOLINT
   *@param session_id 待删除的Session的SessionId
   *@return： 若成功则返回YSOS_ERROR_SUCCESS，否则返回相应错误值  //NOLINT
   */
  virtual int RemoveSession(const std::string &session_id);

  /**
   *@brief 关闭AppManager  //NOLINT
   *@return： 若成功则返回YSOS_ERROR_SUCCESS，否则返回相应错误值  //NOLINT
   */
  virtual int Stop(void);

 public:
  /// Handle Function
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
  /**
   *@brief 打开AppManagerServer，开始监听  //NOLINT
   *@return： 若成功则返回YSOS_ERROR_SUCCESS，否则返回相应错误值  //NOLINT
   */
  virtual int StartServer(const AppManagerParamPtr &app_param_ptr);
  /**
   *@brief 打开AppManagerClient  //NOLINT
   *@return： 无  //NOLINT
   */
  virtual void StartClient(void);

  /**
   *@brief 关闭AppManager  //NOLINT
   *@return： 若成功则返回YSOS_ERROR_SUCCESS，否则返回相应错误值  //NOLINT
   */
  virtual int StopServer(void);

  /**
   *@brief 关闭AppManager  //NOLINT
   *@param conf_dir 配置文件目录
   *@return： 若成功则返回YSOS_ERROR_SUCCESS，否则返回相应错误值  //NOLINT
   */
  virtual int ParseConfigFile(const std::string &conf_dir);

  /**
   *@brief 获取Client配置文件的根结点  //NOLINT
   *@param doc Xml的文件数据结构
   *@param file_name 待解析的文件
   *@param next_element 下一个结点的名称
   *@return： 若成功则返回YSOS_ERROR_SUCCESS，否则返回相应错误值  //NOLINT
   */
  tinyxml2::XMLElement *GetRootElement(tinyxml2::XMLDocument &doc, const std::string &file_name, const std::string &next_element="");

  /**
   *@brief 关闭AppManager  //NOLINT
   *@param client_file client配置文件
   *@return： 若成功则返回ClientInfo指针，失败返回NULLL  //NOLINT
   */
  virtual ClientInfoPtr ParseClientInfo(const std::string &client_file);

  /**
   *@brief 建立了一个连接，用于服务器端
   *@param session_id 该连接的SessionID
   *@param platform_sdk_ptr sdk端指针
   *@return 成功返回0，失败返回其他值
   */
  virtual int HandleConnected(const std::string& session_id, const PlatformRPCPtr& platform_sdk_ptr, const std::string& service_param);

 protected:
  SessionMapType             session_map_;
  PlatformRPCPtr             platform_rpc_;
  ClientInfoList             client_info_list_;
  ClientPlatformRPCList      client_platform_rpc_list_;
};

}  // end of namespace

#endif

