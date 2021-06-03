/**
 *@file baseconnection.h
 *@brief base connection interface
 *@version 1.0
 *@author dhongqian
 *@date Created on: 2016-06-23 10:06:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */


#ifndef SFP_BASE_CONNECTION_IMPL_H_  //NOLINT
#define SFP_BASE_CONNECTION_IMPL_H_  //NOLINT

/// Private Headers //  NOLINT
#include "../../../public/include/core_help_package/baseinterfaceimpl.h"
#include "../../../public/include/sys_interface_package/connectioninterface.h"
#include "../../../public/include/sys_interface_package/common.h"
///  Boost Headers       // NOLINT
#include <boost/function.hpp>

namespace ysos {

class YSOS_EXPORT BaseConnectionImpl : public ConnectionInterface, public BaseInterfaceImpl {
  DISALLOW_COPY_AND_ASSIGN(BaseConnectionImpl);
  DECLARE_CREATEINSTANCE(BaseConnectionImpl);
  DECLARE_PROTECT_CONSTRUCTOR(BaseConnectionImpl);

 public:

  virtual ~BaseConnectionImpl(void);

  /**
  *@brief 基本初始化  // NOLINT
  *@param param： 初始化的参数  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int Initialize(void *param=NULL);

  /**
   *@brief 读取数据函数  // NOLINT
   *@param input_buffer_ptr[Input]： 读取数据的缓冲，内部包含实际读取数据长度  // NOLINT
   *@param input_length[Input]： 缓冲长度  // NOLINT
   *@param output_buffer_ptr[Out]： 读取数据的缓冲，内部包含实际读取数据长度  // NOLINT
   *@param out_length[Out]： 缓冲长度  // NOLINT
   *@param context_ptr[Input]： 传输接口的上下文，与具体实现密切相关  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Read(BufferInterfacePtr input_buffer_ptr, int input_length, void* context_ptr = NULL);

  /**
  *@brief 写入数据函数  // NOLINT
  *@param input_buffer_ptr[Input]： 读取数据的缓冲，内部包含实际读取数据长度  // NOLINT
  *@param input_length[Input]： 缓冲长度  // NOLINT
  *@param output_buffer_ptr[Out]： 读取数据的缓冲，内部包含实际读取数据长度  // NOLINT
  *@param out_length[Out]： 缓冲长度  // NOLINT
  *@param context_ptr[Input]： 传输接口的上下文，与具体实现密切相关  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int Write(BufferInterfacePtr input_buffer_ptr, int input_length, void* context_ptr = NULL);

  /**
    *@brief 是否允许写入  // NOLINT
    *@param is_enable[Input]： 允许写入开关量  // NOLINT
    *@return： 无  // NOLINT
  */
  virtual void EnableWrite(bool is_enable = true);

  /**
    *@brief 是否允许读出  // NOLINT
    *@param is_enable[Input]： 允许读出开关量  // NOLINT
    *@return： 无  // NOLINT
  */
  virtual void EnableRead(bool is_enable = true);

  /**
  *@brief 是否封装数据  // NOLINT
  *@param is_enable[Input]： 是否封装开关量  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void EnableWrap(bool is_enable = true);

  /**
  *@brief 打开并初始化传输模块  // NOLINT
  *@param param[Input]： 输入的配置参数  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int Open(void *params);

  /**
  *@brief 关闭传输模块  // NOLINT
  *@param ： 无  // NOLINT
  *@return： 无  // NOLINT
  */
  virtual void Close(void *param=NULL);

 protected:
  /**
  *@brief  配置文件中设置的Key、Value对，会在Initialized函数中调用 //  NOLINT
  *@param key 配置参数中的Key  // NOLINT
  *@param value 与Key对应的Value  // NOLINT
  *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
  */
  virtual int Initialized(const std::string &key, const std::string &value);

 private:
  /**
  *@brief  读取配置文件中的参数，进行配置 //  NOLINT
  *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
  */
  int ConfigInitialize(void);

 protected:
  bool                                 is_enable_read_;   ///< 是否启用Read功能
  bool                                 is_enable_write_;   ///< 是否启用Write功能
  bool                                 is_enable_wrap_;     ///< 是否启用Wrap功能
  std::string                          remark_;        ///<  driver对象的备注  //  NOLINT
  std::string                          version_;       ///<  driver对象的版本号 //  NOLINT
};
typedef boost::shared_ptr<ysos::BaseConnectionImpl> BaseConnectionImplPtr;
} ///< namespace ysos

#endif  /// SFP_BASE_CONNECTION_IMPL_H_   //NOLINT