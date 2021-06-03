/**
 *@file IoInfoFactory.h
 *@brief Definition of IoInfoFactory
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-21 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef SFP_IO_INTERFACE_FACTORY_H_  // NOLINT
#define SFP_IO_INTERFACE_FACTORY_H_  // NOLINT

/// ysos private headers //  NOLINT
#include "../../../public/include/core_help_package/baseinterfaceimpl.h"
#include "../../../public/include/sys_interface_package/strategyinterface.h"
#include "../../../public/include/core_help_package/utility.h"

namespace ysos {
/*************************************************************************************************************
 *                      BaseIoInfoImpl                                                               *
 *************************************************************************************************************/
class YSOS_EXPORT IoInfoFactory: public BaseInterfaceImpl {
  DISALLOW_COPY_AND_ASSIGN(IoInfoFactory);
  DECLARE_PROTECT_CONSTRUCTOR(IoInfoFactory);

//  friend class Singleton<IoInfoFactory>;

 public:
//  BaseIoInfoImpl(const std::string &strClassName  ="BaseIoInfoImpl");
    virtual ~IoInfoFactory();

 public:
  /**
    *@brief  创建一个指定类型的IoInfoInterface对象   //  NOLINT
    *@param  object_type 指定的类型   //  NOLINT
    *@param  object_name 设定object的逻辑名   //  NOLINT
    *@return 成功返回ioinfointerface对象，失败返回NULL  //  NOLINT
    */
  virtual IoInfoInterfacePtr CreateObject(const std::string &object_type, const std::string &object_name);

  DECLARE_SINGLETON_VARIABLE(IoInfoFactory);
};
}
#define GetIoInfoFactory  ysos::IoInfoFactory::Instance
#endif  // SFP_IO_INTERFACE_FACTORY_H_  // NOLINT
