/**
 *@file PlatformInfo.h
 *@brief all Platform Info
 *@version 1.0
 *@author dhongqian
 *@date Created on: 2016-05-11 10:06:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef SFP_PLATFORM_INFO_H_
#define SFP_PLATFORM_INFO_H_

/// Private Ysos Headers
#include "../../../public/include/sys_interface_package/common.h"
#include "../../../public/include/core_help_package/baseinterfaceimpl.h"
#include "../../../protect/include/sys_framework_package/serviceinfotable.h"
/// stl Headers
#include <map>
/// boost Headers
#include <boost/shared_ptr.hpp>

namespace ysos {
class LightLock;
class PlatformInfo;
typedef boost::shared_ptr<PlatformInfo> PlatformInfoPtr;
/**
 *@brief  平台的公用信息 // NOLINT
*/
class YSOS_EXPORT PlatformInfo: public BaseInterfaceImpl {
  DISALLOW_COPY_AND_ASSIGN(PlatformInfo);

 public:
  PlatformInfo(const std::string& platform="PlatformInfo");
  ~PlatformInfo();

  /**
    *@brief  通过Message的Name直接获取message id 值 //  NOLINT
    *@param  message_name  要获取事件ID的事件名称 //  NOLINT
    *@return 成功返回非0值，失败返回0  //  NOLINT
    */
  uint32_t  GetMessageIdByName(const std::string &message_name);
  /**
    *@brief  通过Message的ID直接获取message Name 值 //  NOLINT
    *@param  message_id  要获取事件名称的ID值 //  NOLINT
    *@return 成功返回事件名，失败返回空  //  NOLINT
    */
  std::string  GetMessageNameById(const uint32_t &message_id);
  /**
    *@brief  通过Service的Name直接获取event id 值 //  NOLINT
    *@param  service_name  要获取的事件对应的服务名称 //  NOLINT
    *@return 成功返回非0值，失败返回0  //  NOLINT
    */
  uint32_t GetMessageIdByServiceName(const std::string &service_name);
  /**
    *@brief  通过Service的Name直接获取Service的ID值 //  NOLINT
    *@param  service_name  要获取的ID的名称 //  NOLINT
    *@return 成功返回ID，失败返回0  //  NOLINT
    */
  uint32_t  GetServiceIdByName(const std::string &service_name);
  /**
    *@brief  通过服务的ID直接获取服务的名称 //  NOLINT
    *@param  service_id  要获取服务的ID值 //  NOLINT
    *@return 成功返回服务名，失败返回空  //  NOLINT
    */
  std::string  GetServiceNameById(const uint32_t &service_id);
  /**
    *@brief  通过服务名获取服务的结构体 //  NOLINT
    *@param  service_name  要获取的服务名 //  NOLINT
    *@return 成功返回服务结构体对象，失败返回NULL  //  NOLINT
    */
  ServiceInfoTable::ServiceConfInfoPtr  GetServiceByName(const std::string &service_name);
  /**
    *@brief  获取平台的配置目录 //  NOLINT
    *@return 成功返回目录名，失败返回空  //  NOLINT
    */
  const std::string  GetConfPath(void) const;

  static PlatformInfoPtr Instance();

 private:
  static PlatformInfoPtr platform_info_ptr_;
  static boost::shared_ptr<LightLock> light_lock_ptr_;
};
#define GetPlatformInfo PlatformInfo::Instance
}
#endif
