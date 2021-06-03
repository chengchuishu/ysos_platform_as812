/**
 *@file BaseIoInfoImpl.h
 *@brief Definition of BaseIoInfoImpl
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-21 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef SFP_BASE_INFO_IMPL_H_  // NOLINT
#define SFP_BASE_INFO_IMPL_H_  // NOLINT

/// stl headers //  NOLINT
#include <map>
#include <list>
/// boost headers //  NOLINT
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/make_shared.hpp>
#include <boost/xpressive/xpressive.hpp>
/// ysos private headers //  NOLINT
#include "../../../public/include/sys_interface_package/baseinterface.h"
#include "../../../public/include/sys_interface_package/moduleinterface.h"
#include "../../../public/include/sys_interface_package/strategyinterface.h"
#include "../../../public/include/core_help_package/utility.h"
//#include "../../../protect/include/os_hal_package/messageimpl.h"
#include "../../../public/include/os_hal_package/bufferutility.h"
#include "../../../protect/include/sys_framework_package/ioinfoutility.h"

namespace ysos {
/*************************************************************************************************************
 *                      BaseIoInfoImpl                                                                       *
 *************************************************************************************************************/
class YSOS_EXPORT BaseIoInfoImpl: public IoInfoInterface, public BaseInterfaceImpl {
  DISALLOW_COPY_AND_ASSIGN(BaseIoInfoImpl);

 public:
  BaseIoInfoImpl(const std::string &strClassName  ="BaseIoInfoImpl");
    virtual ~BaseIoInfoImpl();

    enum BaseIoInfoImpl_ProPerty {
    FIRST_OPERATOR=IoInfoInterface_ProPerty_End+1,
    SECOND_OPERATOR,
    FIRST_VALUE,
    SECOND_VALUE,
    EXPRESSION,
    TRANSITION_INFO_ADD,
    TRANSITION_INFO_REMOVE,
    IS_MATCHED,
    BaseIoInfoImpl_ProPerty_End
  };

 public:
  /**
    *@brief  对输入的数据进行匹配如果匹配，将数据添加进Msg_Queue_，同时返回True，否则返回False   //  NOLINT
    *@param  input_ptr 待匹配的数据   //  NOLINT
    *@param  output_ptr 过滤后的结果，如果不需要返回值，可以为NULL  //  NOLINT
    *@param  context_ptr 上下文，可以为NULL //  NOLINT
    *@return 成功返回true，否则返回false  //  NOLINT
    */
  virtual bool IsMatched(BufferInterfacePtr input_buffer, BufferInterfacePtr ouput_buffer, void *context_ptr);
  /**
     *@brief 获取接口的属性值  // NOLINT
     *@param type_id[Input]： 属性的ID  // NOLINT
     *@param type[Input/Output]：属性值的值  // NOLINT
     *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
     */
  virtual int GetProperty(int type_id, void *type);
  /**
   *@brief 设置接口的属性值  // NOLINT
   *@param type_id[Input]： 属性的ID  // NOLINT
   *@param type[input]： 属性值的值  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int SetProperty(int type_id, void *type);

 protected:
  virtual int Wrap(BufferInterfacePtr input_buffer, BufferInterfacePtr ouput_buffer, void *context_ptr);
  virtual int UnWrap(BufferInterfacePtr input_buffer, BufferInterfacePtr ouput_buffer, void *context_ptr);

 protected:
  CallbackInterfacePtr       callback_ptr_;           ///<  设置回调函数 //  NOLINT
  TransitionInfoInterface*   transition_info_ptr_;
  std::string                condition_expression_;
  bool                       is_mached_;            ///<  the last matched result //  NOLINT
};

template<typename T>
class YSOS_EXPORT BaseIoInfoImplExtend: public BaseIoInfoImpl {
  DISALLOW_COPY_AND_ASSIGN(BaseIoInfoImplExtend);

 public:
  BaseIoInfoImplExtend(const std::string &strClassName  ="BaseIoInfoImplExtend"):
    BaseIoInfoImpl(strClassName) {
  }

  virtual ~BaseIoInfoImplExtend() {
  }

  virtual bool IsMatched(BufferInterfacePtr input_buffer, BufferInterfacePtr ouput_buffer, void *context_ptr) {
    uint8_t* buffer = GetBufferUtility()->GetBufferData(input_buffer);
    if (NULL == buffer) {
      return false;
    }

    /// string特殊处理 //  NOLINT
    bool is_handled = IsMatchedString(buffer);
    if (!is_handled) {
      is_handled = IsMatched(buffer);;
    }

    if(is_mached_) {
      Wrap(input_buffer, ouput_buffer, NULL);
    }

    return is_mached_;
  }

 protected:
  virtual bool IsMatchedString(const uint8_t *buffer_ptr) {
    std::string type_name = typeid(value_).name();
    // not string
    if (std::string::npos == type_name.find("std::basic_string")) {
      return false;
    }

    std::string value_str = (char*)buffer_ptr;
    is_mached_ = (value_str == condition_expression_);
    YSOS_LOG_DEBUG("ioinfo name: " << GetName() << "  value: " << value_str << "  result: " << is_mached_);
    return true;
  }

  virtual bool IsMatched(const uint8_t *buffer_ptr) {
    value_ = *((T*)buffer_ptr);
    std::string value_str = GetUtility()->ConvertToString(value_);
    std::string current_expression = boost::replace_all_copy(condition_expression_, "$(ysos)", value_str);
    is_mached_ = GetIoinfoUtility()->CalculateExpression(current_expression, value_);
    YSOS_LOG_DEBUG("ioinfo name: " << GetName() << "  value: " << value_);

    return is_mached_;
  }

 protected:
  T                        value_;
};
//typedef  boost::shared_ptr<BaseIoInfoInterface>      BaseIoInfoImplPtr;
}
#endif  // SIP_BASE_MODULE_IMPL_H_  // NOLINT
