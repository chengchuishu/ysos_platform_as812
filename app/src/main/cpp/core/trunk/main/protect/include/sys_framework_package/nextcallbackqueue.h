/**
 *@file NextCallbackQueue.h
 *@brief Definition of NextCallbackQueue
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-21 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef SIP_NEXT_CALLBACK_QUEUE_H_  // NOLINT
#define SIP_NEXT_CALLBACK_QUEUE_H_  // NOLINT

/// ysos private headers //  NOLINT
#include "../../../public/include/os_hal_package/config.h"
#include "../../../public/include/sys_interface_package/common.h"  // NOLINT
#include "../../../public/include/core_help_package/baseinterfaceimpl.h"
#include "../../../public/include/sys_interface_package/callbackqueue.h"  // NOLINT
#include "../../../public/include/core_help_package/templatepriqueue.h"  // NOLINT

namespace ysos {
/*
  @brief Module中的NextCallbackQueue对列
*/
class YSOS_EXPORT NextCallbackQueue : public CallbackQueueInterface, public BaseInterfaceImpl {
  DECLARE_CREATEINSTANCE(NextCallbackQueue);
  DISALLOW_COPY_AND_ASSIGN(NextCallbackQueue);
  DECLARE_PROTECT_CONSTRUCTOR(NextCallbackQueue);
public:
  ~NextCallbackQueue();
  /**
   *@brief 添加Callback到队列中  // NOLINT
   *@param callback[Input]： CallbackInterface指针  // NOLINT
   *@param owner_id[Input]： CallbackInterface隶属模块ID，如Module  // NOLINT
   *@param priority[Input]： CallbackInterface优先级  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int AddCallback(CallbackInterfacePtr callback, INT64 owner_id=0, UINT32 priority = 0);  // NOLINT
  /**
  *@brief 从队列中移除CallbackInterface的指针  // NOLINT
  *@param callback[Input]： CallbackInterface指针  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int RemoveCallback(CallbackInterfacePtr callback);
  /**
   *@brief 从队列中移除CallbackInterface的指针，按照隶属模块ID，当owner_id为0时，删除所有  // NOLINT
   *       Callback                  // NOLINT
   *@param owner_id[Input]： CallbackInterface指针  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int RemoveByID(INT64 owner_id);
  /**
   *  nextcallbackqueue里不支持这个函数
   */
  virtual bool FindCallback(CallbackInterfacePtr *callback, CallbackIODataTypePair key);  // NOLINT
  /**
   *@brief  获取第一个或下一个Callback
   *@param first_not_next:  true 获取第一个Callback false 获取下一个Callback
   *@return 返回获得的Callback，失败返回NULL
   */
  virtual CallbackInterfacePtr FindCallback(bool first_not_next = false);

private:
  /**
   *@brief  callback是否与owner_id匹配
   *@param callback  待匹配的callback
   *@param owner_id  所属的owner_id
   *@param is_delete 匹配后，是否将callback中的owner_id信息删除,true删除，false不删除
   *@return 返回获得的Callback，失败返回NULL
   */
  bool IsMatchedCallback(CallbackInterfacePtr callback, INT64 owner_id, bool is_delete=false);

 protected:
  TemplatePriQueue<UINT32, CallbackInterfacePtr>  callback_queue_;
  typedef TemplatePriQueue<UINT32, CallbackInterfacePtr>::iterator CallbackQueueIterator;  // NOLINT
  CallbackQueueIterator  cur_queue_it_;
};
}
#endif  // SIP_NEXT_CALLBACK_QUEUE_H_  // NOLINT