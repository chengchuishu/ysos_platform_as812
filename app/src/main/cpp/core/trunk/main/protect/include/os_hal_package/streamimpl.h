/**
 *@file streamimpl.h
 *@brief Definition of streamimpl based on Buffer
 *@version 0.1
 *@author XuLanyue
 *@date Created on: 2016-04-27 08:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef STREAM_IMPL_H                                                           //NOLINT
#define STREAM_IMPL_H                                                           //NOLINT

#include "../../../public/include/os_hal_package/lock.h"
#include "../../../public/include/sys_interface_package/common.h"
#include "../../../public/include/sys_interface_package/streaminterface.h"
#include <boost/shared_ptr.hpp>

namespace ysos {

class YSOS_EXPORT StreamImpl : public StreamInterface {
 public:
  enum StreamStatus {
    Error                   = 0,
    BufferNotSpecified      = 1,
    Empty                   = 2,
    Normal                  = 3,
    Full                    = 4,
  };
  StreamImpl(BufferInterface* buffer_ptr,
    uint32_t usable_data_length_at_init = 0);
  virtual ~StreamImpl();
  virtual int GetBufferAndLength(UINT8** ppBuffer, UINT32 *piLength);
  virtual int GetMaxLength(UINT32 *piLength);
  virtual int SetLength(UINT32 iLength, UINT32 iPrefix = 0);
  virtual int GetPrefixLength(UINT32 *piPrefix);
  virtual int GetAllocator(BufferPoolInterfacePtr *ppAllocator);

//  试图将指定数量的字节写入从当前查找指针开始的流对象,                                   //NOLINT
//  若成功,则移动当前写指针,返回YSOS_ERROR_SUCCESS,                                  //NOLINT
//  若puCountOfBytesWritten非空则其所指置为实际写入流的字节数;                         //NOLINT
//  否则返回YSOS_ERROR_FAILED.//NOLINT
  virtual int Write(
    uint8_t* pData,                             //要将此写入流                    //NOLINT
    uint32_t uCountOfBytesToWrite,              //要写入流的字节数                 //NOLINT
    uint32_t* puCountOfBytesWritten);           //指向实际写入流的字节数            //NOLINT

//  试图从流对象的当前查找指针开始的内存起,读入指定数量的字节                              //NOLINT
//  若成功,则移动当前读指针,返回YSOS_ERROR_SUCCESS,                                  //NOLINT
//  若puCountOfBytesRead非空则其所指置为实际读自流的字节数;                            //NOLINT
//  否则返回YSOS_ERROR_FAILED.//NOLINT
  virtual int Read(
    uint8_t* pData,                             //要将此写入流                    //NOLINT
    uint32_t uCountOfBytesToRead,               //要读自流的字节数                 //NOLINT
    uint32_t* puCountOfBytesRead);              //指向实际读自流的字节数            //NOLINT

//  试图将流对象的当前查找指针更改到相对于iOrigin                                       //NOLINT
//  (流的开头(0), 当前查找指针(1), 流的结尾(2))偏移量为iOffset的新位置.                  //NOLINT
//  若成功,则返回YSOS_ERROR_SUCCESS,                                              //NOLINT
//  若pNewPosition非空则其所指置为流的当前查找指针相对于流的开头的偏移量字节数;             //NOLINT
//  否则返回YSOS_ERROR_FAILED.                                                    //NOLINT
  virtual int Seek(
    int32_t iOffset,                            //偏移量字节数                    //NOLINT
    int32_t iOrigin,                            //偏移量相对于                    //NOLINT
    uint32_t* pNewPosition);                    //指向新的偏移量字节数              //NOLINT

//  更改流对象的大小                                                               //NOLINT
//  若成功,则返回YSOS_ERROR_SUCCESS,                                              //NOLINT
//  否则返回YSOS_ERROR_FAILED.                                                    //NOLINT
  virtual int SetSize(
    uint32_t uNewLength);                       //流的新大小,以字节数表示           //NOLINT

//  确保对在事务处理模式下打开的流对象所做的任何更改都能反映在父级存储中.                     //NOLINT
//  若成功,则返回YSOS_ERROR_SUCCESS,                                              //NOLINT
//  否则返回YSOS_ERROR_FAILED.                                                    //NOLINT
  virtual void Flush();

//  放弃自上次Flush调用以来对流所做的更改.                                            //NOLINT
//  若成功,则返回YSOS_ERROR_SUCCESS,                                              //NOLINT
//  否则返回YSOS_ERROR_FAILED.                                                    //NOLINT
  virtual void Discard();

 private:
  //  参见StreamStatus                                                           //NOLINT
  int m_iStatus;

  //  Lock
  LightLock m_oLock;

  //  BufferInterface指针                                                        //NOLINT
  BufferInterface* m_pBufferInterface;

  //  将原始Buffer解释为一个circular_buffer                                        //NOLINT
  uint8_t* m_pBuffer;

  //  原始最大长度                                                                //NOLINT
  uint32_t m_uMaxBufLen;

  //  当前缓冲区长度                                                               //NOLINT
  uint32_t m_uCurBufLen;
  uint32_t m_uPrefixLen;

  //  当前队列长度                                                                
  uint32_t m_uCurQueueLen;  ///< 当前队列长度//NOLINT

  //  [m_uCurQueueBegin, m_uCurQueueEnd)
  uint32_t m_uCurQueueBegin;
  uint32_t m_uCurQueueEnd;

  ///  在Seek之后, 如果m_bRefreshReadRequired为true, 那么在Read之后, 更新为false       //NOLINT
  bool m_bRefreshReadRequired;

  ///  当前读指针                                                                  //NOLINT
  uint32_t m_uCurReadPos;

  ///  当前写指针                                                                  //NOLINT
  uint32_t m_uCurWritePos;

  ///  若Flush了, 则置为true.     //NOLINT
  bool flush_flag_;

  //  若Discard了, 则置为true.   //NOLINT
  bool discard_flag_;
};

typedef boost::shared_ptr<StreamImpl> StreamImplPtr;

}   // namespace ysos

#endif  //  STREAM_IMPL_H                                                       //NOLINT
