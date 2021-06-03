/**
 *@file BaseRepeaterModuleImpl.h
 *@brief Repeater base implement
 *@version 1.0
 *@author Pan
 *@date Created on: 2016-05-31 18:47:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef SIP_BASE_REPEATER_IMPL_H_  // NOLINT
#define SIP_BASE_REPEATER_IMPL_H_  // NOLINT

/// stl headers //  NOLINT
#include <map>
#include <set>
/// ysos private headers //  NOLINT
#include "../../../protect/include/sys_framework_package/basemoduleimpl.h"         //NOLINT
#include "../../../protect/include/sys_framework_package/basemodulecallbackimpl.h" //NOLINT
#include "../../../protect/include/sys_framework_package/nextcallbackqueue.h" //NOLINT
#include "../../../public/include/sys_interface_package/strategyinterface.h" //NOLINT
#include "../../../protect/include/sys_framework_package/basetransitioninfoimpl.h"

#define RETURNONFAIL(res) if (YSOS_ERROR_SUCCESS != res) return res;

namespace ysos {
/*
  @brief Repeater基类
*/
class YSOS_EXPORT BaseRepeaterModuleImpl : public BaseModuleImpl {
  DECLARE_CREATEINSTANCE(BaseRepeaterModuleImpl);
  DISALLOW_COPY_AND_ASSIGN(BaseRepeaterModuleImpl);
  DECLARE_PROTECT_CONSTRUCTOR(BaseRepeaterModuleImpl);

 public:
  virtual ~BaseRepeaterModuleImpl(void);

  typedef  std::multimap<ModuleInterfacePtr, ModuleInterfacePtr>   RepeaterMap;
  typedef  std::list<TransitionInfoInterfacePtr>                   TransitionInfoPtrList;
  typedef  std::set<TransitionPtr>                                TransitionPtrList;
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
   *@brief 获得被控制模块的状态  // NOLINT
   *@param timeout[Input]： 超时值  // NOLINT
   *@param state[Output]：状态  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int GetState(UINT32 time_out, INT32 *state, LPVOID param = NULL);

  /**
  *@brief 获取接口的属性值  // NOLINT
  *@param type_id[Input]： 属性的ID  // NOLINT
  *@param type[Output]： 属性值的值  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  int GetProperty(int type_id, void *type);

  /**
   *@brief 设置接口的属性值  // NOLINT
   *@param type_id[Input]： 属性的ID  // NOLINT
   *@param type[input]： 属性值的值  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  int SetProperty(int type_id, void *type);
  /**
   *@brief Module用来接收输入数据，设置引用计数等
   *@param control_id 标记是哪种操作命令
   *@param param 与命令对应的参数
   *@return 成功返回0，否则失败
   */
  int Ioctl(INT32 control_id, LPVOID param /* = NULL */);
  /**
   *@brief 增加回调函数
   *@param callback: 待增加的回调函数
   *@param owner_id: 标识Callback来自于哪一个模块。为0时，Callback添加进precallbackqueue，
                   其他值，添加进nextcallbackqueue
   *@param type:
   *@return 成功返回0，否则失败
   */
  int AddCallback(CallbackInterfacePtr callback, INT64& owner_id, INT32 type);  // NOLINT
  /**
   *@brief 删除属于owner_id模块的所有Callback
   *@param owner_id: 标识Module
   *@return 成功返回0，否则失败
   */
  int RemoveCallback(INT64& owner_id);

  int UpdateNextCallbackQueueMap();

  int GetRepeatedNextCallbackQueue(ModuleInterfacePtr prev_module,CallbackQueueInterfacePtr& next_call_queue);

  bool IsCallbackOf(ModuleInterfacePtr module_ptr,CallbackInterfacePtr test_callback_ptr);

  int Observe(BufferInterfacePtr data_ptr,void* context_ptr);

  //ModuleInterfacePtr RetrievePrevModule()const;
  void SetMyModuleLink(ModuleLinkInterfacePtr module_link_ptr) {
    my_module_link_ptr_ = module_link_ptr;
  }

  void SetRepeatMap(std::multimap<ModuleInterfacePtr,ModuleInterfacePtr> repeat_map) {
    repeat_map_ = repeat_map;
    UpdatePrevModuleList();
  }

  std::multimap<ModuleInterfacePtr,ModuleInterfacePtr> GetRepeatMap()const {
    return repeat_map_;
  }

 protected:
  /**
  *@brief Module用来接收输入数据，设置引用计数等
  *@param control_id 标记是哪种操作命令
  *@param team_param 存储命令的TeamParam
  *@param param 与命令对应的参数
  *@return 成功返回0，否则失败
  */
  int RealIoctl(INT32 control_id, TeamParamsInterface<default_variant_t> *team_parm, LPVOID param /* = NULL */);
  /**
    *@brief  当team_param_for_data_flow_与team_param_for_control_flow_不同时，执行这个函数，保持两个//  NOLINT
    *         变量相同, 由子类实现具体业务相关的代码  //  NOLINT
    *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
    */
  int RealUpdateTeamParam(TeamParamsInterface<default_variant_t> *team_parm);
  /**
   *@brief  初始化ModuleDataInfo //  NOLINT
   *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
   */
  int InitalDataInfo();
  /**
   *@brief 类似于模块的初始化功能，子类实现，只关注业务
   *@param open需要的参数
   *@return 成功返回0，否则失败
  */
  int RealOpen(LPVOID param = NULL);
  /**
   *@brief 运行，子类实现，只关注业务  // NOLINT
   *@param ： 无  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  int RealRun();
  /**
   *@brief 暂停，子类实现，只关注业务 // NOLINT
   *@param ： 无  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  int RealPause();
  /**
  *@brief 停止运行，关闭模块，子类实现，只关注业务
  *       只有当ref_cout为0时，才能正常关闭
  *@return 成功返回0，否则失败
  */
  int RealStop();
  /**
   *@brief 关闭，，子类实现，只关注业务
   *@return 成功返回0，否则失败
   */
  int RealClose();
  /**
  *@brief 返回当前Callback对应的PrevModule指针  // NOLINT
  *@param context[Input]： 回调处理的上下文数据,是BaseModuelCallbackContext* 类型  // NOLINT
  *@return： 成功返回Module指针，失败返回NULL  // NOLINT
  */
  BaseModuleImpl *GetPrevModule(void *context);
  /**
  *@brief 判断service_name是否是Transition相匹配  // NOLINT
  *@param transition_ptr[Input]： 数据处理  // NOLINT
  *@param service_name[Input]： 待处理的服务  // NOLINT
  *@return： 匹配返回true，不匹配返回false  // NOLINT
  */
  bool IfMatchedByTransition(TransitionPtr transition_ptr, const std::string &service_name);

  void UpdatePrevModuleList();

 protected:
  ModuleLinkInterfacePtr my_module_link_ptr_;

 private:
  void EraseSingleMappingPair(
    std::pair<ModuleInterfacePtr, ModuleInterfacePtr> pair_to_erase);
  //overwrite team param type
  //using type std::multimap instead of team param
  RepeaterMap                               repeat_map_;
  RepeaterMap                               repeat_map_clone_;  //clone is for module using
  //  从repeat_map_获取Repeater的PrevModuleList//NOLINT
  //  当repeat_map_有变时重建//NOLINT
  //  目的是在控制时不重复执行//NOLINT
  ModuleInterfacePtrList                                      prev_module_list_;
  CallbackQueueInterfacePtr                                   repeat_next_callback_queue_ptr_;
  std::multimap<ModuleInterfacePtr,CallbackQueueInterfacePtr> next_callback_queue_map_;

  //  listening for strategy
  TransitionPtrList                                           transition_list_;//clone is for module using
  TransitionPtrList                                           transition_list_clone_;

  // thread data lock
  LightLock* transition_list_lock_,*callback_queue_lock_,*repeat_map_lock_;
  LightLock* ioctrl_lock_;
};
}
#endif    //SIP_BASE_REPEATER_IMPL_H_  //NOLINT
