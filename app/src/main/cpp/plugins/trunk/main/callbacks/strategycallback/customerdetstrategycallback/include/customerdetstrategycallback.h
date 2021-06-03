/**
 *@file customerdetstrategycallback.h
 *@brief the customer detect strategy call back
 *@version 1.0
 *@author LiShengJun
 *@date Created on: 2017-08-21 10:00:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

#ifndef CUSTOMER_DET_CALLBACK_H_
#define CUSTOMER_DET_CALLBACK_H_

#include <boost/timer.hpp>
#include <json/json.h>

/// Private Headers //  NOLINT
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basemoduleimpl.h"
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basemodulecallbackimpl.h"
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basestrategycallbackimpl.h"
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/packageconfigimpl.h"
#include "../../../../../../core/trunk/main/protect/include/os_hal_package/datainterfacemanager.h"

namespace ysos {

#define DISTANCE_RANGE_MAXNUM 6

struct CstmFaceInfo {
  // base
  int track_id;
  float rate;
  float x_rate;
  // rcgn
  int age;
  int gender;
  std::string id_card;
  std::string name;
};

/**
 *@brief  障碍物检测策略层 //NOLINT
*/
class YSOS_EXPORT CustomerDetStrategyCallback : public BaseStrategyCallbackImpl {
  DECLARE_CREATEINSTANCE(CustomerDetStrategyCallback);
  DISALLOW_COPY_AND_ASSIGN(CustomerDetStrategyCallback);
  DECLARE_PROTECT_CONSTRUCTOR(CustomerDetStrategyCallback);

 public:
  ~CustomerDetStrategyCallback(void);

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
  *@brief 检查是否可进入服务状态  // NOLINT
  */
  int CheckAvailable(const std::string &robot_state);

  /**
  *@brief 客户靠近  // NOLINT
  */
  int CustomerArrive(CallbackInterface *context);

  /**
  *@brief 开始服务              // NOLINT
   @greet_type -1 不切主页      // NOLINT
   @greet_type 0  不需要问候    // NOLINT
   @greet_type 1  重新问候      // NOLINT
   @greet_type 2  同一个人      // NOLINT
  */
  int CustomerServe(int greet_type, CallbackInterface *context);

  /**
  *@brief 客户离开  // NOLINT
  */
  int CustomerLeave(const std::string &robot_state, CallbackInterface *context);

  /**
  *@brief 通知前端客户信息  // NOLINT
  */
  void NotifyCustomerServe(CallbackInterface *context);

  /**
  *@brief 通知前端客户离开  // NOLINT
  */
  void NotifyCustomerLeave(CallbackInterface *context);

  /**
  *@brief 恢复人离开状态  // NOLINT
  */
  int ResumeIdleState(const std::string &robot_state, const std::string &words, CallbackInterface *context);

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
  *@brief 解析人脸信息  // NOLINT
   @retval -1 取得失败  // NOLINT
   @retval 0  检测信息  // NOLINT
   @retval 1  识别信息  // NOLINT
   @retval 2  1比1信息  // NOLINT
  */
  int ParserFaceInfo(BufferInterfacePtr input_buffer, int &face_num, CstmFaceInfo &face_info);

  /**
  *@brief 人脸是否切换  // NOLINT
   @retval YSOS_ERROR_SUCCESS 切换  // NOLINT
   @retval YSOS_ERROR_SKIP  同一个  // NOLINT
   @retval YSOS_ERROR_FAILED 待确认  // NOLINT
  */
  int TrackIdUpdated(int face_num, CstmFaceInfo &face_info);

  /**
  *@brief 清理状态（设成无人状态，关闭计时器）  // NOLINT
  */
  void ResetStatus();

  /**
  *@brief 清理状态（设成无人状态，关闭计时器）  // NOLINT
  */
  void ClearCustomerInfo();

  /**
  *@brief 开始人脸识别计时器  // NOLINT
  */
  void StartTimer(boost::posix_time::ptime &last_tm);

  /**
  *@brief 计时器  // NOLINT
  */
  double GetTimeElapse(boost::posix_time::ptime &last_tm);

 private:
  std::string low_power_refuse_;     ///< 低电量警告语（定点巡航，原地巡航命令用）  //NOLINT
  bool charging_serve_;              ///< 充电时服务否  //NOLINT
  bool static_immediate_;            ///< 静止模式是否立刻切到无人  //NOLINT

  bool audio_open_;                  ///< 是否采集音频

  // 巡航准备（不识别人）
  double prepare_cruise_tm_;                 ///< 准备巡航时间        //NOLINT
  boost::posix_time::ptime prepare_start_;   ///< 准备巡航开始计时    //NOLINT

  // 到达判定
  double greet_dis_;                     ///< 问候距离              //NOLINT
  double rcg_max_wait_;                  ///< 人脸检测等待时间      //NOLINT
  boost::posix_time::ptime rcg_start_;   ///< 人脸检测开始计时      //NOLINT
  boost::posix_time::ptime HandleMessage_start_;   ///< 人脸检测开始计时      //NOLINT
  // 识别标记位
  int facedet_flag_;                     ///< 人脸识别标记          //NOLINT
  int facedet_collect_;                  ///< 人脸识别结果记录(bit1: 1-n; bit2: 1-1)  //NOLINT

  // 客户切换
  bool may_change_;                          ///< 发现人脸更新       //NOLINT
  boost::posix_time::ptime chg_face_tm_;     ///< 人脸更新开始时间   //NOLINT
  double face_chg_ctu_;                      ///< 阈值               //NOLINT
  float face_chg_rate_;                      ///< 阈值               //NOLINT

  // 客户信息
  int customer_num_;
  CstmFaceInfo customer_info_;

  // 离开判定
  boost::posix_time::ptime last_serve_tm_;     ///< 上次服务时间（近处有人脸）  //NOLINT
  unsigned int range_num;                      ///< 区间个数    //NOLINT
  double dis_range_[DISTANCE_RANGE_MAXNUM];    ///< 距离区间    //NOLINT
  double wait_time_[DISTANCE_RANGE_MAXNUM];    ///< 距离区间对应的的等待时间    //NOLINT
  // 近距离无人脸提示语
  bool near_remind_falg_;                      ///< 提示语标记                 //NOLINT
  int near_remind_tm_;                         ///< 近距离无人脸超时时间       //NOLINT
  std::string near_remind_;                    ///< 近距离无人脸超时播报       //NOLINT
  // 等候区有人脸提示语
  bool mid_remind_falg_;                       ///< 提示语标记                 //NOLINT
  std::string mid_remind_;                     ///< 中距离有人脸提示语         //NOLINT
  // 底盘数据防抖动
  int enfc_wait_tag_;                         ///< 侧面离开标记（0:未计时 1:不需要 2:计时中） //NOLINT
  boost::posix_time::ptime enfc_wait_start_;  ///< 侧面离开计时    //NOLINT
  double enforce_wait_;                       ///< 侧面离开等待    //NOLINT

  std::string farewell_;             ///< 告别语        //NOLINT
  std::string genderM,genderF,genderFM;       ///默认为先生，女士  ,大家
  // (写共享内存) int server_state_;  ///< 机器人状态      //NOLINT

  DataInterfacePtr data_ptr_;       ///< 内存指针        //NOLINT
  std::string send_name_flag,send_name;   //上送的打招呼的事件
};

}///< namespace ysos    //NOLINT

#endif///< CUSTOMER_DET_CALLBACK_H_    //NOLINT