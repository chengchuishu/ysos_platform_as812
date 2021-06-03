/**   //NOLINT
  *@file strategysearchpeople.cpp
  *@brief Definition of
  *@version 0.1
  *@author wanfenfen
  *@date Created on: 2018:5:3   15:12
  *@copyright Copyright(c) 2016 YunShen Technology. All rights reserved.
  * 
  *@howto Usage:
  *@todo
  */
/// self headers
#include "../include/strategysearchpeople.h"

/// ysos headers
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"

/// private headers
#include "../include/literobotcapacitystrategycallback.h"


namespace ysos {
StrategySearchPeople::StrategySearchPeople(StrategySearchPeopleDoSomethingInterface* search_people_do_something_interface) :
  search_people_do_something_interface_(search_people_do_something_interface)
  ,config_greet_dis_(1.2)
  ,config_leave_distance_(1.5)
  ,atomic_current_people_status_(TDPeopleStatus_NotHave)
  ,config_one_tolerate_noface_millisconds_(3000)
  ,config_many_tolerate_noface_millisconds_(2000)
  ,config_noface_tolerate_noface_millisconds_(5000)
  ,config_one_change_face_milliseconds_(1500)
  ,config_one_max_wait_detail_faceinfo_(3000)
  ,config_many_to_one_wait_milliseconds_(5000)
  ,config_many_to_one_face_distance_min_(0.3)
  {
  assert(search_people_do_something_interface_);
  logger_ = GetUtility()->GetLogger("ysos.literobotcapacitystrategy");

  wait_other_people_begin_clock_ =0;
  wait_other_people_track_id_ =0;

  all_have_inteacte_begin_clock_ = 0;
}


StrategySearchPeople::~StrategySearchPeople() {

}

int StrategySearchPeople::Initialized(const std::string &key, const std::string &value) {

  //if ("static_immediate" == key) {
  //  static_immediate_ = (value == "1");
  //}
  if ("greet_distance" == key) {
    config_greet_dis_ = atof(value.c_str());
  } else if ("leave_distance" == key) {
    config_leave_distance_ = atof(value.c_str());
  }
  //else if ("recognise_max_wait" == key) {
  //  rcg_max_wait_ = atof(value.c_str());
  //}
  //else if ("face_chg_continue" == key) {
  //  face_chg_ctu_ = atof(value.c_str());
  //}
  else if ("one_tolerate_noface_millisconds" == key) {
    config_one_tolerate_noface_millisconds_ = atoi(value.c_str());
  } else if ("many_tolerate_noface_millisconds" == key) {
    config_many_tolerate_noface_millisconds_ = atoi(value.c_str());
  } else if ("noface_tolerate_noface_millisconds" == key) {
    config_noface_tolerate_noface_millisconds_ = atoi(value.c_str());
  } else if ("one_change_face_milliseconds" == key) {
    config_one_change_face_milliseconds_ = atoi(value.c_str());
  } else if ("one_max_wait_detail_faceinfo" == key) {
    config_one_max_wait_detail_faceinfo_ = atoi(value.c_str());
  }else if("many_to_one_wait_milliseconds" == key) {
    config_many_to_one_wait_milliseconds_ = atoi(value.c_str());
  }else if("many_to_one_face_distance_min" == key) {
    config_many_to_one_face_distance_min_ = atof(value.c_str());
  }
  else {
    return YSOS_ERROR_FAILED;
  }
  return YSOS_ERROR_SUCCESS;
}


int StrategySearchPeople::StartSearchPeople() {
  SetCurrentPeopleStatus(TDPeopleStatus_NotHave);
  return YSOS_ERROR_SUCCESS;
}
int StrategySearchPeople::StopSearchPeople() {
  /// 清空人脸数据
  vec_units_current_service_.clear();
  SetCurrentPeopleStatus(TDPeopleStatus_NotHave);
  return YSOS_ERROR_SUCCESS;
}


TDPeopleStatus StrategySearchPeople::GetCurrentPeopleStatus() {
  return static_cast<TDPeopleStatus>(atomic_current_people_status_.load());
}

int StrategySearchPeople::SetCurrentPeopleStatus(TDPeopleStatus  new_people_status) {
  atomic_current_people_status_ = (int)new_people_status;
  return YSOS_ERROR_SUCCESS;
}

int StrategySearchPeople::SearchingPeople(std::vector<TDFaceDetectUnit>& vec_people_info, CallbackInterface *context) {
  std::vector<int> vec_pick_person_index;     ///< 待选中的人的索引号列表
  int pick_person_index_min_distance =-1;     ///< 待选中的人中最近的一个人的索引号
  int imatch_index =-1;                       ///< 如果是单个对象，当前摄像头中服务对象是否还在

  TDPeopleStatus old_checkpeople_status = GetCurrentPeopleStatus();
  switch (old_checkpeople_status) {
  case TDPeopleStatus_NotHave: {
    /// 切换到有人状态
    /*
    1. 只有1人脸且距离 <= Nm（有详细人脸信息），其他人脸距离>Nm  ->单人服务
    2. 至少2人脸距离<=Nm  ->多人服务
    3. 无人脸 + 屏幕有操作  --> 无人脸模式服务
    */

    PickPerson(vec_people_info, config_greet_dis_, &vec_pick_person_index, &pick_person_index_min_distance);
    if (1 == vec_pick_person_index.size()) {   ///< 单人模式服务
      if ((1 ==vec_people_info[vec_pick_person_index[0]].have_perfect_face_ || -1 == vec_people_info[vec_pick_person_index[0]].have_perfect_face_)
         ) { ///< 有详细人脸结果有返回（有或没有匹配到的人脸）
        if (1 == vec_people_info[vec_pick_person_index[0]].have_perfect_face_) {
          assert(vec_people_info[vec_pick_person_index[0]].perfect_face_1n_ptr_ && vec_people_info[vec_pick_person_index[0]].perfect_face_1n_ptr_->vec_faces.size() >0);
          if (!(vec_people_info[vec_pick_person_index[0]].perfect_face_1n_ptr_ && vec_people_info[vec_pick_person_index[0]].perfect_face_1n_ptr_->vec_faces.size() >0)) {
            YSOS_LOG_ERROR_CUSTOM(logger_, "error, not have detail infi");
            goto FB_ERROR;
          }
        }

        YSOS_LOG_INFO_CUSTOM(logger_, "switch to have people :FB_SWIITCH_TO_HAVE_ONE， 有详细人脸结果");
        goto FB_SWIITCH_TO_HAVE_ONE;
      } else {
        // 详细人脸信息还没有到,最多等5秒
        int another_trackid = vec_people_info[vec_pick_person_index[0]].face_detect_info_ptr_->trackid;
        if (wait_other_people_track_id_ == another_trackid) {
          if ((clock() - wait_other_people_begin_clock_) > config_one_max_wait_detail_faceinfo_) {
            YSOS_LOG_INFO_CUSTOM(logger_, "switch to have people :FB_SWIITCH_TO_HAVE_ONE， 无详细人脸结果，但等待超时：" << config_one_max_wait_detail_faceinfo_);
            goto FB_SWIITCH_TO_HAVE_ONE;
          }
        } else {
          wait_other_people_track_id_  = another_trackid;
          wait_other_people_begin_clock_ = clock();
        }
      }
    } else if (vec_pick_person_index.size() >=2) { ///< 多人模式
      YSOS_LOG_INFO_CUSTOM(logger_, "switch to FB_SWIITCH_TO_HAVE_MANDY");
      goto FB_SWIITCH_TO_HAVE_MANDY;
    }

    goto FB_CURRENT_IS_NOTHAVE;
    break;
  }
  case TDPeopleStatus_Have: {
    /// 切换到无人
    /// 1.单人服务：无人脸（连续2s） +  无交互（2s内屏幕无操作）
    /// 2.多人服务：无人脸（连续1s） +  无交互（1s内屏幕无操作）
    /// 3.无人脸模式：无人脸 +  无交互（5s内屏幕无操作）
    PickPerson(vec_people_info, config_leave_distance_, &vec_pick_person_index, &pick_person_index_min_distance);

    if (vec_pick_person_index.size() <=0) { ///< 服务范围内没人
      int have_no_server_face_time = clock() - latest_serve_face_check_clock_;
      int have_no_interactive_time = clock() - all_have_inteacte_begin_clock_;
      if (1 == vec_units_current_service_.size() && have_no_server_face_time >= config_one_tolerate_noface_millisconds_ && have_no_interactive_time >=config_one_tolerate_noface_millisconds_) { ///< 单人服务
        YSOS_LOG_INFO_CUSTOM(logger_, "switch to no people:单人服务：无人脸（连续2s） +  无交互（2s内屏幕无操作)");
        goto FB_SWITCH_NOTHAVE;
      } else if (vec_units_current_service_.size() >=2 && have_no_server_face_time >= config_many_tolerate_noface_millisconds_ && have_no_interactive_time >= config_many_tolerate_noface_millisconds_) { ///< 多人服务
        YSOS_LOG_INFO_CUSTOM(logger_, "switch to no opeople:多人服务：无人脸（连续1s） +  无交互（1s内屏幕无操作）");
        goto FB_SWITCH_NOTHAVE;
      } else if (0 == vec_units_current_service_.size() && have_no_interactive_time >= config_noface_tolerate_noface_millisconds_) { ///< 无人脸模式
        YSOS_LOG_INFO_CUSTOM(logger_, "switch to no peoople:无人脸模式：无人脸 +  无交互（5s内屏幕无操作）");
        goto FB_SWITCH_NOTHAVE;
      } else {
        goto FB_CURRENT_IS_HAVE_PEOPLE;
      }
    }
    assert(vec_pick_person_index.size() >0);
    assert(pick_person_index_min_distance >=0);

    /// 切换人
    /// 1. 单人服务：至少有1人脸 +  另一人脸最近  <Nm（连续超过1s + 已获取详细人脸数据） -->算换人 ->单人服务
    /// 2. 多人服务：至少有1人脸  ->多人服务
    /// 3.多人服务：只有1人脸且距离<=Nm （连续超过1s）+ 其他人脸距离>Nm  ->单人服务(不实现）
    /// 4. 无人脸模式服务：无人脸 + 有交互（Ms内屏幕有操作）->原先的人还在 ->无人脸模式服务
    /// 5.无人脸模式服务: 只有1人脸且距离<=Nm + 其他人脸距离>Nm  ->单人服务
    /// 6.无人脸模式服务：至少2人脸距离<=Nm  ->多人服务
    if (1 == vec_units_current_service_.size()) { ///<  单人服务模式
      CheckMatchPeopleIndex(vec_units_current_service_[0], vec_people_info, vec_pick_person_index, imatch_index);
      if(vec_pick_person_index.size() >1) {
        /// 2.单人服务：至少2人脸距离<=Nm） -->多人服务（不播放欢迎语）
        goto FB_CHANGE_PEOPLE_ONE2MANDY;
      }
      if (-1 == imatch_index && (clock() - latest_serve_face_check_clock_) >config_one_change_face_milliseconds_) { ///< 服务对象不在当前摄像头视频中
        /* 针对虹软人脸作的优化：
           虹软人脸缺陷：部分人脸被挡住了，人脸比对分数会比较低，导致会被认为是同一个人。
         切换人优化：
         单人服务：如果当前服务人脸和待切换的人脸都没有详细人脸信息（未注册），同时性别一样，则不切换，*/
        if (1 != vec_units_current_service_[0].have_perfect_face_ && 1 !=vec_people_info[pick_person_index_min_distance].have_perfect_face_
          && vec_units_current_service_[0].face_detect_info_ptr_->gender == vec_people_info[pick_person_index_min_distance].face_detect_info_ptr_->gender) {
          goto FB_CURRENT_IS_HAVE_PEOPLE;
        }

        /// 待服务对象的详细人脸信息已返回
        if (-1 == vec_people_info[pick_person_index_min_distance].have_perfect_face_ || 1 == vec_people_info[pick_person_index_min_distance].have_perfect_face_) {
          YSOS_LOG_INFO_CUSTOM(logger_, "change people:FB_CHANGE_PEOPLE_ONE2ONE:单人服务：至少有1人脸 +  另一人脸最近  <Nm（连续超过1s + 已获取详细人脸数据） -->算换人 ->单人服务");
          goto FB_CHANGE_PEOPLE_ONE2ONE;
        } else {
          /// 如果详细人脸信息没返回，则最多等5秒
          int another_trackid = vec_people_info[vec_pick_person_index[0]].face_detect_info_ptr_->trackid;
          if (wait_other_people_track_id_ == another_trackid) {
            if ((clock() - wait_other_people_track_id_) > config_one_max_wait_detail_faceinfo_) {
              YSOS_LOG_INFO_CUSTOM(logger_, "change people:FB_CHANGE_PEOPLE_ONE2ONE:单人服务：至少有1人脸 +  另一人脸最近  <Nm（连续超过1s + 已获取详细人脸数据(人脸5秒超时了）） -->算换人 ->单人服务");
              goto FB_CHANGE_PEOPLE_ONE2ONE;
            }
          } else {
            wait_other_people_track_id_  = another_trackid;
            wait_other_people_begin_clock_ = clock();
          }
        }
      }
    } else if (0 == vec_units_current_service_.size()) {  ///< 无人脸模式
      if (1 == vec_pick_person_index.size()) {
        YSOS_LOG_INFO_CUSTOM(logger_, "change people:FB_CHANGE_PEOPLE_NONE2ONE, 无人脸模式服务: 只有1人脸且距离<=Nm + 其他人脸距离>Nm  ->单人服务");
        goto FB_CHANGE_PEOPLE_NONE2ONE;
      } else if (vec_pick_person_index.size() >=2) {
        YSOS_LOG_INFO_CUSTOM(logger_, "change people:FB_CHANGE_PEOPLE_NONE2MANDY, 无人脸模式服务：至少2人脸距离<=Nm  ->多人服务");
        goto FB_CHANGE_PEOPLE_NONE2MANDY;
      }
    } else {  ///< 多人服务模式
      assert(vec_units_current_service_.size() >=2);
     // 3.多人服务：只有1人脸且距离<=Nm （连续超过5s）+ 其他人脸距离>Nm  ->单人服务
     // 4.多人服务：只有1人脸且距离<=0.5m  ->单人服务
      if(1 == vec_pick_person_index.size()) {
        TDFaceDetectUnit& face_tmp_ref = vec_people_info[vec_pick_person_index[0]];
        if(face_tmp_ref.face_detect_info_ptr_->estimate_distance <= config_many_to_one_face_distance_min_) {
          goto FB_CHANGE_PEOPLE_MANDY2ONE;
        }else {

          int another_trackid =face_tmp_ref.face_detect_info_ptr_->trackid;
          if (wait_other_people_track_id_ == another_trackid) {
            if ((clock() - wait_other_people_begin_clock_) > config_many_to_one_wait_milliseconds_) {  ///< 多人服务，切换到单人服务的最长等待时间
              YSOS_LOG_INFO_CUSTOM(logger_, "switch to have people :FB_SWIITCH_TO_HAVE_ONE， 无详细人脸结果，但等待超时：" << config_one_max_wait_detail_faceinfo_);
              goto FB_CHANGE_PEOPLE_MANDY2ONE;
            }
          } else {
            wait_other_people_track_id_  = another_trackid;
            wait_other_people_begin_clock_ = clock();
          }
        }
      }

    }

    goto FB_CURRENT_IS_HAVE_PEOPLE;
    break;
  }
  default: {
    assert(false &&"not support people status");
    goto FB_ERROR;
  }
  }

  return YSOS_ERROR_SUCCESS;
FB_ERROR: {
    search_people_do_something_interface_->DoSomeThingSwitchError(context);
  }
  return YSOS_ERROR_FAILED;
FB_SWITCH_NOTHAVE: {                     ///< 切换到无人状态
    SetCurrentPeopleStatus(TDPeopleStatus_NotHave);
    std::vector<TDFaceDetectUnit> tmp_pre_server_face = vec_units_current_service_;
    vec_units_current_service_.clear();                                              ///< 清空当前服务对象

    search_people_do_something_interface_->DoSomeThingSwitchNoPeople(TDPeopleStatus_Have, tmp_pre_server_face, context);
  }
  return YSOS_ERROR_SUCCESS;
FB_CURRENT_IS_NOTHAVE: {                ///< 当前还是无人状态
    search_people_do_something_interface_->DoSomeThingCurrentIsNoPeople(context);
  }
  return YSOS_ERROR_SUCCESS;

//FB_SWIITCH_TO_HAVE_NONEFACE:            ///< 切换到无人脸模式，这里不实现，由函数 AddInteractionTouchScreenEvent中实现
//    {
//      vec_units_current_service_ = vec_people_info;
//      search_people_do_something_interface_->DoSomeThingSwitchHavePeople(TDPeopleStatus_NotHave, vec_units_current_service_);
//    }
//    return YSOS_ERROR_SUCCESS;

FB_SWIITCH_TO_HAVE_ONE:                 ///< 切换到单人服务
FB_SWIITCH_TO_HAVE_MANDY: {             ///< 切换到多人服务
    SetCurrentPeopleStatus(TDPeopleStatus_Have);
    latest_serve_face_check_clock_ = clock();
    vec_units_current_service_ = vec_people_info;
    wait_other_people_track_id_ = -1;
    wait_other_people_begin_clock_ = clock();
    search_people_do_something_interface_->DoSomeThingSwitchHavePeople(TDPeopleStatus_NotHave, vec_units_current_service_, context);
  }
  return YSOS_ERROR_SUCCESS;
FB_CHANGE_PEOPLE_ONE2ONE:              ///< 切换人：单人到单人
FB_CHANGE_PEOPLE_ONE2MANDY:            ///< 切换人：单人到多人
FB_CHANGE_PEOPLE_NONE2ONE:             ///< 切换人：无人脸到单人
FB_CHANGE_PEOPLE_NONE2MANDY:           ///< 切换人：无人脸到多人
FB_CHANGE_PEOPLE_MANDY2ONE:            ///< 切换人：多人到单人
  {         
    latest_serve_face_check_clock_ = clock();
    std::vector<TDFaceDetectUnit> old_peoples = vec_units_current_service_;
    vec_units_current_service_.clear();
    //assert(1 == vec_pick_person_index.size());
    for (unsigned int i =0; i < vec_pick_person_index.size(); ++i) {
      vec_units_current_service_.push_back(vec_people_info[vec_pick_person_index[i]]);
    }
    wait_other_people_track_id_ =-1;
    wait_other_people_begin_clock_ = clock();

    search_people_do_something_interface_->DoSomeThingChangePeople(old_peoples, vec_units_current_service_, context);
  }
  return YSOS_ERROR_SUCCESS;

FB_CURRENT_IS_HAVE_PEOPLE: {           ///< 当前是有人状态
    if ((1 == vec_units_current_service_.size() && imatch_index >=0)                  ///< 单人模式，原先的人还在
        ||(vec_units_current_service_.size() >=2 && vec_pick_person_index.size() >0)) { ///< 多人模式，至少还有一个人
      latest_serve_face_check_clock_ = clock();
    }

    search_people_do_something_interface_->DoSomeThingCurrentHavePeople(vec_units_current_service_,all_have_inteacte_begin_clock_, -1 != imatch_index ? true:false,  context);
  }
  return YSOS_ERROR_SUCCESS;
}


/**
 *@brief 添加外部的交互事件:触摸屏幕了
 *@param
 *@return
 */
int StrategySearchPeople::AddInteractionTouchScreenEvent(CallbackInterface *context) {
  TDPeopleStatus old_checkpeople_status = GetCurrentPeopleStatus();
  all_have_inteacte_begin_clock_ = clock();      ///< 记录一下当前交互的时间

  /// 本身是有人状态，交互事件不再处理
  if (TDPeopleStatus_Have == old_checkpeople_status) {
    return YSOS_ERROR_SUCCESS;
  }

  if (TDPeopleStatus_NotHave == old_checkpeople_status) { ///< 无人状态切到有人状态
    SetCurrentPeopleStatus(TDPeopleStatus_Have);
    vec_units_current_service_.clear();
    search_people_do_something_interface_->DoSomeThingSwitchHavePeople(TDPeopleStatus_NotHave, vec_units_current_service_, context);
    return YSOS_ERROR_SUCCESS;
  }
  /// TODO
  return YSOS_ERROR_SUCCESS;
}

int StrategySearchPeople::PickPerson(std::vector<TDFaceDetectUnit>& vec_people_info, double max_distance, std::vector<int>* vec_pick_person_index, int* pick_person_index_min_distance) {
  if (vec_pick_person_index) vec_pick_person_index->clear();
  if (pick_person_index_min_distance) * pick_person_index_min_distance = -1;

  for (unsigned int i =0; i< vec_people_info.size(); ++i) {
    if (vec_people_info[i].face_detect_info_ptr_->estimate_distance <= max_distance) {
      if (vec_pick_person_index) vec_pick_person_index->push_back(i);
      if (pick_person_index_min_distance) {
        if (-1 == *pick_person_index_min_distance) {
          *pick_person_index_min_distance = i;
        } else if (vec_people_info[*pick_person_index_min_distance].face_detect_info_ptr_->estimate_distance > vec_people_info[i].face_detect_info_ptr_->estimate_distance) {
          *pick_person_index_min_distance = i;
        }
      }
    }
  }
  return YSOS_ERROR_SUCCESS;
}

//int StrategySearchPeople::CheckMatchPeopleIndex(const TDFaceDetectUnit& current_server_unit, const std::vector<TDFaceDetectUnit>& now_camrea_unit,int& nMatchIndex) {
//   nMatchIndex =-1;
///* 简单处理，不使用人脸1比1,
//  *       如果trackid一样
//  */
//// find match arrord trackid
//for (int i=0; i< now_camrea_unit.size(); ++i)
//{
//  const TDFaceDetectUnit& tmp_unit_ref = now_camrea_unit[i];
//  if(current_server_unit.face_detect_info_ptr_->trackid == tmp_unit_ref.face_detect_info_ptr_->trackid) {
//    nMatchIndex =i;
//    return YSOS_ERROR_SUCCESS;
//  }
//}
//// not find mathc arrocd trackid     *       如果有详细人脸信息，人脸id一样，则是同一个人
//// serrch accord detail info
//if(1 == current_server_unit.have_perfect_face_ && current_server_unit.perfect_face_1n_ptr_
//  && current_server_unit.perfect_face_1n_ptr_->vec_faces.size() >0
//  ) {
//  for (int i=0; i < now_camrea_unit.size(); ++i)
//  {
//    const TDFaceDetectUnit& tmp_unit_ref = now_camrea_unit[i];
//    if(tmp_unit_ref.perfect_face_1n_ptr_ && tmp_unit_ref.perfect_face_1n_ptr_->vec_faces.size() >0
//      && tmp_unit_ref.perfect_face_1n_ptr_->vec_faces[0]->person_id == current_server_unit.perfect_face_1n_ptr_->vec_faces[0]->person_id) {
//        nMatchIndex =i;
//        return YSOS_ERROR_SUCCESS;
//    }
//  }
//}
//return YSOS_ERROR_SUCCESS;
//}

int StrategySearchPeople::CheckMatchPeopleIndex(const TDFaceDetectUnit& current_server_unit, const std::vector<TDFaceDetectUnit>& now_camrea_unit, std::vector<int>& vec_pick_person_index,int& nMatchIndex) {
  nMatchIndex =-1;
  /* 简单处理，不使用人脸1比1,
    *       如果trackid一样
    */
  // find match arrord trackid
  for (unsigned int i=0; i< vec_pick_person_index.size(); ++i) {
    const TDFaceDetectUnit& tmp_unit_ref = now_camrea_unit[vec_pick_person_index[i]];
    if (current_server_unit.face_detect_info_ptr_->trackid == tmp_unit_ref.face_detect_info_ptr_->trackid) {
      nMatchIndex =i;
      return YSOS_ERROR_SUCCESS;
    }
  }
  // not find mathc arrocd trackid     *       如果有详细人脸信息，人脸id一样，则是同一个人
  // serrch accord detail info
  if (1 == current_server_unit.have_perfect_face_ && current_server_unit.perfect_face_1n_ptr_
      && current_server_unit.perfect_face_1n_ptr_->vec_faces.size() >0
     ) {
    for (unsigned int i=0; i < vec_pick_person_index.size(); ++i) {
      const TDFaceDetectUnit& tmp_unit_ref = now_camrea_unit[vec_pick_person_index[i]];
      if (tmp_unit_ref.perfect_face_1n_ptr_ && tmp_unit_ref.perfect_face_1n_ptr_->vec_faces.size() >0
          && tmp_unit_ref.perfect_face_1n_ptr_->vec_faces[0]->person_id == current_server_unit.perfect_face_1n_ptr_->vec_faces[0]->person_id) {
        nMatchIndex =i;
        return YSOS_ERROR_SUCCESS;
      }
    }
  }
  return YSOS_ERROR_SUCCESS;
}


}