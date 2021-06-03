/*
# MessageImpl.h
# Definition of MessageImpl and MessageQueueImpl
# Created on: 2016-04-25 13:59:20
# Original author: XuLanyue
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
*/
#ifndef YSOS_SFP_CALLBACK_MESSAGE_IMPL_H        //NOLINT
#define YSOS_SFP_CALLBACK_MESSAGE_IMPL_H        //NOLINT

/// Stl Headers
#include <map>
#include <list>
#include <string>
/// Boost Headers
#include <boost/shared_ptr.hpp>
/// Ysos Headers
#if 1   //  disable warning 4250 and 4996
#include "../../../Public/include/os_hal_package/config.h"  // NOLINT
#if (CURRENT_PLATFORM == PLATFORM_WINDOWS)
#pragma warning(disable: 4250)      //  dominance
#pragma warning(disable: 4996)      //  std::_Fill_n
#endif
#endif
#include "../../../Public/include/os_hal_package/lock.h"
#include "../../../Public/include/sys_interface_package/common.h"
#include "../../../Public/include/sys_interface_package/messagehead.h"
#include "../../../Public/include/sys_interface_package/messagetype.h"
#include "../../../Public/include/core_help_package/baseinterfaceimpl.h"
#include "../../../protect/include/os_hal_package/teamparamsimpl.h"
#include "../../../Public/include/sys_interface_package/msginterface.h"
#include "../../../Public/include/sys_interface_package/teamparamsinterface.h"
#include "../../../Public/include/core_help_package/templatepriqueue.h"
#include "../../../protect/include/os_hal_package/messageimpl.h"


//  1, 消息队列使用优先级队列; 0, 消息队列不使用优先级队列//NOLINT
#define USING_PRIORITY_QUEUE_FOR_MESSAGE_QUEUE      0

namespace ysos {
class YSOS_EXPORT CallbackMessageQueueImpl :
  public MessageQueueImpl {
  DISALLOW_COPY_AND_ASSIGN(CallbackMessageQueueImpl)
  DECLARE_PROTECT_CONSTRUCTOR(CallbackMessageQueueImpl)
  DECLARE_CREATEINSTANCE(CallbackMessageQueueImpl)
 public:
  virtual ~CallbackMessageQueueImpl();
 public:
/**
  *@brief 将消息发送出去  // NOLINT
  *@param message_ptr[Input]： 输入的消息  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int Dispose(MsgInterfacePtr message_ptr);
};
typedef boost::shared_ptr<CallbackMessageQueueImpl> CallbackMessageQueueImplPtr;
}   // namespace ysos

#endif  //  YSOS_SFP_CALLBACK_MESSAGE_IMPL_H    //NOLINT
