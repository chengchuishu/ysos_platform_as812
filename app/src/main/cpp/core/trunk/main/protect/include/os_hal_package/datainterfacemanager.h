/**
 *@file DataInterfaceManager.h
 *@brief Definition of data manager
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-05-17 16:20:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef OHP_DATA_INTERFACE_MANAGER_H_         //NOLINT
#define OHP_DATA_INTERFACE_MANAGER_H_         //NOLINT

/// Boost Headers //  NOLINT
#include <boost/shared_ptr.hpp>
/// Private Headers //  NOLINT
#include "../../../public/include/core_help_package/baseinterfaceimpl.h"
#include "../../../public/include/sys_interface_package/datainterface.h"
#include "../../../protect/include/core_help_package/singletontemplate.h"
#include "../../../public/include/core_help_package/utility.h"

namespace ysos {
class DataInterfaceManager;
typedef boost::shared_ptr<DataInterfaceManager>  DataInterfaceManagerPtr;

#define   GLOBAL_DATA_KEY                    "global_data_key"

/**
   *@brief 数据管理接口, 实现数据的装入/查找/读/写机制, 可用于日志管理接口             //NOLINT
   */
class YSOS_EXPORT DataInterfaceManager : virtual public BaseInterfaceImpl {
  DISALLOW_COPY_AND_ASSIGN(DataInterfaceManager);
  DECLARE_PROTECT_CONSTRUCTOR(DataInterfaceManager);

 public:
  ~DataInterfaceManager();

  typedef  std::map<std::string, DataInterfacePtr >           DataInterfaceMap;

  /**
   *@brief 获得一个标识为data_key的DataInterface，如果没有，Manager会自动创建，如果已存在，直接返回    //NOLINT
   *@param data_key[Input]: DataInterface的唯一标识符                                                  //NOLINT
   *@return: 若成功则返回DataInterface指针, 否则返回NULL                                               //NOLINT
   */
  DataInterfacePtr GetData(const std::string &data_key);
  /**
   *@brief 从标识为data_key的DataInterface中获取key对应的值    //NOLINT
   *@param data_key[Input]: DataInterface的唯一标识符                                                  //NOLINT
   *@param key[Input]: 获取Key对应的值                                                                 //NOLINT
   *@return: 若成功则返回DataInterface指针, 否则返回NULL                                               //NOLINT
   */
  std::string GetData(const std::string &data_key, const std::string &key);

  /**
   *@brief 获得一个标识为data_key的DataInterface，如果没有，Manager会自动创建，如果已存在，直接返回    //NOLINT
   *@param data_key[Input]: DataInterface的唯一标识符                                                  //NOLINT
   *@return: 若成功则返回0, 否则返回非0值                                                              //NOLINT
   */
  int DeleteData(const std::string &data_key);
  /**
   *@brief 从标识为data_key的DataInterface中删除key对应的变量    //NOLINT
   *@param data_key[Input]: DataInterface的唯一标识符                                                  //NOLINT
   *@param key[Input]: 获取Key对应的值                                                                 //NOLINT
   *@return: 若成功则返回0, 否则返回非0值                                                              //NOLINT
   */
  int DeleteData(const std::string &data_key, const std::string &key);

 private:
  LightLock                                 data_map_lock_;  ///< data_map_对应的Lock  //  NOLINT
  DataInterfaceMap                          data_map_;  ///<  存储所有的DataInterface //  NOLINT

  DECLARE_SINGLETON_VARIABLE(DataInterfaceManager);
};
} // namespace ysos
#define GetDataInterfaceManager  ysos::DataInterfaceManager::Instance

#endif  //  DATA_MANAGEMENT_INTERFACE_H     //NOLINT
