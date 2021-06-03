/**
 *@file base64protocolimpl.h
 *@brief Definition of Base64ProtocolImpl
 *@version 0.1
 *@author jinchengzhe
 *@date Created on: 2016-05-09 13:12:58
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef  SFP_YSOS_BASE64_PROTOCOL_IMPL_H_  // NOLINT
#define SFP_YSOS_BASE64_PROTOCOL_IMPL_H_  // NOLINT

/// Platform Headers
#include "../../../public/include/sys_interface_package/common.h"  // NOLINT
#include "../../../public/include/sys_interface_package/bufferpool.h" // NOLINT

#include "../../../public/include/core_help_package/utility.h"
#include "../../../public/include/core_help_package/baseinterfaceimpl.h" // NOLINT
#include "../../../public/include/sys_interface_package/protocolinterface.h" // NOLINT

namespace ysos {
/**
 *@brief Base64ProtocolImpl的具体实现  // NOLINT
 */
class YSOS_EXPORT Base64ProtocolImpl : public ProtocolInterface, public BaseInterfaceImpl {
  DISALLOW_COPY_AND_ASSIGN(Base64ProtocolImpl)  //  禁止拷贝和复制
  DECLARE_PROTECT_CONSTRUCTOR(Base64ProtocolImpl)  // 构造函数保护
  DECLARE_CREATEINSTANCE(Base64ProtocolImpl)  // 定义类的全局静态创建函数

 public:
  virtual ~Base64ProtocolImpl();

  /**
  *@brief  获取编码/解码所需内存大小  // NOLINT
  *@param in_buffer[IN]:  编码/解码数据  // NOLINT
  *@param format_id[IN]:  编码/解码类型  // NOLINT
  *@return:  返回编码/解码所需内存大小  // NOLINT
  */
  virtual int GetLength(BufferInterfacePtr in_buffer, ProtocolFormatId format_id);

  /**
  *@brief  解码Base64  // NOLINT
  *@param in_buffer[IN]:  编码数据  // NOLINT
  *@param out_buffer[IN]:  解码数据  // NOLINT
  *@param format_id[IN]:  缺省变量，函数内部不参照  // NOLINT
  *@param context_ptr[IN]:  缺省变量，函数内部不参照  // NOLINT
  *@return:  成功返回 0，否则返回 -1  // NOLINT
  */
  virtual int ParseMessage(BufferInterfacePtr in_buffer, BufferInterfacePtr out_buffer, ProtocolFormatId format_id, void *context_ptr);

  /**
  *@brief  编码Base64  // NOLINT
  *@param in_buffer[IN]:   解码数据  // NOLINT
  *@param out_buffe[IN]:  编码数据  // NOLINT
  *@param format_id[IN]:  缺省变量，函数内部不参照  // NOLINT
  *@param context_ptr[IN]:  缺省变量，函数内部不参照  // NOLINT
  *@return:  成功返回 0，否则返回 -1  // NOLINT
  */
  virtual int FormMessage(BufferInterfacePtr in_buffer, BufferInterfacePtr out_buffer, ProtocolFormatId format_id, void *context_ptr);

 private:
  /**
  *@brief 获取BufferInterfacePtr指向的实际内存指针  // NOLINT
  *@param buffer_ptr[IN]:  BufferInterfacePtr  // NOLINT
  *@param buffer_length_ptr[IN/OUT]:  Buffer长度指针  // NOLINT
  *@return:  BufferInterfacePtr指向的实际内存指针  // NOLINT
  */
  UINT8* GetBuffer(BufferInterfacePtr buffer_ptr, UINT32 * buffer_length_ptr);

  /**
  *@brief  编码Base64  // NOLINT
  *@param in_data_ptr[IN]:  解码数据  // NOLINT
  *@param in_data_length[IN]:  解码数据长度  // NOLINT
  *@return:  编码数据  // NOLINT
  */
  std::string Encode(const unsigned char* in_data_ptr, const int in_data_length);

  /**
  *@brief  编码Base64  // NOLINT
  *@param in_data_ptr[IN]:  解码数据  // NOLINT
  *@param in_data_length[IN]:  解码数据长度  // NOLINT
  *@param out_data_length[IN]: 编码数据长度  // NOLINT
  *@param out_data_ptr[IN/OUT]: 编码数据  // NOLINT
  *@return:  成功返回0，否则返回-1  // NOLINT
  */
  int Encode(const unsigned char* in_data_ptr, const int in_data_length, const UINT32 out_data_length, char* out_data_ptr);

  /**
  *@brief  解码Base64  // NOLINT
  *@param in_data_ptr[IN]:  编码数据  // NOLINT
  *@param in_data_length[IN]: 编码数据长度  // NOLINT
  *@param out_data_length[IN/OUT]: 解码数据长度  // NOLINT
  *@return:  解码数据  // NOLINT
  */
  std::string Decode(const char* in_data_ptr, const int in_data_length, int& out_data_length);

  /**
  *@brief  解码Base64  // NOLINT
  *@param in_data[IN]:  编码数据  // NOLINT
  *@param in_data_length[IN]: 编码数据长度  // NOLINT
  *@param out_data_length[IN]: 解码数据长度  // NOLINT
  *@param out_data[IN/OUT]: 解码数据长度  // NOLINT
  *@return:  成功返回 0，否则返回 -1  // NOLINT
  */
  int Decode(const char* in_data_ptr, const int in_data_length, const UINT32 out_data_length, char* out_data_ptr);

  /**
  *@brief  获取编码所需内存大小  // NOLINT
  *@param in_data_ptr[IN]:  解码数据  // NOLINT
  *@param in_data_length[IN]:  解码数据长度  // NOLINT
  *@return:  编码所需内存大小  // NOLINT
  */
  int GetEncodeBufferLength(const unsigned char* in_data_ptr, const int in_data_length);

  /**
  *@brief  获取解码所需内存大小  // NOLINT
  *@param in_data_ptr[IN]:  编码数据  // NOLINT
  *@param in_data_length[IN]: 编码数据长度  // NOLINT
  *@return:  解码所需内存大小  // NOLINT
  */
  int GetDecodeBufferLength(const char* in_data_ptr, const int in_data_length);
};

/**
 *@brief Base64ProtocolImpl的智能指针  // NOLINT
 */
typedef boost::shared_ptr<Base64ProtocolImpl> Base64ProtocolImplPtr;

/**
*@brief 生成Base64ProtocolImpl智能指针，用于外部参照  // NOLINT
*@return:  Base64ProtocolImpl的智能指针 // NOLINT
*/
extern Base64ProtocolImplPtr CreateInstanceOfBase64ProtocolImplInterface();
}

#endif // SFP_YSOS_BASE64_PROTOCOL_IMPL_H_  // NOLINT
