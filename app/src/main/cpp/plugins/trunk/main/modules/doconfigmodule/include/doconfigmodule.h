/**
 *@file doconfigmodule.h
 *@brief doconfig module interface
 *@version 1.0
 *@author l.j..
 *@date Created on: 2016-10-20 14:50:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

#ifndef DO_CONFIG_MODULE_H_
#define DO_CONFIG_MODULE_H_

/// Private Headers //  NOLINT
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basemoduleimpl.h"
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basemodulecallbackimpl.h"
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/manager_declare.h"
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basethreadmoduleimpl.h"
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/packageconfigimpl.h"
#include "../../../../../../core/trunk/main/protect/include/os_hal_package/datainterfacemanager.h"
#include "../../../../../../core/trunk/main/public/include/os_hal_package/driverinterface.h"
#include "../../../../../../core/trunk/main/public/include/os_hal_package/lock.h"
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"

#include "../../../public/include/common/commonenumdefine.h"
#include "../../../public/include/common/commonstructdefine.h"

/// Boost Headers
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ini_parser.hpp>

struct  LOG{            //< 日志相关信息
  std::string log_name_;
  std::string log_level_;
} *log_info;

namespace ysos {

/**
 *@brief  配置模块逻辑控制层 //NOLINT
*/
class YSOS_EXPORT DoconfigModule : public BaseThreadModuleImpl {
  DECLARE_CREATEINSTANCE(DoconfigModule);
  DISALLOW_COPY_AND_ASSIGN(DoconfigModule);
  DECLARE_PROTECT_CONSTRUCTOR(DoconfigModule);

 public:
  virtual ~DoconfigModule(void);
  /**
  *@brief 配置函数，可对该接口进行详细配置  // NOLINT
  *@param ctrl_id[Input]： 详细配置ID号  // NOLINT
  *@param param[Input]：详细配置的参数  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  int Ioctl(INT32 control_id, LPVOID param = nullptr);

  /**
   *@brief 冲洗，快速将内部缓冲中剩余数据处理完  // NOLINT
   *@param ： 无  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Flush(LPVOID param = nullptr);

  /**
   *@brief 基本初始化  // NOLINT
   *@param param： 初始化的参数  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Initialize(LPVOID param = nullptr);

  /**
   *@brief 清除资源,Initialize的逆操作  // NOLINT
   *@param param： 初始化的参数 // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int UnInitialize(LPVOID param = nullptr);

  /**
   *@brief 获取接口的属性值  // NOLINT
   *@param type_id[Input]： 属性的ID  // NOLINT
   *@param type[Output]：属性值的值  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int GetProperty(int type_id, void *type);

 protected:
  /**
  *@brief 打开并初始化Module计算单元  // NOLINT
  *@param param[Input]： 输入的配置参数  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int RealOpen(LPVOID param = nullptr);
  /**
   *@brief 运行  // NOLINT
   *@param ： 无  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int RealRun(void);
  /**
  *@brief 暂停  // NOLINT
  *@param ： 无  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int RealPause(void);
  /**
  *@brief 停止  // NOLINT
  *@param ： 无  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int RealStop(void);
  /**
  *@brief 关闭该Module  // NOLINT
  *@param ： 无  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int RealClose(void);
  /**
  *@brief  初始化ModuleDataInfo //  NOLINT
  *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
  */
  virtual int InitalDataInfo(void);
  /**
  *@brief  配置文件中设置的Key、Value对，会在Initialized函数中调用 //  NOLINT
  *@param key 配置参数中的Key  // NOLINT
  *@param value 与Key对应的Value  // NOLINT
  *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
  */
  virtual int Initialized(const std::string &key, const std::string &value);

 protected:
  DriverInterfacePtr    driver_prt_;    //<  driver的指针 //  NOLINT

  UINT32 exe_cmd_;						          //< 执行命令
  LightLock lock_;                      //< 互斥变量

 private:
  std::string xml_path_;                //< xml文件路径
};

}
#endif    ///DO_CONFIG_MODULE_H_    //NOLINT