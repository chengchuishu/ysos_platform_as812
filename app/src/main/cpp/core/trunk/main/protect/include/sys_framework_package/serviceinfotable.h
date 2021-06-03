/**
*@file serviceinfotable.h
*@brief Definition of ServiceInfoTable
*@version 0.1
*@author jinchengzhe
*@date Created on: 2016-06-21 15:14:58
*@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
* 
*/

#ifndef SFP_SERVICE_INFO_TABLE_H_
#define SFP_SERVICE_INFO_TABLE_H_

/// Stl Headers

#include "../../../public/include/core_help_package/utility.h"
#include "../../../public/include/core_help_package/baseinterfaceimpl.h"

#include <list>
#include <boost/shared_ptr.hpp>
/// Ysos Headers
#include "../../../public/include/sys_interface_package/common.h"

namespace ysos {

class LightLock;

class ServiceInfoTable;
typedef boost::shared_ptr<ServiceInfoTable> ServiceInfoTablePtr;

class YSOS_EXPORT ServiceInfoTable {
 public:
  struct ServiceConfInfo {
    UINT32 ID;

    //Attribute
    std::string name;
    std::string type;
    std::string callback;
    std::string is_default;

    //Element
    std::string description;
    std::string instance_name;
    std::list<boost::shared_ptr<ServiceConfInfo>> sub_services;

    ServiceConfInfo() : name(""), type(""), callback(""), is_default(""), description("") {}
  };

  typedef boost::shared_ptr<ServiceConfInfo> ServiceConfInfoPtr;
  typedef std::list<ServiceConfInfoPtr> ServiceConfInfoList;
  typedef std::list<ServiceConfInfoPtr>::iterator ServiceConfInfoIterator;

  ~ServiceInfoTable();

  static ServiceInfoTablePtr Instance();

  const std::string GetServiceNameByID(const UINT32 service_ID);
  const UINT32 GetServiceIDByName(const std::string service_name);
  const ServiceConfInfoPtr GetServiceByName(const std::string service_name);
  const ServiceConfInfoList& GetServiceConfInfoList() const;
  int AddServiceConfInfo(const ServiceConfInfoPtr service_conf_info_ptr);
  int DeleteServiceConfInfo(const ServiceConfInfoPtr service_conf_info_ptr);
  void Reset();

 private:
  ServiceInfoTable();
  ServiceInfoTable& operator=(const ServiceInfoTable&);
  ServiceInfoTable(const ServiceInfoTable&);

  static ServiceInfoTablePtr service_info_table_ptr_;
  static boost::shared_ptr<LightLock> light_lock_ptr_;
  ServiceConfInfoList service_conf_info_list_;
protected:
  log4cplus::Logger logger_;
};

}  //end of namespace

#endif
