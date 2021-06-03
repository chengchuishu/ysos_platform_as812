/**
 *@file protocolinterface.h
 *@brief Definition of protocol
 *@version 0.1
 *@author Steven.Shi
 *@date Created on: 2016-04-13 08:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef PROTOCOL_INTERFACE_H  //NOLINT
#define PROTOCOL_INTERFACE_H  //NOLINT

#include "../../../public/include/sys_interface_package/common.h"
#include "../../../public/include/sys_interface_package/bufferpool.h"
#include "../../../public/include/sys_interface_package/baseinterface.h"
#include <boost/shared_ptr.hpp>

namespace ysos {

  /**
   *@brief 协议接口，
     用于对输入数据进行格式解析，对输出数据进行格式封装，
     例如Base64，XML，Json等格式
   */
class YSOS_EXPORT ProtocolInterface : virtual public BaseInterface {
  IMPLEMENT_PUBLIC_EMPTY_DESTRUCTOR(ProtocolInterface)
 public:
  enum Properties {
    IN_DATA_TYPES = 0,
    OUT_DATA_TYPES,
  };
  typedef uint64_t ProtocolFormatId;

  /**
   *@brief 解析消息  // NOLINT
   *@param in_buffer[Input]： 输入的缓冲，内部包换该协议格式数据  // NOLINT
   *@param out_buffer[Output]：输出的缓冲，内部为应用需要的解析后数据  // NOLINT
   *@param format_id[Input]：由具体子类来自定义  // NOLINT
   *@param context_ptr[Input]：协议接口的上下文，与具体实现密切相关  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int ParseMessage(
    BufferInterfacePtr in_buffer,
    BufferInterfacePtr out_buffer,
    ProtocolFormatId format_id,
    void* context_ptr) = 0;

  /**
   *@brief 封装消息  // NOLINT
   *@param in_buffer[Input]： 输入的缓冲，内部为应用输入的具体数据  // NOLINT
   *@param out_buffer[Output]：输出的缓冲，内部为按照该协议封装后的格式数据  // NOLINT
   *@param format_id[Input]：由具体子类来自定义  // NOLINT
   *@param context_ptr[Input]：协议接口的上下文，与具体实现密切相关  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int FormMessage(
    BufferInterfacePtr in_buffer,
    BufferInterfacePtr out_buffer,
    ProtocolFormatId format_id,
    void* context_ptr) = 0;

  /**
   *@brief 获得协议中某种消息的长度，用于外部提前分配缓冲长度  //NOLINT
   *@param in_buffer[Input]： 输入的缓冲，内部为格式数据  //NOLINT
   *@param format_id[Input]：由具体子类来自定义  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  //NOLINT
   */
  virtual int GetLength(
    BufferInterfacePtr in_buffer,
    ProtocolFormatId format_id) = 0;
};

typedef boost::shared_ptr<ProtocolInterface > ProtocolInterfacePtr;

} // namespace ysos

#endif  // PROTOCOL_INTERFACE_H  //NOLINT
