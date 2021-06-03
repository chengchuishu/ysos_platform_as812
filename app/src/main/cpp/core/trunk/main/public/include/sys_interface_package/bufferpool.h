/*
# Copyright (c) 2016 Steven.Shi. All rights reserved.
*/
#ifndef BUFFER_POOL_INTERFACE_H//NOLINT
#define BUFFER_POOL_INTERFACE_H//NOLINT

/// Boost Headers
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
/// Ysos Headers
#include "../../../public/include/sys_interface_package/common.h"
#include "../../../public/include/core_help_package/templatepool.h"
#include "../../../public/include/sys_framework_package/templatefactory.h"

namespace ysos {
class BufferInterface;
class BufferPoolInterface;
class BufferWrapInterface;

typedef boost::shared_ptr<BufferInterface> BufferInterfacePtr;
typedef boost::shared_ptr<BufferWrapInterface> BufferWrapInterfacePtr;
typedef boost::shared_ptr<BufferPoolInterface> BufferPoolInterfacePtr;

class YSOS_EXPORT BufferInterface/*: public boost::enable_shared_from_this<BufferInterface>*/ {
  IMPLEMENT_PUBLIC_EMPTY_DESTRUCTOR(BufferInterface)
 public:
   /**
   *@brief 获取实际的Buffer和最大长度//NOLINT
   *@param buffer： 实际的Buffer//NOLINT
   *@param length:  Buffer可用的最大长度//NOLINT
   *@return:           成功返回实际的长度，否则返回0//NOLINT
  */
  virtual int GetBufferAndLength(UINT8** buffer, UINT32 *length) = 0;
  /**
   *@brief 获取实际的Buffer的最大长度//NOLINT
   *@param length:  Buffer可用的最大长度//NOLINT
   *@return:           成功返回实际的长度，否则返回0//NOLINT
  */
  virtual int GetMaxLength(UINT32 *length) = 0;
  /*
    iLength: 当前Module实际需要使用的长度//NOLINT
    iPrefix: 当前Module需要预留的长度//NOLINT
    当前Module实际需求的Buffer大小是：iLength + iPrefix//NOLINT
  */
  virtual int SetLength(UINT32 length, UINT32 prefix = 0) = 0;
  /**
   *@brief 获取当前Buffer的前缀信息//NOLINT
   *@param prefix:  预留的前缀//NOLINT
   *@return:           成功返回实际的长度，否则返回0//NOLINT
   */
  virtual int GetPrefixLength(UINT32 *prefix) = 0;
  /*
   @brief 获取当前Buffer的管理器//NOLINT
   @param allocator:  Buffer的管理器//NOLINT
   @return:           成功返回0，否则返回错误码//NOLINT
  */
  virtual int GetAllocator(BufferPoolInterfacePtr *allocator) = 0;
  /*
   @brief 重置当前Buffer的管理器//NOLINT
   @param allocator:  Buffer的管理器//NOLINT
   @return:           成功返回0，否则返回错误码//NOLINT
  */
  virtual int ResetAllocator(BufferPoolInterfacePtr allocator=NULL) = 0;
  /*
   @brief 设置当前Buffer的初始值//NOLINT
   @param init_value:  初始值//NOLINT
   @return:           成功返回实际的长度，否则返回0//NOLINT
  */
  //  virtual int MemSet(const unsigned char init_value = 0){return 0;};
};

// 通过内存区(起点和尺寸)直接得到一个Buffer//NOLINT
// 赋给BufferInterfacePtr的BufferWrap应该是动态分配的.//NOLINT
class YSOS_EXPORT BufferWrapInterface : virtual public BufferInterface {
 public:
  // 将地址pBuffer指向的, 长为uLength字节的缓冲区作为buffer//NOLINT
  virtual int SetBuffer(UINT8 *pBuffer, UINT32 uLength) = 0;
};

typedef struct _AllocatorProperties {
  //  buffer number to be allocated
  UINT32 cBuffers;
  //  buffer size for single buffer
  UINT32 cbBuffer;
  //  buffer alignment for single buffer
  UINT32 cbAlign;
  //  the prefix length needed by single buffer, bufferpool不需要//NOLINT
  UINT32 cbPrefix;
} AllocatorProperties;

class YSOS_EXPORT BufferPoolInterface: public boost::enable_shared_from_this<BufferPoolInterface> {
  IMPLEMENT_PUBLIC_EMPTY_DESTRUCTOR(BufferPoolInterface)
 public:
  /*
  pRequest:用户(Module)请求的属性//NOLINT
  pActual: 实际分配的.//NOLINT
  当系统资源不足时, pActual值会与pRequest的不同//NOLINT
  */
  virtual int SetProperties(
    AllocatorProperties *request, AllocatorProperties *actual) = 0;
  /*
    获得当前的属性值//NOLINT
  */
  virtual int GetProperties(AllocatorProperties *props) = 0;
  /*
    执行真正的内存分配//NOLINT
  */
  virtual int Commit() = 0;
  /*
    释放所有已分配的内存//NOLINT
  */
  virtual int Decommit() = 0;
  /*
    请求一个空闲的内存块//NOLINT
  */
  virtual int GetBuffer(BufferInterfacePtr *buffer) = 0;
  /*
    将一个使用中的内存块归还内存池//NOLINT
  */
  virtual int ReleaseBuffer(BufferInterfacePtr buffer) = 0;
  /*
    !!!!!!!!!!!!!!!!!  这个接口慎用
    将一个使用中的内存块归还内存池//NOLINT
  */
  virtual int ReleaseBuffer(BufferInterface *buffer) = 0;
};

/*#define  CreateBufferPool(BUFFER_TYPE) boost::shared_ptr<BufferPoolImpl<BUFFER_TYPE> >(new BufferPoolImpl<BUFFER_TYPE>());//NOLINT*/

}// namespace ysos

#endif  //BUFFER_POOL_INTERFACE_H//NOLINT
