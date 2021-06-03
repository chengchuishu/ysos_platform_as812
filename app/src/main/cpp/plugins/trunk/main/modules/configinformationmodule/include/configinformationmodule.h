/**
 *@file configinformationmodule.h
 *@brief config information module interface
 *@version 1.0
 *@author XueXiaojun
 *@date Created on: 2017-07-28 09:18:00
 *@copyright Copyright (c) 2017 YunShen Technology. All rights reserved.
 * 
 */

#ifndef CONFIGINFORMATION_MODULE_H_
#define CONFIGINFORMATION_MODULE_H_

/// Private Headers //  NOLINT
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basemoduleimpl.h"          //NOLINT
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basemodulecallbackimpl.h"  //NOLINT
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basethreadmoduleimpl.h"    //NOLINT
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basedriverimpl.h"                 //NOLINT
#include "../../../../../../core/trunk/main/public/include/os_hal_package/driverinterface.h"

/// Boost Headers
#include <boost/shared_ptr.hpp>                 //NOLINT
#include <boost/shared_array.hpp>               //NOLINT
#include <boost/typeof/typeof.hpp>              //NOLINT
#include <boost/thread.hpp>                     //NOLINT

/// Ysos Headers //NOLINT
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/manager_declare.h"
#include "../../../../../../core/trunk/main/public/include/os_hal_package/bufferutility.h"
#include "../../../../../../core/trunk/main/public/include/sys_interface_package/ysos_struct.h"
#include <json/json.h>

/// Private Headers  //NOLINT
#include "../../../public/include/common/commonenumdefine.h"
#include "../../../public/include/common/commonstructdefine.h"
//#include <Windows.h>

///< 自定义宏 // NOLINT
#define CMD_CONFIG_MESSAGE_ONE 1001  ///< 首页资源  //NOLINT
#define CMD_CONFIG_MESSAGE_TWO 1002  ///< 程序资源  //NOLINT
#define CMD_CONFIG_READ 1003
#define CMD_CONFIG_WIRTER 1004
#define CMD_DOWN_FILE 1005
#define CMD_CHECK_PAGE_UPDATE 1006
#define CMD_CHECK_SOURCE_UPDATE 1007

namespace ysos {

/**
 *@brief  语音模块逻辑控制层 //NOLINT
*/
class YSOS_EXPORT ConfigInformationModule : public BaseThreadModuleImpl {
  DECLARE_CREATEINSTANCE(ConfigInformationModule);
  DISALLOW_COPY_AND_ASSIGN(ConfigInformationModule);
  DECLARE_PROTECT_CONSTRUCTOR(ConfigInformationModule);

 public:
  virtual ~ConfigInformationModule(void);
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

	/**
	*@brief 心跳线程
	*@return： 
	*/
	static int Check_Update_Thread(LPVOID lpParam);


private:
	DriverInterfacePtr    module_driver_ptr_;      ///< Module模块的Driver指针     //NOLINT
	//CallbackInterfacePtr  module_callback_ptr_;     ///< Module模块的Callback指针   //NOLINT

	std::string releate_driver_name_;               ///< 关联的驱动 逻辑名,目前只支持一个driver         // NOLINT
	std::string releate_callback_name_;             ///< 底盘Callback对象名称       //NOLINT
	boost::thread check_update_thread_;           ///< 检查更新线程句柄                //NOLINT

	BufferInterfacePtr    config_input_buffer_ptr_;  ///< 分配的buffer，心跳操作     //NOLINT
	BufferInterfacePtr    config_output_buffer_ptr_; ///< 分配的buffer，心跳操作     //NOLINT
};

}
#endif    ///CONFIGINFORMATION_MODULE_H_    // NOLINT