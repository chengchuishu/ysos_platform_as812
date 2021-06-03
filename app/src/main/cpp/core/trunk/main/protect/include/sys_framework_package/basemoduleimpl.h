/**
 *@file ModuleImpl.h
 *@brief Definition of ModuleImpl
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-21 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef SIP_BASE_MODULE_IMPL_H_  // NOLINT
#define SIP_BASE_MODULE_IMPL_H_  // NOLINT

/// stl headers //  NOLINT
#include <map>
/// boost headers //  NOLINT
#include <boost/bind.hpp>
#include <boost/function.hpp>
/// ysos private headers //  NOLINT
#include "../../../public/include/os_hal_package/lock.h"
#include "../../../public/include/sys_interface_package/moduleinterface.h"
#include "../../../public/include/sys_interface_package/teamparamsinterface.h"
#include "../../../public/include/core_help_package/baseinterfaceimpl.h"
#include "../../../public/include/os_hal_package/driverinterface.h"
#include "../../../public/include/sys_interface_package/ysos_enum.h"

namespace ysos {
typedef boost::shared_ptr<ModuleInterface::ModuleDataInfo> ModuleDataInfoPtr;
class BaseModuleImpl;

/*
  @brief 实现了在BaseModule中，多线程调用CallbackQueue
*/
class BaseModuleCallbackFunction {
 public:
  BaseModuleCallbackFunction(BufferInterfacePtr input_buffer=NULL, BufferInterfacePtr output_buffer=NULL, void *context_ptr=NULL);
  virtual ~BaseModuleCallbackFunction();

  virtual int operator()(const CallbackInterfacePtr &callback_ptr, BaseModuleImpl *context_ptr=NULL);

 protected:
  BufferInterfacePtr            input_buffer_;
  BufferInterfacePtr            output_buffer_;
  void                         *context_ptr_;
};

/*
  @brief Module的基类
*/
class YSOS_EXPORT BaseModuleImpl : public ModuleInterface, public BaseInterfaceImpl {
  //DECLARE_CREATEINSTANCE(BaseModuleImpl);
  DISALLOW_COPY_AND_ASSIGN(BaseModuleImpl);
  DECLARE_PROTECT_CONSTRUCTOR(BaseModuleImpl);

 public:
  virtual ~BaseModuleImpl(void);

  /**
     *@brief  ModuleStatInfo里记录的是当前Module的后链接的Module状态信息//  NOLINT
     */
  struct ModuleStatInfo {
    int          stat; ///<  该Module当前的状态 //  NOLINT
    INT64        owner_id;

    ModuleStatInfo() {}
  };
  typedef boost::shared_ptr<ModuleStatInfo>   ModuleStatInfoPtr;
  typedef std::map<INT64, ModuleStatInfoPtr>::iterator ModuleStatInfoIterator;

  /**
   *@brief 基本初始化  // NOLINT
   *@param param： 初始化的参数  // NOLINT
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
  *@brief 类似于模块的初始化功能，子类无需实现
  *@param open需要的参数
  *@return 成功返回0，否则失败
  */
  virtual int Open(LPVOID param = NULL, LPVOID context = NULL);
  /**
   *@brief 运行，子类无需实现  // NOLINT
   *@param ： 无  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Run(LPVOID param = NULL);
  /**
   *@brief 停止运行，关闭模块，子类无需实现
   *       只有当ref_cout为0时，才能正常关闭
   *@return 成功返回0，否则失败
   */
  virtual int Stop(LPVOID param = NULL);
  /**
   *@brief 暂停，子类无需实现  // NOLINT
   *@param ： 无  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Pause(LPVOID param = NULL);
  /**
   *@brief 关闭，子类无需实现
   *@return 成功返回0，否则失败
   */
  virtual int Close(LPVOID param = NULL);
  /**
   *@brief Module用来接收输入数据，设置引用计数等
   *@param control_id 标记是哪种操作命令
   *@param param 与命令对应的参数
   *@return 成功返回0，否则失败
   */
  virtual int Ioctl(INT32 control_id, LPVOID param = NULL);
  /**
    *@brief  Buffer由子类实现，这个函数需子类实现//  NOLINT
    *        //  NOLINT
    *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
    */
  virtual int Flush(LPVOID param = NULL);
  /**
   *@brief 获得被控制模块的状态  // NOLINT
   *@param timeout[Input]： 超时值  // NOLINT
   *@param state[Output]：状态  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int GetState(UINT32 time_out, INT32 *state, LPVOID param = NULL);
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
  /**
   *@brief 增加回调函数
   *@param callback: 待增加的回调函数
   *@param owner_id: 标识Callback来自于哪一个模块。为0时，Callback添加进precallbackqueue，
                   其他值，添加进nextcallbackqueue
   *@param type:
   *@return 成功返回0，否则失败
   */
  virtual int AddCallback(CallbackInterfacePtr callback, INT64& owner_id, INT32 type);  // NOLINT
  /**
   *@brief 删除属于owner_id模块的所有Callback
   *@param owner_id: 标识Module
   *@return 成功返回0，否则失败
   */
  virtual int RemoveCallback(INT64& owner_id);
  /**
    *@brief  当team_param_for_data_flow_与team_param_for_control_flow_不同时，执行这个函数，保持两个//  NOLINT
    *         变量相同  //  NOLINT
    *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
    */
  int UpdateTeamParam(void);
  /**
    *@brief   遍历Module的NextCallback //  NOLINT
    *@param function_ptr: 对每一个Next_Callback执行的操作
    *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
    */
  int IteratorNextCallback(BaseModuleCallbackFunction &function_ptr);
  /**
    *@brief   遍历Module的PrevCallback //  NOLINT
    *@param function_ptr: 对每一个Prev_Callback执行的操作
    *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
    */
  int IteratorPrevCallback(BaseModuleCallbackFunction &function_ptr);

  void DumpPreCallback();

  void DumpNextCallback();

 protected:
  /**
   *@brief 类似于模块的初始化功能，子类实现，只关注业务
   *@param open需要的参数
   *@return 成功返回0，否则失败
  */
  virtual int RealOpen(LPVOID param = NULL) = 0;
  /**
   *@brief 运行，子类实现，只关注业务  // NOLINT
   *@param ： 无  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int RealRun() = 0;
  /**
   *@brief 暂停，子类实现，只关注业务 // NOLINT
   *@param ： 无  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int RealPause() = 0;
  /**
  *@brief 停止运行，关闭模块，子类实现，只关注业务
  *       只有当ref_cout为0时，才能正常关闭
  *@return 成功返回0，否则失败
  */
  virtual int RealStop() = 0;
  /**
   *@brief 关闭，，子类实现，只关注业务
   *@return 成功返回0，否则失败
   */
  virtual int RealClose() = 0;
  /**
   *@brief Module用来接收输入数据，设置引用计数等
   *@param control_id 标记是哪种操作命令
   *@param team_param 存储命令的TeamParam
   *@param param 与命令对应的参数
   *@return 成功返回0，否则失败
   */
  virtual int RealIoctl(INT32 control_id, TeamParamsInterface<default_variant_t> *team_parm, LPVOID param = NULL);
  /**
    *@brief  当team_param_for_data_flow_与team_param_for_control_flow_不同时，执行这个函数，保持两个//  NOLINT
    *         变量相同, 由子类实现具体业务相关的代码  //  NOLINT
    *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
    */
  virtual int RealUpdateTeamParam(TeamParamsInterface<default_variant_t> *team_parm);
  /**
   *@brief  更新teamParams，由子类实现//  NOLINT
   *@param team_parm 要更新的teamParams参数，可能是team_param_for_control_flow_的，也可能是team_param_for_data_flow_的
   *@param param 与命令对应的参数
   *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
   */
  /*virtual int UpdateCommand(TeamParamsInterface<default_variant_t> *team_parm, LPVOID param);*/
  /**
    *@brief  初始化ModuleDataInfo //  NOLINT
    *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
    */
  virtual int InitalDataInfo() = 0;
  /**
   *@brief  执行Update_Command命令，由基类实现//  NOLINT
   *@param control_id 标记是哪种操作命令
   *@param param 与命令对应的参数
   *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
   */
  int UpdateControlCommand(INT32 control_id,LPVOID param);
  /**
   *@brief  执行Update_Command命令，由基类实现//  NOLINT
   *@param param 与命令对应的参数
   *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
   */
  int UpdateDataCommand(LPVOID param);
  /**
   *@brief  将Module的状态加入当前模块中//  NOLINT
   *@param owner_id 与命令对应的参数
   *@param mode_stat Module的状态
   *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
   */
  int InsertModuleStatRecord(INT64 owner_id, const int mode_stat=PROP_CLOSE);
  /**
   *@brief  将Module的状态从当前模块中删除//  NOLINT
   *@param owner_id 与命令对应的参数
   *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
   */
  int RemoveModuleStatRecord(INT64 owner_id);
  /**
   *@brief 增加Next回调函数
   *@param callback: 待增加的回调函数
   *@param owner_id: 标识Callback来自于哪一个模块。为0时，Callback添加进precallbackqueue，
                   其他值，添加进nextcallbackqueue
   *@param type:
   *@return 成功返回0，否则失败
   */
  virtual int AddNextCallback(CallbackInterfacePtr callback, INT64& owner_id, INT32 type);
  /**
   *@brief  配置文件中设置的Key、Value对，会在Initialized函数中调用 //  NOLINT
   *@param key 配置参数中的Key  // NOLINT
   *@param value 与Key对应的Value  // NOLINT
   *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
   */
  virtual int Initialized(const std::string &key, const std::string &value);
  /**
   *@brief  根据Value值，设置正确的Module类型 //  NOLINT
   *@param value Module的类型  // NOLINT
   *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
   */
  int SetModuleType(const std::string &value);
  /**
   *@brief  将状态回调函数设置给Driver //  NOLINT
   *@param driver_ptr_ 要设置的Driver指针
   *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
   */
  int SetStatusEventCallback(DriverInterfacePtr driver_ptr_);

 private:
  /**
   *@brief 创建Module的BufferPool，通常在子类赋值完ModuleDataInfo的属性后，调用。
   *       子类如有特殊需求，可重写该函数。
   *       该函数在Initialized函数中，被自动调用，不需要显示调用。
   */
  virtual int AllocateBufferPool(void);
  /**
   *@brief  根据owner_id查找对应的后链接Module//  NOLINT
   *@param owner_id 对应Module的owner_id
   *@return 成功返回ModuleStat的指针，失败返回链表End  //  NOLINT
   */
  ModuleStatInfoPtr FindNextModuleStatInfoByOwnerId(const INT64 &owner_id);
  /**
     *@brief  根据owner_id更新指定Module的状态//  NOLINT
     *@param owner_id 对应Module的owner_id
     *@return Module不存在，返回非0，其余，返回0  //  NOLINT
     */
  int UpdateSpecifyNextModuleStatByOwnerId(const INT64 &owner_id, int stat);
  /**
  *@brief  更新本模块的状态//  NOLINT
  *@param cur_stat 当前Module的状态
  *@param dst_stat 目标Module的状态
  *@param param    透传参数
  *@return 成功返回0，失败返回非0  //  NOLINT
  */
  int UpdateModuleStat(int &cur_stat, int &dst_stat, LPVOID param=NULL);
  /**
   *@brief  根据Module所有的ModuleLink的当前状态，
   *        根据这个状态，与module_stat_比较，进而采取动作，使状态一致//  NOLINT
   *@return 返回当前Module的真实状态  //  NOLINT
   */
  int ComputeModuleLinkCurStat(void);
  /**
   *@brief  读取配置文件中的参数，进行配置 //  NOLINT
   *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
   */
  int ConfigInitialize(void);
  /**
   *@brief  发送模块的状态或事件消息 //  NOLINT
   *@param status_event_code 状态或事件值
   *@param detail   状态或事件的详细信息
   *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
   */
  int SendStatusEventMessage(const std::string &status_event_code, const std::string &detail);
  /**
   *@brief  发送模块的状态或事件消息 //  NOLINT
   *@param status_event_code 状态或事件值
   *@param module_name 该事件所属的模块名字
   *@param detail   状态或事件的详细信息
   *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
   */
  int NotifyStatusEventMessage(const std::string &status_event_code, const std::string &module_name, const std::string &detail);
  /**
   *@brief  封装模块的状态或事件消息 //  NOLINT
   *@param status_event_code 状态或事件值
   *@param module_name 该事件所属的模块名字
   *@param detail   状态或事件的详细信息
   *@return 成功返回事件消息数据，失败返回NULLL  //  NOLINT
   */
  BufferInterfacePtr WrapStatusEventMessage(const std::string &status_event_code, const std::string &module_name, const std::string &detail);

 protected:
  ModuleDataInfo* module_data_info_; ///<  缓冲区数据信息  // NOLINT

  TeamParamsInterface<default_variant_t> *team_param_for_data_flow_;  ///<  子类的公开数据，在使用中，会被加锁保护，仅在data数据流中使用 //  NOLINT
  TeamParamsInterface<default_variant_t> *team_param_for_control_flow_;  ///< 子类的公开数据拷贝，仅在控制流中使用，目的是避免与同时数据流使用，冲突 //  NOLINT

  LightLock                *pre_callback_queue_lock_;
  CallbackQueueInterface   *pre_callback_queue_;        ///< module的precallbackqueue
  LightLock                *next_callback_queue_lock_;
  CallbackQueueInterface   *next_callback_queue_;     ///< module的nextcallbackqueue

  ///<  BufferPool 子类根据需要定义，基类不提供 //  NOLINT

  BufferPoolInterfacePtr      buffer_pool_ptr_; ///<  BufferPool指针 //  NOLINT

 protected:
  int                 module_stat_;   ///<  当前Module的状态  // NOLINT
  std::string         callback_;      ///<  module支持的Callback，如果有多个，以"|"区分 // NOLINT
  std::string         driver_;        ///<  module支持的Driver，有一个或没有  // NOLINT

 private:
  UINT32                     module_run_state_count_;  ///<  模块Run状态的引用的次数 //  NOLINT
  UINT32                     module_open_state_count_;  ///<  模块Open状态的引用的次数 //  NOLINT
  LightLock                 *module_lock_;          ///<  用于独占Module操作_  // NOLINT
  UINT32                     module_ref_count_;  ///<  模块被引用的次数,只在ModuleLink链接时，才计数，表示该模块被几个Module链接  // NOLINT
  INT32                      team_param_for_control_flow_count_;  ///< 与team_param_for_data_flow_count_比较，当不同时，说明team_param的两份数据不同，需同步，以值大的代表的数据为准 //  NOLINT
  INT32                      team_param_for_data_flow_count_;    ///< 代表team_param_for_data_flow_，team_param_for_control_flow_count_代表team_param_for_control_flow_ //  NOLINT
  LightLock                 *team_param_for_control_flow_lock_;  ///< 当team_param_for_control_flow_被修改时，加锁 //  NOLINT
  LightLock                 *team_param_for_data_flow_lock_;  ///< 当team_param_for_data_flow_被修改时，加锁 //  NOLINT
  std::list<INT64>           next_callback_error_list_;       ///< 记录调用NextCallback发生错误时的NextModule的地址
  std::map<INT64, ModuleStatInfoPtr>   module_stat_info_map_;      ///<  记录所有后链接Module的状态 //  NOLINT
  std::string   module_link_name_;      ///<  记录所属ModuleLink的Name //  NOLINT
  uint32_t      module_link_level_;     ///< 当前对象在ModuleLink中的Level
  CallbackInterfacePtr        status_event_callback_ptr_;  ///<  Status_Event专用Callback //  NOLINT
  CallbackInterfacePtr        status_notify_callback_ptr_;  ///< StatusNotifyEvent的Calllback

 private:
  std::string                  remark_;        ///<  callback对象的备注  //  NOLINT
  std::string                  version_;       ///<  callback对象的版本号 //  NOLINT
};
}
#endif  // SIP_BASE_MODULE_IMPL_H_  // NOLINT