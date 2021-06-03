/**   //NOLINT
  *@file literobotcapacitystrategycallback.h
  *@brief Definition of
  *@version 0.1
  *@author Livisen.Wan
  *@date Created on: 2018:4:17   14:01
  *@copyright Copyright(c) 2016 YunShen Technology. All rights reserved.
  * 
  *@howto Usage:
  *@todo
  */

#pragma  once
#ifdef _WIN32
/// windows headers
#include <Windows.h>
#else
#endif
/// boost headers
#include <boost/thread.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/atomic.hpp>

/// third party headers
#include <json/json.h>

/// ysos headers
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basemoduleimpl.h"  //NOLINT
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basemodulecallbackimpl.h"  //NOLINT
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basestrategycallbackimpl.h"  //NOLINT
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/packageconfigimpl.h"
#include "../../../../../../core/trunk/main/protect/include/os_hal_package/datainterfacemanager.h"
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"

/// private headers
#include "../../../public/include/common/facedetectstruct.h"
#include "../include/strategysearchpeople.h"


namespace ysos {

/**
 *@brief
*/
class YSOS_EXPORT LiteRobotCapacityStrategyCallback : public BaseStrategyCallbackImpl, public StrategySearchPeopleDoSomethingInterface {
  DECLARE_CREATEINSTANCE(LiteRobotCapacityStrategyCallback);
  DISALLOW_COPY_AND_ASSIGN(LiteRobotCapacityStrategyCallback);
  DECLARE_PROTECT_CONSTRUCTOR(LiteRobotCapacityStrategyCallback);

 public:

  ~LiteRobotCapacityStrategyCallback(void);

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

  ///inherit from StrategySearchPeopleDoSomethingInterface
  virtual void DoSomeThingSwitchNoPeople(const TDPeopleStatus& pre_people_status, std::vector<TDFaceDetectUnit>& pre_peoople, CallbackInterface *context);     ///< 切到无人
  virtual void DoSomeThingSwitchHavePeople(const TDPeopleStatus& pre_people_status, std::vector<TDFaceDetectUnit>& peoples, CallbackInterface *context);       ///< 切到有人
  virtual void DoSomeThingCurrentHavePeople(std::vector<TDFaceDetectUnit>& peooples, const int lasttime_interactive_clock, const bool current_face_in_camera, CallbackInterface *context);
  virtual void DoSomeThingChangePeople(std::vector<TDFaceDetectUnit>& old_peooples, std::vector<TDFaceDetectUnit>& new_peoples, CallbackInterface *context);   ///< 人换了：从无人脸切到有人脸，从单个人切到另一个人

  void ThreadFunRunSelf();

  /// 2002版本中对应的函数

  /**
   *@brief 处理状态：PrepareService， 根据上下班时间切换到不同状态(这是2002版中对应的策略）
   *@param 
   *@return   
   */
  int ProcessStatePrepareService(CallbackInterface *context);
  int ProcessStatePrepareServiceV2002(CallbackInterface *context);
  int ProcessStatePrepareServiceV2003(CallbackInterface *context);

  /**
   *@brief 处理状态：BeforeWork， 根据上下班时间切换到不同状态
   *@param 
   *@return   
   */
  int ProcessStateBeforeWork(CallbackInterface *context);
  /**
   *@brief 处理状态：StaticSearching， 根据上下班时间切换到不同状态
   *@param 
   *@return 
   */
  int ProcessStateStaticSearching(CallbackInterface *context);
  int ProcessStateStaticSearchingV2002(CallbackInterface *context);
  int ProcessStateStaticSearchingV2003(CallbackInterface *context);

  /**
   *@brief 根据人脸性别和年龄来得到称呼
   *@param
   2.单人：年龄<=15                      ->小朋友
   年龄>15 && <=40 + 男 ->帅哥
   年龄>15 && <=40 + 女  ->美女
   年龄>40 + 男                  ->先生
   年龄>40 +女                  ->女士
   *@return
   */
  std::string GetTitleFromAgeGender(const int& age, const std::string& gender);
  std::string GetAllTitleFromPeople(const std::vector<TDFaceDetectUnit>& peooples);

  /// 人工坐席
  void NotifyCustomerLeave(const TDPeopleStatus& pre_people_status, std::vector<TDFaceDetectUnit>& pre_peoople, CallbackInterface *context);
  void NotifyCustomerServe(const TDPeopleStatus& pre_people_status, std::vector<TDFaceDetectUnit>& peoples, CallbackInterface *context);
  void NotifyArtificialSeatChangePeople(std::vector<TDFaceDetectUnit>& old_peoples, std::vector<TDFaceDetectUnit>& new_peoples, CallbackInterface *context);

  /// 历史服务对象管理，以便对服务过的对象跟踪，播放不同欢迎语
  void HistoryServePeopleAdd(const std::vector<TDFaceDetectUnit>& peoples);               ///< 添加到历史服务对象中，，当前只保存最新一次服务对象人脸数据
  void HistoryServePeopleClear();
  bool HistoryServePeopleHaveRecord(const TDFaceDetectUnit& people);                      ///< 判断人是否服务过，如果有详细人脸则根据详细人脸来判断，如果没有则根据trackid


#ifdef _DEBUG  ///< 单机调试模式下才会执行的代码
protected:
  bool HaveTouchScreen();         ///< 是否点击过屏幕，调试模式下（自己电脑上，不在机器人上），没有点屏幕事件，所以这里写段代码在程序中检测是否点了屏幕
  bool IsTeamviewDesktopConn();   ///< teamview 是否开启,用鼠标的位置来判断是否点过屏幕，判断不了是用户点的，还是teamvew之类远程用户点的。

  POINT last_time_user_clock_screen_point_;         ///< 用户最近一次点击屏幕的坐标
  //int config_min_not_move_cursor_distance;                             ///<;鼠标范围没有超过多少，表示屏幕被点了,即 abs(x1-x2）+ abs(y1-y2)的值，默认是50
#endif

  
 private:
  /// 配置项
  std::string hint_start_to_work_;        ///< 上班提示语音
  std::string hint_before_work_;          ///<  非上班时间 上前前提示语音，
  std::string hint_off_to_work_;          ///< 非上班时间 下班后提示语 ，
  std::string hint_helloword_;            ///< 有人招呼语  
  std::string hint_farewell_;             ///< 告别语        //NOLINT
  std::string hint_mid_remind_;                     ///< 正前方中距离有人脸提示         //NOLINT


  boost::recursive_mutex rmutex_member_;
  boost::shared_ptr<boost::thread> thread_run_self_ptr_;
  boost::atomic<int> atomic_thread_run_self_stop_flag_;



  DataInterfacePtr data_ptr_;       //< 内存指针
  StrategySearchPeople strategy_search_people_;
  //std::list<TDFaceDetectUnit> history_server_people_;           ///< 历史服务对象，当前只保存最新一次服务对象人脸数据
  std::vector<TDFaceDetectUnit> history_server_people_;

};

}///< namespace ysos    //NOLINT
