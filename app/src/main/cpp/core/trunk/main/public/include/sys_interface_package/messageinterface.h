/**
 *@file messageinterface.h
 *@brief Definition of message
 *@version 0.1
 *@author Steven.Shi
 *@date Created on: 2016-04-13 08:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef MESSAGE_INTERFACE_H                     //NOLINT
#define MESSAGE_INTERFACE_H                     //NOLINT

#include "../../../public/include/sys_interface_package/callbackqueue.h"
#include "../../../public/include/sys_interface_package/teamparamsinterface.h"
#include <boost/shared_ptr.hpp>

namespace ysos {

template <typename VariantType> class TeamParamsInterface;
class BaseInterface;
class CallbackInterface;

  /**
   *@brief 消息的接口定义,主要用于系统内部的消息传递,包括命令、状态、参数等,        //NOLINT
       主要用于系统内部各个模块之间的信息交互                                   //NOLINT
       这里MessageHead对应的item_id = 0, section_id = 0;                  //NOLINT
       这里MessageBody对应的item_id = 1, section_id = 0;                  //NOLINT
       其中,消息的负载数据包括:                                             //NOLINT
       MessageHead + MessageBody(MessageData + MessageContent)          //NOLINT
       相应地, MessageHead.message_length包含了                           //NOLINT
       整个MessageHead + MessageBody的长度                                //NOLINT
       MessageHead.message_type = MessageData.message_id_               //NOLINT
   */
class YSOS_EXPORT MessageInterface : virtual public TeamParamsInterface<> {
  IMPLEMENT_PUBLIC_EMPTY_DESTRUCTOR(MessageInterface)
 public:
  typedef struct {
    UINT32 message_id_;                         ///< 消息的ID
    CallbackInterfacePtr source_callback_;      ///< 消息的发起源
    CallbackInterfacePtr destination_callback_; ///< 消息的目标
    BufferInterfacePtr message_data_;           ///< 消息的负载数据
  } MessageData;
};
typedef boost::shared_ptr<MessageInterface> MessageInterfacePtr;
typedef MessageInterface::MessageData MessageData;
typedef boost::shared_ptr<MessageData> MessageDataPtr;

  /**
   *@brief 消息队列的接口定义，主要用于消息的缓冲，内部采用优先级队列
   */
class YSOS_EXPORT MessageQueueInterface : virtual public BaseInterface {
  IMPLEMENT_PUBLIC_EMPTY_DESTRUCTOR(MessageQueueInterface)
 public:
  enum MessageID {
    MessageAny = 0,
  };
  enum RemoveFlag {
    PM_NoRemove = 0,
    PM_Remove,
  };
  enum FindType {
    FromID = 0,
    FromSource,
    FromDestination,
  };

  /**
   *@brief 插入消息到消息队列//NOLINT
   *@param message_ptr[Input]： 输出的消息//NOLINT
   *@param message_id[Input]：可根据消息ID来获得消息，默认值为MessageAny//NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值//NOLINT
   */
  virtual int Push(
    MessageInterfacePtr message_ptr,
    int message_id = MessageAny) = 0;

  /**
   *@brief 从消息队列中根据消息ID获得消息  // NOLINT
   *@param message_ptr[Output]： 输出的消息  // NOLINT
   *@param message_id[Input]：可根据消息ID来获得消息，默认值为MessageAny  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Get(
    MessageInterfacePtr &message_ptr,
    int message_id = MessageAny) = 0;

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
    int remove_flag = PM_NoRemove) = 0;

  /**
   *@brief 将消息发送出去  // NOLINT
   *@param message_ptr[Input]： 输入的消息  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Dispose(MessageInterfacePtr message_ptr) = 0;

  /**
   *@brief 从队列中根据消息ID清除消息, 若为MessageAny则清除全部信息  // NOLINT
   *@param message_id[Input]： 输入的消息ID  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Clear(int message_id = MessageAny) = 0;

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
    const MessageData& message_data) = 0;

  /**
   *@brief 返回消息队列中当前的消息数量//NOLINT
   */
  virtual uint32_t GetMessageAmount() = 0;
};

//  NextToDo: 可以暂时考虑将相关逻辑委托给Callback//NOLINT
//  发送消息, 待消息处理完成之后返回//NOLINT
//  The sender sends the message to message_receiver
//  (inserts the message to MessageQueue)
//  and waits until message_receiver processes:
//    dst_callback.Callback();
//    if (src_callback) src_callback.Callback();
int SendMessage(void* message_receiver, MessageInterfacePtr message_ptr);

//  发送消息之后立即返回//NOLINT
//  The sender sends the message to message_receiver
//  (inserts the message to MessageQueue) and returns.
//  In the future, message_receiver processes:
//    dst_callback.Callback();
//    if (src_callback) src_callback.Callback();
int PostMessage(void* message_receiver, MessageInterfacePtr message_ptr);

typedef boost::shared_ptr<MessageQueueInterface> MessageQueueInterfacePtr;

}   //  namespace ysos

#endif  //  MESSAGE_INTERFACE_H                 //NOLINT
