/**
*@file streamimpl.cpp
*@brief Implement of streamimpl based on Buffer
*@date Created on: 2016-04-28 13:59:20
*@author Original author: XuLanyue
*@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
* 
*/
#include "../../../public/include/sys_interface_package/common.h"
#include "../../../protect/include/os_hal_package/streamimpl.h"
#include "../../../public/include/sys_interface_package/streaminterface.h"
#include <boost/shared_ptr.hpp>

namespace ysos {
StreamImpl::StreamImpl(BufferInterface* buffer_ptr,
  uint32_t usable_data_length_at_init) {
  m_iStatus = BufferNotSpecified;
  //  m_oLock;
  bool bValid = false;
  uint8_t* pBuffer = NULL;
  uint32_t uMaxBufLen = 0;
  if (buffer_ptr) {
    buffer_ptr->GetBufferAndLength(&pBuffer, &uMaxBufLen);
    if (pBuffer && uMaxBufLen > 0)
    {
      bValid = true;
    }
  }
  if (bValid) {
    m_pBufferInterface = buffer_ptr;
    m_pBuffer = pBuffer;
    m_uMaxBufLen = uMaxBufLen;
    m_uCurQueueLen = 0;
    if (usable_data_length_at_init == 0) {
      m_iStatus = Empty;
    } else if (usable_data_length_at_init >= m_uMaxBufLen) {
      m_iStatus = Full;
      m_uCurQueueLen = m_uMaxBufLen;
    } else {
      m_iStatus = Normal;
      m_uCurQueueLen = usable_data_length_at_init;
    }
    m_uCurBufLen = m_uMaxBufLen;
    m_uCurQueueBegin = 0;
    m_uCurQueueEnd = m_uMaxBufLen + 1;
  } else {
    m_pBufferInterface = NULL;
      m_iStatus = Empty;
    m_pBuffer = NULL;
    m_uMaxBufLen = 0;
    m_uCurBufLen = 0;
    m_uCurQueueLen = 0;
    m_uCurQueueBegin = 0;
    m_uCurQueueEnd = 0;
  }
  flush_flag_ = false;
  m_bRefreshReadRequired = false;
  m_uCurReadPos = 0;
  m_uCurWritePos = 0;
}

StreamImpl::~StreamImpl() {
  //
}

int StreamImpl::GetBufferAndLength(UINT8** ppBuffer, UINT32 *piLength) {
  int ret_value = YSOS_ERROR_INVALID_ARGUMENTS;
  if (m_pBufferInterface) {
    ret_value = m_pBufferInterface->GetBufferAndLength(ppBuffer, piLength);
  }
  return ret_value;
}

int StreamImpl::GetMaxLength(UINT32 *piLength) {
  int ret_value = YSOS_ERROR_INVALID_ARGUMENTS;
  if (m_pBufferInterface) {
    ret_value = m_pBufferInterface->GetMaxLength(piLength);
  }
  return ret_value;
}

int StreamImpl::SetLength(UINT32 iLength, UINT32 iPrefix) {
  int return_value = YSOS_ERROR_INVALID_ARGUMENTS;
  if (iLength > 0 && iLength + iPrefix <= m_uMaxBufLen) {
    AutoLockOper autoLock(&m_oLock);
    m_uCurBufLen = iLength + iPrefix;
    m_uPrefixLen = iPrefix;
    return_value = YSOS_ERROR_SUCCESS;
  }
  return return_value;
}

int StreamImpl::GetPrefixLength(UINT32 *piPrefix) {
  int return_value = YSOS_ERROR_INVALID_ARGUMENTS;
  if (piPrefix != NULL) {
    AutoLockOper autoLock(&m_oLock);
    *piPrefix = m_uPrefixLen;
    return_value = YSOS_ERROR_SUCCESS;
  }
  return return_value;
}

int StreamImpl::GetAllocator(BufferPoolInterfacePtr *ppAllocator) {
  int return_value = YSOS_ERROR_INVALID_ARGUMENTS;
  if (ppAllocator != NULL) {
    if (m_pBufferInterface) {
      m_pBufferInterface->GetAllocator(ppAllocator);
    } else {
      *ppAllocator = NULL;
    }
    return_value = YSOS_ERROR_SUCCESS;
  }
  return return_value;
}

//  试图将指定数量的字节写入从当前查找指针开始的流对象,                                   //NOLINT
//  若成功,则移动当前写指针,返回YSOS_ERROR_SUCCESS,                                  //NOLINT
//  若puCountOfBytesWritten非空则其所指置为实际写入流的字节数;                         //NOLINT
//  否则返回YSOS_ERROR_FAILED.//NOLINT
int StreamImpl::Write(
  uint8_t* pData,                               //要将此写入流                    //NOLINT
  uint32_t uCountOfBytesToWrite,                //要写入流的字节数                 //NOLINT
  uint32_t* puCountOfBytesWritten) {            //指向实际写入流的字节数            //NOLINT
  int return_value = YSOS_ERROR_INVALID_ARGUMENTS;
  if (flush_flag_ == true) {
    return YSOS_ERROR_FAILED;
  }
  if (pData != NULL && uCountOfBytesToWrite > 0) {
    AutoLockOper autoLock(&m_oLock);

    //  最多可写多少字节?                                                          //NOLINT
    uint32_t uMaxToWrite = m_uCurBufLen - m_uCurQueueLen;
    if (uMaxToWrite > 0) {
      if (uMaxToWrite > uCountOfBytesToWrite) {
        uMaxToWrite = uCountOfBytesToWrite;
      }
      //  若写指针已到了缓冲区的尾部则移到头部.                                        //NOLINT
      if (m_uCurWritePos >= m_uCurBufLen) {
        m_uCurWritePos = 0;
      }

      //  因为是循环缓冲区, 所以需要计算分为两部分复制时各部分的长度.                      //NOLINT
      uint32_t uPart1 = m_uCurBufLen - m_uCurWritePos;
      uint32_t uPart2 = 0;
      if (uMaxToWrite > uPart1) {
        uPart2 = uMaxToWrite - uPart1;
      } else {
        uPart1 = uMaxToWrite;
      }

      //  写到流                                                                 //NOLINT
      memcpy(m_pBuffer + m_uCurWritePos, pData, uPart1);
      if (uPart2 > 0) {
        memcpy(m_pBuffer, pData + uPart1, uPart2);
      }

      //  更新队列的长度和写指针的位置                                                //NOLINT
      m_uCurQueueLen += uMaxToWrite;
      if (m_uCurQueueLen == m_uCurBufLen) {
        m_iStatus = Full;
      } else {
        m_iStatus = Normal;
      }
      m_uCurWritePos = (m_uCurWritePos + uMaxToWrite) % m_uCurBufLen;
      m_uCurQueueEnd = m_uCurWritePos;

      if (puCountOfBytesWritten != NULL) {
        *puCountOfBytesWritten = uMaxToWrite;
      }
    } else {                                    //  FULL
      if (puCountOfBytesWritten != NULL) {
        *puCountOfBytesWritten = 0;
      }
    }
    return_value = YSOS_ERROR_SUCCESS;
  }
  return return_value;
}

//  试图从流对象的当前查找指针开始的内存起,读入指定数量的字节                              //NOLINT
//  若成功,则移动当前读指针,返回YSOS_ERROR_SUCCESS,                                  //NOLINT
//  若puCountOfBytesRead非空则其所指置为实际读自流的字节数;                            //NOLINT
//  否则返回YSOS_ERROR_FAILED.                                                    //NOLINT
int StreamImpl::Read(
  uint8_t* pData,                             //要将此写入流                      //NOLINT
  uint32_t uCountOfBytesToRead,               //要读自流的字节数                   //NOLINT
  uint32_t* puCountOfBytesRead) {             //指向实际读自流的字节数              //NOLINT
  int return_value = YSOS_ERROR_INVALID_ARGUMENTS;
  if (pData != NULL && uCountOfBytesToRead > 0) {
    AutoLockOper autoLock(&m_oLock);
    if (puCountOfBytesRead != NULL) {
      *puCountOfBytesRead = 0;
    }
    //  最多可读多少字节?                                                          //NOLINT
    uint32_t uMaxToRead = m_uCurQueueLen;
    if (uMaxToRead > 0) {
      if (uMaxToRead > uCountOfBytesToRead) {
        uMaxToRead = uCountOfBytesToRead;
      }
        //  若读指针已到了缓冲区的尾部则移到头部.                                      //NOLINT
        if (m_uCurReadPos >= m_uCurBufLen) {
          m_uCurReadPos = 0;
        }

        //  因为是循环缓冲区, 所以需要计算分为两部分复制时各部分的长度.                    //NOLINT
        uint32_t uPart1 = m_uCurBufLen - m_uCurReadPos;
        uint32_t uPart2 = 0;
        if (uMaxToRead > uPart1) {
          uPart2 = uMaxToRead - uPart1;
        } else {
          uPart1 = uMaxToRead;
        }

        //  读自流                                                               //NOLINT
        memcpy(pData, m_pBuffer + m_uCurReadPos, uPart1);
        if (uPart2 > 0) {
          memcpy(pData + uPart1, m_pBuffer, uPart2);
        }

        //  更新队列的长度和读指针的位置                                                //NOLINT
        m_uCurQueueLen -= uMaxToRead;
        if (m_uCurQueueBegin == 0) {
          flush_flag_ = false;
        }
        if (m_uCurQueueLen == 0) {
          m_iStatus = Empty;
        } else {
          m_iStatus = Normal;
        }
        m_uCurReadPos = (m_uCurReadPos + uMaxToRead) % m_uCurBufLen;
        m_uCurQueueBegin = m_uCurReadPos;

        if (puCountOfBytesRead != NULL) {
          *puCountOfBytesRead = uMaxToRead;
        }
      }
    //} else {                                    //  EMPTY
    //  if (puCountOfBytesRead != NULL) {
    //    *puCountOfBytesRead = 0;
    //  }
    //}
    return_value = YSOS_ERROR_SUCCESS;
  }
  return return_value;
}

//  试图将流对象的当前查找指针更改到相对于iOrigin                                       //NOLINT
//  (流的开头(0), 当前查找指针(1), 流的结尾(2))偏移量为iOffset的新位置.                  //NOLINT
//  若成功,则返回YSOS_ERROR_SUCCESS,                                              //NOLINT
//  若pNewPosition非空则其所指置为流的当前查找指针相对于流的开头的偏移量字节数;             //NOLINT
//  否则返回YSOS_ERROR_FAILED.                                                    //NOLINT
int StreamImpl::Seek(
  int32_t iOffset,                            //偏移量字节数                      //NOLINT
  int32_t iOrigin,                            //偏移量相对于                      //NOLINT
  uint32_t* pNewPosition) {                   //指向新的偏移量字节数                //NOLINT
  int return_value = YSOS_ERROR_SUCCESS;
  bool bForRead = true;
  bool bForWrite = true;
  AutoLockOper autoLock(&m_oLock);
  int32_t uCurQueueLen = (int32_t)m_uCurQueueLen;
  if (iOrigin == STREAM_SEEK_SET) {
    if (iOffset == 0) {
      if (pNewPosition != NULL) {
        *pNewPosition = m_uCurReadPos;
      }
    } else if (iOffset > 0) {
      if (iOffset > uCurQueueLen) {
        //  超出尾部                                      //NOLINT
        return_value = YSOS_ERROR_FAILED;
      } else {
        //  一旦做了Seek, 后续有读操作时队列的长度将发生变化    //NOLINT
        m_uCurReadPos = iOffset;
        //m_uCurQueueLen -= iOffset;
        //m_uCurQueueBegin += iOffset;
        m_bRefreshReadRequired = true;
      }
    } else if (iOffset < 0) {
      return_value = YSOS_ERROR_FAILED;
    }
  } else if (iOrigin == STREAM_SEEK_CUR) {
    if (iOffset == 0) {
      if (pNewPosition != NULL) {
        *pNewPosition = m_uCurReadPos;
      }
    } else {
      int32_t iLenPart1 = (m_uCurReadPos + m_uCurQueueLen - m_uCurQueueBegin) % m_uCurQueueLen;
      if (iLenPart1 == 0 && m_uCurReadPos != m_uCurQueueBegin) {
        iLenPart1 = m_uCurQueueLen;
      }
      int32_t iLenPart2 = m_uCurQueueLen - iLenPart1;
      if (iLenPart1 + iOffset < 0 || iOffset > iLenPart2) {
        //  超出范围                                      //NOLINT
        return_value = YSOS_ERROR_FAILED;
      } else {
        //  一旦做了Seek, 后续有读操作时队列的长度将发生变化    //NOLINT
        m_uCurReadPos = m_uCurReadPos + iOffset;
        *pNewPosition = m_uCurReadPos;
        m_bRefreshReadRequired = true;
      }
    }
  } else if (iOrigin == STREAM_SEEK_END) {
    if (iOffset > 0 || uCurQueueLen + iOffset < 0) {
      return_value = YSOS_ERROR_FAILED;
    } else if (iOffset == 0) {
      m_uCurReadPos = m_uCurQueueEnd;
      *pNewPosition = m_uCurReadPos;
      m_bRefreshReadRequired = true;
    } else {
      return_value = YSOS_ERROR_INVALID_ARGUMENTS;
      m_uCurReadPos = m_uCurQueueEnd + iOffset;
      *pNewPosition = m_uCurReadPos;
      m_bRefreshReadRequired = true;
    }
  }
  return return_value;
}

//  更改流对象的大小                                                               //NOLINT
//  若成功,则返回YSOS_ERROR_SUCCESS,                                              //NOLINT
//  否则返回YSOS_ERROR_FAILED.                                                    //NOLINT
int StreamImpl::SetSize(
  uint32_t uNewLength) {                      //流的新大小,以字节数表示             //NOLINT
  int return_value = YSOS_ERROR_INVALID_ARGUMENTS;
  if (uNewLength > 0 && uNewLength <= m_uMaxBufLen) {
    //  安全考虑: 是否存在引用                                                      //NOLINT
    m_iStatus = Empty;
    m_uCurBufLen = uNewLength;
    m_uCurQueueLen = 0;
    m_uCurReadPos = 0;
    m_uCurWritePos = 0;
    return_value = YSOS_ERROR_SUCCESS;
  }
  return return_value;
}

//  确保对在事务处理模式下打开的流对象所做的任何更改都能反映在父级存储中.                     //NOLINT
//  若成功,则返回YSOS_ERROR_SUCCESS,                                              //NOLINT
//  否则返回YSOS_ERROR_FAILED.                                                    //NOLINT
void StreamImpl::Flush() {
  AutoLockOper autoLock(&m_oLock);
  flush_flag_ = true;
}

//  放弃自上次Flush调用以来对流所做的更改.                                            //NOLINT
//  若成功,则返回YSOS_ERROR_SUCCESS,                                              //NOLINT
//  否则返回YSOS_ERROR_FAILED.                                                    //NOLINT
void StreamImpl::Discard() {
  //  安全考虑: 是否存在引用                                                        //NOLINT
  AutoLockOper autoLock(&m_oLock);
  m_iStatus = Empty;
  m_uCurQueueLen = 0;
  m_uCurReadPos = 0;
  m_uCurWritePos = 0;
  flush_flag_ = false;
}

//  StreamInterfacePtr CreateInstanceOfStreamInterfacePtr() {
//    return StreamImplPtr(new StreamImpl());
//  }

}   // namespace ysos
