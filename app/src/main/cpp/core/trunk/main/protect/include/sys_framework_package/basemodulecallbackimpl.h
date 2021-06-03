/**
  *@file BaseCallbackImpl.h
  *@brief Definition of BaseCallbackImpl
  *@version 0.1
  *@author dhongqian
  *@date Created on: 2016-04-21 13:59:20
  *@copyright Copyright(c) 2016 YunShen Technology. All rights reserved.
  * 
  */
#ifndef SIP_BASE_MODULE_CALLBACK_H_  // NOLINT
#define SIP_BASE_MODULE_CALLBACK_H_  // NOLINT

/// stl headers //  NOLINT
#include <map>
#include <string>
/// ysos private headers //  NOLINT
#include "../../../public/include/os_hal_package/config.h"
#include "../../../public/include/sys_interface_package/common.h"  // NOLINT
#include "../../../public/include/sys_interface_package/callbackqueue.h"  // NOLINT
#include "../../../public/include/core_help_package/baseinterfaceimpl.h"
#include "../../../protect/include/sys_framework_package/basecallbackimpl.h"

namespace ysos {
class BaseModuleImpl;
class ModuleInterface;

/**
  *@brief  BaseModule的Callback的Context参数  //  NOLINT
  *        //  NOLINT
  */
struct BaseModuelCallbackContext {
  enum MSG_TYPE {NORMAL_DATA=1, STATUS_EVENT=2};   ///<  标记Message的类型  //  NOLINT
  ModuleInterface       *prev_module_ptr;  ///< 前一个Module的指针
  CallbackInterface     *prev_callback_ptr;  ///< 前一个Module的Callback指针
  ModuleInterface       *cur_module_ptr;  ///< 当前Module的指针
  CallbackInterface     *cur_callback_ptr;  ///< 当前的Callback指针
  BufferInterfacePtr    *context_ptr;  ///<  附加参数 //  NOLINT
  MSG_TYPE               msg_type;     ///<  Message类型 //  NOLINT

  BaseModuelCallbackContext() {
    prev_callback_ptr = cur_callback_ptr = NULL;
    prev_module_ptr = cur_module_ptr = NULL;
    context_ptr = NULL;
    msg_type =  NORMAL_DATA;
  }
};

/**
  *@brief  BaseModule的Callback  //  NOLINT
  */
class YSOS_EXPORT BaseModuleCallbackImpl : public BaseCallbackImpl {
  //DECLARE_CREATEINSTANCE(BaseModuleCallbackImpl);
  DISALLOW_COPY_AND_ASSIGN(BaseModuleCallbackImpl);
  DECLARE_PROTECT_CONSTRUCTOR(BaseModuleCallbackImpl);

 public:
  //enum Callback_Property {
  //  OWNER_ID=100, ///< Callback所属的ID
  //  UNIQUE_KEY,   ///<  获取属于该Callback的唯一Key //  NOLINT
  //  Callback_Property_End
  //};

  typedef std::multimap<INT64, INT64> BaseModuleCallbackImplMap;
  typedef std::multimap<INT64, INT64>::iterator BaseModuleCallbackImplIterator;

  virtual ~BaseModuleCallbackImpl();

  /**
   *@brief 基本初始化  // NOLINT
   *@param param： 初始化的参数  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Initialize(void *param=NULL);
  /**
  *@brief 回调接口的实现函数  // NOLINTㅐ
  *@param input_buffer[Input]： 输入数据缓冲  // NOLINT
  *@param output_buffer[Output]：回调处理后输出的数据缓冲  // NOLINT
  *@param context[Input]： 回调处理的上下文数据,是BaseModuelCallbackContext* 类型  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int Callback(BufferInterfacePtr input_buffer, BufferInterfacePtr output_buffer, void *context = NULL); // NOLINT
  /**
  *@brief 设置回调的输入输出数据类型  // NOLINT
  *@param input_type[Input]： 输入数据类型  // NOLINT
  *@param output_type[Intput]：输出的数据类型  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int SetType(const CallbackDataType input_type, const CallbackDataType output_type);
  /**
  *@brief 获得回调的输入输出数据类型  // NOLINT
  *@param input_type[Output]： 输入数据类型  // NOLINT
  *@param output_type[Output]：输出的数据类型  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int GetType(CallbackDataType *input_type, CallbackDataType *output_type);
  /**
  *@brief 回调处理是否准备好  // NOLINT
  *@param ： 无  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int IsReady();
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
  *@brief 遍历当前Module中所有匹配的NextCallback  // NOLINT
  *@param input_buffer[Input]： 输入数据缓冲  // NOLINT
  *@param output_buffer[Output]：回调处理后输出的数据缓冲  // NOLINT
  *@param context[Input]： 回调处理的上下文数据,是BaseModuelCallbackContext* 类型  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int InvokeNextCallback(BufferInterfacePtr input_buffer, BufferInterfacePtr output_buffer, void *context);
  /**
    *@brief  获取一个唯一的Key,Value对, 默认返回<input_type_, output_type_> //  NOLINT
    *@return 成功返回结果  //  NOLINT
    */
  virtual CallbackIODataTypePair GetUniqueKey(void);
  /**
  *@brief 返回当前Callback对应的Module指针  // NOLINT
  *@param context[Input]： 回调处理的上下文数据,是BaseModuelCallbackContext* 类型  // NOLINT
  *@return： 成功返回Module指针，失败返回NULL  // NOLINT
  */
  BaseModuleImpl *GetCurModule(void *context);
  /**
  *@brief 返回当前Callback对应的PrevModule指针  // NOLINT
  *@param context[Input]： 回调处理的上下文数据,是BaseModuelCallbackContext* 类型  // NOLINT
  *@return： 成功返回Module指针，失败返回NULL  // NOLINT
  */
  BaseModuleImpl *GetPrevModule(void *context);
  /**
  *@brief 返回当前Callback对应的PrevCallback指针  // NOLINT
  *@param context[Input]： 回调处理的上下文数据,是BaseModuelCallbackContext* 类型  // NOLINT
  *@return： 成功返回Callback指针，失败返回NULL  // NOLINT
  */
  CallbackInterface *GetPrevCallback(void *context);
  /**
  *@brief 当前模块是否是在Run状态  // NOLINT
  *@param cur_module[Input]： 当前的Module  // NOLINT
  *@param cur_module[Input]： 当前的Module的Prev Module  // NOLINT
  *@return： true Run状态，false 非Run状态  // NOLINT
  */
  bool IsCurModuleRun(ModuleInterface *cur_module, ModuleInterface *prev_module_ptr);

 protected:
  /**
   *@brief  分配一个新的Buffer，由基类实现//  NOLINT
   *@param module_ptr[Input]： Callback所属的Module  // NOLINT
   *@param buffer_ptr[output]: 分配到的Buffer
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  int AllocateBuffer(BaseModuleImpl *module_ptr, BufferInterfacePtr *buffer_ptr);
  /**
   *@brief  释放Buffer，由基类实现//  NOLINT
   *@param module_ptr[Input]： Callback所属的Module  // NOLINT
   *@param buffer_ptr[output]: 分配到的Buffer
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  int ReleaseBuffer(BaseModuleImpl *module_ptr, BufferInterfacePtr buffer_ptr);

 public:
  /**
  *@brief 遍历当前Module中所有匹配的NextCallback  // NOLINT
  *@param input_buffer[Input]： 输入数据缓冲  // NOLINT
  *@param output_buffer[Output]：回调处理后输出的数据缓冲  // NOLINT
  *@param callback[Input]： 当前NextCallbackQueue中的NextCallback，NextCallbackQueue中有几个Callback，该函数就会被调用几次  // NOLINT
  *@param context[Input]： 回调处理的上下文数据,是BaseModuelCallbackContext* 类型，与Callback中的Contex同  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int InvokeNextCallback(BufferInterfacePtr input_buffer, BufferInterfacePtr output_buffer, CallbackInterfacePtr callback, void *context);

//  protected:
//   CallbackDataType             input_type_;   ///< callback接受的输入数据类型
//   CallbackDataType             output_type_;   ///< callback处理后的输出数据类型

 private:
  BaseModuleCallbackImplMap    owner_id_map_;  ///< 用以标记Callback属于哪一个owner
//   std::string                  remark_;        ///<  callback对象的备注  //  NOLINT
//   std::string                  version_;       ///<  callback对象的版本号 //  NOLINT
};
}
#endif  // SIP_CALLBACK_BASE_H_  // NOLINT
