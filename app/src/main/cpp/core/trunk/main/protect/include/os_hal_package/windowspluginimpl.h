/*
# WindowsPluginImpl.h
# Definition of WindowsPluginImpl
# Created on: 2016-04-25 13:59:20
# Original author: dhongqian
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
*/
#ifndef YSOS_OHP_WINDOWS_PLUGIN_H_                                                 //NOLINT
#define YSOS_OHP_WINDOWS_PLUGIN_H_                                                 //NOLINT

#include "../../../protect/include/sys_interface_package/plugininterface.h"
 #include "../../../public/include/core_help_package/baseinterfaceimpl.h"
#include <string>

namespace ysos {

  /**
   *@brief 加载Windows平台的插件，
       可用于加载Module、Callback、Protocol等各种平台插件
   */
class YSOS_EXPORT WindowsPluginImpl: public PluginInterface, public BaseInterfaceImpl{
  DECLARE_CREATEINSTANCE(WindowsPluginImpl);
  DISALLOW_COPY_AND_ASSIGN(WindowsPluginImpl);
  DECLARE_PROTECT_CONSTRUCTOR(WindowsPluginImpl);

 public:
   ~WindowsPluginImpl();

   /**
   *@brief 基本初始化  // NOLINT
   *@param param： 初始化的参数 // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Initialize(void *param=NULL);

  /**
   *@brief 清除资源,Initialize的逆操作  // NOLINT
   *@param param： 初始化的参数 // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int UnInitialize(void *param=NULL);

/**
   *@brief 加载指定的插件  // NOLINT
   *@param plugin_name[Input]： 待加载的插件的名称  // NOLINT
   *@return： 成功返回插件的Handle，失败返回空  // NOLINT
   */
  virtual YSOS_PLUGIN_TYPE LoadPlugin(const std::string &plugin_name);

  /**
   *@brief 查询指定插件中的函数  // NOLINT
   *@param plugin_handle[Input]： 待查询的插件Handle  // NOLINT
   *@param function_name[Intput]：待查询的插件中的函数名  // NOLINT
   *@return： 成功返回函数指针，失败返回空  // NOLINT
   */
  virtual void* LookupPlugin(const YSOS_PLUGIN_TYPE plugin_handle, const std::string &function_name);

  /**
   *@brief  卸载指定的插件 // NOLINT
   *@param input_type[Output]： 待卸载的插件的Handle  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int UnloadPlugin(YSOS_PLUGIN_TYPE plugin_handle);
};

typedef boost::shared_ptr<WindowsPluginImpl> WindowsPluginPtr;
}
#endif  ///< YSOS_OHP_WINDOWS_PLUGIN_H_ NOLINT