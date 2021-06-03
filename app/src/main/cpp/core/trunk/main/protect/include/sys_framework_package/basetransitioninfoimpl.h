/**
 *@file BaseStrategyImpl.h
 *@brief Definition of BaseStrategyImpl
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-21 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef SFP_BASE_TRANSITION_INFO_IMPL_H_  // NOLINT
#define SFP_BASE_TRANSITION_INFO_IMPL_H_  // NOLINT

/// stl Headers
#include <map>
#include <list>
/// boost Headers
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/make_shared.hpp>
#include <boost/xpressive/xpressive.hpp>
/// Private Ysos Headers
#include "../../../public/include/sys_interface_package/baseinterface.h"
#include "../../../public/include/sys_interface_package/moduleinterface.h"
#include "../../../protect/include/sys_framework_package/modulelinkimpl.h"
#include "../../../public/include/sys_interface_package/strategyinterface.h"
#include "../../../public/include/sys_interface_package/callbackqueue.h"
#include "../../../public/include/core_help_package/callbacklist.h"
#include "../../../public/include/core_help_package/utility.h"
#include "../../../protect/include/os_hal_package/msgimpl.h"
#include "../../../public/include/os_hal_package/bufferutility.h"
#include "../../../protect/include/sys_framework_package/ioinfoutility.h"
#include "../../../protect/include/sys_framework_package/baseioinfoimpl.h"
#include "../../../protect/include/sys_framework_package/platformservice.h"

namespace ysos {
typedef boost::shared_ptr<std::list<ModuleInterfacePtr> >     RepeaterInterfaceListPtr;
typedef std::list<ModuleInterfacePtr>                         RepeaterList;
typedef std::map<std::string, RepeaterInterfaceListPtr >      ServiceRepeaterMap;
typedef std::map<std::string, CallbackListPtr>                ServiceMap;
typedef ServiceMap::iterator                                  CommandMapIterator;
typedef std::list<std::string>                                StringList;
typedef std::list<PlatformService::ServiceInfoPtr >           ServiceList;
typedef uint32_t                                              StatType;
#define AllStat                                               0

static int g_state_id = 1;

struct Transition {
  std::string                name;
  std::string                from_str;   ///< cur state
  std::string                to_str;     ///<  destination state  //  NOLINT
  StatType                   from;   ///< cur state
  StatType                   to;     ///<  destination state  //  NOLINT
  ServiceMap                 service_map;
  StringList                 service_list;        ///<  service_name|callback_name //  NOLINT
  ServiceList                service_info_list;   ///< ServiceInfoPtr指针链表
  ServiceMap                 event_map;
  StringList                 event_list;
  ServiceRepeaterMap         service_repeater_map_;
  RepeaterList               repeater_list;  ///<  与service_repeater_map不同的是，repeater_list里的Repeater不存在重复 //  NOLINT
  TransitionInfoInterfacePtr transition_info;

  ~Transition() {
    service_map.clear();
    service_list.clear();
    service_info_list.clear();
    event_map.clear();
    event_list.clear();
    service_repeater_map_.clear();
    repeater_list.clear();
  }
};
typedef boost::shared_ptr<Transition> TransitionPtr;
typedef std::list<TransitionPtr>    TransitionList;
typedef std::list<TransitionPtr>::iterator    TransitionListIterator;
typedef StringList                StateList;
typedef StringList::iterator      StateListIterator;

struct Mechanism {
  typedef StringList  StateList;
  typedef StringList  TStringList;
  std::string  name;   ///<  statemachine name //  NOLINT
  std::string  type;   ///<  type: statemachine
  std::string  file;   ///<  子状态机文件名称   //  NOLINT
  //INT32        parent_id;
  std::string  parent_state_machine;  ///< parent statemachine name
  std::string  parent_state;          ///< parent state name
  StateList stat_list;                ///< children states
  std::string  transition_file_name;
  TStringList   event_list;
  TStringList   service_list;
  TransitionList         transition_list;
};
typedef boost::shared_ptr<Mechanism>  MechanismPtr;

/**
 *@brief Transition的上下文
 */
struct TransitionContext {
  ModuleInterface *module_ptr;  ///<  数据所属的module //  NOLINT
  CallbackInterface *callback_ptr;  ///<  数据所属的Callback //  NOLINT
  TransitionInfoInterface *transition_ptr;  ///<  被调用的Callback所属的Transition //  NOLINT
  std::string message_type;  ///<  自定义MessageType //  NOLINT

  TransitionContext() {
    module_ptr = NULL;
    callback_ptr = NULL;
    transition_ptr = NULL;
  }
};
/*************************************************************************************************************
 *                      BaseTransitionInfoImpl                                                               *
 *************************************************************************************************************/
class BaseTransitionInfoImpl: public TransitionInfoInterface, public BaseInterfaceImpl {
  DECLARE_CREATEINSTANCE(BaseTransitionInfoImpl);
  DISALLOW_COPY_AND_ASSIGN(BaseTransitionInfoImpl);
  DECLARE_PROTECT_CONSTRUCTOR(BaseTransitionInfoImpl);
 public:
  ~BaseTransitionInfoImpl();

  typedef    std::list<IoInfoInterfacePtr>          IOInfoList;

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
  *@brief  对输入的数据进行过滤   //  NOLINT
  *@param  input_ptr 待过滤的数据   //  NOLINT
  *@param  output_ptr 过滤后的结果，如果不需要返回值，可以为NULL  //  NOLINT
  *@param  context_ptr 上下文，可以为NULL //  NOLINT
  *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
  */
  virtual int Filter(BufferInterfacePtr input_ptr, BufferInterfacePtr output_ptr=NULL, void *context_ptr=NULL);
  /**
    *@brief  对输入的数据进行匹配   //  NOLINT
    *@param  input_ptr 待匹配的数据   //  NOLINT
    *@param  output_ptr 过滤后的结果，如果不需要返回值，可以为NULL  //  NOLINT
    *@param  context_ptr 上下文，可以为NULL //  NOLINT
    *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
    */
  virtual bool IsMatched(BufferInterfacePtr input_ptr, BufferInterfacePtr output_ptr=NULL, void *context_ptr=NULL);
  /**
  *@brief 获取消息头，由ioinfo用消息头封闭数据
  *@param  input_ptr 待匹配的数据   //  NOLINT
  *@param  output_ptr 过滤后的结果，如果不需要返回值，可以为NULL  //  NOLINT
  *@param  context_ptr 上下文，可以为NULL //  NOLINT
  *@return 成功返回头，失败返回NULL  //  NOLINT
  */
  virtual uint32_t GetMessageID(BufferInterfacePtr input_ptr, BufferInterfacePtr output_ptr=NULL, void *context_ptr=NULL);
  /**
  *@brief 添加消息进Strategy MessageQueue中
  *@param  message_ptr 待添加的数据   //  NOLINT
  *@param  context_ptr 上下文，可以为NULL //  NOLINT
  *@return 成功返回头，失败返回NULL  //  NOLINT
  */
  virtual int AddMessageToQueue(MsgInterfacePtr message_ptr, void *context_ptr=NULL);
  /**
   *@brief 获取接口的属性值  // NOLINT
   *@param type_id[Input]： 属性的ID  // NOLINT
   *@param type[Output]：属性值的值  // NOLINT
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
  /**
  *@brief 设置状态满足切换的标识  //  NOLINT
  *@param  module_name 数据所属的Module //  NOLINT
  *@param  data_type 数据的类型 //  NOLINT
  *@return 成功返回0，失败返回其他值  //  NOLINT
  */
  int SetSwitchFlag(const std::string &module_name, const std::string &data_type);
  /**
  *@brief 获取TransitionContext  //  NOLINT
  *@param  context_ptr 根据Context_ptr获取TransitionContext //  NOLINT
  *@return TransitionContext对象  //  NOLINT
  */
  TransitionContext GetTransitionContext(void *context_ptr);
  /**
  *@brief 构建IoInfo  //  NOLINT
  *@param  ioinfo 根据ioinfo获取IoInfoInterface对象  //  NOLINT
  *@return 无  //  NOLINT
  */
  virtual void ConstructIoInfo(IoInfoPtr ioinfo);
  /**
  *@brief 切换条件全部清零  //  NOLINT
  *@return 无  //  NOLINT
  */
  void Reset(void);


 protected:
  enum InfoType {LOGIC_TYPE=0, INFO_TYPE, InfoTypeEnd};
  struct LogicIoInfo {
    InfoType type;
    std::string value;
    IoInfoPtr   ioinfo_ptr;
  };
  typedef boost::shared_ptr<LogicIoInfo> LogicIoInfoPtr;
  CallbackInterfacePtr    callback_ptr_;           ///<  设置回调函数 //  NOLINT
  uint32_t matched_msg_id_;     ///< 切换匹配的message id

 private:
  //std::string  name_;
  IOInfoList                               ioInfo_list_;
  StrategyInterface                       *strategy_ptr_;
  std::list<LogicIoInfoPtr>                logic_ioinfo_list_;
  std::string                              expression_;
  //std::map<std::string, std::string>       ioinfo_event_map_;  ///<  (ioinfo_name, event_name) //  NOLINT
  LightLock                               *ioinfo_match_result_lock_;
  std::map<std::string, std::string>       ioinfo_match_result_;  ///<  (ioinfo_name, ioinfo->isMatched()) //  NOLINT
  bool                                     is_matched_for_callback_;  ///<  当Callback非空时用 //  NOLINT
};
typedef boost::shared_ptr<BaseTransitionInfoImpl> BaseTransitionInfoImplPtr;

typedef std::map<std::string, ModuleLinkInfoPtr>             ModuleLinkMap;
typedef std::map<std::string, ModuleLinkInfoPtr>::iterator   ModuleLinkMapIterator;
typedef std::list<TransitionPtr>                             TransitionList;
typedef std::list<TransitionPtr>::iterator                   TransitionListIterator;

class BaseStateMachineImpl;
typedef boost::shared_ptr<BaseStateMachineImpl>   BaseStateMachineImplPtr;
typedef std::list<BaseStateMachineImplPtr>        BaseStateMachineImplPtrList;
typedef std::map<StatType, BaseStateMachineImplPtr>        BaseStateMachineImplPtrMap;
}
#endif  // SIP_BASE_MODULE_IMPL_H_  // NOLINT
