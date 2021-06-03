/**
 *@file datamanagementinterface.h
 *@brief Definition of data
 *@version 0.1
 *@author XuLanyue
 *@date Created on: 2016-05-17 16:20:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef DATA_MANAGEMENT_INTERFACE_H         //NOLINT
#define DATA_MANAGEMENT_INTERFACE_H         //NOLINT

#include "../../../public/include/sys_interface_package/baseinterface.h"
#include <boost/shared_ptr.hpp>

namespace ysos {

class BaseInterface;

/**
   *@brief 数据管理接口, 实现数据的装入/查找/读/写机制, 可用于日志管理接口             //NOLINT
   */
class DataManagementInterface : virtual public BaseInterface {
 public:

  /**
   *@brief 装入数据                                                           //NOLINT
   *@param data_uri[Input]: 数据的统一资源定位符, 如:数据文件全路径文件名.          //NOLINT
   *@param part_id[Input]: 期望装入的数据的部分ID, 默认为"", 意为全部.             //NOLINT
   *@param options[Input]: 期望装入数据时使用的选项, 默认为0.                     //NOLINT
   *@return: 若成功则返回YSOS_ERROR_SUCCESS, 否则返回相应错误值.                  //NOLINT
   */
  virtual int Load(
    const std::string &data_uri,
    const std::string &part_id = "",
    uint32_t options = 0) = 0;

  /**
   *@brief 查找数据                                                           // NOLINT
   *@param data_to_find[Input]: 要查找的数据.                                 //NOLINT
   *@param options[Input]: 查找数据时使用的选项, 默认为0.                        //NOLINT
   *@return: 若成功则返回查找到的数据的位置, 否则返回-1.                            //NOLINT
   */
  virtual int64_t Find(
    const std::string &data_to_find,
    uint32_t options = 0) = 0;

  /**
   *@brief 读取数据                                                           //NOLINT
   *@param data_key[Input]: 要读取数据的key.                                  //NOLINT
   *@param data_object_ptr[Output]: 存放将读取的对象.                          //NOLINT
   *@param part_id[Input]: 数据的部分ID, 默认为"", 意为全部.                     //NOLINT
   *@param options[Input]: 读取数据时使用的选项, 默认为0.                        //NOLINT
   *@return: 若成功则返回YSOS_ERROR_SUCCESS, 否则返回相应错误值.                  //NOLINT
   */
  virtual int Read(
    const std::string &data_key,
    void* data_object_ptr,
    const std::string &part_id = "",
    uint32_t options = 0) = 0;

  /**
   *@brief 写入数据                                                           //NOLINT
   *@param data_key[Input]: 要写入数据的key.                                  //NOLINT
   *@param data_object_ptr[Input]: 存放将写入的对象.                           //NOLINT
   *@param part_id[Input]: 数据的部分ID, 默认为"", 意为全部.                     //NOLINT
   *@param options[Input]: 写入数据时使用的选项, 默认为0.                        //NOLINT
   *@return: 若成功则返回YSOS_ERROR_SUCCESS, 否则返回相应错误值.                  //NOLINT
   */
  virtual int Write(
    const std::string &data_key,
    const void* data_object_ptr,
    const std::string &part_id = "",
    uint32_t options = 0) = 0;
};

typedef boost::shared_ptr<DataManagementInterface> DataManagementInterfacePtr;

} // namespace ysos

#endif  //  DATA_MANAGEMENT_INTERFACE_H     //NOLINT
