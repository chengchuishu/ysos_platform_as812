/**
 *@file doconfigdriver.h
 *@brief doconfig interface
 *@version 1.0
 *@author l.j..
 *@date Created on: 2016-10-20 13:48:23
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

#ifndef DO_CONFIG_INTERFACE_H_    //NOLINT
#define DO_CONFIG_INTERFACE_H_    //NOLINT

#ifdef _WIN32
#include <Windows.h>
//#include <WinSock2.h>
#include <Iphlpapi.h>
#else
#endif

#include <fstream>
#include <json/json.h>
#include <deque>

//ysos Headers    //NOLINT
#include "../../../../../../core/trunk/main/protect/include/os_hal_package/datainterfacemanager.h"
#include "../../../../../../core/trunk/main/public/include/core_help_package/baseinterfaceimpl.h"
#include "../../../../../../core/trunk/main/public/include/os_hal_package/driverinterface.h"
#include "../../../../../../core/trunk/main/public/include/sys_interface_package/common.h"
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"
#include "../../../../../../core/trunk/main/public/include/core_help_package/xmlutil.h"
//com ctrl Headers    //NOLINT

#include "../../../public/include/common/commonenumdefine.h"
#include "../../../public/include/common/commonstructdefine.h"


///tinyxml Headers
#include <tinyxml2/tinyxml2.h>

struct  LOG {           //< 日志相关信息
  std::string log_name_;
  std::string log_level_;
} *log_info;

namespace ysos {

/**
 *@brief  配置模块驱动层 //NOLINT
*/
class YSOS_EXPORT DoconfigDriver : virtual public DriverInterface,public BaseInterfaceImpl {
  DECLARE_CREATEINSTANCE(DoconfigDriver);
  DECLARE_PROTECT_CONSTRUCTOR(DoconfigDriver);

 public:
  virtual ~DoconfigDriver(void);
  /**
   *@brief 设置接口的属性值  // NOLINT
   *@param type_id[Input]： 属性的ID  // NOLINT
   *@param type[input]： 属性值的值  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int SetProperty(int type_id, void *type);
  /**
   *@brief 从底层驱动中读取数据  // NOLINT
   *@param pBuffer[Output]： 读取的数据缓冲  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Read(BufferInterfacePtr pBuffer, DriverContexPtr context_ptr=NULL);

  /**
   *@brief 写数据到底层驱动中  // NOLINT
   *@param pBuffer[Input]： 写入的数据缓冲  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Write(BufferInterfacePtr input_buffer, BufferInterfacePtr output_buffer=NULL, DriverContexPtr context_ptr=NULL);

  /**
   *@brief 控制驱动状态/配置参数到驱动/读取状态等  // NOLINT
   *@param iCtrlID[Input]： ID  // NOLINT
   *@param pInputBuffer[Input]： 写入的数据缓冲  // NOLINT
   *@param pOutputBuffer[Output]： 读出的数据缓冲  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Ioctl(int iCtrlID, BufferInterfacePtr pInputBuffer, BufferInterfacePtr pOutputBuffer);

  /**
   *@brief 打开底层驱动，并传入配置参数  // NOLINT
   *@param pParams[Input]： 驱动所需的打开参数  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Open(void *pParams);

  /**
   *@brief 关闭底层驱动，并传入配置参数  // NOLINT
   *@param ： 无  // NOLINT
   *@return： 无  // NOLINT
   */
  virtual void Close(void *pParams = nullptr);

  /**
   *@brief 修改xml文件值  // NOLINT
   *@param key[Input]： 字段名  // NOLINT
   *@param valeue[Input]： 字段值  // NOLINT
   *@param isModify[Input]： 是否修改， true 修改 false 添加  // NOLINT
   *@return： 无  // NOLINT
   */
  int ChangeXml(const std::string &key, const std::string &value, bool is_modify = true);
  /**
   *@brief 以utf8格式打开xml文件  // NOLINT
   *@return： 无  // NOLINT
   */
  int OpenXml();
  /**
   *@brief 获取机器ip地址  // NOLINT
   *@return： ip[Output]： 本机ip地址  // NOLINT
   */
  void GetIP(std::string &ip);
  /**
   *@brief 读取日志新信息  // NOLINT
   *@return： ip[Output]： 日志  // NOLINT
   */
  void My_ReadFile(const std::string &name, const std::string &level, std::string &info);

 private:
  std::string xml_path_;                   //< xml文件路径
  std::string search_module_;              //< 上抛的模块信息
  DataInterfacePtr data_ptr_;              //< 内存指针

  tinyxml2::XMLDocument *my_document_;       //< 解析xml
  std::deque<std::string> my_log_;           //< 日志队列
  bool is_read_;                             //< 是否读完日志

};///< End class DoconfigDriverInterface   //NOLINT

};

#endif  //DO_CONFIG_INTERFACE_H_