/*
# MsgImpl.h
# Definition of MsgImpl and MsgQueueImpl
# Created on: 2016-04-25 13:59:20
# Original author: dhongqian
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
*/
#ifndef YSOS_SFP_MSG_IMPL_H                                                 //NOLINT
#define YSOS_SFP_MSG_IMPL_H                                                 //NOLINT


/// Stl Headers
#include <map>
#include <list>
#include <string>
/// Boost Headers
#include <boost/make_shared.hpp>
/// Ysos Headers
#include "../../../public/include/os_hal_package/config.h"  // NOLINT
#include "../../../public/include/os_hal_package/lock.h"
#include "../../../public/include/sys_interface_package/common.h"
#include "../../../public/include/core_help_package/baseinterfaceimpl.h"
#include "../../../public/include/sys_interface_package/msginterface.h"
#include "../../../public/include/core_help_package/templatepriqueue.h"

namespace ysos {

/**
  *@brief 消息的实现
  */
class YSOS_EXPORT MsgImpl: virtual public MsgInterface {
 public:
  MsgImpl(const uint32_t max_size=0, const uint32_t message_id=0, BufferPoolInterfacePtr allocate=NULL);
  MsgImpl(const uint32_t message_id, BufferInterfacePtr message_body);
  /**
   *@brief 将data_ptr封闭成BufferInterface类型对象,谨慎使用
   */
  MsgImpl(uint8_t *data_ptr, const UINT32 data_length, bool is_delegate=false);
  virtual ~MsgImpl();

  ///////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////   Msg 本体的函数          ////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////
  /**
   *@brief  获取当前消息的消息ID //NOLINT
   *@return： 成功返回消息ID，失败返回0 //NOLINT
   */
  virtual uint32_t GetMessageID(void);
  /**
   *@brief  设置当前消息的消息ID //NOLINT
   *@param message_id[Input]： 待设置的消息ID//NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值 //NOLINT
   */
  virtual int SetMessageID(const uint32_t &message_id);
  /**
   *@brief  获取当前消息的消息体 //NOLINT
   *@return： 成功返回消息体，失败返回NULLL //NOLINT
   */
  virtual BufferInterfacePtr GetMessageBody(void);
  /**
   *@brief  设置当前消息的消息体 //NOLINT
   *@param message_body_ptr[Input]： 待设置的消息体//NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值 //NOLINT
   */
  virtual int SetMessageBody(BufferInterfacePtr message_body_ptr);
  /**
   *@brief  设置当前消息的source callback和destination callback //NOLINT
   *@param source_callback_ptr[Input]： 待设置的source callback //NOLINT
   *@param destination_callback_ptr[Input]： 待设置的destination callback //NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值 //NOLINT
   */
  virtual int SetCalllback(CallbackInterfacePtr source_callback_ptr, CallbackInterfacePtr destination_callback_ptr);
  /**
   *@brief  执行当前消息的source callback //NOLINT
   *@param input_buffer_ptr[Input]： 输入数据 //NOLINT
   *@param output_buffer_ptr[Input]： 输出数据 //NOLINT
   *@param contex_ptr[Input]： 上下文 //NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值 //NOLINT
   */
  virtual int RunSourceCallback(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr, void *contex_ptr = NULL);
  /**
   *@brief  执行当前消息的destination callback //NOLINT
   *@param input_buffer_ptr[Input]： 输入数据 //NOLINT
   *@param output_buffer_ptr[Input]： 输出数据 //NOLINT
   *@param contex_ptr[Input]： 上下文 //NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值 //NOLINT
   */
  virtual int RunDestinationCallback(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr, void *contex_ptr = NULL);

  ///////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////   BufferInterface的函数          /////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////
  /**
   *@brief 获取实际的Buffer和最大长度  // NOLINT
   *@param buffer： 实际的Buffer  // NOLINT
   *@param length:  Buffer可用的最大长度  // NOLINT
   *@return:           成功返回实际的长度，否则返回0  // NOLINT
  */
  virtual int GetBufferAndLength(UINT8** buffer, UINT32 *length);
  /**
   *@brief 获取实际的Buffer的最大长度  // NOLINT
   *@param length:  Buffer可用的最大长度  // NOLINT
   *@return:           成功返回实际的长度，否则返回0  // NOLINT
  */
  virtual int GetMaxLength(UINT32 *length);
  /**
   *@brief 设置要使用的Buffer长度和前缀Prefix  // NOLINT
   *@param length:  实际要用到的最大长度  // NOLINT
   *@param prefix:  预留的前缀  // NOLINT
   *@return:           成功返回实际的长度，否则返回0  // NOLINT
  */
  virtual int SetLength(UINT32 length, UINT32 prefix = 0);
  /**
   *@brief 获取当前Buffer的前缀信息  // NOLINT
   *@param prefix:  预留的前缀  // NOLINT
   *@return:           成功返回实际的长度，否则返回0  // NOLINT
   */
  virtual int GetPrefixLength(UINT32 *prefix);
  /*
   @brief 获取当前Buffer的管理器  // NOLINT
   @param allocator:  Buffer的管理器  // NOLINT
   @return:           成功返回实际的长度，否则返回0  // NOLINT
  */
  virtual int GetAllocator(BufferPoolInterfacePtr *allocator);
  /*
   @brief 重置当前Buffer的管理器//NOLINT
   @param allocator:  Buffer的管理器//NOLINT
   @return:           成功返回0，否则返回错误码 //NOLINT
  */
  virtual int ResetAllocator(BufferPoolInterfacePtr allocator=NULL);

 protected:
  uint32_t           msg_id_;
  BufferInterfacePtr  msg_body_ptr_;
  CallbackInterfacePtr  source_callback_ptr_;
  CallbackInterfacePtr  destination_callback_ptr_;
};
typedef boost::shared_ptr<MsgImpl> MsgImplPtr;


class YSOS_EXPORT MsgPool {
  DISALLOW_COPY_AND_ASSIGN(MsgPool);

public:
  MsgPool(const std::string &instance_name = "MsgPool");
  virtual ~MsgPool(void);

  typedef std::list<MsgInterfacePtr >      MsgList;
  /**
   *@brief 插入消息到消息队列//NOLINT
   *@param message_ptr[Input]： 输出的消息//NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值//NOLINT
   */
  virtual int Push(MsgInterfacePtr message_ptr);

  /**
    *@brief 从消息队列中根据消息ID获得消息,并从对死中删除取到的消息  // NOLINT
    *@return： 成功返回Msg指针，失败返回NULL  // NOLINT
    */
  virtual MsgInterfacePtr Get(void);

  /**
    *@brief 从消息队列中根据消息ID获得消息  // NOLINT
    *@param remove_flag[Input]：取完消息后是否移除消息，默认值为不移除PM_NoRemove  // NOLINT
    *@return： 成功返回Msg指针，失败返回NULL  // NOLINT
    */
  virtual MsgInterfacePtr Peek(int remove_flag = PROP_PM_NO_REMOVE);

  /**
    *@brief 清除全部信息  // NOLINT
    *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
    */
  virtual int Clear(void);

  /**
    *@brief 从消息队列中根据消息ID获得消息  // NOLINT
    *@param find_type[Input]：消息的查找方式  // NOLINT
    *@param message_data[Input]：消息查找时需要的查找信息  // NOLINT
    *@return： 成功返回Msg指针，失败返回NULL  // NOLINT
    */
  virtual MsgInterfacePtr Find(int find_type, const BufferInterfacePtr& message_data);

  /**
    *@brief 返回消息队列中当前的消息数量//NOLINT
    */
  virtual int GetMessageAmount();

protected:
  MutexLock         msg_list_lock_;    ///< 消息队列锁
  MsgList           msg_list_;    ///< 消息队列
};
typedef boost::shared_ptr<MsgPool> MsgPoolPtr;

/**
  *@brief 消息队列的实现
  */
class YSOS_EXPORT MsgQueueImpl : public MsgQueueInterface, public BaseInterfaceImpl {
  DISALLOW_COPY_AND_ASSIGN(MsgQueueImpl)
  DECLARE_PROTECT_CONSTRUCTOR(MsgQueueImpl)
  DECLARE_CREATEINSTANCE(MsgQueueImpl)
 public:
  virtual ~MsgQueueImpl();

  typedef std::map<int, MsgPoolPtr >       MessageMap;

 public:
  /**
   *@brief 插入消息到消息队列//NOLINT
   *@param message_ptr[Input]： 输出的消息//NOLINT
   *@param message_id[Input]：可根据消息ID来获得消息，默认值为MessageAny//NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值//NOLINT
   */
  virtual int Push(MsgInterfacePtr message_ptr, int message_id = PROP_MESSAGE_ANY);

  /**
    *@brief 从消息队列中根据消息ID获得消息,并从对死中删除取到的消息  // NOLINT
    *@param message_ptr[Output]： 输出的消息  // NOLINT
    *@param message_id[Input]：可根据消息ID来获得消息，默认值为MessageAny  // NOLINT
    *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
    */
  virtual int Get(MsgInterfacePtr &message_ptr, int message_id = PROP_MESSAGE_ANY);

  /**
    *@brief 从消息队列中根据消息ID获得消息  // NOLINT
    *@param message_ptr[Output]： 输出的消息  // NOLINT
    *@param message_id[Input]：可根据消息ID来获得消息，默认值为MessageAny  // NOLINT
    *@param remove_flag[Input]：取完消息后是否移除消息，默认值为不移除PM_NoRemove  // NOLINT
    *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
    */
  virtual int Peek(MsgInterfacePtr &message_ptr, int message_id = PROP_MESSAGE_ANY, int remove_flag = PROP_PM_NO_REMOVE);

  /**
    *@brief 将消息发送出去  // NOLINT
    *@param message_ptr[Input]： 输入的消息  // NOLINT
    *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
    */
  virtual int Dispose(MsgInterfacePtr message_ptr);

  /**
    *@brief 从队列中根据消息ID清除消息, 若为MessageAny则清除全部信息  // NOLINT
    *@param message_id[Input]： 输入的消息ID  // NOLINT
    *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
    */
  virtual int Clear(int message_id = PROP_MESSAGE_ANY);

  /**
    *@brief 从消息队列中根据消息ID获得消息  // NOLINT
    *@param message_ptr[Output]： 输出的消息  // NOLINT
    *@param find_type[Input]：消息的查找方式  // NOLINT
    *@param message_data[Input]：消息查找时需要的查找信息  // NOLINT
    *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
    */
  virtual int Find(MsgInterfacePtr &message_ptr, int find_type, const BufferInterfacePtr& message_data);

  /**
    *@brief 返回消息队列中当前的消息数量//NOLINT
    */
  virtual int GetMessageAmount();

 protected:
   MutexLock         msg_map_lock_;   ///< message map lock
   MessageMap        msg_map_;        ///< message map

};
typedef boost::shared_ptr<MsgQueueImpl> MsgQueueImplPtr;

}   // namespace ysos

#endif  //  YSOS_MESSAGE_IMPL_H                                             //NOLINT
