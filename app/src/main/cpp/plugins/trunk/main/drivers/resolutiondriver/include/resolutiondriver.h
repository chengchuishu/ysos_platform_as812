/**
 *@file resolutiondriver.h
 *@brief resolution interface
 *@version 1.0
 *@author l.j..
 *@date Created on: 2016-10-20 13:48:23
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

#ifndef RESOLUTION_INTERFACE_H_    //NOLINT
#define RESOLUTION_INTERFACE_H_    //NOLINT

#ifdef _WIN32
#include <Windows.h>
#include   <shlobj.h>   
#pragma   comment(lib,   "shell32.lib")
#else
#endif

#include <json/json.h>

//ysos Headers    //NOLINT
#include "../../../../../../core/trunk/main/protect/include/os_hal_package/datainterfacemanager.h"
#include "../../../../../../core/trunk/main/public/include/core_help_package/baseinterfaceimpl.h"
#include "../../../../../../core/trunk/main/public/include/os_hal_package/driverinterface.h"
#include "../../../../../../core/trunk/main/public/include/sys_interface_package/common.h"
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"
//com ctrl Headers    //NOLINT

#include "../../../public/include/common/commonenumdefine.h"
#include "../../../public/include/common/commonstructdefine.h"

///tiny Header
#include <tinyxml2/tinyxml2.h>

namespace ysos {

/**
 *@brief  分辨率模块驱动层 //NOLINT
*/
class YSOS_EXPORT ResolutionDriver : virtual public DriverInterface,public BaseInterfaceImpl {
  DECLARE_CREATEINSTANCE(ResolutionDriver);
  DECLARE_PROTECT_CONSTRUCTOR(ResolutionDriver);

 public:

	 enum DEL_CACHE_TYPE
	 {
		 File,
		 Cookie 
	 };

  virtual ~ResolutionDriver(void);
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
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Read(BufferInterfacePtr pBuffer, DriverContexPtr context_ptr=NULL);

  /**
   *@brief 写数据到底层驱动中  // NOLINT
   *@param pBuffer[Input]： 写入的数据缓冲  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Write(BufferInterfacePtr input_buffer, BufferInterfacePtr output_buffer=NULL, DriverContexPtr context_ptr=NULL);

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
  virtual int Open(void *pParams);

  /**
   *@brief 打开底层驱动，并传入配置参数  // NOLINT
   *@param ： 无  // NOLINT
   *@return： 无  // NOLINT
   */
  virtual void Close(void *pParams = nullptr);

	/**
	*@brief 修改分辨率并写入文件  // NOLINT
	*@param width[Input]： 横向像素  // NOLINT
	*@param height[Input]： 纵向像素  // NOLINT
	*@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值   // NOLINT
	*/
	int ScreenResolutionModify(UINT32 width, UINT32 height);

	bool DeleteUrlCache(DEL_CACHE_TYPE type);

	bool EmptyDirectory(std::string szPath, bool bDeleteDesktopIni = false,   bool bWipeIndexDat = false);
	
	bool WipeFile(std::string szDir, std::string szFile);

 private:
  int width_;                               //< 屏幕横向像素
  int height_;                              //< 屏幕纵向像素
	std::string xml_path_;                    //< 分辨率xml文件路径

  DataInterfacePtr data_ptr_;               //< 内存指针
	tinyxml2::XMLDocument *my_document_;      //< 解析xml文件指针

};///< End class ResolutionDriverInterface   //NOLINT

};

#endif  //RESOLUTION_INTERFACE_H_