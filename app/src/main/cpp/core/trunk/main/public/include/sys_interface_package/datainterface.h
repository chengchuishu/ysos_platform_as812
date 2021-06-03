/**
 *@file datainterface.h
 *@brief Definition of DataInterface
 *@version 0.1
 *@author jinchengzhe
 *@date Created on: 2016-11-23 17:04:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef SIP_DATA_INTERFACE_H       //NOLINT
#define SIP_DATA_INTERFACE_H       //NOLINT

/// ThirdParty Headers
#include <boost/shared_ptr.hpp>
/// Platform Headers
#include "../../../public/include/sys_interface_package/baseinterface.h"

namespace ysos {
/**
*@brief Data接口
*/
class YSOS_EXPORT DataInterface {
  IMPLEMENT_PUBLIC_EMPTY_DESTRUCTOR(DataInterface)

  /**
  *@brief 读取数据                                                           //NOLINT
  *@param key[Input]: 要读取的数据的key.                                     //NOLINT
  *@param value[Output]: 存放将读取的对象.                                   //NOLINT
  *@param part_id[Input]: 数据的部分ID, 默认为"session".                     //NOLINT
  *@return: 若成功则返回YSOS_ERROR_SUCCESS, 否则返回相应错误值.              //NOLINT
  */
  virtual int GetData(const std::string& key, std::string& value) = 0;

  /**
   *@brief 写入数据                                                           //NOLINT
   *@param key[Input]: 要写入的数据的key.                                     //NOLINT
   *@param value[Input]: 存放将写入的对象.                                    //NOLINT
   *@return: 若成功则返回YSOS_ERROR_SUCCESS, 否则返回相应错误值.              //NOLINT
   */
  virtual int SetData(const std::string& key, const std::string& value) = 0;

  /**
   *@brief 删除数据                                                           //NOLINT
   *@param key[Input]: 要删除的数据的key.                                     //NOLINT
   *@return: 若成功则返回YSOS_ERROR_SUCCESS, 否则返回相应错误值.              //NOLINT
   */
  virtual int DeleteData(const std::string& key) = 0;

  /**
   *@brief 删除全部数据数据                                                    //NOLINT
   *@return: 若成功则返回YSOS_ERROR_SUCCESS, 否则返回相应错误值.               //NOLINT
   */
  virtual int ClearAllData(void) = 0;
};

typedef boost::shared_ptr<DataInterface> DataInterfacePtr;
} // namespace ysos

#endif  //  SIP_DATA_INTERFACE_H   //NOLINT