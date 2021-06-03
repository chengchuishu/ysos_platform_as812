/**
 *@file messageinterface.h
 *@brief Definition of message
 *@version 0.1
 *@author Steven.Shi
 *@date Created on: 2016-04-13 08:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef SIP_MSG_INTERFACE_H                     //NOLINT
#define SIP_MSG_INTERFACE_H                     //NOLINT

#include "../../../public/include/sys_interface_package/callbackqueue.h"
#include "../../../public/include/sys_interface_package/ysos_enum.h"

namespace ysos {

class BaseInterface;
class CallbackInterface;

/**
  *@brief 消息接口
  */
class YSOS_EXPORT MsgInterface : virtual public BufferInterface {
  IMPLEMENT_PUBLIC_EMPTY_DESTRUCTOR(MsgInterface)

public:
  /**
   *@brief  获取当前消息的消息ID //NOLINT
   *@return： 成功返回消息ID，失败返回0 //NOLINT
   */
  virtual uint32_t GetMessageID(void) = 0;
  /**
   *@brief  设置当前消息的消息ID //NOLINT
   *@param message_id[Input]： 待设置的消息ID//NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值 //NOLINT
   */
  virtual int SetMessageID(const uint32_t &message_id) = 0;
  /**
   *@brief  获取当前消息的消息体 //NOLINT
   *@return： 成功返回消息体，失败返回NULLL //NOLINT
   */
  virtual BufferInterfacePtr GetMessageBody(void) = 0;
  /**
   *@brief  设置当前消息的消息体 //NOLINT
   *@param message_body_ptr[Input]： 待设置的消息体//NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值 //NOLINT
   */
  virtual int SetMessageBody(BufferInterfacePtr message_body_ptr) = 0;
  /**
   *@brief  设置当前消息的source callback和destination callback //NOLINT
   *@param source_callback_ptr[Input]： 待设置的source callback //NOLINT
   *@param destination_callback_ptr[Input]： 待设置的destination callback //NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值 //NOLINT
   */
  virtual int SetCalllback(CallbackInterfacePtr source_callback_ptr, CallbackInterfacePtr destination_callback_ptr) = 0;
  /**
   *@brief  执行当前消息的source callback //NOLINT
   *@param input_buffer_ptr[Input]： 输入数据 //NOLINT
   *@param output_buffer_ptr[Input]： 输出数据 //NOLINT
   *@param contex_ptr[Input]： 上下文 //NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值 //NOLINT
   */
  virtual int RunSourceCallback(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr, void *contex_ptr = NULL) = 0;
  /**
   *@brief  执行当前消息的destination callback //NOLINT
   *@param input_buffer_ptr[Input]： 输入数据 //NOLINT
   *@param output_buffer_ptr[Input]： 输出数据 //NOLINT
   *@param contex_ptr[Input]： 上下文 //NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值 //NOLINT
   */
  virtual int RunDestinationCallback(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr, void *contex_ptr = NULL) = 0;
};
typedef boost::shared_ptr<MsgInterface> MsgInterfacePtr;

/**
 *@brief 消息队列的接口定义，主要用于消息的缓冲，内部采用优先级队列
 */
class YSOS_EXPORT MsgQueueInterface : virtual public BaseInterface {
  IMPLEMENT_PUBLIC_EMPTY_DESTRUCTOR(MsgQueueInterface)

public:
 /**
   *@brief 插入消息到消息队列//NOLINT
   *@param message_ptr[Input]： 输出的消息//NOLINT
   *@param message_id[Input]：可根据消息ID来获得消息，默认值为MessageAny//NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值//NOLINT
   */
  virtual int Push(MsgInterfacePtr message_ptr, int message_id = PROP_MESSAGE_ANY) = 0;

  /**
   *@brief 从消息队列中根据消息ID获得消息,并从对死中删除取到的消息  // NOLINT
   *@param message_ptr[Output]： 输出的消息  // NOLINT
   *@param message_id[Input]：可根据消息ID来获得消息，默认值为MessageAny  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Get(MsgInterfacePtr &message_ptr, int message_id = PROP_MESSAGE_ANY) = 0;

  /**
   *@brief 从消息队列中根据消息ID获得消息  // NOLINT
   *@param message_ptr[Output]： 输出的消息  // NOLINT
   *@param message_id[Input]：可根据消息ID来获得消息，默认值为MessageAny  // NOLINT
   *@param remove_flag[Input]：取完消息后是否移除消息，默认值为不移除PM_NoRemove  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Peek(MsgInterfacePtr &message_ptr, int message_id = PROP_MESSAGE_ANY, int remove_flag = PROP_PM_REMOVE) = 0;

  /**
   *@brief 将消息发送出去  // NOLINT
   *@param message_ptr[Input]： 输入的消息  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Dispose(MsgInterfacePtr message_ptr) = 0;

  /**
   *@brief 从队列中根据消息ID清除消息, 若为MessageAny则清除全部信息  // NOLINT
   *@param message_id[Input]： 输入的消息ID  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Clear(int message_id = PROP_MESSAGE_ANY) = 0;

  /**
   *@brief 从消息队列中根据消息ID获得消息  // NOLINT
   *@param message_ptr[Output]： 输出的消息  // NOLINT
   *@param find_type[Input]：消息的查找方式  // NOLINT
   *@param message_data[Input]：消息查找时需要的查找信息  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Find(MsgInterfacePtr &message_ptr, int find_type, const BufferInterfacePtr& message_data) = 0;

  /**
   *@brief 返回消息队列中当前的消息数量//NOLINT
   */
  virtual int GetMessageAmount() = 0;
};

typedef boost::shared_ptr<MsgQueueInterface> MsgQueueInterfacePtr;

}   //  namespace ysos

#endif  //  MESSAGE_INTERFACE_H                 //NOLINT
