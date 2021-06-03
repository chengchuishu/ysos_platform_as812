/*
# basedataimpl.h
# Definition of BaseDataImpl
# Created on: 2016-11-23 17:20:13
# Original author: jinchengzhe
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
# version 0.1, 20161123, created by JinChengZhe
*/
#ifndef OHP_BASE_DATA_IMPL_H_
#define OHP_BASE_DATA_IMPL_H_

/// C++ Standard Headers
#include <map>
/// Platform Headers
#include "../../../public/include/sys_interface_package/common.h"
#include "../../../public/include/core_help_package/baseinterfaceimpl.h"
#include "../../../public/include/sys_interface_package/datainterface.h"

namespace ysos {

class LightLock;

/**
*@brief BaseDataImpl的具体实现  // NOLINT
*/
class BaseDataImpl;
typedef boost::shared_ptr<BaseDataImpl> BaseDataImplPtr;
class YSOS_EXPORT BaseDataImpl : public DataInterface, public BaseInterfaceImpl {
  DISALLOW_COPY_AND_ASSIGN(BaseDataImpl)  //  禁止拷贝和复制
  DECLARE_PROTECT_CONSTRUCTOR(BaseDataImpl)  // 构造函数保护
  DECLARE_CREATEINSTANCE(BaseDataImpl)  // 定义类的全局静态创建函数

 public:
  typedef std::map<std::string, std::string> DataMap;
  typedef std::map<std::string, std::string>::iterator DataMapIterator;

  /**
  *@brief  析构函数  // NOLINT
  *@param  无  // NOLINT
  *@return:  无  // NOLINT
  */
  virtual ~BaseDataImpl();

  /**
  *@brief 读取数据                                                           //NOLINT
  *@param key[Input]: 要读取的数据的key.                                     //NOLINT
  *@param value[Output]: 存放将读取的对象.                                   //NOLINT
  *@param part_id[Input]: 数据的部分ID, 默认为"session".                     //NOLINT
  *@return: 若成功则返回YSOS_ERROR_SUCCESS, 否则返回相应错误值.              //NOLINT
  */
  virtual int GetData(const std::string& key, std::string& value);

  /**
   *@brief 写入数据                                                           //NOLINT
   *@param key[Input]: 要写入的数据的key.                                     //NOLINT
   *@param value[Input]: 存放将写入的对象.                                    //NOLINT
   *@return: 若成功则返回YSOS_ERROR_SUCCESS, 否则返回相应错误值.              //NOLINT
   */
  virtual int SetData(const std::string& key, const std::string& value);

  /**
   *@brief 删除数据                                                           //NOLINT
   *@param key[Input]: 要删除的数据的key.                                     //NOLINT
   *@return: 若成功则返回YSOS_ERROR_SUCCESS, 否则返回相应错误值.              //NOLINT
   */
  virtual int DeleteData(const std::string& key);

  /**
   *@brief 删除全部数据数据                                                   //NOLINT
   *@return: 若成功则返回YSOS_ERROR_SUCCESS, 否则返回相应错误值.              //NOLINT
   */
  virtual int ClearAllData(void);

 private:
  DataMap data_map_;   ///< 保存数据
  LightLock ll_lock_;   ///< 锁，读写map用
};

}

#endif