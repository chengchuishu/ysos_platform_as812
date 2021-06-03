/**
 *@file basethreadmodule.h
 *@brief base threading module impl
 *@version 1.0
 *@author Pan
 *@date Created on: 2016-06-15 20:06:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef SFP_BASE_THREAD_MODULE_H  //NOLINT
#define SFP_BASE_THREAD_MODULE_H  //NOLINT

/// boost headers //  NOLINT
#include <boost/thread.hpp>
/// ysos private headers //  NOLINT
#include "../../../protect/include/sys_framework_package/threadcallbackimpl.h"
#include "../../../protect/include/sys_framework_package/basemoduleimpl.h"         //NOLINT
#include "../../../protect/include/sys_framework_package/basemodulecallbackimpl.h" //NOLINT
#include "../../../public/include/os_hal_package/event.h"

namespace ysos {
class BaseThreadModuleCallbackImpl;
/**
 *@brief  thread module // NOLINT
*/
class YSOS_EXPORT BaseThreadModuleImpl : public BaseModuleImpl {
  DECLARE_PROTECT_CONSTRUCTOR(BaseThreadModuleImpl);
  DISALLOW_COPY_AND_ASSIGN(BaseThreadModuleImpl);
  DECLARE_CREATEINSTANCE(BaseThreadModuleImpl);

 public:
  virtual ~BaseThreadModuleImpl();

  struct ThreadData {
    EventInterface* event_ptr;   ///<  event //  NOLINT
    bool is_stoped;         ///<  是否已经停止 //  NOLINT
    bool is_exited;         ///<  是否已经退出 //  NOLINT
    int timeout;            ///<  超时时间  //  NOLINT
    void* p_data;           ///<  参数 //  NOLINT
  };

  /*enum BaseThreadModuleProperty {
    THREADDATA= BaseModuleImpl_Property_End+1,   ///<  当前Module的NextCallbackQueue //  NOLINT
    THREADCALLBACK,
    BaseThreadModule_Property_End
  };*/

  /**
   *@brief 设置接口的属性值  // NOLINT
   *@param type_id[Input]： 属性的ID  // NOLINT
   *@param type[input]： 属性值的值  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int SetProperty(int type_id, void *type);

  /**
  *@brief 获取接口的属性值  // NOLINT
  *@param type_id[Input]： 属性的ID  // NOLINT
  *@param type[Output]：属性值的值  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int GetProperty(int type_id, void *type);

 protected:
  /**
   *@brief 类似于模块的初始化功能，子类实现，只关注业务
   *@param open需要的参数
   *@return 成功返回0，否则失败
  */
  int RealOpen(LPVOID param = NULL);
  /**
   *@brief 运行，子类实现，只关注业务  // NOLINT
   *@param ： 无  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  int RealRun();
  /**
   *@brief 暂停，子类实现，只关注业务 // NOLINT
   *@param ： 无  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  int RealPause();
  /**
  *@brief 停止运行，关闭模块，子类实现，只关注业务
  *       只有当ref_cout为0时，才能正常关闭
  *@return 成功返回0，否则失败
  */
  int RealStop();
  /**
   *@brief 关闭，，子类实现，只关注业务
   *@return 成功返回0，否则失败
   */
  int RealClose();
  /**
    *@brief  初始化ModuleDataInfo //  NOLINT
    *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
    */
  int InitalDataInfo();
  /**
   *@brief  配置文件中设置的Key、Value对，会在Initialized函数中调用 //  NOLINT
   *@param key 配置参数中的Key  // NOLINT
   *@param value 与Key对应的Value  // NOLINT
   *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
   */
  virtual int Initialized(const std::string &key, const std::string &value);

 public:
  /**
  *@brief 为每一个Callback创建一个单独的线程  // NOLINT
  *@param callback[Input]： 当前PreCallbackQueue中的NextCallback，PreCallbackQueue中有几个Callback，该函数就会被调用几次  // NOLINT
  *@param context[Input]： 回调处理的上下文数据,是BaseModuelCallbackContext* 类型，与Callback中的Contex同  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int CreateThreadFromCallback(CallbackInterfacePtr callback, void *context);

 protected:
  ThreadData                             *thread_data_;   ///<  当前构造函数中创建，析构函数中释放 //  NOLINT

 private:
  BaseModuelCallbackContext             *callback_context_ptr_;
  boost::thread_group                    thread_group_;
  //BaseThreadModuleCallbackImpl          *thread_callback_ptr_;
};
} // namespace ysos

#endif    //BASE_THREAD_MODULE_H  //NOLINT