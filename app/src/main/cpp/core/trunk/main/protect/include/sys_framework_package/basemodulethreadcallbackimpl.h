/**
  *@file BaseModuleThreadCallbackImpl.h
  *@brief Definition of BaseModuleThreadCallbackImpl
  *@version 0.1
  *@author dhongqian
  *@date Created on: 2016-04-21 13:59:20
  *@copyright Copyright(c) 2016 YunShen Technology. All rights reserved.
  * 
  */
#ifndef SIP_BASE_MODULE_THREAD_CALLBACK_H_  // NOLINT
#define SIP_BASE_MODULE_THREAD_CALLBACK_H_  // NOLINT

/// stl headers //  NOLINT
#include <map>
#include <string>
/// ysos private headers //  NOLINT
#include "../../../public/include/os_hal_package/config.h"
#include "../../../public/include/sys_interface_package/common.h"  // NOLINT
#include "../../../public/include/sys_interface_package/callbackqueue.h"  // NOLINT
#include "../../../public/include/core_help_package/baseinterfaceimpl.h"
#include "../../../protect/include/sys_framework_package/basemodulecallbackimpl.h"
#include <boost/thread.hpp>

namespace ysos {

/**
  *@brief  BaseModule的ThreadCallback  //  NOLINT
  */
class YSOS_EXPORT BaseModuleThreadCallbackImpl : public BaseModuleCallbackImpl {
  //DECLARE_CREATEINSTANCE(BaseModuleCallbackImpl);
  DISALLOW_COPY_AND_ASSIGN(BaseModuleThreadCallbackImpl);
  DECLARE_PROTECT_CONSTRUCTOR(BaseModuleThreadCallbackImpl);

 public:

  virtual ~BaseModuleThreadCallbackImpl();

  /**
    * 用于消息队列
    */
  struct ThreadCallbackParam {
    BufferInterfacePtr  buffer_ptr;      ///< pre module的输入Bufffer
    BaseModuelCallbackContext  *callback_context;    ///< 与这个Bufffer相匹配的Context
  };
  typedef boost::shared_ptr<ThreadCallbackParam>     ThreadCallbackParamPtr;
  typedef std::list<ThreadCallbackParamPtr >         ThreadCallbackParamList;

  /**
   *@brief 基本初始化  // NOLINT
   *@param param： 初始化的参数  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Initialize(void *param=NULL);
  /**
   *@brief 清除资源,Initialize的逆操作  // NOLINT
   *@param param： 初始化的参数 // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int UnInitialize(void *param=NULL);
  /**
  *@brief 回调接口的实现函数  // NOLINTㅐ
  *@param input_buffer[Input]： 输入数据缓冲  // NOLINT
  *@param output_buffer[Output]：回调处理后输出的数据缓冲  // NOLINT
  *@param context[Input]： 回调处理的上下文数据,是BaseModuelCallbackContext* 类型  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int Callback(BufferInterfacePtr input_buffer, BufferInterfacePtr output_buffer, void *context = NULL); // NOLINT

 protected:
  /**
  *@brief 子类真正的实现函数  // NOLINT
  *@param input_buffer[Input]： 输入数据缓冲  // NOLINT
  *@param next_input_buffer[Output]： 当前Callback对input_buffer处理后的结果存在next_input_buffer,并作为NextCallback的输入  // NOLINT
  *@param pre_output_buffer[Output]：回调处理后输出的数据缓冲  // NOLINT
  *@param context[Input]： 回调处理的上下文数据,是BaseModuelCallbackContext* 类型  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int RealCallback(BufferInterfacePtr input_buffer, BufferInterfacePtr next_input_buffer,
                           BufferInterfacePtr pre_output_buffer, void *context) = 0;
  /**
  *@brief 复制一个Calllback Context  // NOLINT
  *@param callback_context[Input]： 待复制的Calllback Context  // NOLINT
  *@return： 成功返回复制的Context指针，失败返回NULLL  // NOLINT
  */
  BaseModuelCallbackContext *CloneCallbackContext(BaseModuelCallbackContext *callback_context);
  /**
  *@brief 子类真正的实现函数  // NOLINT
  *@param input_buffer[Input]： 输入数据缓冲  // NOLINT
  *@param next_input_buffer[Output]： 当前Callback对input_buffer处理后的结果存在next_input_buffer,并作为NextCallback的输入  // NOLINT
  *@param pre_output_buffer[Output]：回调处理后输出的数据缓冲  // NOLINT
  *@param context[Input]： 回调处理的上下文数据,是BaseModuelCallbackContext* 类型  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int ThreadCallback(void);
  /**
  *@brief 从Param List中获取一个参数  // NOLINT
  *@return： 成功返回参数指针，失败返回NULL  // NOLINT
  */
  ThreadCallbackParamPtr GetCallbackParam(void);
  /**
  *@brief  配置文件中设置的Key、Value对，会在Initialized函数中调用 //  NOLINT
  *@param key 配置参数中的Key  // NOLINT
  *@param value 与Key对应的Value  // NOLINT
  *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
  */
  virtual int Initialized(const std::string &key, const std::string &value);

 protected:
  bool                          is_new_msg_;  ///< 当有新数据加入时，置为true,当有数据被取出时，置为false

 private:
  MutexLock                     thread_callback_param_list_lock_;
  ThreadCallbackParamList       thread_callback_param_list_;
  bool                          is_run_;       ///< 当调用uninitialize时，Calllback线程会退出
  boost::thread_group           thread_group_;
  bool                          is_interrupt_;
};
}
#endif  // SIP_CALLBACK_BASE_H_  // NOLINT
