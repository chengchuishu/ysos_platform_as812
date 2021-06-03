/** //NOLINT
  *@file powermanagerdriver.h
  *@brief Definition of PowerManagerDriver for windows.
  *@version 0.1
  *@author wangxg
  *@date Created on: 2016/11/22   15:55
  *@copyright Copyright(c) 2016 YunShen Technology. All rights reserved.
  *
*/

#ifndef YSOS_PLUGIN_DRIVER_POWERMANAGER_H_  //NOLINT
#define YSOS_PLUGIN_DRIVER_POWERMANAGER_H_  //NOLINT

#ifdef _WIN32
/// OS Headers
  #include <Windows.h>
  #include <assert.h>
#else
  #include <assert.h>
#endif




/// Boost Headers
#include <boost/shared_ptr.hpp>     //NOLINT
#include <boost/thread/thread.hpp>  //NOLINT
#include <boost/shared_array.hpp>   //NOLINT
#include <boost/thread/mutex.hpp>   //NOLINT
#include <boost/atomic.hpp>

/// Ysos Headers
#include "../../../../../../core/trunk/main/public/include/sys_interface_package/common.h"			    //NOLINT
#include "../../../../../../core/trunk/main/public/include/sys_interface_package/bufferpool.h"		  //NOLINT
#include "../../../../../../core/trunk/main/public/include/core_help_package/baseinterfaceimpl.h"	//NOLINT
#include "../../../../../../core/trunk/main/public/include/os_hal_package/driverinterface.h"       //NOLINT
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basedriverimpl.h"       //NOLINT
#include "../../../../../../core/trunk/main/protect/include/os_hal_package/datainterfacemanager.h"

/// Public Headers
//#include "../../../public/include/CnComm.h"


namespace ysos {

/**
*@brief Power Manager Driver
*/
class YSOS_EXPORT  PowerManagerDriver: public BaseDriverImpl {
  DISALLOW_COPY_AND_ASSIGN(PowerManagerDriver)
  DECLARE_PROTECT_CONSTRUCTOR(PowerManagerDriver)
  DECLARE_CREATEINSTANCE(PowerManagerDriver)

 public:
  virtual ~PowerManagerDriver();

 public:
  /**
  *@brief inherit from BaseInterface
  *@param param
  *@return success return YSOS_ERROR_SUCCESS,
  *        fail return one of(YSOS_ERROR_LOGIC_ERROR)
  */
  virtual int Initialize(void *param=NULL);

  /**
  *@brief inherit from BaseInterface
  *@param iTypeId[Input] property id
  *@param piType[Output] property value
  *@return success return YSOS_ERROR_SUCCESS,
  *        fail return one of (YSOS_ERROR_INVALID_ARGUMENTS)
  */
  virtual int GetProperty(int itypeid, void *itypeptr);

  /**
  *@brief inherit from BaseInterface
  *@param iTypeId[Input] property id
  *@param piType[Input] property value
  *@return success return YSOS_ERROR_SUCCESS,
  *        fail return (YSOS_ERROR_INVALID_ARGUMENTS)
  */
  virtual int SetProperty(int itypeid, void *itypeptr);

  /**
  *@brief inherit from DriverInterface
  *@      only open device
  *@param pParams[Input] Serial  Port,default use 9600, n, 8, 1 config
  *@return success return YSOS_ERROR_SUCCESS,
  *        fail return YSOS_ERROR_FAILED
  */
  virtual int Open(void *paramsptr = NULL);

  /**
  *@brief inherit from DriverInterface
  */
  virtual void Close(void *paramsptr = NULL);

  /**
  *@brief inherit from DriverInterface;
  *@param pBuffer[Input/Output]
  *@return success return YSOS_ERROR_SUCCESS，
  *        fail return one of the error types
  */
  virtual int Read(BufferInterfacePtr bufferptr, DriverContexPtr contextptr=NULL);

  /**
  *@brief inherit from DriverInterface;
  *@param pBuffer[Input/Output]
  *@return success return YSOS_ERROR_SUCCESS
  *        fail return one of the error types
  */
  virtual int Write(BufferInterfacePtr inputbuffer, BufferInterfacePtr outputbuffer=NULL, DriverContexPtr contextptr=NULL);

  /**
  *@brief inherit from DriverInterface ,setting input or output buffer
  *@param iCtrlID[Input] 1 only write; 2 only read; 3 read and write
  *@param pInputBuffer[Input/Output]
  *@param pOutputBuffer[Input/Output]
  *@return success return YSOS_ERROR_SUCCESS
  *        fail return one of the error types
  */
  virtual int Ioctl(int ictrlid, BufferInterfacePtr inputbufferptr, BufferInterfacePtr outputbufferptr);

 private:
  /**
  *@brief
  *@open device
  *@param dwPort[Input]
  *@param dwBaudRate[Input]
  *@param btParity[Input]
  *@param btUINT8Size[Input]
  *@param btStopBits[Input]
  *@return success return YSOS_ERROR_SUCCESS,
  *        fail return YSOS_ERROR_FAILED
  */
  virtual int Open(UINT32 iport, UINT32 ibaudrate, UINT8 btparity = 0, UINT8 btbytesize = 8, UINT8 btstopbits = 0);

  /**
  *@brief get serial read timeout
  *@param bufferptr[Input]
  *@param ibuffersize[Input] pBuffer Size
  *@return Send Buffer Length
  */
  virtual int SetReadTimeOut(UINT32 itimeout);


  /**
  *@brief get serial read timeout
  *@param bufferptr[Input]
  *@param ibuffersize[Input] pBuffer Size
  *@return Send Buffer Length
  */
  virtual int GetReadTimeOut(UINT32 *itimeout_ptr);

  /**
  *@brief 判断输入命令是否有要转义字符  // NOLINT
  *@param data[Input]： 要发送的命令  // NOLINT
  *@param len[Input]： 命令的长度  // NOLINT
  *@param new_data[Output]： 转义后的命令  // NOLINT
  *@param new_len[Output]： 转义后命令的长度  // NOLINT
  *@return： 无  // NOLINT
  */
  void	JudgeData(UINT8 *data, UINT32 len, UINT8 *new_data, UINT32 *new_len);

  void ReBootDown();

 private:
  //CnComm   comport_pm_;           ///< Power Manager Serial Class

  UINT32    comport_port_;        ///< Port
  UINT32    comport_baudrate_;    ///< dwBaudRate
  UINT32    comport_parity_;      ///< btParity
  UINT32    comport_bytesize_;    ///< btByteSize
  UINT32    comport_stopbits_;    ///< btStopBits
  UINT32    comport_read_timeout_;///< Read TimeOut

  DataInterfacePtr data_ptr_;
};

typedef boost::shared_ptr<PowerManagerDriver> PowerManagerDriverPtr;

}  ///< End namespace ysos

#endif  ///< YSOS_PLUGIN_DRIVER_POWERMANAGER_H_   //NOLINT
