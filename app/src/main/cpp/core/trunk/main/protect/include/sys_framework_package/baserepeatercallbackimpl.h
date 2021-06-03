/**
  *@file BaseRepeaterCallbackImpl.h
  *@brief The base class of all Repeater callbacks
  *@version 1.0
  *@author dhongqian
  *@date Created on: 2016-05-31 18:03:27
  *@copyright Copyright(c) 2016 YunShen Technology. All rights reserved.
  * 
  */
#ifndef REPEATER_CALLBACK_BASE_H_  // NOLINT
#define REPEATER_CALLBACK_BASE_H_  // NOLINT

/// ysos private headers //  NOLINT
#if 1   //  disable warning 4250 and 4996
#include "../../../public/include/os_hal_package/config.h"
#if (CURRENT_PLATFORM == PLATFORM_WINDOWS)
#pragma warning(disable: 4250)      //  dominance
#pragma warning(disable: 4996)      //  std::_Fill_n
#endif
#endif

/// Ysos Headers //  NOLINT
#include "../../../protect/include/sys_framework_package/basemodulecallbackimpl.h"  // NOLINT
#include  "baserepeatermoduleimpl.h"

namespace ysos {
/**
  *@brief  Repeater的基类Callback  //  NOLINT
  *        //  NOLINT
  */
class YSOS_EXPORT BaseRepeaterCallbackImpl : public BaseModuleCallbackImpl {
  DECLARE_CREATEINSTANCE(BaseRepeaterCallbackImpl);
  DISALLOW_COPY_AND_ASSIGN(BaseRepeaterCallbackImpl);
  DECLARE_PROTECT_CONSTRUCTOR(BaseRepeaterCallbackImpl);

 public:
  /**
   *@brief 析构函数
   */
  virtual ~BaseRepeaterCallbackImpl();
  /**
  *@brief 回调接口的实现函数  // NOLINT
  *@param input_buffer[Input]： 输入数据缓冲  // NOLINT
  *@param output_buffer[Output]：回调处理后输出的数据缓冲  // NOLINT
  *@param context[Input]： 回调处理的上下文数据,是BaseModuelCallbackContext* 类型  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int Callback(BufferInterfacePtr input_buffer, BufferInterfacePtr output_buffer, void *context = NULL); // NOLINT
  /**
  *@brief 获取接口的属性值  // NOLINT
  *@param type_id[Input]： 属性的ID  // NOLINT
  *@param type[Output]：属性值的值  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int GetProperty(int type_id, void *type);

  /**
   *@brief 设置接口的属性值  // NOLINT
   *@param type_id[Input]： 属性的ID  // NOLINT
   *@param type[input]： 属性值的值  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int SetProperty(int type_id, void *type);

 protected:
   /**
   *@brief 回调接口的实现函数  // NOLINT
   *@param input_buffer[Input]： 输入数据缓冲  // NOLINT
   *@param output_buffer[Output]：回调处理后输出的数据缓冲  // NOLINT
   *@param context[Input]： 回调处理的上下文数据  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int RealCallback(BufferInterfacePtr input_buffer, BufferInterfacePtr output_buffer, BufferInterfacePtr external_buf, void *context);
  /**
   *@brief 调用Repeater的Observer  // NOLINT
   *@param input_buffer[Input]： 输入数据缓冲  // NOLINT
   *@param context[Input]： 回调处理的上下文数据  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  int Observer(BufferInterfacePtr input_buffer, void *context);
  /**
   *@brief 在Callback开始时调用Observer  // NOLINT
   *@param input_buffer[Input]： 输入数据缓冲  // NOLINT
   *@param context[Input]： 回调处理的上下文数据  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int FirstObserver(BufferInterfacePtr input_buffer, void *context);
  /**
   *@brief 在Callback结束前调用Observer  // NOLINT
   *@param input_buffer[Input]： 输入数据缓冲  // NOLINT
   *@param context[Input]： 回调处理的上下文数据  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int LastObserver(BufferInterfacePtr input_buffer, void *context);

  /**
   *@brief Switch的上下文数据
   */
  struct SwitchContext {
    BufferInterfacePtr input_buffer;  ///<  下一个Callback的input_buffer //  NOLINT
    BufferInterfacePtr output_buffer; ///<  下一个Callback的output_buffer //  NOLINT
    int                ret;           ///<  下一个Callback执行的返回值  //  NOLINT

    SwitchContext(BufferInterfacePtr input_ptr, BufferInterfacePtr output_ptr) {
      input_buffer = input_ptr;
      output_buffer = output_ptr;
      ret = YSOS_ERROR_SUCCESS;
    }
  };

  /**
   *@brief 根据输入的信息，决定下一步的动作  // NOLINT
   *@param input_buffer[Input]： 输入数据缓冲  // NOLINT
   *@param output_buffer[Output]：回调处理后输出的数据缓冲  // NOLINT
   *@param callback[Input]： 当前NextCallbackQueue中的NextCallback，NextCallbackQueue中有几个Callback，该函数就会被调用几次  // NOLINT
   *@param context[Input]： 回调处理的上下文数据,是BaseModuelCallbackContext* 类型，与Callback中的Contex同  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Switch(SwitchContext *switch_context, CallbackInterfacePtr callback, void *context);
  /**
  *@brief 返回当前Callback对应的Module指针  // NOLINT
  *@param context[Input]： 回调处理的上下文数据,是BaseModuelCallbackContext* 类型  // NOLINT
  *@return： 成功返回Module指针，失败返回NULL  // NOLINT
  */
  BaseRepeaterModuleImpl *GetCurRepeater(void *context);
  /**
   *@brief 遍历当前Module中所有匹配的NextCallback  // NOLINT
   *@param input_buffer[Input]： 输入数据缓冲  // NOLINT
   *@param output_buffer[Output]：回调处理后输出的数据缓冲  // NOLINT
   *@param context[Input]： 回调处理的上下文数据  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int InvokeNextCallback(BufferInterfacePtr input_buffer, BufferInterfacePtr output_buffer, void *context);
  /**
    *@brief  获取一个唯一的Key,Value对, 默认返回<input_type_, output_type_> //  NOLINT
    *@return 成功返回结果  //  NOLINT
    */
  virtual CallbackIODataTypePair GetUniqueKey(void);

  

 protected:
  ModuleInterfacePtr                prev_module_ptr_;  ///<  前一个Moduel //  NOLINT
};
}
#endif  // REPEATER_CALLBACK_BASE_H_  // NOLINT
