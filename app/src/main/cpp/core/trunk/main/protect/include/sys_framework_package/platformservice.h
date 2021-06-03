/**
 *@file ServiceEvent.h
 *@brief ServiceEvent id map
 *@version 1.0
 *@author dhongqian
 *@date Created on: 2016-05-11 10:06:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef SFP_PLATFORM_SERVICE_H_
#define SFP_PLATFORM_SERVICE_H_

/// std headers //  NOLINT
#include <map>
/// boost headers  //  NOLINT
#include <boost/shared_ptr.hpp>
/// ysos headers //  NOLINT
#include "../../../public/include/sys_interface_package/common.h"
#include "../../../public/include/core_help_package/baseinterfaceimpl.h"
#include "../../../public/include/core_help_package/utility.h"
#include "../../../protect/include/core_help_package/propertytree.h"

namespace ysos {
class PropertyTree;
/**
  *@brief  解析service.xml，用户可以通过服务名或服务别名，获取对应的服务对象  //  NOLINT
  */
class YSOS_EXPORT PlatformService: public BaseInterfaceImpl {
  DISALLOW_COPY_AND_ASSIGN(PlatformService);
  DECLARE_PROTECT_CONSTRUCTOR(PlatformService);

//  friend class Singleton<PlatformService>;

 public:
  virtual ~PlatformService();

  struct ServiceInfo {
    typedef std::list<std::string> ServiceInfoList;
    std::string  name;  ///<  service name, unique  //  NOLINT
    std::string  alias;  ///< alias name, unique    //  NOLINT
    std::string  type;  ///<  module、callback、select、composit //  NOLINT
    std::string  description;  ///<  description of this service //  NOLINT
    std::string  default_type;  ///<  only used by select type //  NOLINT     //need update for linux  warn:default
    std::list<std::string>  service_list;  ///<  used by select and composit //  NOLINT
  };
  typedef boost::shared_ptr<ServiceInfo>        ServiceInfoPtr;
  typedef std::map<std::string, ServiceInfoPtr >   ServiceInfoMap;

  /**
    *@brief  解析Xml文件 //  NOLINT
    *@param  file_name  待解析的Xml的文件名 //  NOLINT
    *@return 成功返回0，失败返回其余值  //  NOLINT
    */
  int ParseService(const std::string &file_name);
  /**
    *@brief  通过Service的Name直接获取Service对象指针 //  NOLINT
    *@param  service_name  要获取的事件对应的服务名称 //  NOLINT
    *@return 成功返回服务的指针，失败返回NULL  //  NOLINT
    */
  ServiceInfoPtr GetService(const std::string &service_name);

  /**
    *@brief  通过Service的Name直接获取Service的RealName //  NOLINT
    *@param  service_name  要获取的事件对应的服务名称 //  NOLINT
    *@return 成功返回服务的真实名称，失败返回""  //  NOLINT
    */
  std::string GetServiceRealName(const std::string &service_name);

  ServiceInfoMap& GetServiceMap(void);
  ServiceInfoMap& GetAliasServiceMap(void);

  void Dump(void);

 protected:
  /**
   *@brief  解析Xml文件 //  NOLINT
   *@param  file_name  待解析的Xml的文件名 //  NOLINT
   *@return 成功返回0，失败返回其余值  //  NOLINT
   */
  int ParseService(TreeNodeIterator service_ptr);
  /**
   *@brief  解析Xml文件 //  NOLINT
   *@param  file_name  待解析的Xml的文件名 //  NOLINT
   *@return 成功返回0，失败返回其余值  //  NOLINT
   */
  int ParseIncludeService(TreeNodeIterator service_ptr);
  /**
   *@brief  获取Service的子服务 //  NOLINT
   *@param  service_info_ptr  服务的结构对象 //  NOLINT
   *@param  service_ptr  服务的XML文件结点 //  NOLINT
   *@return 成功返回0，失败返回其他值  //  NOLINT
   */
  int GetChildService(ServiceInfoPtr service_info_ptr, TreeNodeIterator service_ptr);

 private:
  ServiceInfoMap     service_map_;     ///<  name,service_info //  NOLINT
  ServiceInfoMap     alias_service_map_;     ///<  alias,service_info //  NOLINT

  DECLARE_SINGLETON_VARIABLE(PlatformService);
};
}
#define GetPlatformService  ysos::PlatformService::Instance
#endif  ///< SFP_SERVICE_EVENT_H_
