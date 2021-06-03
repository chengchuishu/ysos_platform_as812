/**   //NOLINT
  *@file DisplayDriver.h
  *@brief Definition of DisplayDriver for windows.
  *@version 0.1
  *@author dhongqian
  *@date Created on: 2016/6/13   19:45
  *@copyright Copyright(c) 2016 YunShen Technology. All rights reserved.
  *
  */
#ifndef YSOS_PLUGIN_DISPLAY_DRIVER_H_  //NOLINT
#define YSOS_PLUGIN_DISPLAY_DRIVER_H_  //NOLINT

/// windows headers
//#include <afxwin.h>
#ifdef _WIN32
  #include <Windows.h>
  #include <winnt.h>
  #include <winternl.h>
#else

#endif
/// ysos headers
#include "../../../../../../core/trunk/main/public/include/core_help_package/baseinterfaceimpl.h"
#include "../../../../../../core/trunk/main/public/include/os_hal_package/driverinterface.h"
/// private headers
#include "../../../public/include/common/commonenumdefine.h"
#include "../../../public/include/common/commonstructdefine.h"

namespace ysos {

/**
  *@brief DisplayDriver for windows.
  */
class YSOS_EXPORT  DisplayDriver: public DriverInterface, public BaseInterfaceImpl {
  DISALLOW_COPY_AND_ASSIGN(DisplayDriver)
  DECLARE_PROTECT_CONSTRUCTOR(DisplayDriver)
  DECLARE_CREATEINSTANCE(DisplayDriver)

 public:
  virtual ~DisplayDriver();


    /**
  *@brief 在系统退出时，会依次调Module、Callback和Driver的uninitialize的,在Close后
   *@param 
   *@return   
   */
  virtual int RealUnInitialize(void *param=NULL);

  /**
   *@brief 从底层驱动中读取数据  // NOLINT
   *@param pBuffer[Output]： 读取的数据缓冲  // NOLINT
   *@param context_ptr[In|Out]： Driver上下文  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Read(BufferInterfacePtr pBuffer, DriverContexPtr context_ptr=NULL);

  /**
   *@brief 写数据到底层驱动中  // NOLINT
   *@param input_buffer[Input]： 写入的数据缓冲  // NOLINT
   *@param output_buffer[Output]： 读取的数据缓冲  // NOLINT
   *@param context_ptr[In|Out]： Driver上下文  // NOLINT
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
  virtual void Close(void *pParams = NULL);

  /**
   *@brief 设置接口的属性值  // NOLINT
   *@param type_id[Input]： 属性的ID  // NOLINT
   *@param type[input]： 属性值的值  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int SetProperty(int type_id, void *type);

protected:
  /**
   *@brief 通过name获得窗体句柄  // NOLINT
   *@param name 窗体的名称 // NOLINT
   *@return 无  // NOLINT
   */
  void GetHwndByName(const std::string name);
  /**
   *@brief 检查当前的句柄，是否有更换 // NOLINT
   *@param cur_hwnd 当前的窗体句柄 // NOLINT
   *@return 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT  // NOLINT
   */
  int CheckHDC(void *cur_hwnd);
 
 private:
  //* add for linux
   /// void        *hwnd_;  ///<  目标窗体的句柄 //  NOLINT
   //void        *pre_hwnd_;   ///< 上一个目标窗体
   //HDC          pre_hdc_;    ///< 上一个HDC
   //HDC          cur_hdc_;    ///< 当前的HDC
   int          left_;  ///<  目标区域的最左边 //  NOLINT
   int          top_;   ///<  目标区域的最顶端 //  NOLINT
   int          width_;  ///<  图片的宽度 //  NOLINT
   int          height_;  ///<  图片的高度 //  NOLINT
   std::string  title_;   ///<  窗体的Title //  NOLINT
   bool         is_mirror_mode_;  ///<  是否是镜面显示模式,即，是否使用水平相反显示，是，相反，否正常 //  NOLINT
  //*/
};

typedef boost::shared_ptr<DisplayDriver> DisplayDriverPtr;

}  ///< namespace ysos
#endif  /// <YSOS_PLUGIN_DISPLAY_DRIVER_H_   //NOLINT