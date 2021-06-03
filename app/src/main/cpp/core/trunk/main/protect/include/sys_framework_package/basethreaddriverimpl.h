/**
 *@file basethreaddriverimpl.h
 *@brief base driver interface
 *@version 1.0
 *@author dhongqian
 *@date Created on: 2016-06-23 10:06:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */


#ifndef SFP_BASE_THREAD_DRIVER_IMPL_H_  //NOLINT
#define SFP_BASE_THREAD_DRIVER_IMPL_H_  //NOLINT

/// Private Headers //  NOLINT
#include "../../../protect/include/sys_framework_package/basedriverimpl.h"
/// Bost Headers
#include <boost/thread.hpp>

namespace ysos {

class YSOS_EXPORT BaseThreadDriverImpl : public BaseDriverImpl {
  DISALLOW_COPY_AND_ASSIGN(BaseThreadDriverImpl);
  DECLARE_CREATEINSTANCE(BaseThreadDriverImpl);
  DECLARE_PROTECT_CONSTRUCTOR(BaseThreadDriverImpl);

 public:

  virtual ~BaseThreadDriverImpl(void);

  /**
    * 用于消息队列
    */
  //struct DriverParam {
  //  BufferInterfacePtr  buffer_ptr;      ///< pre module的输入Bufffer
  //  // BaseModuelCallbackContext  *callback_context;    ///< 与这个Bufffer相匹配的Context
  //};
  // typedef boost::shared_ptr<DriverParam>     DriverParamPtr;
  typedef BufferInterface      DriverParam;
  typedef BufferInterfacePtr   DriverParamPtr;
  typedef std::list<DriverParamPtr >         ThreadDriverParamList;

  /**
  *@brief 基本初始化  // NOLINT
  *@param param： 初始化的参数  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int Initialize(void *param=NULL);

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
   *@brief 清除资源,Initialize的逆操作  // NOLINT
   *@param param： 初始化的参数 // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int UnInitialize(void *param=NULL);

 protected:
  /**
  *@brief 子类真正的实现函数  // NOLINT
  *@param input_buffer[Input]： 输入数据缓冲  // NOLINT
  *@param context[Input]： 回调处理的上下文数据,是BaseModuelCallbackContext* 类型  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int RealCallback(BufferInterfacePtr input_buffer, void *context);
  /**
  *@brief 子类真正的实现函数  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int ThreadCallback(void);
  /**
  *@brief 增加一个待处理的数据  // NOLINT
  *@param input_buffer[Input]： 待处理的数据  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  int SetDriverParam(BufferInterfacePtr buffer_ptr);
  /**
  *@brief 从Param List中获取一个参数  // NOLINT
  *@return： 成功返回参数指针，失败返回NULL  // NOLINT
  */
  DriverParamPtr GetDriverParam(void);

 protected:
  MutexLock                     thread_driver_param_list_lock_;
  ThreadDriverParamList         thread_driver_param_list_;
  bool                          is_run_;       ///< 当调用uninitialize时，Calllback线程会退出
  boost::thread_group           thread_group_;
  bool                          is_new_msg_;  ///< 当有新数据加入时，置为true,当有数据被取出时，置为false
  // bool                          is_interrupt_;

};
typedef boost::shared_ptr<ysos::BaseThreadDriverImpl> BaseThreadDriverImplPtr;
} ///< namespace ysos

#endif  /// SFP_BASE_DRIVER_IMPL_H_   //NOLINT