/**
 *@file timetriggermodule.h
 *@brief time trigger module
 *@version 1.0
 *@author z.gg
 *@date Created on: 2020-9-1 9:10:00
 *@copyright Copyright (c) 2020 YS Technology. All rights reserved.
 *
 */

#ifndef TIME_TRIGGER_MODULE_H_
#define TIME_TRIGGER_MODULE_H_

#include <vector>
#include <boost/timer.hpp>

/// Private Headers //  NOLINT
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basemoduleimpl.h"
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basemodulecallbackimpl.h"
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/manager_declare.h"
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/packageconfigimpl.h"
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basethreadmoduleimpl.h"
#include "../../../../../../core/trunk/main/protect/include/os_hal_package/datainterfacemanager.h"
#include "../../../../../../core/trunk/main/public/include/os_hal_package/driverinterface.h"
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"

#include "../../../public/include/common/commonenumdefine.h"
#include "../../../public/include/common/commonstructdefine.h"

namespace ysos {

/**
 *@brief  定时器模块逻辑控制层 //NOLINT
*/
class YSOS_EXPORT TimeTriggerModule : public BaseThreadModuleImpl {
  DECLARE_CREATEINSTANCE(TimeTriggerModule);
  DISALLOW_COPY_AND_ASSIGN(TimeTriggerModule);
  DECLARE_PROTECT_CONSTRUCTOR(TimeTriggerModule);

 public:
  virtual ~TimeTriggerModule(void);
  /**
  *@brief 配置函数，可对该接口进行详细配置  // NOLINT
  *@param ctrl_id[Input]： 详细配置ID号  // NOLINT
  *@param param[Input]：详细配置的参数  // NOLINT
  *@return： 成功返回TDOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  int Ioctl(INT32 control_id, LPVOID param = nullptr);

  /**
   *@brief 冲洗，快速将内部缓冲中剩余数据处理完  // NOLINT
   *@param ： 无  // NOLINT
   *@return： 成功返回TDOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Flush(LPVOID param = nullptr);

  /**
   *@brief 基本初始化  // NOLINT
   *@param param： 初始化的参数  // NOLINT
   *@return： 成功返回TDOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Initialize(LPVOID param = nullptr);

  /**
   *@brief 清除资源,Initialize的逆操作  // NOLINT
   *@param param： 初始化的参数 // NOLINT
   *@return： 成功返回TDOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int UnInitialize(LPVOID param = nullptr);

  /**
   *@brief 获取接口的属性值  // NOLINT
   *@param type_id[Input]： 属性的ID  // NOLINT
   *@param type[Output]：属性值的值  // NOLINT
   *@return： 成功返回TDOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int GetProperty(int type_id, void *type);

 protected:
  /**
  *@brief 打开并初始化Module计算单元  // NOLINT
  *@param param[Input]： 输入的配置参数  // NOLINT
  *@return： 成功返回TDOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int RealOpen(LPVOID param = nullptr);
  /**
   *@brief 运行  // NOLINT
   *@param ： 无  // NOLINT
   *@return： 成功返回TDOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int RealRun(void);
  /**
  *@brief 暂停  // NOLINT
  *@param ： 无  // NOLINT
  *@return： 成功返回TDOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int RealPause(void);
  /**
  *@brief 停止  // NOLINT
  *@param ： 无  // NOLINT
  *@return： 成功返回TDOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int RealStop(void);
  /**
  *@brief 关闭该Module  // NOLINT
  *@param ： 无  // NOLINT
  *@return： 成功返回TDOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int RealClose(void);
  /**
  *@brief  初始化ModuleDataInfo //  NOLINT
  *@return 成功返回TDOS_ERROR_SUCCESS，否则失败  //  NOLINT
  */
  virtual int InitalDataInfo(void);

  /**
  *@brief  配置文件中设置的Key、Value对，会在Initialized函数中调用 //  NOLINT
  *@param key 配置参数中的Key  // NOLINT
  *@param value 与Key对应的Value  // NOLINT
  *@return 成功返回TDOS_ERROR_SUCCESS，否则失败  //  NOLINT
  */
  virtual int Initialized(const std::string &key, const std::string &value);

  /**
  *@brief  定点事件判断函数 //  NOLINT
  *@return 需要上抛事件为true，否则false  //  NOLINT
  */
  bool SpecialEvent(std::string cur_time);

  /**
  *@brief  普通时间事件判断函数 //  NOLINT
  *@return 需要上抛事件为true，否则false  //  NOLINT
  */
  bool NormalEvent(std::string cur_time);

 protected:
  DriverInterfacePtr    driver_prt_;    //< driver的指针 //  NOLINT

 private:
  UINT32 time_interval_;                    //< 时间间隔
  UINT32 event_interval_;                   //< 事件间隔
  std::string period_;                      //< 时间段
  std::string event_time_;                  //< 事件时间
  std::vector<TimePeriod> time_period_;     //< 时间段容器
  std::vector<std::string> event_data_;     //< 事件时间容器
  boost::timer tCount_;                     //< 定时器
  clock_t clockBegin_;                      //< 定时器
  clock_t clockEnd_;                        //< 定时器
  LightLock lock_;                          //< 互斥变量
  DataInterfacePtr data_ptr_;               //< 内存指针

};

}
#endif    ///TIME_TRIGGER_MODULE_H_    //NOLINT