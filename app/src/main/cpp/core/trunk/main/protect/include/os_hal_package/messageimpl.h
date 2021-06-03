/*
# MessageImpl.h
# Definition of MessageImpl and MessageQueueImpl
# Created on: 2016-04-25 13:59:20
# Original author: XuLanyue
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
*/
#ifndef YSOS_SFP_MESSAGE_IMPL_H                                                 //NOLINT
#define YSOS_SFP_MESSAGE_IMPL_H                                                 //NOLINT

#if 1   //  disable warning 4250 and 4996
#include "../../../public/include/os_hal_package/config.h"  // NOLINT
#if (CURRENT_PLATFORM == PLATFORM_WINDOWS)
#pragma warning(disable: 4250)      //  dominance
#pragma warning(disable: 4996)      //  std::_Fill_n
#endif
#endif
#include "../../../public/include/os_hal_package/lock.h"
#include "../../../public/include/sys_interface_package/common.h"
#include "../../../public/include/sys_interface_package/messagehead.h"
#include "../../../public/include/sys_interface_package/messagetype.h"
#include "../../../public/include/core_help_package/baseinterfaceimpl.h"
#include "../../../protect/include/os_hal_package/teamparamsimpl.h"
#include "../../../public/include/sys_interface_package/messageinterface.h"
#include "../../../public/include/sys_interface_package/teamparamsinterface.h"
#include "../../../public/include/core_help_package/templatepriqueue.h"
#include <map>
#include <list>
#include <string>
#include <boost/make_shared.hpp>


#define CASE_SAME_THREAD                                0   /// 同一个线程//NOLINT
#define CASE_SAME_PROCESS_DIFFERENT_THREAD              1   /// 线程间//NOLINT
#define CASE_SAME_MACHINE_DIFFERENT_PROCESS             2   /// 进程间//  http_socket//NOLINT
#define CASE_DIFFERENT_MACHINE                          3   /// 机器间//  http_socket//NOLINT

//  1, 消息队列使用优先级队列; 0, 消息队列不使用优先级队列//NOLINT
#define USING_PRIORITY_QUEUE_FOR_MESSAGE_QUEUE      0

namespace ysos {
class YSOS_EXPORT MessageImpl :
  virtual public MessageInterface,
  virtual public TeamParamsInterface<>
{
 public:
#if 1
  MessageImpl(const std::string& strClassName = "MessageImpl");
#endif
  virtual ~MessageImpl();
  virtual int WriteItem(uint32_t iItemID, uint32_t iSectionID, void *pValue);
  virtual int ReadItem(uint32_t iItemID, uint32_t iSectionID, void *pValue);
  virtual int FindItem(uint32_t iItemID, uint32_t iSectionID);
  virtual int CopyFrom(void* source_team_params_ptr);
  virtual int SetBuffer(UINT8 *pBuffer, UINT32 iLength);
 private:
  //  这里,一个完整的Msg中,message_head_ptr_是Head, message_body_ptr_是Body     //NOLINT
  //  TeamParams[0]: message_head_ptr_;
  //  TeamParams[1]: message_body_ptr_;
  MessageHeadPtr message_head_ptr_;
  MessageDataPtr message_body_ptr_;
IMPLEMENT_BUFFER_WRAP_INTERFACE
};

typedef boost::shared_ptr<MessageImpl> MessageImplPtr;

uint32_t GetPriorityOfMessage(uint32_t message_id);
uint32_t GetPriorityOfMessage(MessageHead* message_head);
uint32_t GetPriorityOfMessage(MessageInterfacePtr message_ptr);

//class MessagePriorityCompare {
//public:
//  bool operator()(MessageInterfacePtr t1, MessageInterfacePtr t2) {
//    return GetPriorityOfMessage(t1) > GetPriorityOfMessage(t2);
//  }
//};

class MessagePriorityCompare {
public:
  bool operator()(uint32_t t1,uint32_t t2) {
    if (t1 == t2) {
      return false;
    }
    return GetPriorityOfMessage(t1) > GetPriorityOfMessage(t2);
  }
};

class YSOS_EXPORT MessageQueueImpl :
  public MessageQueueInterface,
  public BaseInterfaceImpl {
  DISALLOW_COPY_AND_ASSIGN(MessageQueueImpl)
  DECLARE_PROTECT_CONSTRUCTOR(MessageQueueImpl)
  DECLARE_CREATEINSTANCE(MessageQueueImpl)
 public:
  virtual ~MessageQueueImpl();
 public:
  /**
   *@brief 插入消息到消息队列//NOLINT
   *@param message_ptr[Input]： 输出的消息//NOLINT
   *@param message_id[Input]：可根据消息ID来获得消息，默认值为MessageAny//NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值//NOLINT
   */
  virtual int Push(MessageInterfacePtr message_ptr, int message_id);

/**
  *@brief 从消息队列中根据消息ID获得消息  // NOLINT
  *@param message_ptr[Output]： 输出的消息  // NOLINT
  *@param message_id[Input]：可根据消息ID来获得消息，默认值为MessageAny  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int Get(MessageInterfacePtr &message_ptr, int message_id);

/**
  *@brief 从消息队列中根据消息ID获得消息  // NOLINT
  *@param message_ptr[Output]： 输出的消息  // NOLINT
  *@param message_id[Input]：可根据消息ID来获得消息，默认值为MessageAny  // NOLINT
  *@param remove_flag[Input]：取完消息后是否移除消息，默认值为不移除PM_NoRemove  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int Peek(
    MessageInterfacePtr message_ptr,
    int message_id = MessageAny,
    int remove_flag = PM_NoRemove);

/**
  *@brief 将消息发送出去  // NOLINT
  *@param message_ptr[Input]： 输入的消息  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int Dispose(MessageInterfacePtr message_ptr);

/**
  *@brief 从队列中根据消息ID清除消息, 若为MessageAny则清除全部信息  // NOLINT
  *@param message_id[Input]： 输入的消息ID  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int Clear(int message_id = MessageAny);

/**
  *@brief 从消息队列中根据消息ID获得消息  // NOLINT
  *@param message_ptr[Output]： 输出的消息  // NOLINT
  *@param find_type[Input]：消息的查找方式  // NOLINT
  *@param message_data[Input]：消息查找时需要的查找信息  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int Find(
    MessageInterfacePtr message_ptr,
    FindType find_type,
    const MessageData& message_data);

/**
  *@brief 返回消息队列中当前的消息数量//NOLINT
  */
  virtual uint32_t GetMessageAmount();
  protected:
#if USING_PRIORITY_QUEUE_FOR_MESSAGE_QUEUE
  TemplatePriQueue<uint32_t, MessageInterfacePtr, MessagePriorityCompare> message_queue_;
#else
  std::list<MessageInterfacePtr> message_queue_;
#endif
  LightLock oLock;
};
typedef boost::shared_ptr<MessageQueueImpl> MessageQueueImplPtr;

#if USING_PRIORITY_QUEUE_FOR_MESSAGE_QUEUE
typedef TemplatePriQueue<uint32_t, MessageInterfacePtr>::iterator MQIT;
#else
typedef std::list<MessageInterfacePtr>::iterator MQIT;
#endif

}   // namespace ysos

#endif  //  YSOS_MESSAGE_IMPL_H                                             //NOLINT
