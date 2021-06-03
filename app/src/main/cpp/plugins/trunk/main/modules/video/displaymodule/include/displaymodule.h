/**
 *@file DisplayModule.h
 *@brief display module  
 *@version 1.0
 *@author dhongqian
 *@date Created on: 2016-05-26 14:39:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

#ifndef YSOS_PLUGIN_DISPLAY_MODULE_H_  //NOLINT
#define YSOS_PLUGIN_DISPLAY_MODULE_H_  //NOLINT

/// ysos headers
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basemoduleimpl.h"         //NOLINT
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basemodulecallbackimpl.h" //NOLINT
#include "../../../../../../core/trunk/main/public/include/os_hal_package/driverinterface.h"

/// private headers
#include "../../../public/include/common/commonenumdefine.h"
#include "../../../public/include/common/commonstructdefine.h"

namespace ysos {
  /**
   *@brief  ASR Module callback // NOLINT
  */
class YSOS_EXPORT DisplayModule: public BaseModuleImpl
{
  DECLARE_PROTECT_CONSTRUCTOR(DisplayModule);
  DISALLOW_COPY_AND_ASSIGN(DisplayModule);
  DECLARE_CREATEINSTANCE(DisplayModule);

public:
  /**
   *@brief 获取接口的属性值  // NOLINT
   *@param type_id[Input]： 属性的ID  // NOLINT
   *@param type[Output]：属性值的值  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int GetProperty(int type_id, void *type);
  /**
   *@brief 设置接口的属性值  // NOLINT
   *@param type_id[Input]： 属性的ID  // NOLINT
   *@param type[input]： 属性值的值  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int SetProperty(int type_id, void *type);
  /**
  *@brief 配置函数，可对该接口进行详细配置  // NOLINT
  *@param ctrl_id[Input]： 详细配置ID号  // NOLINT
  *@param param[Input]：详细配置的参数  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int Ioctl(INT32 control_id, LPVOID param = NULL);

protected:
  /**
   *@brief  配置文件中设置的Key、Value对，会在Initialized函数中调用 //  NOLINT
   *@param key 配置参数中的Key  // NOLINT
   *@param value 与Key对应的Value  // NOLINT
   *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
   */
  virtual int Initialized(const std::string &key, const std::string &value);

  /**
   *@brief 类似于模块的初始化功能，子类实现，只关注业务
   *@param open需要的参数
   *@return 成功返回0，否则失败
  */
  int RealOpen(LPVOID param = NULL);
  /**
   *@brief 运行，子类实现，只关注业务  // NOLINT
   *@param ： 无  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  int RealRun();
  /**
   *@brief 暂停，子类实现，只关注业务 // NOLINT
   *@param ： 无  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  int RealPause();
  /**
  *@brief 停止运行，关闭模块，子类实现，只关注业务
  *       只有当ref_cout为0时，才能正常关闭
  *@return 成功返回0，否则失败
  */
  int RealStop();
  /**
   *@brief 关闭，，子类实现，只关注业务
   *@return 成功返回0，否则失败
   */
   int RealClose();
  /**
    *@brief  初始化ModuleDataInfo //  NOLINT
    *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
    */
  virtual int InitalDataInfo();
  /**
    *@brief  更新图片的宽度和高度 //  NOLINT
    *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
    */
  int SetImageWidthAndHeight(const std::string &image_width_height);

private:
  DriverInterfacePtr   driver_ptr_;  ///<  driver的指针 //  NOLINT
  int          left_;  ///<  目标区域的最左边 //  NOLINT
  int          top_;   ///<  目标区域的最顶端 //  NOLINT
  int          width_;  ///<  图片的宽度 //  NOLINT
  int          height_;  ///<  图片的高度 //  NOLINT
  std::string  title_;  ///<  目标窗体的Title //  NOLINT
  bool         is_mirror_mode_;  ///<  是否是镜面显示模式,即，是否使用水平相反显示，是，相反，否正常 //  NOLINT
  bool         is_capture_image_;   ///< 现在是否要抓拍图片
  bool         is_query_admin_;     // 是否检索管理员
};

} // namespace ysos

#endif    //YSOS_PLUGIN_DISPLAY_MODULE_H_  //NOLINT