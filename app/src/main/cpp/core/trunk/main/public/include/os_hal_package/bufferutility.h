/**
 *@file BufferUtility.h
 *@brief Definition of Utility
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-21 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef CHP_BUFFER_UTILITY_H  //NOLINT
#define CHP_BUFFER_UTILITY_H  //NOLINT

/// stl headers //  NOLINT
#include <string>
#include <list>
#include <fstream>
/// boost headers //  NOLINT
#include <boost/shared_ptr.hpp>
/// ysos private headers //  NOLINT
#include "../../../public/include/sys_interface_package/baseinterface.h"
#include "../../../public/include/core_help_package/baseinterfaceimpl.h"
#include "../../../public/include/sys_interface_package/bufferpool.h"
#include "../../../public/include/sys_interface_package/msginterface.h"
#include "../../../protect/include/core_help_package/singletontemplate.h"

namespace ysos {
/**
  *@brief  工具类，使用方式：  //  NOLINT
  *        简化BufferInterface的使用 ... //  NOLINT
  */
class BufferUtility;
typedef boost::shared_ptr<BufferUtility>  BufferUtilityPtr;
class YSOS_EXPORT BufferUtility: public BaseInterfaceImpl {
  DISALLOW_COPY_AND_ASSIGN(BufferUtility);
  DECLARE_PROTECT_CONSTRUCTOR(BufferUtility);

 public:
  ~BufferUtility();

  /**
    *@brief  创建一个BuffferPoolPtr //  NOLINT
    *@param  prefix_size Bufffer Poool中Bufffer的Prefix值 //  NOLINT
    *@param  max_buffer_size  Bufffer Pool中Bufffer的最大长度值 //  NOLINT
    *@param  buffer_number Bufffer Poool中Bufffer的数量值 //  NOLINT
    *@return 成功返回BuffferPoolPtr指针，失败返回NULLL  //  NOLINT
    */
  BufferPoolInterfacePtr CreateBufferPool(const uint32_t max_buffer_size, const int buffer_number, const uint32_t prefix_size=0);
  /**
    *@brief  从BuffferPoool中获得一个Bufffer //  NOLINT
    *@param  bufffer_pool_ptr 目标BuffferPoool //  NOLINT
    *@return 成功返回BuffferPtr指针，失败返回NULLL  //  NOLINT
    */
  BufferInterfacePtr GetBufferFromBufferPool(const BufferPoolInterfacePtr &bufffer_pool_ptr);
  /**
    *@brief  初始化Buffer的值为initial_value //  NOLINT
    *@param  buffer_ptr 待初始化的buffer //  NOLINT
    *@param  initial_value 待初始化的内容 //  NOLINT
    *@return 成功返回0，其他失败  //  NOLINT
    */
  int InitialBuffer(const BufferInterfacePtr &buffer_ptr, const char initial_value=0);
  /**
    *@brief  创建一个长度为buffer_length,初始值为initial_value的Buffer //  NOLINT
    *@param  buffer_length Buffer的长度 //  NOLINT
    *@param  initial_value Buffer的初始值 //  NOLINT
    *@return 成功返回Buffer指针，失败否回NULL  //  NOLINT
    */
  BufferInterfacePtr CreateBuffer(const int buffer_length, const char initial_value=0);
  /**
    *@brief  创建一个以str为蓝本的Buffer //  NOLINT
    *@param  str 待创建的内容 //  NOLINT
    *@return 成功返回Buffer指针，失败否回NULL  //  NOLINT
    */
  BufferInterfacePtr CreateBuffer(const std::string &str);
  /**
    *@brief  克隆一个Buffer //  NOLINT
    *@param  src_buffer_ptr 待克隆的Buffer //  NOLINT
    *@return 成功返回Buffer指针，失败否回NULL  //  NOLINT
    */
  BufferInterfacePtr CloneBuffer(const BufferInterfacePtr &src_buffer_ptr);
  /**
  *@brief  将Buffer封装成BufferInterface类型 //  NOLINT
  *@param  buffer 待封装的Buffer //  NOLINT
  *@param  buffer_length Buffer的长度 //  NOLINT
  *@param  is_delegate Buffer的管理是否委托给BufferInterface //  NOLINT
  *@return 成功返回Buffer指针，失败否回NULL  //  NOLINT
    */
  BufferInterfacePtr WrapBuffer(uint8_t *&buffer, const int buffer_length, bool is_delegate=false);
  /**
    *@brief  从指定Buffer_ptr中获得数据指针 //  NOLINT
    *@param  buffer_ptr 目标Buffer //  NOLINT
    *@return 成功返回数据指针，失败否回NULL  //  NOLINT
    */
  uint8_t *GetBufferData(const BufferInterfacePtr &buffer_ptr);
  /**
  *@brief  从指定Buffer_ptr中获得Prefix数据长度 //  NOLINT
  *@param  buffer_ptr 目标Buffer //  NOLINT
  *@return 成功返回数据长度，失败否回0  //  NOLINT
    */
  int GetBufferPrefixLength(const BufferInterfacePtr &buffer_ptr);
  /**
  *@brief  设置Buffer的Prefix长度 //  NOLINT
  *@param  buffer_ptr 目标Buffer //  NOLINT
  *@param  prefix_length 要设置的Buffer的Prefix长度 //  NOLINT
  *@return 成功返回0，其余失败  //  NOLINT
    */
  int SetBufferPrefixLength(const BufferInterfacePtr &buffer_ptr, const uint32_t prefix_length);
  /**
  *@brief  从指定Buffer_ptr中获得数据长度 //  NOLINT
  *@param  buffer_ptr 目标Buffer //  NOLINT
  *@return 成功返回数据长度，失败否回0  //  NOLINT
    */
  int GetBufferLength(const BufferInterfacePtr &buffer_ptr);
  /**
  *@brief  设置Buffer的数据长度，Buffer的Prifix保持不变 //  NOLINT
  *@param  buffer_ptr 目标Buffer //  NOLINT
  *@param  buffer_length 要设置的Buffer长度 //  NOLINT
  *@return 成功返回0，其余失败  //  NOLINT
    */
  int SetBufferLength(const BufferInterfacePtr &buffer_ptr, const uint32_t buffer_length);
  /**
  *@brief  从指定Buffer_ptr中获得可用的最大数据长度（max_length - prefix_length） //  NOLINT
  *@param  buffer_ptr 目标Buffer //  NOLINT
  *@return 成功返回数据长度，失败否回0  //  NOLINT
    */
  int GetBufferUsableLength(const BufferInterfacePtr &buffer_ptr);
  /**
  *@brief  从指定Buffer_ptr中获得Buffer最大长度 //  NOLINT
  *@param  buffer_ptr 目标Buffer //  NOLINT
  *@return 成功返回数据长度，失败否回0  //  NOLINT
  */
  int GetBufferMaxLength(const BufferInterfacePtr &buffer_ptr);
  /**
  *@brief   将string中的内容，拷贝到指定的BufferInterface中//  NOLINT
  *@param  src 源string //  NOLINT
  *@param  out_buffer_ptr 目标BufferInterface //  NOLINT
  *@return 成功返回0，其余失败  //  NOLINT
    */
  int CopyStringToBuffer(const std::string &src, const BufferInterfacePtr &out_buffer_ptr);
  /**
    *@brief  创建一个MessageInterface指针 //  NOLINT
    *@param  message_head_ptr 消息头 //  NOLINT
    *@param  message_body_ptr 消息体 //  NOLINT
    *@return 成功返回Message指针，失败否回NULL  //  NOLINT
    */
/*  MessageInterfacePtr CreateMessage(MessageHeadPtr &message_head_ptr, MessageDataPtr &message_data_ptr);*/
  /**
    *@brief  获得指定消息头 //  NOLINT
    *@param  message_ptr 目标消息 //  NOLINT
    *@return 成功返回消息头，失败返回NULL  //  NOLINT
    */
  /*MessageHeadPtr GetMessageHead(const MessageInterfacePtr &message_ptr);*/
  /**
    *@brief  获得指定消息头 //  NOLINT
    *@param  message_ptr 目标消息 //  NOLINT
    *@return 成功返回消息头，失败返回NULL  //  NOLINT
    */
  /*MessageHeadPtr GetMessageHead(MessageInterface *message_ptr);*/
  /**
  *@brief  获得指定消息体 //  NOLINT
  *@param  message_ptr 目标消息 //  NOLINT
  *@return 成功返回消息体，失败返回NULL  //  NOLINT
    */
  /*MessageDataPtr GetMessageBody(const MessageInterfacePtr &message_ptr);*/
  /**
  *@brief  获得指定消息体 //  NOLINT
  *@param  message_ptr 目标消息 //  NOLINT
  *@return 成功返回消息体，失败返回NULL  //  NOLINT
    */
  /*MessageDataPtr GetMessageBody(MessageInterface *message_ptr);*/
  /**
  *@brief  从消息中获取消息体，再从消息体中获取到最终数据 //  NOLINT
  *@param  message_ptr 目标消息 //  NOLINT
  *@return 成功返回数据，失败返回NULL  //  NOLINT
    */
  /*BufferInterfacePtr GetDataFromMessageBody(const MessageInterfacePtr &message_ptr);*/
  /**
  *@brief  从消息中获取消息体，再从消息体中获取到最终数据 //  NOLINT
  *@param  message_ptr 目标消息 //  NOLINT
  *@return 成功返回数据，失败返回NULL  //  NOLINT
    */
  /*BufferInterfacePtr GetDataFromMessageBody(MessageInterface *message_ptr);*/
  /**
  *@brief  从消息队列中获取第一条消息 //  NOLINT
  *@param  message_queue_ptr 目标消息队列 //  NOLINT
  *@return 成功返回消息，失败返回NULL  //  NOLINT
    */
  MsgInterfacePtr GetMessageFromQueue(const MsgQueueInterfacePtr &msg_queue_ptr);

  DECLARE_SINGLETON_VARIABLE(BufferUtility);
};
#define GetBufferUtility  BufferUtility::Instance
}
#endif /* CHP_BUFFER_UTILITY_H */
