/**
 *@file PreCallbackQueue.h
 *@brief Definition of PreCallbackQueue
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-21 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef SIP_PRE_CALLBACK_QUEUE_H_  // NOLINT
#define SIP_PRE_CALLBACK_QUEUE_H_  // NOLINT

/// stl headers //  NOLINT
#include <map>
/// ysos private headers //  NOLINT
#include "../../../public/include/sys_interface_package/common.h"  // NOLINT
#include "../../../public/include/sys_interface_package/baseinterface.h"  // NOLINT

#include "../../../public/include/core_help_package/utility.h"
#include "../../../public/include/core_help_package/baseinterfaceimpl.h"
#include "../../../public/include/sys_interface_package/callbackqueue.h"  // NOLINT

namespace ysos {
/*
  @brief Module中的PreCallbackQueue对列
*/
class YSOS_EXPORT PreCallbackQueue : public CallbackQueueInterface, public BaseInterfaceImpl {
  DECLARE_CREATEINSTANCE(PreCallbackQueue);
  DISALLOW_COPY_AND_ASSIGN(PreCallbackQueue);
  DECLARE_PROTECT_CONSTRUCTOR(PreCallbackQueue);

 public:
   ~PreCallbackQueue();
  /**
   *@brief 添加Callback到队列中  // NOLINT
   *@param callback[Input]： CallbackInterface指针  // NOLINT
   *@param owner_id[Input]： CallbackInterface隶属模块ID，如Module  // NOLINT
   *@param priority[Input]： CallbackInterface优先级  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int AddCallback(CallbackInterfacePtr callback, INT64 owner_id = 0, UINT32 priority = 0);  // NOLINT
  /**
   *@brief 从队列中移除CallbackInterface的指针  // NOLINT
   *@param callback[Input]： CallbackInterface指针  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int RemoveCallback(CallbackInterfacePtr callback);
  /**
   *@brief 从队列中移除CallbackInterface的指针，按照隶属模块ID  // NOLINT
   *@param owner_id[Input]： CallbackInterface指针  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int RemoveByID(INT64 owner_id);
  /**
   *@brief 根据key获取到指定的callback  // NOLINT
   *@param callback[output]： 获得的callback  // NOLINT
   *@param key[Input]： 唯一标识一个callback  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual bool FindCallback(CallbackInterfacePtr *callback, CallbackIODataTypePair key);  // NOLINT
  /**
   *@brief  获取第一个或下一个Callback
   *@param first_not_next:  true 获取第一个Callback false 获取下一个Callback
   *@return 返回获得的Callback，失败返回NULL
   */
  virtual CallbackInterfacePtr FindCallback(bool first_not_next = false);

 protected:
  std::map<CallbackIODataTypePair, CallbackInterfacePtr> callback_map_;
  typedef std::map<CallbackIODataTypePair, CallbackInterfacePtr>::iterator MapIterator;  // NOLINT
  MapIterator          cur_iterator_;
};
}
#endif  // SIP_PRE_CALLBACK_QUEUE_H_  // NOLINT