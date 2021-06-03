/**
 *@file baseinterface.h
 *@brief Definition of baseinterface
*@version 0.1
 *@author Steven.Shi
*@date Created on: 2016-04-21 13:59:20
*@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
*/

#ifndef BASE_INTERFACE_H  //NOLINT
#define BASE_INTERFACE_H  //NOLINT

#if 1   //  disable warning 4250 and 4996
#include "../../../public/include/os_hal_package/config.h"  // NOLINT
#if (CURRENT_PLATFORM == PLATFORM_WINDOWS)
#pragma warning(disable: 4250)      //  dominance
#pragma warning(disable: 4996)      //  std::_Fill_n
#endif
#endif

#include "../../../public/include/sys_interface_package/common.h"
#include <string>
#include <sstream>
#include <boost/uuid/uuid.hpp>

namespace ysos {

typedef boost::uuids::uuid uuid;

/**
  *@brief  对于Ioctl、SetProperty和GetProperty，在很多情况下，第二个参数type需要传递进一个输入，一个输出, //  NOLINT
  *        对于这种情况，提供了一种通用的参数类型     // NOLINT
  */
struct PropertyInfo {
  int             id;
  std::string     str_id;
  void           *param;
};

/**
 *@brief BaseInterface是整个架构内所有接口的统一基接口，
     内部预定义了实例名称，实例的UUID，实例的类型
 */
class YSOS_EXPORT BaseInterface {
  IMPLEMENT_PUBLIC_EMPTY_DESTRUCTOR(BaseInterface)
 public:
  /**
   *@brief BaseInterface的基本属性
   */
  enum BaseProperties {
    Name = 10000, ///< 名字
    Type, ///< 类型
    UUID, ///< 唯一标识符
  };

  /**
   *@brief 基本初始化  // NOLINT
   *@param param： 初始化的参数  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Initialize(void *param=NULL) = 0;

  /**
   *@brief 是否初始化过  // NOLINT
   *@return： 成功返回true，失败返回false  // NOLINT
   */
  virtual bool IsInitialized(void) = 0;

  /**
   *@brief 是否初始化成功  // NOLINT
   *@return： 成功返回true，失败返回false  // NOLINT
   */
  virtual bool IsInitSucceeded(void) = 0;

  /**
   *@brief 清除资源,Initialize的逆操作  // NOLINT
   *@param param： 初始化的参数 // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int UnInitialize(void *param=NULL) = 0;

  /**
     *@brief 获取接口的实例名称  // NOLINT
     *@param 无  // NOLINT
     *@return:返回实际实例名称  // NOLINT
     */
  virtual std::string GetName(bool is_full_name = false) const = 0;

  /**
     *@brief 获取接口的实例UUID  // NOLINT
     *@param： 无  // NOLINT
     *@return:返回实际实例UUID  // NOLINT
     */
  virtual uuid GetUUID() const = 0;

  /**
     *@brief 获取接口的属性值  // NOLINT
     *@param type_id[Input]： 属性的ID  // NOLINT
     *@param type[Input/Output]：属性值的值  // NOLINT
     *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
     */
  virtual int GetProperty(int type_id, void *type) = 0;

  /**
   *@brief 设置接口的属性值  // NOLINT
   *@param type_id[Input]： 属性的ID  // NOLINT
   *@param type[input]： 属性值的值  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int SetProperty(int type_id, void *type) = 0;
};

} // namespace ysos

#endif // BASE_INTERFACE_H  //NOLINT
