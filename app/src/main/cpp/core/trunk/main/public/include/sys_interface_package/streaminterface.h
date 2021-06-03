/**
 *@file streaminterface.h
 *@brief Definition of streaminterface
 *@version 0.1
 *@author XuLanyue
 *@date Created on: 2016-04-27 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef SIP_STREAM_INTERFACE_H                                                  //NOLINT
#define SIP_STREAM_INTERFACE_H                                                  //NOLINT

/// Boost Headers
#include <boost/shared_ptr.hpp>
/// Ysos Headers
#include "../../../public/include/sys_interface_package/common.h"
//#include "../../../public/include/core_help_package/templatefactory.h"
#include "../../../public/include/sys_interface_package/bufferpool.h"

namespace ysos {
/**
 *@brief 数据输入输出的流接口.                                                      //NOLINT
 *适用于数据量很大的情形,如图像和视频,一般利用通用的RTP/RTMP等通用格式传输编码流或裸数据       //NOLINT
 */
class YSOS_EXPORT StreamInterface : virtual public BufferInterface {
  IMPLEMENT_PUBLIC_EMPTY_DESTRUCTOR(StreamInterface)
 public:
  enum StreamSeekType {
    STREAM_SEEK_SET = 0,
    STREAM_SEEK_CUR,
    STREAM_SEEK_END
  };

  /**
   *@brief 试图将指定数量的字节写入流                                                //NOLINT
   *@param [in]data_ptr: 将其所指数据,写入流                                        //NOLINT
   *@param [in]count_of_byte_to_write: 要写入流的字节数                            //NOLINT
   *@param [out]count_of_byte_written_ptr: 若非空则其所指置为实际写入流的字节数        //NOLINT
   *@return: 若成功则返回YSOS_ERROR_SUCCESS,否则返回非YSOS_ERROR_SUCCESS            //NOLINT
  */
  virtual int Write(
    uint8_t* data_ptr,
    uint32_t count_of_byte_to_write,
    uint32_t* count_of_byte_written_ptr) = 0;

  /**
   *@brief 试图从流读入指定数量的字节                                                //NOLINT
   *@param [out]data_ptr: 自流所读的数据,写入其所指                                  //NOLINT
   *@param [in]count_of_byte_to_read: 要读自流的字节数                             //NOLINT
   *@param [out]count_of_byte_read_ptr: 若非空则其所指置为实际读自流的字节数           //NOLINT
   *@return: 若成功则返回YSOS_ERROR_SUCCESS,否则返回非YSOS_ERROR_SUCCESS            // NOLINT
  */
  virtual int Read(
    uint8_t* data_ptr,
    uint32_t count_of_byte_to_read,
    uint32_t* count_of_byte_read_ptr) = 0;

  /**
   *@brief 试图将流的当前查找指针更改到相对于iOrigin(参考StreamSeekType)                //NOLINT
   *偏移量为iOffset的新位置.                                                       //NOLINT
   *@param [in]offset: 偏移量字节数自流所读的数据,写入其所指                           //NOLINT
   *@param [in]origin: 偏移量相对于(流的开头(0), 当前查找指针(1), 流的结尾(2))          //NOLINT
   *@param [out]new_position_ptr: 若非空则其所指置为流的当前查找指针相对于流的开头的偏移量 //NOLINT
   *@return: 若成功则返回YSOS_ERROR_SUCCESS,否则返回非YSOS_ERROR_SUCCESS            // NOLINT
  */
  virtual int Seek(
    int32_t offset,
    int32_t origin,
    uint32_t* new_position_ptr) = 0;

  /**
   *@brief 试图更改流对象的大小                                                     //NOLINT
   *@param [in]new_size: 流的新大小,以字节数表示                                    //NOLINT
   *@return: 若成功则返回YSOS_ERROR_SUCCESS,否则返回非YSOS_ERROR_SUCCESS            // NOLINT
  */
  virtual int SetSize(
    uint32_t new_size) = 0;

  /**
   *@brief 在流中还剩余数据时只读不写                                                //NOLINT
  */
  virtual void Flush() = 0;

  /**
   *@brief 丢弃流中剩余的数据                                                       //NOLINT
  */
  virtual void Discard() = 0;
};

typedef boost::shared_ptr<StreamInterface> StreamInterfacePtr;

}   // namespace ysos

#endif  //  SIP_STREAM_INTERFACE_H                                              //NOLINT
