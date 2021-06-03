/**
 *@file baseinterface.h
 *@brief Definition of baseinterface
*@version 0.1
 *@author Steven.Shi
*@date Created on: 2016-04-21 13:59:20
*@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
*/

#ifndef BASE_INTERFACE_IMPL_H  //NOLINT
#define BASE_INTERFACE_IMPL_H  //NOLINT

#if 1   //  disable warning 4250 and 4996
#include "../../../public/include/os_hal_package/config.h"  // NOLINT
#if (CURRENT_PLATFORM == PLATFORM_WINDOWS)
#pragma warning(disable: 4250)      //  dominance
#pragma warning(disable: 4251)      //  dominance
#pragma warning(disable: 4996)      //  std::_Fill_n
#endif
#endif

/// Stl Headers
#include <string>
#include <sstream>
/// Boost Headers
#include <boost/uuid/uuid.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
/// 3rdParty Headers
#include <log4cplus/logger.h>
/// Ysos Headers
#include "../../../public/include/sys_interface_package/common.h"
#include "../../../public/include/sys_interface_package/baseinterface.h"
#include "../../../public/include/sys_framework_package/templatefactory.h"
#include "../../../public/include/sys_framework_package/templatemanager.h"

namespace ysos {

typedef boost::uuids::uuid uuid;

/**
 *@brief BaseInterfaceImpl是对BaseInterface的一个基本实现，
     如果没有特殊的需求，接口的实现子类可直接继承此实现
 */
class YSOS_EXPORT BaseInterfaceImpl : virtual public BaseInterface {
  DECLARE_CREATEINSTANCE(BaseInterfaceImpl);
  DECLARE_PROTECT_CONSTRUCTOR(BaseInterfaceImpl);

 public:
  virtual ~BaseInterfaceImpl(void);
  virtual int Initialize(void *param=NULL);
  virtual int UnInitialize(void *param=NULL);
  virtual bool IsInitialized(void);
  virtual bool IsInitSucceeded(void);
  virtual std::string GetName(bool is_full_name = false) const;
  virtual uuid GetUUID() const;
  virtual int GetProperty(int type_id, void *type);
  virtual int SetProperty(int type_id, void *type);

 protected:
  virtual int RealInitialize(void *param=NULL);
  virtual int RealUnInitialize(void *param=NULL);

 public:
  static std::string GetThisClassName();
  static void SetClassName(const std::string &class_name);

 protected:
  static std::string class_name_;
  static uuid class_uuid_;
  std::string instance_name_;
  std::string logic_name_;
  uuid instance_uuid_;
  bool is_initialize_;
  bool is_init_succeeded_;
  log4cplus::Logger logger_;
};

} // namespace ysos

#endif // BASE_INTERFACE_IMPL_H  //NOLINT
