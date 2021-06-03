/**
 *@file driverinterface.h
 *@brief Definition of driver
 *@version 0.1
 *@author Steven.Shi
 *@date Created on: 2016-04-13 08:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef DRIVER_INTERFACE_H  //NOLINT
#define DRIVER_INTERFACE_H  //NOLINT

#include "../../../public/include/sys_interface_package/common.h"
#include "../../../public/include/sys_interface_package/bufferpool.h"
#include "../../../public/include/sys_interface_package/baseinterface.h"
#include <boost/shared_ptr.hpp>

namespace ysos {

class BaseInterface;
class ModuleInterface;
/**
 *@brief Driver上下文
 */
struct DriverContex {
  ModuleInterface *module_ptr;  ///<  Driver所属的Module //  NOLINT
  int              flag;       ///< Module传递的标识 //  NOLINT
  void            *context_ptr;  ///<  Driver上下文 //  NOLINT

  DriverContex() {
    module_ptr = NULL;
    context_ptr = NULL;
    flag = 0;
  }
};
typedef boost::shared_ptr<DriverContex> DriverContexPtr;

  /**
   *@brief 驱动接口，用于管理底层驱动封装实例，
   */
class YSOS_EXPORT DriverInterface : virtual public BaseInterface {
  IMPLEMENT_PUBLIC_EMPTY_DESTRUCTOR(DriverInterface)
 public:
  enum Properties {
    Information = 9000,  ///< Driver Information
    OnRead,   ///< On Read
    OnWrite,  ///< On Write
    Protocol,  ///< The protocol needed by this driver
  };

  /**
   *@brief 从底层驱动中读取数据  // NOLINT
   *@param pBuffer[Output]： 读取的数据缓冲  // NOLINT
   *@param context_ptr[In|Out]： Driver上下文  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Read(BufferInterfacePtr pBuffer, DriverContexPtr context_ptr=NULL) = 0;

  /**
   *@brief 写数据到底层驱动中  // NOLINT
   *@param input_buffer[Input]： 写入的数据缓冲  // NOLINT
   *@param output_buffer[Output]： 读取的数据缓冲  // NOLINT
   *@param context_ptr[In|Out]： Driver上下文  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Write(BufferInterfacePtr input_buffer, BufferInterfacePtr output_buffer=NULL, DriverContexPtr context_ptr=NULL) = 0;

  /**
   *@brief 控制驱动状态/配置参数到驱动/读取状态等  // NOLINT
   *@param iCtrlID[Input]： ID  // NOLINT
   *@param pInputBuffer[Input]： 写入的数据缓冲  // NOLINT
   *@param pOutputBuffer[Output]： 读出的数据缓冲  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Ioctl(int iCtrlID, BufferInterfacePtr pInputBuffer, BufferInterfacePtr pOutputBuffer) = 0;

  /**
   *@brief 打开底层驱动，并传入配置参数  // NOLINT
   *@param pParams[Input]： 驱动所需的打开参数  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Open(void *pParams) = 0;

  /**
   *@brief 打开底层驱动，并传入配置参数  // NOLINT
   *@param ： 无  // NOLINT
   *@return： 无  // NOLINT
   */
  virtual void Close(void *pParams = NULL) = 0;
};

typedef boost::shared_ptr<DriverInterface> DriverInterfacePtr;

} // namespace ysos

#endif  // DRIVER_INTERFACE_H  //NOLINT
