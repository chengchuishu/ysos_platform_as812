/**
 *@file condition entity.h
 *@brief Definition of all the condition entity
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-06-21 08:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef SFP_CONDITION_ENTITY_H_       //NOLINT
#define SFP_CONDITION_ENTITY_H_       //NOLINT

/// Platform Headers
#include "../../../public/include/core_help_package/baseinterfaceimpl.h"
#include "../../../protect/include/sys_framework_package/platformrpcentity.h"
#include "../../../public/include/sys_interface_package/callbackqueue.h"
#include "../../../protect/include/core_help_package/propertytree.h"
#include "../../../protect/include/core_help_package/singletontemplate.h"

namespace ysos {
/**
  *@brief condition实例的基类  //NOLINT
  */
class YSOS_EXPORT BaseCondition: public BaseInterfaceImpl {
//   DECLARE_CREATEINSTANCE(BaseAgentImpl);
//   DISALLOW_COPY_AND_ASSIGN(BaseAgentImpl);
//   DECLARE_PROTECT_CONSTRUCTOR(BaseAgentImpl);

 public:
  BaseCondition(const std::string &strClassName = "BaseCondition");
  virtual ~BaseCondition(void) {};

  /**
  *@brief 根据文件名解析配置文件  // NOLINT
  *@param file_name[Input]： 配置文件名  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  // int FromString(const std::string &file_name);
  /**
  *@brief 根据输入的字符串解析  // NOLINT
  *@param config_str[Input]： 配置字符串  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int FromString(const std::string &config_str) = 0;
  /**
  *@brief 初始化  // NOLINT
  *@param param[Input]： 初始化参数  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int DoAction(CallbackInterface *context) = 0;
  /**
  *@brief 获取Tag值  // NOLINT
  *@return： 返回Tag值  // NOLINT
  */
  virtual std::string GetTag(void);

 protected:
  /**
  *@brief 根据输入的字符串解析  // NOLINT
  *@param config_str[Input]： 配置字符串  // NOLINT
  *@return： 成功返回PropertyTreePtr，失败返回NULL  // NOLINT
  */
  PropertyTreePtr ParseString(const std::string &config_str);

 protected:
  bool                 if_ignore_error_;  ///< 是否忽略错误
  std::string          tag_;              ///< 用于标识Errror的处理逻辑
};
typedef boost::shared_ptr<BaseCondition> BaseConditionPtr;
/**
  *@brief event condition的实现类  //NOLINT
  */
class YSOS_EXPORT EventCondition: public BaseCondition {
//   DECLARE_CREATEINSTANCE(BaseAgentImpl);
//   DISALLOW_COPY_AND_ASSIGN(BaseAgentImpl);
//   DECLARE_PROTECT_CONSTRUCTOR(BaseAgentImpl);

 public:
  EventCondition(const std::string &strClassName = "EventCondition");
  virtual ~EventCondition(void);

  /**
  *@brief 根据输入的字符串解析  // NOLINT
  *@param config_str[Input]： 配置字符串  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int FromString(const std::string &config_str);
  /**
  *@brief 初始化  // NOLINT
  *@param param[Input]： 初始化参数  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int DoAction(CallbackInterface *context);

 protected:
  ReqRegServiceEventServiceParam  reg_service_event_req_;   ///< 事件注册的参数
};
/**
  *@brief ioctl condition的实现类  //NOLINT
  */
class YSOS_EXPORT IoctlCondition: public BaseCondition {
//   DECLARE_CREATEINSTANCE(BaseAgentImpl);
//   DISALLOW_COPY_AND_ASSIGN(BaseAgentImpl);
//   DECLARE_PROTECT_CONSTRUCTOR(BaseAgentImpl);

 public:
  IoctlCondition(const std::string &strClassName = "IoctlCondition");
  virtual ~IoctlCondition(void);

  /**
  *@brief 根据输入的字符串解析  // NOLINT
  *@param config_str[Input]： 配置字符串  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int FromString(const std::string &config_str);
  /**
  *@brief 初始化  // NOLINT
  *@param param[Input]： 初始化参数  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int DoAction(CallbackInterface *context);

 protected:
  /**
  *@brief 通过protocol和key获得格式化后的值  // NOLINT
  *@return： 返回结果  // NOLINT
  */
  std::string FromatValue(void);

 protected:
  ReqIOCtlServiceParam    ioctl_req_;   ///< ioctl的请求参数
  std::string             protcol_;     ///< 待使用的protocol名称
  std::string             key_;         ///< 待使用的key值
  std::string             ioctl_value_;  ///< 保存ioctl_req的原始Value值
};
/**
  *@brief ready condition的实现类  //NOLINT
  */
class YSOS_EXPORT ReadyCondition: public BaseCondition {
//   DECLARE_CREATEINSTANCE(BaseAgentImpl);
//   DISALLOW_COPY_AND_ASSIGN(BaseAgentImpl);
//   DECLARE_PROTECT_CONSTRUCTOR(BaseAgentImpl);

 public:
  ReadyCondition(const std::string &strClassName = "ReadyCondition");
  virtual ~ReadyCondition(void);

  /**
  *@brief 根据输入的字符串解析  // NOLINT
  *@param config_str[Input]： 配置字符串  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int FromString(const std::string &config_str);
  /**
  *@brief 初始化  // NOLINT
  *@param param[Input]： 初始化参数  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int DoAction(CallbackInterface *context);

 protected:
  ReqReadyServiceParam     ready_req_;     ///< Ready的请求参数
};
/**
  *@brief switchcondition的实现类  //NOLINT
  */
class YSOS_EXPORT SwitchCondition: public BaseCondition {
//   DECLARE_CREATEINSTANCE(BaseAgentImpl);
//   DISALLOW_COPY_AND_ASSIGN(BaseAgentImpl);
//   DECLARE_PROTECT_CONSTRUCTOR(BaseAgentImpl);

 public:
  SwitchCondition(const std::string &strClassName = "SwitchCondition");
  virtual ~SwitchCondition(void);

  /**
  *@brief 根据输入的字符串解析  // NOLINT
  *@param config_str[Input]： 配置字符串  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int FromString(const std::string &config_str);
  /**
  *@brief 初始化  // NOLINT
  *@param param[Input]： 初始化参数  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int DoAction(CallbackInterface *context);

 protected:
  ReqSwitchServiceParam switch_req_;    ///< Switch的请求参数
};

/**
  *@brief switchnotifycondition的实现类  //NOLINT
  */
class YSOS_EXPORT SwitchNotifyCondition: public BaseCondition {
//   DECLARE_CREATEINSTANCE(BaseAgentImpl);
//   DISALLOW_COPY_AND_ASSIGN(BaseAgentImpl);
//   DECLARE_PROTECT_CONSTRUCTOR(BaseAgentImpl);

 public:
  SwitchNotifyCondition(const std::string &strClassName = "SwitchNotifyCondition");
  virtual ~SwitchNotifyCondition(void);

  /**
  *@brief 根据输入的字符串解析  // NOLINT
  *@param config_str[Input]： 配置字符串  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int FromString(const std::string &config_str);
  /**
  *@brief 初始化  // NOLINT
  *@param param[Input]： 初始化参数  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int DoAction(CallbackInterface *context);

 protected:
  ReqSwitchNotifyServiceParam switch_notify_req_;    ///< SwitchNotify的请求参数
};

/**
  *@brief eventnotifycondition的实现类  //NOLINT
  */
class YSOS_EXPORT EventNotifyCondition: public BaseCondition {
//   DECLARE_CREATEINSTANCE(BaseAgentImpl);
//   DISALLOW_COPY_AND_ASSIGN(BaseAgentImpl);
//   DECLARE_PROTECT_CONSTRUCTOR(BaseAgentImpl);

 public:
  EventNotifyCondition(const std::string &strClassName = "EventNotifyCondition");
  virtual ~EventNotifyCondition(void);

  /**
  *@brief 根据输入的字符串解析  // NOLINT
  *@param config_str[Input]： 配置字符串  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int FromString(const std::string &config_str);
  /**
  *@brief 初始化  // NOLINT
  *@param param[Input]： 初始化参数  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int DoAction(CallbackInterface *context);

 protected:
  ReqEventNotifyServiceParam event_notify_req_;    ///< EventNotify的请求参数
};

/**
  *@brief send event condition的实现类  //NOLINT
  */
class YSOS_EXPORT SendEventCondition: public BaseCondition {
//   DECLARE_CREATEINSTANCE(BaseAgentImpl);
//   DISALLOW_COPY_AND_ASSIGN(BaseAgentImpl);
//   DECLARE_PROTECT_CONSTRUCTOR(BaseAgentImpl);

 public:
  SendEventCondition(const std::string &strClassName = "SendEventCondition");
  virtual ~SendEventCondition(void);

  /**
  *@brief 根据输入的字符串解析  // NOLINT
  *@param config_str[Input]： 配置字符串  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int FromString(const std::string &config_str);
  /**
  *@brief 初始化  // NOLINT
  *@param param[Input]： 初始化参数  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int DoAction(CallbackInterface *context);

 protected:
  std::string                    event_name_;
  std::string                    content_;
};

/**
  *@brief set data service condition的实现类  //NOLINT
  */
class YSOS_EXPORT SetDataCondition: public BaseCondition {
//   DECLARE_CREATEINSTANCE(BaseAgentImpl);
//   DISALLOW_COPY_AND_ASSIGN(BaseAgentImpl);
//   DECLARE_PROTECT_CONSTRUCTOR(BaseAgentImpl);

 public:
  SetDataCondition(const std::string &strClassName = "SetDataCondition");
  virtual ~SetDataCondition(void);

  /**
  *@brief 根据输入的字符串解析  // NOLINT
  *@param config_str[Input]： 配置字符串  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int FromString(const std::string &config_str);
  /**
  *@brief 初始化  // NOLINT
  *@param param[Input]： 初始化参数  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int DoAction(CallbackInterface *context);

 protected:
  std::string                    data_interface_name_;  ///< data_interface的Key // NOLINT
  std::string                    data_key;              ///< 要存储的Key
  std::string                    data_content;          ///< 要存储的内容
};

/**
  *@brief callback condition的实现类  //NOLINT
  */
class YSOS_EXPORT CallbackCondition: public BaseCondition {
//   DECLARE_CREATEINSTANCE(BaseAgentImpl);
//   DISALLOW_COPY_AND_ASSIGN(BaseAgentImpl);
//   DECLARE_PROTECT_CONSTRUCTOR(BaseAgentImpl);

 public:
  CallbackCondition(const std::string &strClassName = "SetDataCondition");
  virtual ~CallbackCondition(void);

  /**
  *@brief 根据输入的字符串解析  // NOLINT
  *@param config_str[Input]： 配置字符串  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int FromString(const std::string &config_str);
  /**
  *@brief 初始化  // NOLINT
  *@param param[Input]： 初始化参数  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int DoAction(CallbackInterface *context);

 protected:
  /**
  *@brief 参数格式化  // NOLINT
  *@param param[Input] key 参数key  // NOLINT
  *@param param[Input] value 参数value  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int FormatParam(const std::string &key, const std::string &value);


 protected:
  std::string                    callback_name_;  ///< 要调用的Callback的逻辑名 // NOLINT
  std::string                    data_key;              ///< 要存储的Key
  std::string                    data_content;          ///< 要存储的内容
  CallbackInterfacePtr           callback_ptr_;    ///< 要调用的Callback的指针
  BufferInterfacePtr             buffer_ptr_;      ///< 缓存指针
};


/**
  *@brief condition manager的实现类  //NOLINT
  */
class YSOS_EXPORT ConditionManager: public BaseInterfaceImpl {
  DISALLOW_COPY_AND_ASSIGN(ConditionManager);
  DECLARE_PROTECT_CONSTRUCTOR(ConditionManager);

 public:
  virtual ~ConditionManager(void);

  /**
  *@brief 获得一个Condition指针  // NOLINT
  *@param type[Input]： Condition的类型  // NOLINT
  *@param config_str[Input]： 配置字符串  // NOLINT
  *@return： 成功返回Condition的指针，失败返回NULL  // NOLINT
  */
  virtual BaseConditionPtr GetCondition(const std::string &type, const std::string &config_str);

  DECLARE_SINGLETON_VARIABLE(ConditionManager);
};
#define GetConditionManager  ConditionManager::Instance
}
#endif  //  SFP_CONDITION_ENTITY_H_   //NOLINT
