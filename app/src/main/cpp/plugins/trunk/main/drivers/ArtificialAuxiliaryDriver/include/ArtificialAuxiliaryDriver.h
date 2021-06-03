/**
 *@file ArtificialAuxiliaryDriver.h
 *@brief Artificial Auxiliary interface
 *@version 1.0
 *@author l.j..
 *@date Created on: 2016-10-20 13:48:23
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

#ifndef ARTIFICIAL_AUXILIARY_INTERFACE_H_    //NOLINT
#define ARTIFICIAL_AUXILIARY_INTERFACE_H_    //NOLINT

#ifdef _WIN32
#include <Windows.h>
#else
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#endif
//#include <Winsock2.h>
#include <json/json.h>

//ysos Headers    //NOLINT
#include "../../../../../../core/trunk/main/public/include/core_help_package/baseinterfaceimpl.h"
#include "../../../../../../core/trunk/main/public/include/os_hal_package/driverinterface.h"
#include "../../../../../../core/trunk/main/public/include/sys_interface_package/common.h"
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"
#include "../../../../../../core/trunk/main/protect/include/os_hal_package/datainterfacemanager.h"
//com ctrl Headers    //NOLINT

#include "../../../public/include/common/commonenumdefine.h"
#include "../../../public/include/common/commonstructdefine.h"

// boost headers
#include <boost/scoped_ptr.hpp>
#include <boost/thread/thread.hpp>
#include <boost/atomic.hpp>

namespace ysos {

/**
 *@brief  C1C2通信模块驱动层 //NOLINT
*/
class YSOS_EXPORT ArtificialAuxiliaryDriver : virtual public DriverInterface,public BaseInterfaceImpl {
  DECLARE_CREATEINSTANCE(ArtificialAuxiliaryDriver);
  DECLARE_PROTECT_CONSTRUCTOR(ArtificialAuxiliaryDriver);

 public:

  virtual ~ArtificialAuxiliaryDriver(void);
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

 private:
  /**
  *@brief  连接Socket //  NOLINT
  *@param  无  // NOLINT
  *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
  */
  int Connect();

  /**
  *@brief  断开连接Socket //  NOLINT
  *@param  无  // NOLINT
  *@return 无  //  NOLINT
  */
  void DisConnect();

  /**
  *@brief  发送 //  NOLINT
  *@param pInputBuffer[Input]： 写入的数据缓冲  // NOLINT
  *@param pOutputBuffer[Output]： 读出的数据缓冲  // NOLINT
  *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
  */
  int IoctlSend(BufferInterfacePtr pInputBuffer, BufferInterfacePtr pOutputBuffer);

  /**
  *@brief  接收 //  NOLINT
  *@param pInputBuffer[Input]： 写入的数据缓冲  // NOLINT
  *@param pOutputBuffer[Output]： 读出的数据缓冲  // NOLINT
  *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
  */
  int IoctlRecv(BufferInterfacePtr pInputBuffer, BufferInterfacePtr pOutputBuffer);
  unsigned int ThreadProcessConnect();
 private:
  int sockclient_;              //< socket句柄
  /*bool*/boost::atomic<int> artificialauxiliary_open_;                   //< 是否打开串口
  // 线程
  boost::scoped_ptr<boost::thread> thread_ptr_;

};///< End class ArtificialAuxiliaryDriverInterface   //NOLINT

};

#endif  //ARTIFICIAL_AUXILIARY_INTERFACE_H_  //NOLINT