/**
 *@file BaseStateMachineImpl.h
 *@brief Definition of BaseStateMachineImpl
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-21 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef SFP_BASE_STAT_MACHINE_IMPL_H_  // NOLINT
#define SFP_BASE_STAT_MACHINE_IMPL_H_  // NOLINT

/// 3rdparty headers //  NOLINT
//#include <tinyxml2/tinyxml2.h>
#include <tinyxml2/tinyxml2.h>
/// ysos private headers //  NOLINT
#include "../../../protect/include/sys_framework_package/basetransitioninfoimpl.h"
#include "../../../protect/include/sys_framework_package/statemachinemanager.h"
#include "../../../protect/include/sys_framework_package/serviceinfotable.h"
#include "../../../public/include/os_hal_package/lock.h"
#include "../../../public/include/sys_interface_package/msginterface.h"
#include "../../../public/include/core_help_package/callbacklist.h"
#include "../../../protect/include/sys_framework_package/basecallbackimpl.h"
#include "../../../protect/include/sys_framework_package/platformservice.h"
#include "../../../protect/include/sys_framework_package/baseagentimpl.h"
#include "../../../protect/include/sys_framework_package/basestrategyservicecallbackimpl.h"

namespace ysos {
/*************************************************************************************************************
 *                      BaseStateMachineImpl                                                                 *
 *************************************************************************************************************/
//typedef ModuleInterface   RepeaterInterface;
//typedef ModuleInterfacePtr   RepeaterInterfacePtr;
class StateMachineManager;
/**
 *@brief  状态机基类实现 // NOLINT
*/
class YSOS_EXPORT BaseStateMachineImpl : public BaseStrategyServiceCallbackImpl, public boost::enable_shared_from_this<BaseStateMachineImpl> {
  DECLARE_CREATEINSTANCE(BaseStateMachineImpl);
  DISALLOW_COPY_AND_ASSIGN(BaseStateMachineImpl);
  DECLARE_PROTECT_CONSTRUCTOR(BaseStateMachineImpl);

 public:
  virtual ~BaseStateMachineImpl(void);

  typedef  MutexLock                            StateMachineLock;
  typedef  std::map<std::string, std::string>   RegEventMap;
  typedef  std::multimap<std::string, std::string>  EventServiceMap;

  //enum StateMachine_Property {
  //  PARENT = 10,
  //  PARENT_NAME,
  //  PARENT_STATE_NAME,
  //  CHILD,
  //  MANAGER,
  //  STRATEGY,
  //  MECHANISM,
  //  STATE,      // 切换State，或获取当前State
  //  CURRENT_STATE_NAME,
  //  SWITCH_REQ,
  //  REG_SERVICE_EVENT_REQ,
  //  UN_REG_SERVICE_EVENT_REQ,
  //  READY_REQ,
  //  IOCTL_REQ,
  //  StateMachine_Property
  //};

  struct AgentDispatchInfo {
    std::string  service_name;
    uint32_t     event_type;
    std::string  event_key;
    std::string  callback_name;
    std::string  param;

    AgentDispatchInfo() {
      event_type = 0;
    }
  };
  typedef  boost::shared_ptr<AgentDispatchInfo>        AgentDispatchInfoPtr;
  typedef  std::list<TransitionPtr>                    TransitionList;
  typedef  std::multimap<StatType, TransitionPtr>      StateIdTransitionMap;
  typedef  std::multimap<std::string, TransitionPtr>   StateTransitionMap;
  typedef std::map<std::string, StatType>                    StateNameIDMap;
  typedef std::map<StatType, std::string>                    StateIDNameMap;

  struct IoctlContex {
    BaseStateMachineImpl *state_machine_ptr;
    void*                   service_info_ptr;
    //ServiceInfoPtr        service_info_ptr;
  };

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
   *@brief 回调接口的实现函数  // NOLINT
   *@param input_buffer[Input]： 输入数据缓冲  // NOLINT
   *@param output_buffer[Output]：回调处理后输出的数据缓冲  // NOLINT
   *@param context[Input]： 回调处理的上下文数据  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Callback(BufferInterfacePtr input_buffer, BufferInterfacePtr output_buffer, void *context = NULL);
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
  /////////////////////////////////////////////////////////////////////////////////////////
  //////////////        Utility        ///////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////
  /**
  *@brief 从BufferPtr的指针中，获取到BufferPtr中的Buffer // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  std::string GetBufferFromBufferInterfacePtrPtr(void *buffer_ptr_ptr);
  /**
  *@brief 通过指定的Name获得Agent指针 // NOLINT
  *@param  name  Agent的Name   // NOLINT
  *@return： 成功返回Agent指针，失败返回NULL  // NOLINT
  */
  BaseAgentImpl *GetAgentByName(const std::string &name);
  /**
  *@brief 获取Agent注册过来的Callback Name // NOLINT
  *@param  event_name  获取与Event_Name对应的Callback_Name   // NOLINT
  *@return： 成功返回Callback_Name，失败返回空  // NOLINT
  */
  std::string GetAgentCallbackName(const std::string &event_name);
  /////////////////////////////////////////////////////////////////////////////////////////
  //////////////        State Relate   ///////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////
  /**
  *@brief 改变状态 // NOLINT
  *@param  is_chagne_parent 是否切换父状态，父状态切换时，Manager里的Cur_State_Machine不切换   // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int ChangeStat(const uint32_t next_stat, bool is_chagne_parent=false);
  /**
  *@brief 改变状态 // NOLINT
  *@param  is_chagne_parent 是否切换父状态，父状态切换时，Manager里的Cur_State_Machine不切换   // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int ChangeStat(const std::string &next_stat, bool is_chagne_parent=false);
  /**
  *@brief 改变状态 // NOLINT
  *@param  is_chagne_parent 是否切换父状态，父状态切换时，Manager里的Cur_State_Machine不切换   // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int RealChangeStat(const uint32_t next_stat, bool is_chagne_parent=false);
  /**
  *@brief 获取当前状态机活动的状态名称 // NOLINT
  *@param  is_full_name 是否返回状态全名,默认是false   // NOLINT
  *@return： 成功返回状态名，失败返回空  // NOLINT
  */
  std::string GetCurStateName(bool is_full_name=false);
  /**
  *@brief 获取当前Strategy活动状态的名称 // NOLINT
  *@return： 成功返回状态名，失败返回空  // NOLINT
  */
  std::string GetCurActiveStateName(void);
  /**
  *@brief 检查状态切换的实际路径 // NOLINT
  *@param  state_name 待检状态名   // NOLINT
  *@return： 成功返回状态名，失败返回空  // NOLINT
  */
  int CheckStatePath(const std::string &state_name);
  /**
  *@brief 检查状态切换的实际路径 // NOLINT
  *@param  state_list 待检状态名   // NOLINT
  *@param  checked true 只检测，不切换，false 既检测，又切换   // NOLINT
  *@return： 成功返回状态名，失败返回空  // NOLINT
  */
  int CheckStatePath(std::list<std::string> &state_list, bool checked=true);
  /**
    *@brief 检查子状态机的状态切换的实际路径 // NOLINT
    *@param  state_list 待检状态名   // NOLINT
    *@param  checked true 只检测，不切换，false 既检测，又切换   // NOLINT
    *@return： 成功返回状态名，失败返回空  // NOLINT
    */
  int CheckCurStatePath(std::list<std::string> &state_list, bool checked=true);
  /**
    *@brief 检查父状态机的状态切换的实际路径 // NOLINT
    *@param  state_list 待检状态名   // NOLINT
    *@param  checked true 只检测，不切换，false 既检测，又切换   // NOLINT
    *@return： 成功返回状态名，失败返回空  // NOLINT
    */
  int CheckParentStatePath(std::list<std::string> &state_list, bool checked=true);
  /**
    *@brief 检查子状机的状态切换的实际路径 // NOLINT
    *@param  state_name 要检查的子状态机名   // NOLINT
    *@param  state_list 待检状态名   // NOLINT
    *@param  checked true 只检测，不切换，false 既检测，又切换   // NOLINT
    *@return： 成功返回状态名，失败返回空  // NOLINT
    */
  int CheckSubStatePath(std::string &state_name, std::list<std::string> &state_list, bool checked=true);
  /**
      *@brief 通过StateName获得子状态机 // NOLINT
      *@return： 成功返回状态状态机，失败返回NULL  // NOLINT
      */
  BaseStateMachineImplPtr GetChildStateMachineByStateName(const std::string &state_name);
  /**
    *@brief 通过StateName获得StateId // NOLINT
    *@return： 成功返回状态ID，失败返回0  // NOLINT
    */
  StatType GetStateIdByStateName(const std::string &state_name);
  /**
      *@brief  stat是否是当前StatMachine支持的Stat //  NOLINT
      *@param  stat 待检测的stat   // NOLINT
      *@return 成功返回true，失败返回false  //  NOLINT
      */
  bool IsValidStat(const uint32_t &next_state);
  /**
      *@brief  退出当前状态 //  NOLINT
      *@return 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  //  NOLINT
      */
  int ExitState();
  /**
    *@brief 更新事件注册表
    *@return 成功返回0，否则失败
    */
  virtual int UpdateEventTable(ServiceMap &event_map, ServiceMap &dst_event_map);
  /**
   *@brief 更新命令注册表
   *@return 成功返回0，否则失败
   */
  virtual int UpdateCmdTable(ServiceMap &event_map, ServiceMap &dst_cmd_map);
  /**
  *@brief 更新TransitionMap表，实现state_id与transition的映射，更新了transition中的state_id值 // NOLINT
  *@param state_id_map[input]： state_name和state_id的映射表  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  int UpdateTransitionMap(std::map<std::string, StatType> &state_id_map);
  /**
   *@brief 通过parent_state_name获得对应的state_id // NOLINT
   *@param state_id_map[input]： state_name和state_id的映射表  // NOLINT
   *@return： 成功返回值YSOS_ERROR_SUCCESS，失败返回相应错误  // NOLINT
   */
  int UpdateParentStateId(std::map<std::string, StatType> &state_id_map);

  /**
   *@brief  清除与状态相关的事件和命令 //  NOLINT
   *@return 无  //  NOLINT
   */
  void ClearStateEventAndCmd(void);
  /**
   *@brief  根据状态链表，产生状态的Name和ID的对应关系 //  NOLINT
   *@param state_list[input]： 状态列表  // NOLINT
   *@return 无  //  NOLINT
   */
  void GenerateStateIdNameRelationship(StateList &state_list);

 protected:
  /////////////////////////////////////////////////////////////////////////////////////////
  //////////////   Construct Service   ///////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////
  /**
  *@brief  构建所有transition中的Service //  NOLINT
  *@param  transition_list  待构建的Transition   //  NOLINT
  *@return 成功返回ID，失败返回YSOS_ERROR_INVALID_ARGUMENTS  //  NOLINT
  */
  int ConstructTransitionService(TransitionList &transition_list);
  /**
   *@brief  构建所有的Service //  NOLINT
   *@param  service_name_list  待构建的Service   //  NOLINT
   *@return 成功返回ID，失败返回YSOS_ERROR_INVALID_ARGUMENTS  //  NOLINT
   */
  int ConstructService(std::list<std::string> &service_name_list, TransitionPtr &transation_ptr);
  /**
   *@brief  构建Service //  NOLINT
   *@param  service_info  待构建的Service   //  NOLINT
   *@param  transation_ptr  Service所属的Transition   //  NOLINT
   *@return 成功返回ID，失败返回YSOS_ERROR_INVALID_ARGUMENTS  //  NOLINT
   */
  int ConstructService(PlatformService::ServiceInfoPtr service_info, TransitionPtr &transation_ptr);
  /**
   *@brief  注册服务 //  NOLINT
   *@param  service_info  待注册的Service   //  NOLINT
   *@param  transation_ptr  Service所属的Transition   //  NOLINT
   *@return 成功返回ID，失败返回YSOS_ERROR_INVALID_ARGUMENTS  //  NOLINT
   */
  int RegisterService(PlatformService::ServiceInfoPtr service_info, TransitionPtr &transation_ptr);
  /**
   *@brief  从all_callback_list中过滤掉used_callback_list中的相同成员 //  NOLINT
   *@param  used_callback_list  已经使用的成员  //  NOLINT
   *@param  all_callback_list  总的成员数  //  NOLINT
   *@return 成功返回CallbackName，失败返回空  //  NOLINT
   */
  std::string GetTransitionInfoCallbackName(std::list<std::string> &all_callback_list, std::list<std::string> &used_callback_list);
  /**
   *@brief  通过ServiceName获取Repeater, 并注册给transation_ptr //  NOLINT
   *@param  service_name  要获取service_name需求的Repeater   //  NOLINT
   *@param  transation_ptr  将获取到的Repeater注册进transation_ptr   //  NOLINT
   *@return 成功返回repeater，失败返回NULL  //  NOLINT
   */
  int RegisterRepeaterByServiceName(const std::string &service_name, TransitionPtr &transation_ptr);
  /**
   *@brief 通过service的名称，获得Repeater指针 // NOLINT
   *@param service_name[input]： 需要的服务名  // NOLINT
   *@return： 成功返回Repeater指针，失败返回空  // NOLINT
   */
  //RepeaterInterfacePtr GetRepeaterByServiceName(const std::string &service_name);
  /**
   *@brief 对当前状态里的所有Repeater进行过滤，保证不存在重复的Repeater // NOLINT
   *@param service_name[input]： 需要的服务名  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
   */
  int FileterRepeater(const std::string &service_name);
  /**
   *@brief 根据service_list,对所有的Service执行Run操作 // NOLINT
   *@param service_list[input]： 需要的服务名列表  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
   */
  int RunService(const StringList &service_list);
  /**
  *@brief 根据service_name对Service执行Run操作 // NOLINT
  *@param service_name[input]： 需要的服务名  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
  */
  int RunService(const std::string &service_name);
  /**
   *@brief 根据service_list,对所有的Service执行Stop操作 // NOLINT
   *@param service_name[input]： 需要的服务名  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
   */
  int StopService(const StringList &service_list);
  /**
   *@brief 根据service_name对Service执行Run操作 // NOLINT
   *@param service_name[input]： 需要的服务名  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
   */
  int StopService(const std::string &service_name);

 protected:
  /////////////////////////////////////////////////////////////////////////////////////////
  //////////////      Handle MSG/CMD   ///////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////
  /**
  *@brief  处理Switch Service服务请求 //  NOLINT
  *@param service_xml 待解析的消息  //  NOLINT
  *@param if_need_dispatch 是否需要发送Dispatch，默认为true,发送  //  NOLINT
  *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
  */
  int HandleSwitchService(const std::string &service_xml, bool if_need_dispatch=true);
  /**
  *@brief  处理RegServiceEvent Service服务请求 //  NOLINT
  *@param service_xml 待解析的消息  //  NOLINT
  *@param if_need_dispatch 是否需要发送Dispatch，默认为true,发送  //  NOLINT
  *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
  */
  int HandleRegServiceEventService(const std::string &service_xml, bool if_need_dispatch=true);
  /**
  *@brief  处理UnregServiceEvent Service服务请求 //  NOLINT
  *@param service_xml 待解析的消息  //  NOLINT
  *@param if_need_dispatch 是否需要发送Dispatch，默认为true,发送  //  NOLINT
  *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
  */
  int HandleUnregServiceEventService(const std::string &service_xml, bool if_need_dispatch=true);
  /**
   *@brief 处理Ready Service服务请求 // NOLINT
   *@param service_xml[input]： 需要处理的消息  // NOLINT
   *@param if_need_dispatch 是否需要发送Dispatch，默认为true,发送  //  NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误 // NOLINT
   */
  virtual int HandleReadyService(const std::string &service_xml, bool if_need_dispatch=true);
  /**
   *@brief  处理Ioctl Service服务请求 //  NOLINT
   *@param  param Callback的参数  //  NOLINT
   *@param if_need_dispatch 是否需要发送Dispatch，默认为true,发送  //  NOLINT
   *@return 成功返回0，否则失败  //  NOLINT
   */
  int HandleIoctlService(const std::string &service_xml, bool if_need_dispatch=true);
  /**
   *@brief  处理Ioctl Service服务请求 //  NOLINT
   *@param  service_info Ioctl Service的参数  //  NOLINT
   *@return 成功返回0，否则失败  //  NOLINT
   */
  int HandleIoctlService(const ReqIOCtlServiceParam &service_info, bool if_need_dispatch=true);
  /**
   *@brief 处理Strategy的命令（cmd_queue_)
   *@param cmd 待处理的命令
   *@return 成功返回0，否则失败
  */
  virtual int HandleCommand(const MsgInterfacePtr cmd);
  /**
    *@brief 处理Strategy的命令（cmd_queue_)
    *@return 成功返回true，否则失败
   */
  virtual bool HandleCommand(void);
  /**
  *@brief 处理Strategy的事件和数据(msg_queue_)  // NOLINT
  *@param msg 待处理的消息
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int HandleMessage(const MsgInterfacePtr msg);
  /**
  *@brief 处理Strategy的事件和数据(msg_queue_)  // NOLINT
  *@return： 成功返回true，失败返回相应错误值  // NOLINT
  */
  virtual bool HandleMessage(void);
  /**
    *@brief 处理Strategy的事件和数据(msg_queue_)  // NOLINT
    *@return： 成功返回true，失败返回相应错误值  // NOLINT
    */
  virtual int HandleMessageAndCmd(ServiceMap &cmd_msg_map, const MsgInterfacePtr &cmd_msg,std::string &event_cmd_name, StateMachineLock *lock);
  /**
    *@brief  从msg_queue_里取得数据 //  NOLINT
    *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
    */
  MsgInterfacePtr GetMessage(void);
  /**
    *@brief  从cmd_queue_里取得数据 //  NOLINT
    *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
    */
  MsgInterfacePtr GetCmd(void);
  /**
    *@brief  添加子状态机 //  NOLINT
    *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
    */
  int AddChild(BaseStateMachineImplPtr &child_state_machine);
  /**
   *@brief  通过state_id查找transition //  NOLINT
   *@param  state_id  状态的名字   //  NOLINT
   *@return 成功返回0，失败返回其他值  //  NOLINT
   */
  int GetTransitionByStateID(const StatType &state_id, TransitionList &transition_list);
  /**
   *@brief  检查当前的Message是否满足状态变换的条件 //  NOLINT
   *@param  msg  待检查的消息   //  NOLINT
   *@return 成功返回0，否则失败  //  NOLINT
   */
  int CheckStateChangeCondition(const MsgInterfacePtr  msg);
  /**
   *@brief  将组合状态名解析出来 //  NOLINT
   *@param  state_name  组合状态名，格式：状态1@状态2@状态3...   //  NOLINT
   *@param  state_list  解析后的状态名存储在state_list中...   //  NOLINT
   *@return 成功返回0，否则失败  //  NOLINT
   */
  int SplitStateName(const std::string &state_name, std::list<std::string> &state_list);
  /**
   *@brief  更新当前状态的Repeater信息 //  NOLINT
   //*@param  cur_state  当前状态   //  NOLINT
   *@param  is_register  true 注册， false 反注册   //  NOLINT
   *@return 成功返回0，否则失败  //  NOLINT
   */
  int RegisterTransitionInfoToRepeater(/*const StatType &cur_state, TransitionList &transition_list, */bool is_register);
  /**
   *@brief  将TransitionInfo注册给Repeater //  NOLINT
   *@param  repleater_list  待注册的Repeater列表   //  NOLINT
   *@param  transition_ptr  待注册的transition   //  NOLINT
   *@return 成功返回0，否则失败  //  NOLINT
   */
  int RegisterTransitionInfoToRepeater(RepeaterList &repleater_list, TransitionPtr transition_ptr, bool is_register=true);
  /**
   *@brief  调用Agent的Dispatch //  NOLINT
   *@param  agent_dipatch_info Dispatch的信息  //  NOLINT
   *@return 成功返回0，否则失败  //  NOLINT
   */
  int InvokeAgentDispatch(AgentDispatchInfo *&agent_dipatch_info);
  /**
   *@brief 匹配Message对应的事件是否与DoEvent和RegisterEvent注册进来的事件匹配，如果匹配，就优先处理 // NOLINT
   *@param message_ptr[input]： 需要处理的消息  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误 // NOLINT
   */
  int HandleMessageByRegisterEvent(MsgInterfacePtr message_ptr);

 protected:
  /////////////////////////////////////////////////////////////////////////////////////////
  //////////////      Handle Agent Request/Response   /////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////
  /**
   *@brief 发送EventNotify请求事件 // NOLINT
   *@param event_name[input]： 需要处理的事件名称  // NOLINT
   *@param callback_name[input]： 需要处理的函数名称  // NOLINT
   *@param msg_ptr[input]： 要传送的数据  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误 // NOLINT
   */
  int DoEventNotifyService(const std::string &event_name, const std::string &callback_name, MsgInterfacePtr &msg_ptr);
  /**
   *@brief 发送SwitchNotify请求事件 // NOLINT
   *@param next_stat[input]： 要切换的状态名称  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误 // NOLINT
   */
  int DoSwitchNotifyService(const std::string &next_stat);
  /**
   *@brief 发送IoCtl的Dispatch事件 // NOLINT
   *@param ret_code[input]： 操作的返回码  // NOLINT
   *@param service_info[input]： 执行操作的Ioctl的信息  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误 // NOLINT
   */
  int DoIOCtlDispatch(const int &ret_code, const ReqIOCtlServiceParam &service_info);
  /**
   *@brief 发送Ready的Dispatch事件 // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误 // NOLINT
   */
  int DoReadyDispatch(void);
  /**
   *@brief 发送UnregServiceEvent的Dispatch事件 // NOLINT
   *@param ret_code[input]： 操作的返回码  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误 // NOLINT
   */
  int DoUnregServiceEventDispatch(const int &ret_code);
  /**
   *@brief 发送RegServiceEvent的Dispatch事件 // NOLINT
   *@param ret_code[input]： 操作的返回码  // NOLINT
   *@param service_name[input]： 执行操作的Service Name  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误 // NOLINT
   */
  int DoRegServiceEventDispatch(const int &ret_code, const std::string &service_name);
  /**
   *@brief 发送Switch的Dispatch事件 // NOLINT
   *@param ret_code[input]： 操作的返回码  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误 // NOLINT
   */
  int DoSwitchDispatch(const int &ret_code);
  /**
   *@brief 通过状态ID，获得状态名 // NOLINT
   *@param state_id[input]： 待获取的状态ID  // NOLINT
   *@return： 成功返回状态名，失败返回空 // NOLINT
   */
  std::string GetStateNameByID(const StatType state_id);
  /**
   * Ioctl请求的参数结构
   */
  struct IoctlParam {
    int  id;        ///< ioctl的ctrl_id
    std::string value_str;   ///< ioctl的传参
    std::string value_type;  ///< ioctl的传参类型
  };
  /**
     *@brief  解析字符串，获得IoctlPram结构 //  NOLINT
     *@param  param 待解析的字符串  //  NOLINT
     *@return 成功返回0，否则失败  //  NOLINT
     */
  //virtual IoctlParam GetIoctlParam(const std::string &param);

 protected:
  BaseStateMachineImplPtrMap         child_map_;   ///<  当前状态机拥有的子状态机 //  NOLINT
  BaseStateMachineImplPtr             parent_ptr_;   ///<  当前状态机的父状态机指针  //  NOLINT
  StrategyInterface*                   strategy_ptr_;  ///<  所属的Strategy指针 //  NOLINT
  StatType                             state_num_;  ///< 子状态的数量
  StatType                             max_child_state_num_; ///<  不大于1000 //  NOLINT

 private:
  StateMachineLock               *state_machine_lock_; ///<  state_machine本身的Lock //  NOLIN
  std::string                     state_machine_name_;  ///< 状态机的名称    //  NOLINT
  std::string                     parent_state_name_;    ///< parent state name
  StatType                        parent_state_name_id_;    ///< parent state name id
  StatType                        cur_stat_;  ///<  当前状态标识 //  NOLINT
  StateMachineLock               *cmd_table_lock_;
  ServiceMap                      cmd_table_;   ///<  当前状态下的Command表 //  NOLINT
  ServiceMap                      common_cmd_table_;  ///< 通用状态下的Command表
  StateMachineLock               *event_table_lock_;
  ServiceMap                      event_table_;   ///<  当前状态下的Event表
  ServiceMap                      common_event_table_;  ///< 通用状态下的Event表
// StateTransitionMap              stat_transition_map_;  ///<  状态对应的Transition //  NOLINT
  StateIdTransitionMap            stat_id_transition_map_;  ///<  状态对应的Transition //  NOLINT
  StateList                       state_list_;   ///<  当前状态机下的所有状态 //  NOLINT
  StateMachineLock               *cur_transition_list_lock_;
  std::list<TransitionPtr>        cur_transition_list_;  ///<  当前状态对应的transition_ptr //  NOLINT
  StateMachineLock               *service_map_lock_;    ///<  used for reg_event_map_ and do_event_map_  //  NOLINT
  RegEventMap                     reg_event_map_;  ///<  记录RegSeviceEvent     //  NOLINT
  RegEventMap                     do_event_map_;   ///<  记录DoServiceEvent     //  NOLINT
  RegEventMap                     event_service_map_;   ///<  记录要Run、Stop服务的event     //  NOLINT
  StateMachineManager*              state_manager_ptr_;
  StateNameIDMap                              state_name_id_map_;
  StateIDNameMap                              state_id_name_map_;
  bool                             is_run_;   ///< true 在状态中， false exit状态
  bool                             is_ready_;  ///<  true 在OnReady状态， false 不在OnReady状态 //  NOLINT
  bool                             is_changed_parent_;  ///< 是否是父状态机
};
}
#endif  // SFP_BASE_STAT_MACHINE_IMPL_H_  // NOLINT
