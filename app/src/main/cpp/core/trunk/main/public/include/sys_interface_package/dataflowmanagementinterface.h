/**
 *@file dataflowmanagementinterface.h
 *@brief Definition of dataflowmanagementinterface
 *@version 0.1
 *@author XuLanyue
 *@date Created on: 2016-05-17 18:43:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef DATA_FLOW_MANAGEMENT_INTERFACE_H        //NOLINT
#define DATA_FLOW_MANAGEMENT_INTERFACE_H        //NOLINT

#include "../../../public/include/sys_interface_package/baseinterface.h"
#include <boost/shared_ptr.hpp>

/// 日志选项                                      //NOLINT
#define LOG_OPTIONS_ENCRYPT                     1   ///< 加密日志

#define SHARE_AREA_SESSION                      "session"
#define SHARE_AREA_APPLICATION                  "application"
#define SHARE_AREA_DATAFLOW                     "dataflow"
#define SHARE_AREA_TASK                         "task"

namespace ysos {

class BaseInterface;

/**
   *@brief 数据管理接口, 实现数据的装入/查找/读/写机制, 可用于日志管理接口             //NOLINT
   */
class DataflowManagementInterface : virtual public BaseInterface {
 public:

  /**
   *@brief 读取数据                                                           //NOLINT
   *@param data_key[Input]: 要读取的数据的key.                                 //NOLINT
   *@param data_object_ptr[Output]: 存放将读取的对象.                          //NOLINT
   *@param part_id[Input]: 数据的部分ID, 默认为"session".                      //NOLINT
   *@return: 若成功则返回YSOS_ERROR_SUCCESS, 否则返回相应错误值.                  //NOLINT
   */
  virtual int GetData(
    const std::string &data_key,
    void* data_object_ptr,
    const std::string &part_id = SHARE_AREA_SESSION) = 0;

  /**
   *@brief 写入数据                                                           //NOLINT
   *@param data_key[Input]: 要写入的数据的key.                                 //NOLINT
   *@param data_object_ptr[Input]: 存放将写入的对象.                           //NOLINT
   *@param part_id[Input]: 数据的部分ID, 默认为"session".                      //NOLINT
   *@return: 若成功则返回YSOS_ERROR_SUCCESS, 否则返回相应错误值.                  //NOLINT
   */
  virtual int SetData(
    const std::string &data_key,
    const void* data_object_ptr,
    const std::string &part_id = SHARE_AREA_SESSION) = 0;

  /**
   *@brief 删除数据                                                           //NOLINT
   *@param data_key[Input]: 要删除的数据的key.                                 //NOLINT
   *@param part_id[Input]: 数据的部分ID, 默认为"session".                      //NOLINT
   *@return: 若成功则返回YSOS_ERROR_SUCCESS, 否则返回相应错误值.                  //NOLINT
   */
  virtual int DeleteData(
    const std::string &data_key,
    const std::string &part_id = SHARE_AREA_SESSION) = 0;

  /**
   *@brief 删除全部数据数据                                                    //NOLINT
   *@param part_id[Input]: 数据的部分ID, 默认为"session".                      //NOLINT
   *@return: 若成功则返回YSOS_ERROR_SUCCESS, 否则返回相应错误值.                  //NOLINT
   */
  virtual int ClearAllData(
    const std::string &data_key,
    const void* data_object_ptr,
    const std::string &part_id = SHARE_AREA_SESSION) = 0;
};

typedef boost::shared_ptr<DataflowManagementInterface> DataflowManagementInterfacePtr;

#if 1   //  1, DeclareManagerOrFactoryInInterfaceDef
/// 声明了DataflowManagementInterface的工厂
DECLARE_INTERFACEFACTORY(DataflowManagementInterface);

/// 声明了DataflowManagementInterface的管理器
DECLARE_INTERFACEMANAGER(DataflowManagementInterface);
#endif
} // namespace ysos

#endif  //  DATA_FLOW_MANAGEMENT_INTERFACE_H    //NOLINT
