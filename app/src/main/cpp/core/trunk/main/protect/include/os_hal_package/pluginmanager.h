/*
# PluginManager.h
# Definition of PluginManager
# Created on: 2016-04-25 13:59:20
# Original author: dhongqian
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
*/
#ifndef YSOS_OHP_PLUGIN_MANAGER_H_                                                 //NOLINT
#define YSOS_OHP_PLUGIN_MANAGER_H_                                                 //NOLINT

//#include "../../../protect/include/sys_interface_package/plugininterface.h"
#include "../../../public/include/core_help_package/baseinterfaceimpl.h"
#include "../../../protect/include/sys_interface_package/plugininterface.h"
#include "../../../public/include/core_help_package/utility.h"
#include <string>
#include <map>

namespace ysos {

  /**
   *@brief 加载Windows平台的插件，
       可用于加载Module、Callback、Protocol等各种平台插件
   */
class YSOS_EXPORT PluginManager: public BaseInterfaceImpl{
  DECLARE_CREATEINSTANCE(PluginManager);
  DISALLOW_COPY_AND_ASSIGN(PluginManager);
  DECLARE_PROTECT_CONSTRUCTOR(PluginManager);

/*  friend class Singleton<PluginManager>;*/

 public:
   ~PluginManager();

   typedef std::map<std::string, void*>  PluginMap;
   typedef std::map<std::string, void*>::iterator  PluginMapIterator;
   typedef std::vector<std::string>      FileArray;

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
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int LoadPlugin(const std::string &plugin_name);

  /**
   *@brief 查找指定的插件  // NOLINT
   *@param plugin_name[Input]： 待查找的插件的名称  // NOLINT
   *@param default_plugin_name[Input]： 为空时，插件在libplugin_name.dll里找,非空，在default_plugin_name里查找  // NOLINT
   *@return： 成功返回插件的创建函数指针，失败返回空  // NOLINT
   */
  virtual void* LookupPlugin(const std::string &plugin_name, const std::string &default_plugin_name="");
  /**
   *@brief 查找指定的插件  // NOLINT
   *@param plugin_handle[Input]： 待查找的插件的句柄  // NOLINT
   *@param function_name[Input]： 要查找的插件的名称  // NOLINT
   *@return： 成功返回插件的创建函数指针，失败返回空  // NOLINT
   */
  virtual void* LookupPlugin(const YSOS_PLUGIN_TYPE plugin_handle, const std::string &function_name);
  /**
   *@brief 获得插件Map的头信息
   *@return： 成功返回插件Map的头，失败返回插件Map的尾  // NOLINT
   */
  PluginMapIterator PluginHead(void);
  /**
   *@brief 获得插件Map的尾信息
   *@return： 返回插件Map的尾  // NOLINT
   */
  PluginMapIterator PluginEnd(void);
  /**
   *@brief 获得指定的插件信息
   *@param plugin_name[Input]： 待查找的插件的名称  // NOLINT
   *@return： 成功返回插件信息，失败返回插件Map的尾  // NOLINT
   */
  PluginMapIterator FindPlugin(const std::string &plugin_name);

  /**
   *@brief  卸载所有插件 // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual void UnloadPlugin(void);

protected:
  PluginMap        plugin_map_;  ///<  已有的可用插件 //  NOLINT
  PluginInterfacePtr  plugin_;   ///<  插件加载工具 //  NOLINT

  DECLARE_SINGLETON_VARIABLE(PluginManager);
};

//typedef boost::shared_ptr<PluginManager> PluginManagerPtr;
#define GetPluginManager  ysos::PluginManager::Instance
}
#endif  ///< YSOS_OHP_PLUGIN_MANAGER_H_ NOLINT