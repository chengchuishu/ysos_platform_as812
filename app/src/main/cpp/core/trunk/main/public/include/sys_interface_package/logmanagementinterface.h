/**
 *@file logmanagementinterface.h
 *@brief Definition of logmanagementinterface
 *@version 0.1
 *@author XuLanyue
 *@date Created on: 2016-05-17 19:00:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef LOG_MANAGEMENT_INTERFACE_H          //NOLINT
#define LOG_MANAGEMENT_INTERFACE_H          //NOLINT

#include "../../../public/include/sys_interface_package/baseinterface.h"
#include <boost/shared_ptr.hpp>

/// 日志选项                                      //NOLINT
#define LOG_OPTIONS_ENCRYPT                     1   ///< 加密日志

namespace ysos {

class BaseInterface;

/**
   *@brief 数据管理接口, 实现数据的装入/查找/读/写机制, 可用于日志管理接口             //NOLINT
   */
class LogManagementInterface : virtual public BaseInterface {
 public:
   ///error,
   ///info,
   ///debug,
   ///trace,
   ///all

  /**
   *@brief 装入日志                                                           //NOLINT
   *@param full_path_file_name[Input]: 日志的文件全路径文件名.                  //NOLINT
   *@return: 若成功则返回YSOS_ERROR_SUCCESS, 否则返回相应错误值.                  //NOLINT
   */
  virtual int Load(
    const std::string &full_path_file_name) = 0;

  /**
   *@brief 同步查找                                                           // NOLINT
   *@param data_to_find[Input]: 要查找的数据.                                 //NOLINT
   *@param options[Input]: 查找数据时使用的选项, 默认为0.                        //NOLINT
   *@return: 若成功则返回查找到的数据的位置, 否则返回-1.                            //NOLINT
   */
  virtual int64_t SearchLog(
    const std::string &data_to_find) = 0;

  /**
   *@brief 写入数据                                                           //NOLINT
   *@param data_key[Input]: 要写入数据的key.                                  //NOLINT
   *@param content[Input]: 要写入的内容.                                      //NOLINT
   *@param level[Input]: 日志级别, 默认为"all", 意为全部.                       //NOLINT
   *@param options[Input]: 写入数据时使用的选项, 默认为0.                        //NOLINT
   *@return: 若成功则返回YSOS_ERROR_SUCCESS, 否则返回相应错误值.                  //NOLINT
   */
  virtual int WriteLog(
    const void* content,
    const std::string &level = "all",
    uint32_t options = 0) = 0;
};

typedef boost::shared_ptr<LogManagementInterface> LogManagementInterfacePtr;

#if 1   //  1, DeclareManagerOrFactoryInInterfaceDef
/// 声明了LogManagementInterface的工厂
DECLARE_INTERFACEFACTORY(LogManagementInterface);

/// 声明了LogManagementInterface的管理器
DECLARE_INTERFACEMANAGER(LogManagementInterface);
#endif
} // namespace ysos

#endif  //  LOG_MANAGEMENT_INTERFACE_H      //NOLINT
