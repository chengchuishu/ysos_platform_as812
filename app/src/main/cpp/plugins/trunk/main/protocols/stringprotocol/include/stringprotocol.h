/**
 *@file FinancialCommonProtocol.h
 *@brief Definition of financial common protocol
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-13 08:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef YSOS_PLUGIN_STRING_PROTOCOL_H_
#define  YSOS_PLUGIN_STRING_PROTOCOL_H_

/// Ysos Headers
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/baseprotocolimpl.h"
#include "../../../../../../core/trunk/main/public/include/core_help_package/jsonutility.h"

namespace ysos {

class StringProtocol: public BaseProtocolImpl {
  DISALLOW_COPY_AND_ASSIGN(StringProtocol)  //  禁止拷贝和复制
  DECLARE_PROTECT_CONSTRUCTOR(StringProtocol)  // 构造函数保护
  DECLARE_CREATEINSTANCE(StringProtocol)  // 定义类的全局静态创建函数

 public:
  virtual ~StringProtocol();

  /**
   *@brief 解析消息  // NOLINT
   *@param in_buffer[Input]： 输入的缓冲，内部包换该协议格式数据  // NOLINT
   *@param out_buffer[Output]：输出的缓冲，内部为应用需要的解析后数据  // NOLINT
   *@param format_id[Input]：由具体子类来自定义  // NOLINT
   *@param context_ptr[Input]：协议接口的上下文，与具体实现密切相关  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int ParseMessage(BufferInterfacePtr in_buffer, BufferInterfacePtr out_buffer, ProtocolFormatId format_id, void* context_ptr);

  /**
   *@brief 封装消息  // NOLINT
   *@param in_buffer[Input]： 输入的缓冲，内部为应用输入的具体数据  // NOLINT
   *@param out_buffer[Output]：输出的缓冲，内部为按照该协议封装后的格式数据  // NOLINT
   *@param format_id[Input]：由具体子类来自定义  // NOLINT
   *@param context_ptr[Input]：协议接口的上下文，与具体实现密切相关  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int FormMessage(BufferInterfacePtr in_buffer, BufferInterfacePtr out_buffer, ProtocolFormatId format_id, void* context_ptr);

  /**
   *@brief 获得协议中某种消息的长度，用于外部提前分配缓冲长度  //NOLINT
   *@param in_buffer[Input]： 输入的缓冲，内部为格式数据  //NOLINT
   *@param format_id[Input]：由具体子类来自定义  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  //NOLINT
   */
  virtual int GetLength(BufferInterfacePtr in_buffer, ProtocolFormatId format_id);

protected:
/**
   *@brief 根据Format协议，对Value进行格式化  // NOLINT
   *@param value[Input]： 待格式化的数据  // NOLINT
   *@param format[Input]： 协议名称  // NOLINT
   *@return： 成功返回Buffer指针，失败返回NULL  // NOLINT
   */
  BufferInterfacePtr FormatValue(const std::string &value, const std::string &format);
};

}

#endif