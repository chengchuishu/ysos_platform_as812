/**
 *@file appmanager.h
 *@brief Definition of appmanager
 *@version 0.1
 *@author Steven.Shi
 *@date Created on: 2016-05-17 08:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef SIP_APP_MANAGER_INTERFACE_H       //NOLINT
#define SIP_APP_MANAGER_INTERFACE_H       //NOLINT

#include "../../../public/include/sys_interface_package/baseinterface.h"
#include "../../../public/include/sys_interface_package/sessioninterface.h"
#include <boost/shared_ptr.hpp>

namespace ysos {

/**
  *@brief 代理接口，实现上层应用与核心平台之间的通讯机制  //NOLINT
  */
class YSOS_EXPORT AppManagerInterface {
  IMPLEMENT_PUBLIC_EMPTY_DESTRUCTOR(AppManagerInterface)

 public:

  struct YSOS_EXPORT AppManagerParam {
    std::string address;   ///<  ipv4的地址  //  NOLINT
    short       port;      ///<  ipv4的地址 //  NOLINT
    std::string session_type;   ///<  session的类型 //  NOLINT
  };
  typedef boost::shared_ptr<AppManagerParam>   AppManagerParamPtr;

  /**
   *@brief  启动AppManager：创建Server和对应的SessionImpl //NOLINT
   *@param app_param_ptr appmanager的Start参数
   *@param conf_dir 配置文件目录
   *@return： 若成功则返回YSOS_ERROR_SUCCESS，否则返回相应错误值  //NOLINT
   */
  virtual int Start(const AppManagerParamPtr &app_param_ptr, const std::string &conf_dir="") = 0;

  /**
   *@brief  添加一个Session //NOLINT
   *@param session_ptr 待添加的Session
   *@return： 若成功则返回YSOS_ERROR_SUCCESS，否则返回相应错误值  //NOLINT
   */
  virtual int AddSession(SessionInterface *session_ptr) = 0;

  /**
   *@brief  通过SessionId查找Session //NOLINT
   *@param session_id 待查找的Session的ID
   *@return： 若成功则返回session指针，否则返回NULL  //NOLINT
   */
  virtual SessionInterface *FindSession(const std::string &session_id) = 0;

  /**
   *@brief  删除指定的Session //NOLINT
   *@param session_id 待删除的Session的SessionId
   *@return： 若成功则返回YSOS_ERROR_SUCCESS，否则返回相应错误值  //NOLINT
   */
  virtual int RemoveSession(const std::string & session_id) = 0;

  /**
   *@brief 关闭AppManager  //NOLINT
   *@return： 若成功则返回YSOS_ERROR_SUCCESS，否则返回相应错误值  //NOLINT
   */
  virtual int Stop(void) = 0;
};

typedef boost::shared_ptr<AppManagerInterface> AppManagerInterfacePtr;

} // namespace ysos

#endif  //  SIP_AGENT_INTERFACE_H   //NOLINT
