/**
 *@file configure_information_driverinterface.h
 *@brief Definition of configure information driver
 *@version 0.9.0.0
 *@author Xue xiaojun
 *@date Created on: 2017-7-27 14:13:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef YSOS_PLUGIN_CONFIG_INFORMATION_DRIVER_IMPL_H
#define YSOS_PLUGIN_CONFIG_INFORMATION_DRIVER_IMPL_H

#include "../../../../../../core/trunk/main/public/include/sys_interface_package/common.h"
#include "../../../../../../core/trunk/main/public/include/sys_interface_package/bufferpool.h"
#include "../../../../../../core/trunk/main/public/include/core_help_package/baseinterfaceimpl.h"
#include "../../../../../../core/trunk/main/public/include/os_hal_package/driverinterface.h"
#include "../../../../../../core/trunk/main/public/include/sys_interface_package/connectioninterface.h"

/// Boost Headers
#include <boost/shared_ptr.hpp>                 //NOLINT
#include <boost/shared_array.hpp>               //NOLINT
#include <boost/property_tree/ptree.hpp>        //NOLINT
#include <boost/property_tree/json_parser.hpp>  //NOLINT
#include <boost/typeof/typeof.hpp>              //NOLINT
#include <boost/thread.hpp>                     //NOLINT


#include "../../../public/include/common/commonenumdefine.h"
#include "../../../public/include/common/commonstructdefine.h"

///< MD5 Header  // NOLINT
#include "../include/MD5.h"

#ifdef _WIN32
#include <windows.h>
#else
#endif

///< 自定义宏 // NOLINT
#define CMD_CONFIG_MESSAGE_ONE 1001  ///< 首页资源  //NOLINT
#define CMD_CONFIG_MESSAGE_TWO 1002  ///< 程序资源  //NOLINT
#define CMD_CONFIG_READ 1003
#define CMD_CONFIG_WIRTER 1004
#define CMD_DOWN_FILE 1005
#define CMD_CHECK_PAGE_UPDATE 1006
#define CMD_CHECK_SOURCE_UPDATE 1007

namespace  ysos {

/**
*@brief 驱动接口，用于管理底层驱动封装实例，
*/
class YSOS_EXPORT ConfigureInformationDriver : public DriverInterface, public BaseInterfaceImpl{
  DISALLOW_COPY_AND_ASSIGN(ConfigureInformationDriver)
  DECLARE_PROTECT_CONSTRUCTOR(ConfigureInformationDriver)
  DECLARE_CREATEINSTANCE(ConfigureInformationDriver)

 public:
  virtual ~ConfigureInformationDriver();

    /**
   *@brief 设置接口的属性值  // NOLINT
   *@param type_id[Input]： 属性的ID  // NOLINT
   *@param type[input]： 属性值的值  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int SetProperty(int type_id, void *type);

  /**
   *@brief 从底层驱动中读取数据  // NOLINT
   *@param pBuffer[Output]： 读取的数据缓冲  // NOLINT
   *@param context_ptr[In|Out]： Driver上下文  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Read(BufferInterfacePtr pBuffer, DriverContexPtr context_ptr = NULL);

  /**
   *@brief 写数据到底层驱动中  // NOLINT
   *@param input_buffer[Input]： 写入的数据缓冲  // NOLINT
   *@param output_buffer[Output]： 读取的数据缓冲  // NOLINT
   *@param context_ptr[In|Out]： Driver上下文  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Write(BufferInterfacePtr input_buffer, BufferInterfacePtr output_buffer = NULL, DriverContexPtr context_ptr = NULL);

  /**
   *@brief 控制驱动状态/配置参数到驱动/读取状态等  // NOLINT
   *@param iCtrlID[Input]： ID  // NOLINT
   *@param pInputBuffer[Input]： 写入的数据缓冲  // NOLINT
   *@param pOutputBuffer[Output]： 读出的数据缓冲  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Ioctl(int iCtrlID, BufferInterfacePtr pInputBuffer, BufferInterfacePtr pOutputBuffer);

  /**
   *@brief 打开底层驱动，并传入配置参数  // NOLINT
   *@param pParams[Input]： 驱动所需的打开参数  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Open(void *pParams = NULL);

  /**
   *@brief 打开底层驱动，并传入配置参数  // NOLINT
   *@param ： 无  // NOLINT
   *@return： 无  // NOLINT
   */
  virtual void Close(void *pParams = NULL);

  virtual int GetProperty(int type_id, void *pi_type_ptr);

private:

	/**
	*@brief 分析后台服务器返回的首页数据  // NOLINT
	*@param ： input_buffer[Input]： 后台服务器返回的首页数据 // NOLINT
	*@return： 无  // NOLINT
	*/
	int AnalyzePageData(BufferInterfacePtr input_buffer_ptr);

  	/**
	*@brief 分析后台服务器返回的资源数据  // NOLINT
	*@param ： input_buffer[Input]： 后台服务器返回的资源数据 // NOLINT
	*@return： 无  // NOLINT
	*/
	int AnalyzeSourceData(BufferInterfacePtr input_buffer_ptr);

	/**
	*@brief 获取本地文件的MD5码  // NOLINT
	*@param ： cul_file_path[Input]： 本地文件的路径 // NOLINT
	*@return： md5码  // NOLINT
	*/
	std::string GetMD5(const char* cul_file_path);

protected:
	ConnectionInterfacePtr connection_down_ptr;  ///< Driver模块的down指针     //NOLINT
	ConnectionInterfacePtr connection_http_ptr;  ///< Driver模块的http指针     //NOLINT

	DriverInterfacePtr seer_ptr;  ///< Driver模块的http指针     //NOLINT

	std::string connection_down_name_;               ///< 关联的驱动 逻辑名,目前只支持一个driver         // NOLINT
	std::string connection_http_name_;             ///< 底盘Callback对象名称       //NOLINT

	MD5 md5_ptr_;

private:
	bool is_update_page_;
	bool is_update_source_;

};

typedef boost::shared_ptr<ConfigureInformationDriver> ConfigureInformationDriverImplPtr;

} ///< namespace ysos

#endif  ///< YSOS_PLUGIN_DRIVER_GAUSSIAN_CHASSIS_IMPL_H_  //NOLINT