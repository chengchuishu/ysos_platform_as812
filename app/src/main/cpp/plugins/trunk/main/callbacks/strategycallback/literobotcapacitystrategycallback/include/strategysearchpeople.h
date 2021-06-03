/**   //NOLINT
  *@file strategysearchpeople.h
  *@brief Definition of
  *@version 0.1
  *@author wanfenfen
  *@date Created on: 2018:5:3   15:12
  *@copyright Copyright(c) 2016 YunShen Technology. All rights reserved.
  * 
  *@howto Usage:
  *@todo
  */

#pragma once


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


namespace ysos {

/**
*@brief definition of people status
*/
enum TDPeopleStatus {
  TDPeopleStatus_NotHave =0,  ///< 无人， 其中“可能有人”为一个中间态，不包含在可用的状态中
  TDPeopleStatus_Have ,  ///< 有人
  TDPeopleStatus_WaitingObject  ///< 等待服务对象
};

class StrategySearchPeopleDoSomethingInterface;
class StrategySearchPeople {
 public:
  StrategySearchPeople(StrategySearchPeopleDoSomethingInterface* search_people_do_something_interface);
  ~StrategySearchPeople();

  virtual int Initialized(const std::string &key, const std::string &value);

  /**
   *@brief 开始查找人，
   *@param
   *@return
   */
  virtual int StartSearchPeople();
  /**
  *@brief 停止查找人
  *@param
  *@return
  */
  virtual int StopSearchPeople();

  /**
   *@brief 检测有人无人
   *@param vec_people_info[Input] 外部传入的人脸数据
   *@return
   */
  virtual int SearchingPeople(std::vector<TDFaceDetectUnit>& vec_people_info, CallbackInterface *context);
  /**
  *@brief 添加外部的交互事件:触摸屏幕了
  *@param
  *@return
  */
  int AddInteractionTouchScreenEvent(CallbackInterface *context);



  TDPeopleStatus GetCurrentPeopleStatus();
  int SetCurrentPeopleStatus(TDPeopleStatus  new_people_status);

 protected:



  /**
   *@brief 选择特定范围内的所有人
   *@param vec_pick_person_index[Output] 选择特定范围内的所有人的索引号
   *@param min_distance_index[Output] 最小距离的人的索引号，如果不传值则返回值
   *@return
   */
  int PickPerson(std::vector<TDFaceDetectUnit>& vec_people_info, double max_distance, std::vector<int>* vec_pick_person_index, int* pick_person_index_min_distance);
  //int CheckMatchPeopleIndex(const TDFaceDetectUnit& current_server_unit, const std::vector<TDFaceDetectUnit>& now_camrea_unit,int& nMatchIndex);
  int CheckMatchPeopleIndex(const TDFaceDetectUnit& current_server_unit, const std::vector<TDFaceDetectUnit>& now_camrea_unit, std::vector<int>& vec_pick_person_index, int& nMatchIndex);


 private:
  StrategySearchPeopleDoSomethingInterface* search_people_do_something_interface_;

  /// 配置项
  double config_greet_dis_;                               ///< 开始服务距离, 单位：米，默认是1.2米
  double config_leave_distance_;                          ///< 人离开时的距离, 单位：米，默认是1.5米
  int config_one_tolerate_noface_millisconds_;            ///< 单人服务，可以允许多久没检测到人脸,默认是3000毫秒,单人服务：无人脸（连续3s） +  无交互（3s内屏幕无操作）
  int config_many_tolerate_noface_millisconds_;           ///< 多人服务，可以允许多久没检测到人脸,默认是2000毫秒,多人服务：无人脸（连续2s） +  无交互（2s内屏幕无操作）
  int config_noface_tolerate_noface_millisconds_;         ///< 无人脸服务，可以允许多久没检测到人脸,默认是5000毫秒,无人脸模式：无人脸 +  无交互（5s内屏幕无操作）
  int config_one_change_face_milliseconds_;               ///< 单人服务，切换人时允许先面服务的人没检测到人脸的时间，默认是1500毫秒
  int config_one_max_wait_detail_faceinfo_;               ///< 单人服务，无详细人脸最长等待时间，单位：毫秒，默认是3000
  int config_many_to_one_wait_milliseconds_;              ///< 多人服务，切换到单人服务的最长等待时间，单位：毫秒，默认是5000
  float config_many_to_one_face_distance_min_;              ///< 多人服务，立即切换到单人服务，人脸估算最小的距离得满足多少，单位是米，默认是0.5

  //////////////////////////////////////////////////////////////////////////
  log4cplus::Logger logger_;

  boost::atomic<int> atomic_current_people_status_;   ///< 当前人的状态，默认是无人
  boost::atomic<long> all_have_inteacte_begin_clock_;      ///< 有交互的开始时间(交互行为包含：点触屏幕），用于计算没有交互的时长, 总共的没有交互的时长

  /// 有人无人数据
  std::vector<TDFaceDetectUnit> vec_units_current_service_;   ///< 当前服务对象，可以有多人，如果当前为服务状态，但数组为空，则表示为无人脸服务模式
  int latest_serve_face_check_clock_;  ///< 最近一次检测到人脸的时间(如果是单人模式，则表示服务对象最近一次检测到人脸的时间）

  int wait_other_people_begin_clock_;   ///< 其他待服务人脸 开始等待时间（单位：毫秒），默认是0
  int wait_other_people_track_id_;      ///< 其他待服务人脸的trackid，默认是0

};



class StrategySearchPeopleDoSomethingInterface {
 public:
  struct StrategySearchPeopleDoSomethingContext {
    int cur_main_status;
    int cur_child_status;
    int order_cruise_move_clock_;   ///< 下定点巡航指令的时钟

    StrategySearchPeopleDoSomethingContext() {
      cur_main_status =0;
      cur_child_status =0;
      order_cruise_move_clock_ =0;
    }
  };
  typedef boost::shared_ptr<StrategySearchPeopleDoSomethingContext> StrategySearchPeopleDoSomethingContextPtr;

  // 注意，以下接口，如果要sleep，使用boost thread中的seleep函数，因为以下接口是在boost thread中被调用的
  virtual void DoSomeThingSwitchError(CallbackInterface *context) {};

  /**
   *@brief
   *@param pre_people_status[Input] 切换到无人前的人的状态
   *@param pre_peoople[Input] 切换到无人前的人的对象
   *@return
   */
  virtual void DoSomeThingSwitchNoPeople(const TDPeopleStatus& pre_people_status, std::vector<TDFaceDetectUnit>& pre_peoople, CallbackInterface *context) {};
  /**
   *@brief
   *@param people_is_missing[Input] 人是否丢了，表示刚开始检测到人，但再次检测时人却不再了，表示在无人状态下开始检测到人，再次检测时人不在了，还是无人状态
   *@param have_filter_face[Input] 是否是被过滤的人脸（比如，检测到人脸，但此人脸是被过滤的）
   *@param max_check_distance[Input] 最大的检测范围
   *@return
   */
  virtual void DoSomeThingCurrentIsNoPeople(CallbackInterface *context) {};


  /**
   *@brief
   *@param pre_people_status[Input] 切换到无人前的人的状态
   *@param peooples[Input] 人的信息,可能有多个人
   *@return
   */
  virtual void DoSomeThingSwitchHavePeople(const TDPeopleStatus& pre_people_status, std::vector<TDFaceDetectUnit>& peoples, CallbackInterface *context) {};
  /**
   *@brief
   *@param people_flag[Input] 0 表示有人（从无人到有人状态）, 1 表示人变了(从有人状态到有人状态), -1 表示出错
   *@return
   */
  virtual void DoSomeThingChangePeople(std::vector<TDFaceDetectUnit>& old_peooples, std::vector<TDFaceDetectUnit>& new_peoples, CallbackInterface *context) {};

  /**
   *@brief 有人状态需要做的事
   *@param not_interactive_clock[Input] 用户没有交互的时长，（说话没有说话，没有点障碍点）
   *@param lasttime_interactive_clock[Input] 最近一次交互的clock
   *@param current_face_in_camera[Input] 当前人脸是否在摄像头中
   *@return
   */
  virtual void DoSomeThingCurrentHavePeople(std::vector<TDFaceDetectUnit>& peoples, const int lasttime_interactive_clock, const bool current_face_in_camera, CallbackInterface *context) {};


  //virtual void DoSomeThingSwitchWaitObject(const TDPeopleStatus& pre_people_status) {CallbackInterface *context};
  //virtual void DoSomeThingCurrentIsWaitObject(const int& iface_number_pick_one, const std::vector<TDFaceDetectInfoPtr>& arr_face_result, CallbackInterface *context) {};

  /**
   *@brief 给用户打招呼
   *@pram people_statusm[Input] 当前用户状态
   *@param vec_need_greet[Inpput] 当前所有需要打招呼的用户，包含模糊信息和详细人脸信息
   *@param current_server_index[Input] 如果是有人状态，当前服务对象参数vec_need_greet中的位置
   *@return
   */
  //virtual void DoSomeThingGreetPeople(const TDPeopleStatus& people_statusm, std::vector<TDFaceDetectUnit>& vec_need_greet, CallbackInterface *context) {};
};

}
