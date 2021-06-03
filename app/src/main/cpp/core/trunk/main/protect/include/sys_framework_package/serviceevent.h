/**
 *@file ServiceEvent.h
 *@brief ServiceEvent id map
 *@version 1.0
 *@author dhongqian
 *@date Created on: 2016-05-11 10:06:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef SFP_SERVICE_EVENT_H_
#define SFP_SERVICE_EVENT_H_

/// Stl Headers
#include <map>
/// Boost Headers
#include <boost/shared_ptr.hpp>
/// Ysos headers
#include "../../../public/include/sys_interface_package/common.h"
#include "../../../public/include/core_help_package/baseinterfaceimpl.h"
#include "../../../public/include/core_help_package/utility.h"

namespace ysos {
/**
  *@brief  解析 event_service_map.xml，用户可以通过服务名，获取对应的事件名  //  NOLINT
  */
class YSOS_EXPORT ServiceEvent: public BaseInterfaceImpl {
  DISALLOW_COPY_AND_ASSIGN(ServiceEvent);
  ServiceEvent(const std::string& message_type="ServiceEvent");
  
 // friend class Singleton<ServiceEvent>;

 public:
  ~ServiceEvent();

  /**
    *@brief  解析Xml文件 //  NOLINT
    *@param  file_name  待解析的Xml的文件名 //  NOLINT
    *@return 成功返回0，失败返回其余值  //  NOLINT
    */
  int ParseServiceEvent(const std::string &file_name);
  /**
    *@brief  通过Service的Name直接获取event的Name //  NOLINT
    *@param  service_name  要获取的事件对应的服务名称 //  NOLINT
    *@return 成功返回事件的名称，失败返回空  //  NOLINT
    */
  std::string GetEventName(const std::string &service_name);

 private:
  std::map<std::string, std::string>     service_event_map_;

  DECLARE_SINGLETON_VARIABLE(ServiceEvent);
};
}
#define GetServiceEvent  ysos::ServiceEvent::Instance
#endif  ///< SFP_SERVICE_EVENT_H_
