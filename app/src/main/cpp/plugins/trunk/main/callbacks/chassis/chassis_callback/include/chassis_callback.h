/**
 *@file gaussian_chassis_callback.h
 *@brief Definition of gaussian chassis callback
 *@version 0.9.0.0
 *@author Lu Min, Wang Xiaogui
 *@date Created on: 2016-12-09 14:28:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef YSOS_PLUGIN_CALLBACK_CHASSIS_H_
#define YSOS_PLUGIN_CALLBACK_CHASSIS_H_

#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basemoduleimpl.h"                  //NOLINT
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basemodulecallbackimpl.h"          //NOLINT
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basethreadmodulecallbackimpl.h"    //NOLINT
#include "../../../../../../core/trunk/main/protect/include/os_hal_package/datainterfacemanager.h"

/// Public Headers
#include "../../../public/include/common/commonenumdefine.h"
#include "../../../public/include/common/commonstructdefine.h"

namespace ysos {
/**
 *@brief Gaussian Chassis Callback
*/
class YSOS_EXPORT ChassisCallback : public BaseThreadModuleCallbackImpl {
  DECLARE_CREATEINSTANCE(ChassisCallback);
  DISALLOW_COPY_AND_ASSIGN(ChassisCallback);
  DECLARE_PROTECT_CONSTRUCTOR(ChassisCallback);

public:
  ~ChassisCallback();

  /**
  *@brief 回调处理是否准备好
  *@param ： 无
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值
  */
  int IsReady();

 protected:
  /**
  *@brief  配置文件中设置的Key、Value对，会在Initialized函数中调用 //  NOLINT
  *@param key 配置参数中的Key  // NOLINT
  *@param value 与Key对应的Value  // NOLINT
  *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
  */
  virtual int Initialized(const std::string &key, const std::string &value);

  /**
  *@brief 回调接口的实现函数
  *@param input_buffer[Input]： 输入数据缓冲
  *@param output_buffer[Output]：回调处理后输出的数据缓冲
  *@param context[Input]： 回调处理的上下文数据
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值
  */
  virtual int RealCallback(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr cur_output_buffer_ptr,BufferInterfacePtr output_buffer_ptr, void *context);
  

private:
  void ObstacleMode();
  void LoactionInfo(const std::string &strbuffer,void *context);
  void NearestLaserInfo(const std::string &strbuffer,void *context);
  void MobileObstacleInfo(const std::string &strbuffer,void *context);
  void MoveMode(BufferInterfacePtr input_buffer,void *context);
  void DeviceStatusInfo(const std::string &strbuffer,void *context);

private:
  int icurent_cycle_index_;                 ///< 当前循环次数 
  BufferPoolInterfacePtr buffer_pool_ptr_;
  DataInterfacePtr data_ptr_;

  int obstacle_det_mode_;                   ///< 0:激光 1:激光+移动 2:移动
  double obstacle_det_angle_;
  double obstacle_det_angle_near_;
  double obstacle_det_near_dis_;
  double obstacle_dis_revise_;
  std::string str_laser_obstacle_input_;
  std::string str_mobile_obstacle_input_;
};

} ///< namespace ysos

#endif  ///< YSOS_PLUGIN_CALLBACK_CHASSIS_H_  //NOLINT
