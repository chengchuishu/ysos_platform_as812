/**
 *@file movepointstrategycallback.h
 *@brief the move to point strategy call back
 *@version 1.0
 *@author LiShengJun
 *@date Created on: 2016-10-20 15:16:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

#ifndef MOVE_POINT_CALLBACK_H_
#define MOVE_POINT_CALLBACK_H_

#include <boost/date_time/posix_time/posix_time.hpp>
#include <json/json.h>

/// Private Headers //  NOLINT
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basemoduleimpl.h"
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basemodulecallbackimpl.h"
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basestrategycallbackimpl.h"
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/packageconfigimpl.h"
#include "../../../../../../core/trunk/main/protect/include/os_hal_package/datainterfacemanager.h"

namespace ysos {

struct MoveCmdParam{
  std::string answer;     ///< 播报语
  std::string name;       ///< 目的地名称
  std::string aim;        ///< 底盘命令参数
};

struct MoveParam{
  std::string move_mode;
  std::string move_state;
};

/**
 *@brief  障碍物检测策略层 //NOLINT
*/
class YSOS_EXPORT MovePointStrategyCallback : public BaseStrategyCallbackImpl {
  DECLARE_CREATEINSTANCE(MovePointStrategyCallback);
  DISALLOW_COPY_AND_ASSIGN(MovePointStrategyCallback);
  DECLARE_PROTECT_CONSTRUCTOR(MovePointStrategyCallback);

 public:
  ~MovePointStrategyCallback(void);

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
  *@brief 解析移动命令  // NOLINT
  */
  int ParserMoveWalk(BufferInterfacePtr input_buffer, MoveCmdParam &cmd_param);
  
  /**
  *@brief 解析移动信息  // NOLINT
  */
  int ParserMoveInfo(BufferInterfacePtr input_buffer, MoveParam &mode_param);

  /**
  *@brief 组装应答语  // NOLINT
  */
  std::string MakeAnswerWords(const std::string& format, const std::string& name);

  /**
  *@brief 开始计时  // NOLINT
  */
  int StartTimer();

  /**
  *@brief 取得持续时间  // NOLINT
  */
  int GetElaspse();

private:
  /* 共通 */
  DataInterfacePtr   data_ptr_;       ///< 内存指针        //NOLINT
  boost::posix_time::ptime timer_start_;
  bool speaked_;

  /* 前后左右转 */
  int move_dir_wait_spk_;             ///< 定向移动        //NOLINT
  int move_dir_wait_go_;              ///< 定向移动        //NOLINT
  std::string md_wait_words_;

  std::string low_power_refuse_;

  /* 定点行走 */
  std::map<std::string,std::string> walk_names_;    // the names of the points
  std::string walk_name_;

  int move_point_wait_spk_;           ///< 定点行走        //NOLINT
  int move_point_wait_go_;            ///< 定点行走        //NOLINT
  std::string mp_wait_words_;

  std::string move_success_words_;
  std::string move_failed_words_;
  std::string move_skip_words_;
  std::string move_unknown_words_;
  std::string low_power_warn_;

  int warning_gap_;                   ///< obstacle warning
  boost::posix_time::ptime last_warning_spk_;
  std::string warning_words_;
};

}///< namespace ysos    //NOLINT

#endif///< MOVE_POINT_CALLBACK_H_    //NOLINT

