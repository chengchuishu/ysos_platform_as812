/**
 *@file autoselfstrategycallback.h
 *@brief the auto self strategy call back
 *@version 1.0
 *@author z.gg.
 *@date Created on: 2020-5-26 10:00:00
 *@copyright Copyright (c) 2020 YS Technology. All rights reserved.
 *
 */

#ifndef AUTO_CALLBACK_H_
#define AUTO_CALLBACK_H_

#include <json/json.h>

/// Private Headers //  NOLINT
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basemoduleimpl.h"  //NOLINT
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basemodulecallbackimpl.h"  //NOLINT
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basestrategycallbackimpl.h"  //NOLINT
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/packageconfigimpl.h"
#include "../../../../../../core/trunk/main/protect/include/os_hal_package/datainterfacemanager.h"
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"

namespace ysos {

/**
 *@brief  任务计划节点定义 //NOLINT
*/
struct Task {
  int action_hour;
  int action_minute;
  int action_type;
  std::string action_place;
};

/**
 *@brief  自主管理策略层 //NOLINT
*/
class YSOS_EXPORT AutoSelfStrategyCallback : public BaseStrategyCallbackImpl {
  DECLARE_CREATEINSTANCE(AutoSelfStrategyCallback);
  DISALLOW_COPY_AND_ASSIGN(AutoSelfStrategyCallback);
  DECLARE_PROTECT_CONSTRUCTOR(AutoSelfStrategyCallback);

 public:
  ~AutoSelfStrategyCallback(void);

  virtual int Initialize(void *param=NULL);
 protected:
  /**
  *@brief 处理事件的消息  // NOLINT
  *@param event_name[Input]： 事件的类型  // NOLINT
  *@param input_buffer[input]：待处理的数据  // NOLINT
  *@param context[Input]： 回调处理的上下文数据  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int HandleMessage(const std::string &event_name, BufferInterfacePtr input_buffer, CallbackInterface *context);

  /**
  *@brief  配置文件中设置的Key、Value对，会在Initialized函数中调用 //  NOLINT
  *@param key 配置参数中的Key  // NOLINT
  *@param value 与Key对应的Value  // NOLINT
  *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
  */
  virtual int Initialized(const std::string &key, const std::string &value);

 private:
  /**
  *@brief 请求平台服务  // NOLINT
  *@param service_name[Input]： 服务名  // NOLINT
  *@param id[Input]： 命令号  // NOLINT
  *@param type[Input]： 参数类型  // NOLINT
  *@param json_value[Input]： 参数数据  // NOLINT
  *@param context[Input]： 回调处理的上下文数据  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  int RequestService(std::string service_name, std::string id, std::string type, std::string json_value, CallbackInterface *context);

  /**
  *@brief 载入任务配置  // NOLINT
  *@param day[Input]： 日期  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  int LoadTask(const boost::posix_time::ptime::date_type &day);

  /**
  *@brief 查询准备执行的任务  // NOLINT
  *@return： 没有任务（了）     YSOS_ERROR_FAILED          // NOLINT
  *@return： 下一个任务时间未到 YSOS_ERROR_SKIP            // NOLINT
  *@return： 下一个任务时间到   YSOS_ERROR_SUCCESS         // NOLINT
  */
  int getTaskReady();

  /**
  *@brief 去充电 // NOLINT
  *@return：是否可以再次尝试
  */
  bool TimeToRetry();

  /**
  *@brief 更新充电信息  // NOLINT
  */
  int UpdateChargeInfo(CallbackInterface *context);

  /**
  *@brief 更新电量信息  // NOLINT
  */
  int UpdateBatteryInfo(CallbackInterface *context);

  /**
  *@brief 更新急停按钮信息  // NOLINT
  */
  int UpdateEmergencyStopInfo(CallbackInterface *context);
  /**
  *@brief 解析充电行走信息  // NOLINT
  */
  int ParserMoveInfo(BufferInterfacePtr input_buffer, std::string &move_state);

  /**
  *@brief 地图初始化  // NOLINT
  *@param context[Input]： 回调处理的上下文数据  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  int MapInitialize(const char* initParam, CallbackInterface *context);

  /**
  *@brief 充电  // NOLINT
  *@param context[Input]： 回调处理的上下文数据  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  int GoHome(CallbackInterface *context);

  /**
  *@brief 充电中途遇移动障碍物  // NOLINT
  *@param context[Input]： 回调处理的上下文数据  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  int GoHomeWarning(CallbackInterface *context);

  /**
  *@brief 充电失败  // NOLINT
  *@param context[Input]： 回调处理的上下文数据  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  int FailedToCharge(CallbackInterface *context);

  /**
  *@brief 充电成功  // NOLINT
  *@param context[Input]： 回调处理的上下文数据  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  int SucceedToCharge(bool inservice, CallbackInterface *context);

  /**
  *@brief 离开充电桩  // NOLINT
  *@param inservice[Input]： 是否在服务中  // NOLINT
  *@param context[Input]： 回调处理的上下文数据  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  int LeaveCharger(bool inservice, CallbackInterface *context);

  /**
  *@brief 上班  // NOLINT
  *@param context[Input]： 回调处理的上下文数据  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  int StartWork(std::string work, CallbackInterface *context);

  /**
  *@brief 设置自动开关机  // NOLINT
  *@param context[Input]： 回调处理的上下文数据  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  int ManagerSelf(CallbackInterface *context);

  /**
  *@brief 通知人离开  // NOLINT
  *@param context[Input]： 回调处理的上下文数据  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  void NotifyCustomerLeave(CallbackInterface *context);

  
  void ThreadFunRunSelf(CallbackInterface *context);

  bool CheckTaskReadyBefore();

 private:
  int charging_;                    //< 是否充电中（0：未，1桩）（线充暂不支持）
  int battery_;                     //< 电量信息（百分比）
  int emergency_stop_;              //< 急停按钮是否按下
  int battery_range_;               //< 电量信息（范围）

  bool mngself_;                    //< 关机标记
  bool poweroff_;                   //< 关机标记

  ///< retry to go home
  bool last_retry_;
  int gohome_cnt_;                       //< 充电计数（0/1/2/3...:充电计数; INFINATE: 成功/失败）
  boost::posix_time::ptime last_failed_; //< 上次充电失败时间

  ///< charging warning
  int warning_gap_;
  boost::posix_time::ptime last_warning_spk_;
  std::string warning_words_;

  ///< charging answer
  std::string succeed_to_charge_;
  std::string wait_for_retry_;
  std::string failed_to_charge_;
  std::string leave_charger_;
  std::string ignore_charge_;

  ///< start to work
  std::string start_to_work_;
  std::string power_full_;
  std::string breakDate_;
  ///< task
  int task_run_idx_;
  int today_task_num_;
  Task *today_task_;
  boost::posix_time::ptime::date_type task_day_;
  std::tm nxt_work_day_;
  
  ///<thread for detect event
  boost::recursive_mutex rmutex_member_;
  boost::shared_ptr<boost::thread> thread_run_self_ptr_;
  boost::atomic<int> atomic_thread_run_self_stop_flag_;

  ///< state -> event
  std::map<std::string, std::string> map_state_event_;  ///< 状态到事件的映射


  DataInterfacePtr data_ptr_;       //< 内存指针
};

}///< namespace ysos    //NOLINT

#endif///< AUTO_CALLBACK_H_    //NOLINT