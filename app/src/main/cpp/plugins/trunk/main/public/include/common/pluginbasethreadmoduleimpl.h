/**   //NOLINT
  *@file PluginBaseThreadModuleImpl.h
  *@brief Definition of plugin 下module的一般通用实现，主要实现功能
  *1. 读取配置文件，默认加载一个驱动 （Initialized 函数，RealOpen 函数）
  *2. 实现 PROP_FUN_CALLABILITY 功能，把参数从 module自动传给driver (GetProperty 函数）
  *3. 读取配置文件，callback传入的数据的数据类型的转换，在传到driver前生效（Initialized 函数，GetProperty 函数，NeedConvertDataTypeInfo 函数）
  *@version 0.1
  *@author wanfenfen
  *@date Created on: 2016:12:14   10:53
  *@copyright Copyright(c) 2016 YunShen Technology. All rights reserved.
  * 
  *@howto Usage:
  *  继承自 PluginBaseThreadModuleImpl 的类，在重写 PluginBaseThreadModuleImpl类public函数之前 必须调用基类 PluginBaseThreadModuleImpl的方法。
  *@todo
  */

#ifndef MODULE_PLUGINBASEMODULEIMPL_H_  //NOLINT
#define MODULE_PLUGINBASEMODULEIMPL_H_  //NOLINT

/// stl headers
#include <list>

/// ysos headers
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basemoduleimpl.h"         //NOLINT
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basemodulecallbackimpl.h" //NOLINT
#include "../../../../../../core/trunk/main/public/include/os_hal_package/driverinterface.h"
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basethreadmoduleimpl.h"

/// private headers
#include "commonenumdefine.h"
#include "commonstructdefine.h"


namespace ysos {
/**
 *@brief  ASR Module callback // NOLINT
*/
class YSOS_EXPORT PluginBaseThreadModuleImpl: public /*BaseModuleImpl*/BaseThreadModuleImpl {
  DECLARE_PROTECT_CONSTRUCTOR(PluginBaseThreadModuleImpl);
  DISALLOW_COPY_AND_ASSIGN(PluginBaseThreadModuleImpl);
  DECLARE_CREATEINSTANCE(PluginBaseThreadModuleImpl);

 public:
  virtual ~PluginBaseThreadModuleImpl();

 protected:
  virtual int Initialized(const std::string &key, const std::string &value);

  /**
   *@brief 类似于模块的初始化功能，子类实现，只关注业务
   *@param open需要的参数
   *@return 成功返回0，否则失败
  */
  int RealOpen(LPVOID param = NULL);

  //todo
  int RealRun();
  int RealPause();


  /**
  *@brief stop时会 调用driver 关闭 list_output_abilities_中标记的能力
  *@return 成功返回0，否则失败
  */
  int RealStop();

  //tdoo
  int RealClose();
  int Ioctl(INT32 control_id, LPVOID param);


  /**
  *@brief inherit from BaseInterface
  *@param iTypeId[Input] property id
  *@param piType[Input] property value
  *@return success return YSOS_ERROR_SUCCESS,
  *        fail return (YSOS_ERROR_INVALID_ARGUMENTS)
  */
  virtual int GetProperty(int iTypeId, void *piType);

  // 由于此函数 InitalDataInfo 在BaseModuleImpl中为纯虚函数，所以这里默认实现返回空
  int InitalDataInfo();

  /// 以下是区别于 PluginBaseModuleImpl 的函数
  //int Initialize(void *param);

 protected:
  /**
   *@brief 是否需要进行数据类型转换
   *@param source_input_datatype[Input] 输入数据类型
   *@param source_output_datatypa[Input] 输出数据类型
   *@param convertInfo[Input] 转换说明
   *@return true if need to  convert data type
   *        false if not need to convert data type
   */
  bool NeedConvertDataTypeInfo(const std::string source_input_datatype, const std::string source_output_datatypa,
                               ModuleDataTypeConvertInfo& convert_info);

  DriverInterfacePtr driver_ptr_;

  std::string releate_driver_name_;   ///< 关联的驱动 逻辑名,目前只支持一个driver
  std::list<ModuleDataTypeConvertInfo> list_module_datatype_convert_info_;

  std::string string_output_abilities_;  ///< 输出能力 用|分开
  std::list<std::string> list_output_abilities_;  ///< 输出能力列表

  std::string driver_param_open_;
  std::string driver_param_close_;
  std::string driver_param_run_;
  std::string driver_param_pause_;
  std::string driver_param_stop_;
};

} // namespace ysos


#endif