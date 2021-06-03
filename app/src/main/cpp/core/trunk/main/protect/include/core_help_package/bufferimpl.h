/**
 *@file BufferImpl.h
 *@brief Definition of BufferImpl
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-21 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef CHP_BUFFER_H_  // NOLINT
#define CHP_BUFFER_H_  // NOLINT

/// ysos private headers //  NOLINT
#include "../../../public/include/sys_interface_package/bufferpool.h"  // NOLINT

namespace ysos {
/**
 *@brief BufferInterface的具体实现
 */
class YSOS_EXPORT BufferImpl : public BufferInterface {
  DISALLOW_COPY_AND_ASSIGN(BufferImpl);
 public:
  BufferImpl(BufferPoolInterfacePtr allocate, const UINT32 max_size);
  /**
   *@brief 将data_ptr封闭成BufferInterface类型对象
   */
  BufferImpl(uint8_t *data_ptr, const UINT32 data_length, bool is_delegate=false);
  virtual ~BufferImpl();

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
  /*
   @brief 设置当前Buffer的初始值  // NOLINT
   @param init_value:  初始值  // NOLINT
   @return:           成功返回实际的长度，否则返回0  // NOLINT
  */
  //  virtual int MemSet(const unsigned char init_value = 0);

 protected:
  
  UINT8                 *max_data_;  ///< 标记数据区的起始地址
  UINT8                 *cur_data_;  ///< 标记实现使用的数据起始地址
  UINT32                 cur_data_len_; ///< 标记实际使用的数据长度
  /// max_data_len >= cur_data_len_+prefix_len_
  /// 标记该Buffer的最大数据长度
  UINT32                 max_data_len_;  
  BufferPoolInterfacePtr allocator_;  ///< 标记Buffer的分配器Handle
  bool                   is_delegate_;  ///< 是否托管Data，默认为true
};

/**
 *@brief BufferPtr默认析构类函数
 */
class YSOS_EXPORT DeleteBufferFunction {
 public:
  void operator()(BufferInterface* buffer_ptr);
};
}
#endif  // SFP_BUFFER_H_  // NOLINT