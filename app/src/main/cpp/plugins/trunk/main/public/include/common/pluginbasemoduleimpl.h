/**
 *@file pluginbasemoduleimpl.h
 *@brief Definition of plugin 下module的一般通用实现，主要实现功能
 *1. 读取配置文件，默认加载一个驱动 （Initialized 函数，RealOpen 函数）
 *2. 实现 PROP_FUN_CALLABILITY 功能，把参数从 module自动传给driver (GetProperty 函数）
 *3. 读取配置文件，callback传入的数据的数据类型的转换，在传到driver前生效（Initialized()/GetProperty()/NeedConvertDataTypeInfo()）
 *@version 2.0
 *@author libaoguo
 *@date Created on: 2017-08-25
 *@copyright Copyright(c) 2017 YunShen Technology. All rights reserved.
 *@how to Usage:
  *  继承自 PluginBaseModuleImpl 的类，在重写 PluginBaseModuleImpl类public函数之前 必须调用基类 PluginBaseModuleImpl的方法。
 *@todo
**/

#ifndef MODULE_PLUGIN_BASE_MODULE_IMPL_H_
#define MODULE_PLUGIN_BASE_MODULE_IMPL_H_

// ysos headers
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basemoduleimpl.h"
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basemodulecallbackimpl.h"
#include "../../../../../../core/trunk/main/public/include/os_hal_package/driverinterface.h"

// private headers
#include "commonenumdefine.h"
#include "commonstructdefine.h"


namespace ysos {
class YSOS_EXPORT PluginBaseModuleImpl: public BaseModuleImpl {
  DECLARE_PROTECT_CONSTRUCTOR(PluginBaseModuleImpl);
  DISALLOW_COPY_AND_ASSIGN(PluginBaseModuleImpl);
 public:
  virtual ~PluginBaseModuleImpl();

  virtual int Ioctl(INT32 control_id, LPVOID param);

  /**
  *@brief 额外处理属性：PROP_FUN_CALLABILITY
  *@param iTypeId[Input] property id
  *@param piType[Input] property value
  *@return success return YSOS_ERROR_SUCCESS,
  *        fail return (YSOS_ERROR_INVALID_ARGUMENTS)
  **/
  virtual int GetProperty(int iTypeId, void *piType);

 protected:
  /**
   *@brief 处理的key有：
   * relate_driver  关联的驱动 driver的逻辑名, 目前只支持一个driver
   * datatype_convert_info 内部数据格式转换(轻量级，只替换一下数据格式说明，数据的具体格式不处理, 
   * 以方便module到driver之间数据的传输格式: callback输入格式｜输出格式｜转换后输入的格式|转换后的输出格式
   * output_ability 输出能力,如果有多个能力，则使用 ｜ 分开
   *@param
   *@return
  **/
  virtual int Initialized(const std::string &key, const std::string &value);

  /**
  *@brief 在系统退出时，会依次调Module、Callback和Driver的uninitialize的,在Close后
   *@param
   *@return
  **/
  virtual int RealUnInitialize(void *param=NULL);

  /**
   *@brief open 前会关联相关的driver
   *@param open需要的参数
   *@return 成功返回0，否则失败
  **/
  int RealOpen(LPVOID param = NULL);

  //todo
  int RealRun();
  int RealPause();
  int RealStop();
  int RealClose();

  // 由于此函数 InitalDataInfo 在BaseModuleImpl中为纯虚函数，所以这里默认实现返回空
  int InitalDataInfo();

 protected:
  /**
   *@brief 是否需要进行数据类型转换
   *@param source_input_datatype[Input] 输入数据类型
   *@param source_output_datatypa[Input] 输出数据类型
   *@param convertInfo[Input] 转换说明
   *@return true if need to  convert data type
   *        false if not need to convert data type
  **/
  bool NeedConvertDataTypeInfo(const std::string source_input_datatype, const std::string source_output_datatypa,
                               ModuleDataTypeConvertInfo& convert_info);

 protected:
  DriverInterfacePtr driver_ptr_;
  std::string releate_driver_name_;
  std::list<ModuleDataTypeConvertInfo> list_module_datatype_convert_info_;
  std::string string_output_abilities_;
  std::list<std::string> list_output_abilities_;
  std::string driver_param_open_;
  std::string driver_param_close_;
  std::string driver_param_run_;
  std::string driver_param_pause_;
  std::string driver_param_stop_;
};

} // namespace ysos

#endif // MODULE_PLUGIN_BASE_MODULE_IMPL_H_