/**
  *@file BaseCallbackImpl.h
  *@brief Definition of BaseCallbackImpl
  *@version 0.1
  *@author dhongqian
  *@date Created on: 2016-04-21 13:59:20
  *@copyright Copyright(c) 2016 YunShen Technology. All rights reserved.
  * 
  */
#ifndef SIP_BASE_CALLBACK_BASE_H_  // NOLINT
#define SIP_BASE_CALLBACK_BASE_H_  // NOLINT

/// stl headers //  NOLINT
#include <string>
/// ysos private headers //  NOLINT
#include "../../../public/include/os_hal_package/config.h"
#include "../../../public/include/sys_interface_package/common.h"  // NOLINT
#include "../../../public/include/sys_interface_package/callbackqueue.h"  // NOLINT
#include "../../../public/include/core_help_package/baseinterfaceimpl.h"

namespace ysos {
/**
 *@brief CallbackInterface的默认实现，可作为所有CallbackInterface的基类
 */
class YSOS_EXPORT BaseCallbackImpl : public CallbackInterface, public BaseInterfaceImpl {
  DECLARE_CREATEINSTANCE(BaseCallbackImpl);
  DISALLOW_COPY_AND_ASSIGN(BaseCallbackImpl);
  DECLARE_PROTECT_CONSTRUCTOR(BaseCallbackImpl);

 public:
  typedef boost::shared_ptr<std::multimap<std::string, std::string> > MapPtr;

  virtual ~BaseCallbackImpl();
  /**
  *@brief 基本初始化  // NOLINT
  *@param param： 初始化的参数  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int Initialize(void *param=NULL);
  /**
  *@brief 回调接口的实现函数  // NOLINT
  *@param input_buffer[Input]： 输入数据缓冲  // NOLINT
  *@param output_buffer[Output]：回调处理后输出的数据缓冲  // NOLINT
  *@param context[Input]： 回调处理的上下文数据,是ModuleInterface* 类型  // NOLINT
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
   *@brief 获取接口的属性值  // NOLINT
   *@param type_id[Input]： 属性的ID  // NOLINT
   *@param type[Output]：属性值的值  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int GetProperty(int type_id, void *type);
  /**
  *@brief 回调处理是否准备好  // NOLINT
  *@param ： 无  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int IsReady();

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
  CallbackDataType             input_type_;   ///< callback接受的输入数据类型
  CallbackDataType             output_type_;   ///< callback处理后的输出数据类型
  std::string                  remark_;        ///<  callback对象的备注  //  NOLINT
  std::string                  version_;       ///<  callback对象的版本号 //  NOLINT
};
}
#endif  // SIP_CALLBACK_BASE_H_  // NOLINT
