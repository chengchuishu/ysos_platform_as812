/**
 *@file BaseStrategyImpl.h
 *@brief Definition of BaseStrategyImpl
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-21 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef SFP_BASE_STRATEGY_IMPL_H_  // NOLINT
#define SFP_BASE_STRATEGY_IMPL_H_  // NOLINT

/// STL Headers
#include <map>
#include <list>
/// ThirdParty Headers
#include <tinyxml2/tinyxml2.h>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
/// Private Ysos Headers
#include "../../../public/include/os_hal_package/lock.h"
#include "../../../public/include/sys_interface_package/strategyinterface.h"
#include "../../../public/include/sys_interface_package/msginterface.h"
#include "../../../protect/include/sys_framework_package/serviceinfotable.h"
#include "../../../public/include/sys_interface_package/agentinterface.h"
#include "../../../protect/include/sys_framework_package/basetransitioninfoimpl.h"

namespace ysos {
typedef std::pair<std::string, AgentInterface*>  AgentServicePair;
/*************************************************************************************************************
 *                      BaseStrategyImpl                                                                     *
 *************************************************************************************************************/
typedef ModuleInterface   RepeaterInterface;
typedef ModuleInterfacePtr   RepeaterInterfacePtr;
/**
  *@brief BaseInterface是整个架构内所有接口的统一基接口，    //  NOLINT
     内部预定义了实例名称，实例的UUID，实例的类型           //  NOLINT
  */
class YSOS_EXPORT BaseStrategyImpl : public StrategyInterface,
  public BaseInterfaceImpl,
  public boost::enable_shared_from_this<BaseStrategyImpl> {
  DECLARE_CREATEINSTANCE(BaseStrategyImpl);
  DISALLOW_COPY_AND_ASSIGN(BaseStrategyImpl);
  DECLARE_PROTECT_CONSTRUCTOR(BaseStrategyImpl);

 public:
  virtual ~BaseStrategyImpl(void);

  //  FackLock
  typedef MutexLock                                          StrategyLock;
  typedef std::map<std::string, AgentInterface*>             AgentMap;

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
  *@brief 加载策略逻辑  // NOLINT              @howto
  *@param strategy_name[Input]： 策略名称  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int LoadStrategy(const std::string &strategy_name);
  /**
   *@brief 创建策略逻辑  // NOLINT
   *@param strategy_name[Input]： 策略名称  // NOLINT
   *@param params[Input]： 策略所需的参数  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int CreateStrategy(const std::string &strategy_name, void *params);
  /**
   *@brief 销毁策略逻辑  // NOLINT
   *@param ： 无  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int DestroyStrategy();
  /**
   *@brief 保存策略逻辑  // NOLINT
   *@param ： 无  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int SaveStrategy();
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
  /**
   *@brief 获得被控制模块的状态  // NOLINT
   *@param timeout[Input]： 超时值  // NOLINT
   *@param state[Output]：状态  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int GetState(UINT32 timeout, INT32 *state, LPVOID param = NULL);

 protected:
  ////////////////////////////////////////////////////////////////////////
  /////////      internal interface    //////////////////////////////////
  //////////////////////////////////////////////////////////////////////
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
   *@brief  解析strategy_xml，判断是哪种机制，而后调用具体的机制解析 //  NOLINT
   *@param strategy_xml 待解析的消息  //  NOLINT
   *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
   */
  int ParseStrategyXml(const std::string &strategy_xml);
  /**
   *@brief  通过Name查找Agent //  NOLINT
   *@param  agent_name  要查找的Agent对应的Service_Name  //  NOLINT
   *@param  all_callback_list  总的成员数  //  NOLINT
   *@return 成功返回Agent，失败返回空  //  NOLINT
   */
  AgentInterface* FindAgentByName(const std::string &service_name);
  /**
   *@brief  新注册一个Agent //  NOLINT
   *@param  agent_ptr  要插入的Agent //  NOLINT
   *@return 成功返回0，否则失败  //  NOLINT
   */
  int RegAgent(AgentInterface *agent_ptr);
  /**
   *@brief  反注册指定的Agent //  NOLINT
   *@param  agent_ptr  要插入的Agent //  NOLINT
   *@return 成功返回0，否则失败  //  NOLINT
   */
  int UnRegAgent(AgentInterface *agent_ptr);
  /**
   *@brief  向Agent发送Dispatch消息 //  NOLINT
   *@param  agent_dispatch_info  要发送的消息  //  NOLINT
   *@return 成功返回0，否则失败  //  NOLINT
   */
  //int OnDispatch(void *agent_dispatch_info);

 protected:
  bool                          is_run_;  ///<  当前Strategy是否在运行 //  NOLINT
  bool                          is_loaded_;  ///<  标识Strategy是否已Load完成  //  NOLINT
  StrategyLock                 *strategy_lock_;
  StrategyLock                 *cmd_queue_lock_;
  StrategyLock                 *msg_queue_lock_;
  MsgQueueInterfacePtr      cmd_queue_;
  MsgQueueInterfacePtr      msg_queue_;
  ControlInterfacePtr           strategy_imp_;  ///<  最终执行策略的类 //  NOLINT
  std::string                   state_name_;    ///< 默认状态名

 private:
  boost::thread             *cur_thread_;           ///< 当前活动的线程
  //std::string                conf_dir_;

  StrategyLock              *agent_map_lock_;        ///<  used for agent_map  //  NOLINT
  AgentMap                   agent_map_;              ///<  service_name --->  agent_ptr  //  NOLINT
};
typedef boost::shared_ptr<BaseStrategyImpl>   BaseStrategyImplPtr;


}
#endif  // SFP_BASE_STRATEGY_IMPL_H_  // NOLINT
