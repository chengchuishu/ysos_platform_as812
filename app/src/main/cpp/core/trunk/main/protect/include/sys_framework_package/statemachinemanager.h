/**
 *@file StateMachineManager.h
 *@brief Definition of StateMachineManager
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-21 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef SFP_BASE_STATE_MACHINE_MANAGER_H_  // NOLINT
#define SFP_BASE_STATE_MACHINE_MANAGER_H_  // NOLINT

/// stl Headers
#include <map>
#include <list>
/// boost headers //  NOLINT
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
/// 3rdparty headers //  NOLINT
#include <tinyxml2/tinyxml2.h>
/// Private Ysos Headers
#include "../../../public/include/os_hal_package/lock.h"
#include "../../../public/include/sys_interface_package/strategyinterface.h"
#include "../../../public/include/sys_interface_package/msginterface.h"
#include "../../../public/include/sys_interface_package/agentinterface.h"
#include "../../../protect/include/sys_framework_package/basetransitioninfoimpl.h"
#include "../../../protect/include/sys_framework_package/basestatemachineimpl.h"

namespace ysos {
/*************************************************************************************************************
 *                      StateMachineManager                                                                  *
 *************************************************************************************************************/
typedef ModuleInterface   RepeaterInterface;
typedef ModuleInterfacePtr   RepeaterInterfacePtr;
/**
 *@brief  状态机管理器，负责状态机的创建与维护 // NOLINT
*/
class YSOS_EXPORT StateMachineManager : public ControlInterface, public BaseInterfaceImpl {
  //DECLARE_CREATEINSTANCE(StateMachineManager);
  DISALLOW_COPY_AND_ASSIGN(StateMachineManager);
  //DECLARE_PROTECT_CONSTRUCTOR(StateMachineManager);

 public:
  StateMachineManager(StrategyInterface* strategy_ptr, const std::string &str="StateMachineManager");
  virtual ~StateMachineManager(void);

  enum StateMachineManager_CMD {
    CMD_AGENT_DISPATCH = 10,
    CMD_MECHANISM,
    CMD_GET_SERVICE_LIST,    ///<  get all service supported by strategy  //  NOLINT
    CMD_GET_MECHANISIM_STATE,           ///<  get mechanism state //  NOLINT
    //GET_STATE_ID,       ///< get state id by state name
    //GET_STATE_NAME,     ///< get state name by state id
    CMD_CHANGE_STATE_MACHINE,
    CMD_GET_STATE_SWITCH_INFO,
    CMD_GET_CURRENT_STATE_NAME,
    StateMachineManager_CMD_END,
  };

  typedef LightLock                                          StateMachineManagerLock;
  typedef std::list<MechanismPtr>                            MechanismPtrList;
  typedef std::map<std::string, StatType>                    StateNameIDMap;
  typedef std::map<StatType, std::string>                    StateIDNameMap;
  typedef std::map<StatType, CallbackInterfacePtr>           StateIDStateMachineMap;

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
   *@brief 配置函数，可对该接口进行详细配置  // NOLINT
   *@param ctrl_id[Input]： 详细配置ID号  // NOLINT
   *@param param[Input]：详细配置的参数  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Ioctl(INT32 ctrl_id, LPVOID param = NULL);
  /**
   *@brief 停止  // NOLINT
   *@param ： 无  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Stop(LPVOID param = NULL);
  /**
   *@brief 暂停  // NOLINT
   *@param ： 无  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Pause(LPVOID param = NULL);
  /**
   *@brief 运行  // NOLINT
   *@param ： 无  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Run(LPVOID param = NULL);
  /**
   *@brief 冲洗，快速将内部缓冲中剩余数据处理完  // NOLINT
   *@param ： 无  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Flush(LPVOID param = NULL);
  /**
   *@brief 获得被控制模块的状态  // NOLINT
   *@param timeout[Input]： 超时值  // NOLINT
   *@param state[Output]：状态  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int GetState(UINT32 timeout, INT32 *state, LPVOID param = NULL);

 protected:
    /**
   *@brief Strategy正式启动  // NOLINT
   *@return： 无  // NOLINT
   */
  virtual void HandleStrategy(void);
 /**
   *@brief  解析mechanis_xml，判断是哪种机制，而后调用具体的机制解析 //  NOLINT
   *@param mechanis_xml 待解析的消息  //  NOLINT
   *@param is_first true为主状态机，false为子状态机  //  NOLINT
   *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
   */
  virtual int ParseMechanismXml(const std::string &mechanis_xml, bool is_first=true);
  /////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////          Construct StateMachine          ///////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////
  /**
    *@brief  构建状态机树，并给每个状态机分配一个唯一的ID //  NOLINT
    *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
    */
  int ConstructStateMachineTree(void);
  /**
   *@brief  构建状态机树，并给每个状态机分配一个唯一的ID //  NOLINT
   *@param  mechanism_ptr  构建该结点及其子孙结点   //  NOLINT
   *@return 成功返回状态机，失败返回NULL  //  NOLINT
   */
  BaseStateMachineImplPtr ConstructStateMachineTree(MechanismPtr mechanism_ptr);
  /**
   *@brief  根据mechanism_ptr信息，构建一个状态机 //  NOLINT
   *@param  mechanism_ptr  构建该结点   //  NOLINT
   *@return 成功返回状态机，失败返回NULL  //  NOLINT
   */
  BaseStateMachineImplPtr ConstructStateMachine(MechanismPtr mechanism_ptr);
  /**
   *@brief  构建mechanism_ptr内的Transition //  NOLINT
   *@param  mechanism_ptr  构建该结点内的Transition   //  NOLINT
   *@param  state_machine_ptr  state_machine_ptr所属的状态机   //  NOLINT
   *@return 成功返回状态机，失败返回NULL  //  NOLINT
   */
  int ConstructTransition(MechanismPtr &mechanism_ptr, BaseStateMachineImplPtr &state_machine_ptr);
  /**
   *@brief  根据name,查找Mechanism //  NOLINT
   *@param  name  要查找的name   //  NOLINT
   *@param  is_child true:查找当前name的child,否则查找自己  // NOLINT
   *@return 成功返回状态机，失败返回NULL  //  NOLINT
   */
  MechanismPtr GetMechanismByName(const std::string &name, bool is_child=true);
  /////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////          utility functions               ///////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////
  /**
   *@brief  获取Service和State的Map信息 //  NOLINT
   *@param  transition_list  从TransitionList里获取service,State信息  //  NOLINT
   *@param  service_state_map  获取的信息存储在这个Map中：state_machine_name,from_state-->to_state   //  NOLINT
   *@return 成功返回0，否则失败  //  NOLINT
   */
  int GeServiceInfo(std::list<std::string> &service_list);
  /**
   *@brief  获取Service信息 //  NOLINT
   *@param  service_list  获取的信息存储在这个Map中：service_name  //  NOLINT
   *@return 成功返回0，否则失败  //  NOLINT
   */
  int GeServiceInfo(TransitionList &transition_list, std::list<std::string> &service_list);
  /**
   *@brief  获取Service和State的Map信息 //  NOLINT
   *@param  service_state_map  获取的信息存储在这个Map中：state_machine_name,from_state-->to_state  //  NOLINT
   *@return 成功返回0，否则失败  //  NOLINT
   */
  int GeStateSwitchInfo(std::multimap<std::string, std::string> &service_state_map);
  /**
   *@brief  获取Service和State的Map信息 //  NOLINT
   *@param  transition_list  从TransitionList里获取service,State信息  //  NOLINT
   *@param  service_state_map  获取的信息存储在这个Map中：state_machine_name,from_state-->to_state   //  NOLINT
   *@return 成功返回0，否则失败  //  NOLINT
   */
  int GeStateSwitchInfo(std::string &state_machine_name, TransitionList &transition_list, std::multimap<std::string, std::string> &service_state_map);

 protected:
  //StateMachineManagerLock                    */*state_machine_manager_lock_*/;
  bool                                        is_run_;   ///<  状态机管理器是否在运行  //  NOLINT
  MechanismPtrList                            free_mechanism_list_;   ///<  还没有解析的Mechanism //  NOLINT
  MechanismPtrList                            used_mechanism_list_;   ///<  已解析的Mechanism //  NOLINT
  CallbackInterfacePtr                        cur_state_machine_;   ///<  当前正在运行的状态机 //  NOLINT
  CallbackInterfacePtr                        default_state_machine_;   ///<  默认的状态机 //  NOLINT
  StatType                                    default_state_id_;   ///<  默认的状态ID //  NOLINT
  BaseStateMachineImplPtrList                 state_machine_list_;   ///<  所有的状态机列表 //  NOLINT

 private:
  boost::thread                              *cur_thread_;           /// 当前活动的线程
  std::string                                 main_mechanism_;       ///< 主Mechanism
  StrategyInterface*                          strategy_ptr_;   ///<  当前状态机策略的Owner //  NOLINT
  std::string                                 conf_dir_;   ///<  配置文件目录，指平台运行的配置文件目录，这个目录下含有strategy、module_link、module等配置文件目录 //  NOLINT
};
}
#endif  // SFP_BASE_STATE_MACHINE_MANAGER_H_  // NOLINT
