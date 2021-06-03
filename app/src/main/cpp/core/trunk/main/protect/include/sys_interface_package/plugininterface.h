/**
* PluginInterface.h
* Definition of PluginInterface
* Created on: 2016-04-25 13:59:20
* Original author: dhongqian
* Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
*/
#ifndef YSOS_SIP_PLUGIN_INTERFACE_H_                                                 //NOLINT
#define YSOS_SIP_PLUGIN_INTERFACE_H_                                                 //NOLINT

#if 1   //  disable warning 4250 and 4996
#include "../../../public/include/os_hal_package/config.h"  // NOLINT
#if (CURRENT_PLATFORM == PLATFORM_WINDOWS)
#pragma warning(disable: 4250)      //  dominance
#pragma warning(disable: 4996)      //  std::_Fill_n
#endif
#endif
#include "../../../public/include/sys_interface_package/common.h"
#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

namespace ysos {

typedef void*  YSOS_PLUGIN_TYPE;

  /**
   *@brief PluginInterface是系统内加载插件的统一接口，
       可用于加载Module、Callback、Protocol等各种平台插件
   */
class YSOS_EXPORT PluginInterface {
  IMPLEMENT_PUBLIC_EMPTY_DESTRUCTOR(PluginInterface)

 public:

/**
   *@brief 加载指定的插件  // NOLINT
   *@param plugin_name[Input]： 待加载的插件的名称  // NOLINT
   *@return： 成功返回插件的Handle，失败返回空  // NOLINT
   */
  virtual YSOS_PLUGIN_TYPE LoadPlugin(const std::string &plugin_name) = 0;

  /**
   *@brief 查询指定插件中的函数  // NOLINT
   *@param plugin_handle[Input]： 待查询的插件Handle  // NOLINT
   *@param function_name[Intput]：待查询的插件中的函数名  // NOLINT
   *@return： 成功返回函数指针，失败返回空  // NOLINT
   */
  virtual void* LookupPlugin(const YSOS_PLUGIN_TYPE plugin_handle, const std::string &function_name) = 0;

  /**
   *@brief  卸载指定的插件 // NOLINT
   *@param input_type[Output]： 待卸载的插件的Handle  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int UnloadPlugin(YSOS_PLUGIN_TYPE plugin_handle) = 0;
};

typedef boost::shared_ptr<PluginInterface> PluginInterfacePtr;
}
#endif  ///< YSOS_SIP_PLUGIN_INTERFACE_H_ NOLINT