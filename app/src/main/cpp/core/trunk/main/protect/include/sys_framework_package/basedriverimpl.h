/**
 *@file basedriverimpl.h
 *@brief base driver interface
 *@version 1.0
 *@author dhongqian
 *@date Created on: 2016-06-23 10:06:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */


#ifndef SFP_BASE_DRIVER_IMPL_H_  //NOLINT
#define SFP_BASE_DRIVER_IMPL_H_  //NOLINT

/// Private Headers //  NOLINT
#include "../../../public/include/core_help_package/baseinterfaceimpl.h"
#include "../../../public/include/os_hal_package/driverinterface.h"
#include "../../../public/include/sys_interface_package/common.h"
///  Boost Headers       // NOLINT
#include <boost/function.hpp>

namespace ysos {

class YSOS_EXPORT BaseDriverImpl : public DriverInterface, public BaseInterfaceImpl {
  DISALLOW_COPY_AND_ASSIGN(BaseDriverImpl);
  DECLARE_CREATEINSTANCE(BaseDriverImpl);
  DECLARE_PROTECT_CONSTRUCTOR(BaseDriverImpl);

 public:

  virtual ~BaseDriverImpl(void);

  /**
  *@brief 基本初始化  // NOLINT
  *@param param： 初始化的参数  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int Initialize(void *param=NULL);
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
   *@brief 关闭底层驱动  // NOLINT
   *@@param pParams[Input]： 驱动所需的关闭参数  // NOLINT
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
  /**
   *@brief 清除资源,Initialize的逆操作  // NOLINT
   *@param param： 初始化的参数 // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int UnInitialize(void *param=NULL);

 protected:
  /**
   *@brief  发送模块的状态或事件消息 //  NOLINT
   *@param status_event_code 状态或事件值
   *@param detail   状态或事件的详细信息
   *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
   */
  int SendStatusEvent(const std::string &status_event_code, const std::string &detail);
  /**
   *@brief  发送模块的状态或事件消息 //  NOLINT
   *@param status_event_code 状态或事件值
   *@param module_name 该事件所属的模块名字
   *@param detail   状态或事件的详细信息
   *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
   */
  int NotifyStatusEvent(const std::string &status_event_code, const std::string &module_name, const std::string &detail);

 protected:
  /**
  *@brief  配置文件中设置的Key、Value对，会在Initialized函数中调用 //  NOLINT
  *@param key 配置参数中的Key  // NOLINT
  *@param value 与Key对应的Value  // NOLINT
  *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
  */
  virtual int Initialized(const std::string &key, const std::string &value);

 private:
  /**
  *@brief  读取配置文件中的参数，进行配置 //  NOLINT
  *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
  */
  int ConfigInitialize(void);

 protected:
  typedef boost::function<int(const std::string&, const std::string &)> status_event_fun;
  status_event_fun                     status_fun_;
  typedef boost::function<int(const std::string&, const std::string &, const std::string &)> status_notify_event_fun;
  status_notify_event_fun                     status_notify_fun_;
  std::string                          remark_;        ///<  driver对象的备注  //  NOLINT
  std::string                          version_;       ///<  driver对象的版本号 //  NOLINT
};
typedef boost::shared_ptr<ysos::BaseDriverImpl> BaseDriverImplPtr;
} ///< namespace ysos

#endif  /// SFP_BASE_DRIVER_IMPL_H_   //NOLINT