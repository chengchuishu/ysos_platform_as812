/**   //NOLINT
  *@file literobotcapacitystrategycallback.cpp
  *@brief Definition of
  *@version 0.1
  *@author Livisen.Wan
  *@date Created on: 2018:4:17   14:02
  *@copyright Copyright(c) 2016 YunShen Technology. All rights reserved.
  * 
  *@howto Usage:
  *@todo
  */

/// self headers
#include "../include/literobotcapacitystrategycallback.h"

/// boost headers
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/thread/lock_guard.hpp>
#include <boost/chrono.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/locale.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

/// ysos headers
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"
#include "../../../../../../core/trunk/main/public/include/os_hal_package/bufferutility.h"

/// private headers
#include "../../../public/include/common/commonenumdefine.h"

namespace ysos {

#define STR_STATUS_PREPARESERVICE                      "PrepareService"
#define STR_STATUS_ONSERVICE                           "OnService"
//#define STR_STATUS_POWERMANAGER_BEFOREWORK            "SuspendService@PowerManager@BeforeWork"
#define STR_STATUS_POWERMANAGER_BEFOREWORK            "WaitingService"
//#define STR_STATUS_TARGETSEARCHING_STATICSEARCHING    "WaitingService@TargetSearching@StaticSearching"
#define STR_STATUS_TARGETSEARCHING_STATICSEARCHING    "WaitingService"


DECLARE_PLUGIN_REGISTER_INTERFACE(LiteRobotCapacityStrategyCallback, CallbackInterface);  ///<  插件的入口，一定要加上 //  NOLINT
LiteRobotCapacityStrategyCallback::LiteRobotCapacityStrategyCallback(const std::string &strClassName):BaseStrategyCallbackImpl(strClassName)
  ,strategy_search_people_(this)
  ,data_ptr_(GetDataInterfaceManager()->GetData(GLOBAL_DATA_KEY))
  ,hint_farewell_("再见，希望下次能再为您服务")
  ,hint_mid_remind_("请离宝宝近一点，可以更好为您服务")
  ,hint_helloword_("")
  ,hint_start_to_work_("欢迎来到机器人的世界") {
  logger_ = GetUtility()->GetLogger("ysos.literobotcapacitystrategy");

  thread_run_self_ptr_ = NULL;
  atomic_thread_run_self_stop_flag_ =0;
}

LiteRobotCapacityStrategyCallback::~LiteRobotCapacityStrategyCallback(void) {
  boost::lock_guard<boost::recursive_mutex> guard(rmutex_member_);
  if (thread_run_self_ptr_) {
    atomic_thread_run_self_stop_flag_ =1;
    thread_run_self_ptr_->interrupt();
    if (thread_run_self_ptr_->joinable()) {
      YSOS_LOG_INFO("wait for thread exit begin");
      thread_run_self_ptr_->join();
      YSOS_LOG_INFO("wait for thread exit end");
    }
    thread_run_self_ptr_ = NULL;
  }
}

int LiteRobotCapacityStrategyCallback::Initialize(void *param) {
  YSOS_LOG_INFO("in Initialize");

  boost::lock_guard<boost::recursive_mutex> guard(rmutex_member_);
  if (thread_run_self_ptr_) {
    bool have_finish = thread_run_self_ptr_->try_join_for(boost::chrono::milliseconds(0));
    if (have_finish) {
      YSOS_LOG_INFO("thread_run_self_ptr_ is run finish");
      thread_run_self_ptr_ = NULL;
    }
  }
  if (NULL == thread_run_self_ptr_) {
    YSOS_LOG_INFO("create thread ThreadFunRunSelf");
    thread_run_self_ptr_.reset(new boost::thread(&LiteRobotCapacityStrategyCallback::ThreadFunRunSelf, this));
  }

  {
    /// 为了兼容YSOS公版，这里会向内存中写入几个值

    /// 为了签到，写入以下值
    data_ptr_->SetData("electricity", "100");
    data_ptr_->SetData("is_charging", "1");
  }

  return BaseStrategyCallbackImpl::Initialize(param);
}

void LiteRobotCapacityStrategyCallback::ThreadFunRunSelf() {
  //YSOS_LOG_INFO("ThreadFunRunSelf begin");

  /// 这时暂时不处理任何事
  return;
}
int LiteRobotCapacityStrategyCallback::HandleMessage(const std::string &event_name, BufferInterfacePtr input_buffer, CallbackInterface *context) {
  YSOS_LOG_DEBUG("HandleMessage: " << event_name);
  if (data_ptr_ == NULL) {
    YSOS_LOG_ERROR("data_ptr_ is NULL!");
    return YSOS_ERROR_FAILED;
  }

  if ("empty_event" == event_name) {
    std::string cur_machine_state;
    data_ptr_->GetData("machine_state", cur_machine_state);
    //MessageBox(NULL, "test", "test", MB_OK);

    // 去掉前端main@
    if (cur_machine_state.length() >0) {
      cur_machine_state = boost::algorithm::erase_first_copy(cur_machine_state, "main@");
    }


    if (STR_STATUS_PREPARESERVICE == cur_machine_state) {
      ProcessStatePrepareService(context);
    } else if (STR_STATUS_POWERMANAGER_BEFOREWORK == cur_machine_state) {
      ProcessStateBeforeWork(context);
    }
  } else if ("face_info" == event_name) { ///< 人脸信息
    const char* buffer = reinterpret_cast<const char*>(GetBufferUtility()->GetBufferData(input_buffer));
    if (buffer == NULL) {
      YSOS_LOG_ERROR("buffer is NULL!");
      return YSOS_ERROR_FAILED;
    }
    //TODO: for debug test
    if (false)
    {
      //TODO:example--封装数据进行相关状态变更
      std::string temp = "{\"type\":\"exception_type\",\"data\":{\"error-level\":0}}";
      int ret = RequestService("default@TemplateModule", boost::lexical_cast<std::string>(CMD_TRANSFER_EVENT), "text", temp, context);
      YSOS_LOG_DEBUG("DoEventNotifyService ---->ret=" << ret);
      temp = "{\"type\":\"exception_type_test\",\"data\":{\"error-level\":1}}";
      ret = RequestService("default@TemplateModule", boost::lexical_cast<std::string>(CMD_TRANSFER_EVENT), "text", temp, context);
      YSOS_LOG_DEBUG("DoEventNotifyService ---->ret=" << ret);
    }
    
    try {
      std::istringstream in_stream(boost::locale::conv::to_utf<char>(buffer, "gbk"));
      boost::property_tree::ptree tree_root, empty_tree;
      boost::property_tree::read_json(in_stream, tree_root);

      boost::property_tree::ptree data_js_value = tree_root.get_child("data", empty_tree);
      std::vector<TDFaceDetectUnit> vec_detect_unit;
      int decoderet = DecodeJson2(data_js_value, vec_detect_unit);

      strategy_search_people_.SearchingPeople(vec_detect_unit, context);

#ifdef _DEBUG   ///< 调试模式下（自己电脑上，不在机器人上），没有点屏幕事件，所以这里写段代码在程序中检测是否点了屏幕
      //if (HaveTouchScreen()) {
      //  return strategy_search_people_.AddInteractionTouchScreenEvent(context);
      //}
#endif
      {
        std::string cur_machine_state;
        data_ptr_->GetData("machine_state", cur_machine_state);
        //MessageBox(NULL, "test", "test", MB_OK);

        // 去掉前端main@
        if (cur_machine_state.length() >0) {
          cur_machine_state = boost::algorithm::erase_first_copy(cur_machine_state, "main@");
        }
        if (STR_STATUS_TARGETSEARCHING_STATICSEARCHING == cur_machine_state) {
          /// 如果到了下班时间，则切换状态
          ProcessStateStaticSearching(context);
        }
      }


    } catch (...) {
      assert(false && "decode json error");
      return YSOS_ERROR_FAILED;
    }
  } else if (event_name == "customer_operation_service") { ///< 操作屏幕事件,YSOS公版中是由前端应用产生的（由前端应用发给YSOS），这里兼容也使用这个事件
    return strategy_search_people_.AddInteractionTouchScreenEvent(context);
  } else {
    YSOS_LOG_ERROR("error, 处理不了的事件:" << event_name);
    assert(false && "处理不了的事件");
  }
  return YSOS_ERROR_SUCCESS;
}

int LiteRobotCapacityStrategyCallback::Initialized(const std::string &key, const std::string &value) {
  YSOS_LOG_INFO("Initialized in LiteRobotCapacityStrategyCallback" << key << ": " << value);
  int n_return = YSOS_ERROR_SUCCESS;

  int base_init = strategy_search_people_.Initialized(key, value);
  if (YSOS_ERROR_SUCCESS == base_init) {
    return YSOS_ERROR_SUCCESS;
  }
  if ("farewell" == key) {
    hint_farewell_ = value;
  } else if ("mid_remind" == key) {
    hint_mid_remind_ = value;
  } else if ("start_to_work" == key) {
    hint_start_to_work_ = value;
  } else if ("helloword" == key) {
    hint_helloword_ = value;
  } else if ("off_to_work" == key) {
    hint_off_to_work_ = value;
  } else if ("before_work" == key) {
    hint_before_work_ = value;
  }

  return n_return;
}

int LiteRobotCapacityStrategyCallback::RequestService(std::string service_name, std::string id, std::string type, std::string json_value, CallbackInterface *context) {
  YSOS_LOG_DEBUG("RequestService "<<service_name <<" " << id<< " " << type<<" " << json_value);

  int n_return = YSOS_ERROR_SUCCESS;
  ReqIOCtlServiceParam req_service;
  req_service.id = id;
  req_service.service_name = service_name;
  if (!type.empty()) {
    req_service.type = type;
  }
  if (!json_value.empty()) {
    req_service.value = json_value;
  }
  n_return = DoIoctlService(req_service, context);
  if (YSOS_ERROR_SUCCESS != n_return) {
    YSOS_LOG_DEBUG("execute DoIoctlService failed, n_return = " << n_return);
  }

  YSOS_LOG_DEBUG("RequestService done and success");
  return n_return;
}

int LiteRobotCapacityStrategyCallback::ProcessStatePrepareServiceV2003(CallbackInterface *context) {
  /// 判断一下签到有没有成功，如果没有成功，则不接着操作
  std::string signin_termid;
  std::string signin_orgid;
  data_ptr_->GetData("signin_termid", signin_termid);
  data_ptr_->GetData("signin_orgid", signin_orgid);
  if(signin_termid.length() <=0 || signin_orgid.length() <=0) {
    YSOS_LOG_ERROR("签到没有成功");
    return YSOS_ERROR_SUCCESS;
  }

  /// 上班时间
  YSOS_LOG_INFO("working time switch to status:" << STR_STATUS_TARGETSEARCHING_STATICSEARCHING);
  ///播放上班语
  YSOS_LOG_INFO("curise_ans "<< hint_start_to_work_);
  data_ptr_->SetData("curise_ans", hint_start_to_work_);
  int rst = DoSwitchNotifyService(STR_STATUS_TARGETSEARCHING_STATICSEARCHING, context);
  strategy_search_people_.StartSearchPeople();
  assert(YSOS_ERROR_SUCCESS == rst);
  return YSOS_ERROR_SUCCESS;
}


int LiteRobotCapacityStrategyCallback::ProcessStatePrepareService(CallbackInterface *context) {
#ifdef _YSOS_LITEROBOT_V2002
  return ProcessStatePrepareServiceV2002(context);
#else
  return ProcessStatePrepareServiceV2003(context);
#endif
}
int LiteRobotCapacityStrategyCallback::ProcessStatePrepareServiceV2002(CallbackInterface *context) {

  /// 判断一下签到有没有成功，如果没有成功，则不接着操作
  std::string signin_termid;
  std::string signin_orgid;
  data_ptr_->GetData("signin_termid", signin_termid);
  data_ptr_->GetData("signin_orgid", signin_orgid);
  if(signin_termid.length() <=0 || signin_orgid.length() <=0) {
    YSOS_LOG_ERROR("签到没有成功");
    return YSOS_ERROR_SUCCESS;
  }

  //根据上班时间 切换到不同状态
  // work time
  std::string strOnWork;
  data_ptr_->GetData("on_work_time",strOnWork);
  std::string strOffWork;
  data_ptr_->GetData("off_work_time",strOffWork);
  //assert(strOnWork.length() >0 && strOffWork.length() >0);

  if (strOnWork.length() <=0 || strOffWork.length() <=0) {
    //YSOS_LOG_ERROR("work time is not set, jump sate SuspendService ,jump to state:" << STR_STATUS_TARGETSEARCHING_STATICSEARCHING);
    //int rst = DoSwitchNotifyService(STR_STATUS_TARGETSEARCHING_STATICSEARCHING, context);
    //上班时间没有值，可能是因为doconfig模块还没有把参数写到内存中，所以这里等一儿
    //YSOS_LOG_INFO("on work time is not set or off work time is not set");
    return YSOS_ERROR_SUCCESS;
  }

  boost::posix_time::time_duration on_work(boost::posix_time::duration_from_string(strOnWork));
  boost::posix_time::time_duration off_work(boost::posix_time::duration_from_string(strOffWork));

  // local time
  boost::posix_time::time_duration local_time(boost::posix_time::second_clock::local_time().time_of_day());
  if (local_time >= on_work && local_time <= off_work) {
    /// 上班时间
    YSOS_LOG_INFO("working time switch to status:" << STR_STATUS_TARGETSEARCHING_STATICSEARCHING);
    /// 延时5秒后切
    //YSOS_LOG_INFO("延时5秒后切");
    //boost::this_thread::sleep_for(boost::chrono::milliseconds(5000));
    ///播放上班语
    YSOS_LOG_INFO("curise_ans "<< hint_start_to_work_);
    data_ptr_->SetData("curise_ans", hint_start_to_work_);
    int rst = DoSwitchNotifyService(STR_STATUS_TARGETSEARCHING_STATICSEARCHING, context);
    strategy_search_people_.StartSearchPeople();
    assert(YSOS_ERROR_SUCCESS == rst);
    return YSOS_ERROR_SUCCESS;
  } else {
    /// 非上班时间
    YSOS_LOG_INFO("not working time switch to status:" << STR_STATUS_POWERMANAGER_BEFOREWORK);
    /// 延时5秒后切
    //YSOS_LOG_INFO("延时5秒后切");
    //boost::this_thread::sleep_for(boost::chrono::milliseconds(5000));
    ///播放beforework语
    YSOS_LOG_INFO("hint_beforework "<< hint_before_work_);
    data_ptr_->SetData("hint_beforework", hint_before_work_);
    int rst = DoSwitchNotifyService(STR_STATUS_POWERMANAGER_BEFOREWORK, context);
    assert(YSOS_ERROR_SUCCESS == rst);
    return YSOS_ERROR_SUCCESS;
  }
  return YSOS_ERROR_FAILED;
}


int LiteRobotCapacityStrategyCallback::ProcessStateBeforeWork(CallbackInterface *context) {
  /// 如果到了上班时间，则切换到上班状态
  std::string strOnWork;
  data_ptr_->GetData("on_work_time",strOnWork);
  std::string strOffWork;
  data_ptr_->GetData("off_work_time",strOffWork);
  assert(strOnWork.length() >0 && strOffWork.length() >0);

  if (strOnWork.length() <=0 || strOffWork.length() <=0) {
    YSOS_LOG_ERROR("work time is not set, jump sate SuspendService ,jump to state:" << STR_STATUS_TARGETSEARCHING_STATICSEARCHING);
    /// 延时5秒后切
    YSOS_LOG_INFO("延时5秒后切");
    //boost::this_thread::sleep_for(boost::chrono::milliseconds(5000));
    int rst = DoSwitchNotifyService(STR_STATUS_TARGETSEARCHING_STATICSEARCHING, context);
    return YSOS_ERROR_SUCCESS;
  }

  boost::posix_time::time_duration on_work(boost::posix_time::duration_from_string(strOnWork));
  boost::posix_time::time_duration off_work(boost::posix_time::duration_from_string(strOffWork));

  // local time
  boost::posix_time::time_duration local_time(boost::posix_time::second_clock::local_time().time_of_day());
  if (local_time >= on_work && local_time <= off_work) {
    /// 上班时间
    YSOS_LOG_INFO("working time switch to status:" << STR_STATUS_TARGETSEARCHING_STATICSEARCHING);
    /// 延时5秒后切
    YSOS_LOG_INFO("延时5秒后切");
    //boost::this_thread::sleep_for(boost::chrono::milliseconds(5000));
    ///播放上班语
    YSOS_LOG_INFO("curise_ans "<< hint_start_to_work_);
    data_ptr_->SetData("curise_ans", hint_start_to_work_);
    int rst = DoSwitchNotifyService(STR_STATUS_TARGETSEARCHING_STATICSEARCHING, context);
    strategy_search_people_.StartSearchPeople();
    assert(YSOS_ERROR_SUCCESS == rst);
    return YSOS_ERROR_SUCCESS;
  }
  return YSOS_ERROR_SUCCESS;
}


int LiteRobotCapacityStrategyCallback::ProcessStateStaticSearching(CallbackInterface *context) {
#ifdef _YSOS_LITEROBOT_V2002
  return ProcessStateStaticSearchingV2002(context);
#else
  return ProcessStateStaticSearchingV2003(context);
#endif
}

int LiteRobotCapacityStrategyCallback::ProcessStateStaticSearchingV2002(CallbackInterface *context) {
  /// 如果到了非上班时间，则切换到BeforeWork状态
  std::string strOnWork;
  data_ptr_->GetData("on_work_time",strOnWork);
  std::string strOffWork;
  data_ptr_->GetData("off_work_time",strOffWork);
  assert(strOnWork.length() >0 && strOffWork.length() >0);

  if (strOnWork.length() <=0 || strOffWork.length() <=0) {
    YSOS_LOG_ERROR("work time is not set, jump sate SuspendService ,jump to state:" << STR_STATUS_TARGETSEARCHING_STATICSEARCHING);
    /// 延时5秒后切
    //YSOS_LOG_INFO("延时5秒后切");
    //boost::this_thread::sleep_for(boost::chrono::milliseconds(5000));
    int rst = DoSwitchNotifyService(STR_STATUS_TARGETSEARCHING_STATICSEARCHING, context);
    return YSOS_ERROR_SUCCESS;
  }

  boost::posix_time::time_duration on_work(boost::posix_time::duration_from_string(strOnWork));
  boost::posix_time::time_duration off_work(boost::posix_time::duration_from_string(strOffWork));

  // local time
  boost::posix_time::time_duration local_time(boost::posix_time::second_clock::local_time().time_of_day());
  if (!(local_time >= on_work && local_time <= off_work)) {
    ///非上班时间
    YSOS_LOG_INFO("not working time switch to status:" << STR_STATUS_POWERMANAGER_BEFOREWORK);
    /// 延时5秒后切
    //YSOS_LOG_INFO("延时5秒后切");
    //boost::this_thread::sleep_for(boost::chrono::milliseconds(5000));
    ///播放offwork语
    YSOS_LOG_INFO("hint_beforework "<< hint_off_to_work_);
    data_ptr_->SetData("hint_beforework", hint_off_to_work_);
    int rst = DoSwitchNotifyService(STR_STATUS_POWERMANAGER_BEFOREWORK, context);
    assert(YSOS_ERROR_SUCCESS == rst);
    return YSOS_ERROR_SUCCESS;
  }
  return YSOS_ERROR_SUCCESS;
}

int LiteRobotCapacityStrategyCallback::ProcessStateStaticSearchingV2003(CallbackInterface *context)  {
  
  return YSOS_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////
void LiteRobotCapacityStrategyCallback::DoSomeThingSwitchNoPeople(const TDPeopleStatus& pre_people_status, std::vector<TDFaceDetectUnit>& pre_peoople, CallbackInterface *context) {
  YSOS_LOG_INFO("DoSomeThingSwitchNoPeople");

  ///通知人离开了(上送人离开事件，人工坐席人离开等）
  NotifyCustomerLeave(pre_people_status, pre_peoople, context);

  /// 播放再见语
  /*
  单人服务：
  1.再见（有详细人脸播详细人脸，否则播模糊人脸）
  多人服务：
  1.再见（女士们，先生们，孩子们），不播放“你们”
  无人脸模式服务：
  1.再见
  */
  std::string titles_all = GetAllTitleFromPeople(pre_peoople);
  if ("你们" == titles_all || "你" == titles_all) {
    titles_all = "";
  }
  std::string tmp_bye_string = titles_all + hint_farewell_;

  /// 兼容以前的配置，使用字段 curise_ans 来播放离开语
  data_ptr_->SetData("curise_ans", tmp_bye_string);
  YSOS_LOG_INFO("set curise_ans value:" << tmp_bye_string);

  /// 状态机切换到WaitingService状态
  YSOS_LOG_INFO("working time switch to status:" << STR_STATUS_TARGETSEARCHING_STATICSEARCHING);
  int rst = DoSwitchNotifyService(STR_STATUS_TARGETSEARCHING_STATICSEARCHING, context);
  return;
}
void LiteRobotCapacityStrategyCallback::DoSomeThingSwitchHavePeople(const TDPeopleStatus& pre_people_status, std::vector<TDFaceDetectUnit>& peoples, CallbackInterface *context) {
  /// 通知人来了（上送人来事件，通知人工坐席等）
  NotifyCustomerServe(pre_people_status, peoples, context);

  /// 播放欢迎语
  /*
  单人服务：
  1.播报普通欢迎语（得等详细人脸信息结果）
  （中间不管时长多长，如果是上个人，播报：我们又见面啦！默认切换到首页，如果不是上个人，播报欢迎语。）
  多人服务：
  1.播报欢迎语（不等详细人脸信息）：你们好，我是棠宝智能机器人（模糊人脸信息，女士们，先生们，孩子们）。
  无人脸模式服务（不等人脸信息）：
  1.播报普通欢迎语
  */
  std::string titles_all = GetAllTitleFromPeople(peoples);
  if(titles_all.length() >0) {
    titles_all += ",";
  }
  std::string greet_word;
  if(peoples.size() <2) {
    greet_word = titles_all+ "你好";
  }else {
    greet_word = titles_all + "你们好";
  }

  if (1 == peoples.size() && HistoryServePeopleHaveRecord(peoples[0])) { ///< 如果是历史服务过的人(即上一次服务过的人）
    greet_word += ",我们又见面啦！";
  }

  if (hint_helloword_.length() >0) {
    greet_word = greet_word + "," + hint_helloword_;
  }

  /// 兼容以前的配置，使用 home_page_greet 来播放欢迎语
  YSOS_LOG_INFO("greet word "<< greet_word);
  data_ptr_->SetData("home_page_greet", greet_word);

  /// 保存到历史服务过的人中
  if (1 == peoples.size()) {
    HistoryServePeopleAdd(peoples);
  } else {
    HistoryServePeopleClear();
  }

  /// 状态机切到服务状态
  YSOS_LOG_INFO("working time switch to status:" << STR_STATUS_ONSERVICE);
  int rst = DoSwitchNotifyService(STR_STATUS_ONSERVICE, context);
  return;
}

void LiteRobotCapacityStrategyCallback::DoSomeThingCurrentHavePeople(std::vector<TDFaceDetectUnit>& peoples, const int lasttime_interactive_clock, const bool current_face_in_camera, CallbackInterface *context) {

  if (1 == peoples.size()) {
    HistoryServePeopleAdd(peoples);
  } else {
    HistoryServePeopleClear();
  }
}
void LiteRobotCapacityStrategyCallback::DoSomeThingChangePeople(std::vector<TDFaceDetectUnit>& old_peooples, std::vector<TDFaceDetectUnit>& new_peoples, CallbackInterface *context) {
  /// 通知人工坐席人来了
  NotifyCustomerServe(TDPeopleStatus_Have, new_peoples, context);

  bool need_play_welcome = false;   ///< 是否播放欢迎语

  /// 以下与公版统一，不播放欢迎语，注释掉
  /// 如果是切换到单个人，同时人有详细人脸信息，则播放人信息，否则不播放
  //if(1 == new_peoples.size() && 1 == new_peoples[0].have_perfect_face_ && new_peoples[0].perfect_face_1n_ptr_
  //  && new_peoples[0].perfect_face_1n_ptr_->vec_faces.size() >0) {
  //    YSOS_LOG_INFO("换单人，新服务对象有详细人脸信息，播放欢迎语");
  //    need_play_welcome = true;
  //}else if(1 == old_peooples.size() && 1 == old_peooples[0].have_perfect_face_) {
  //  YSOS_LOG_INFO("换单人，原先的对象有详细人脸信息，播放欢迎语，表示原先人离开");
  //  need_play_welcome = true;
  //}
  //else {
  //  YSOS_LOG_INFO("换人，但不播放欢迎欢迎语");
  //}

  /// 播放欢迎语
  if(need_play_welcome) {
    std::string titles_all = GetAllTitleFromPeople(new_peoples);
    if(titles_all.length() >0) {
      titles_all += ",";
    }
    std::string greet_word;
    if(new_peoples.size() <2) {
      greet_word = titles_all+ "你好";
    }else {
      greet_word = titles_all + "你们好";
    }
    //TODO:add for linux
    RequestService("default@TtsExtModule", boost::lexical_cast<std::string>(CMD_CUSTOM_MODE), "text", greet_word, context);
  }

  /// 状态机不用切，当前已经是服务状态了

}

std::string LiteRobotCapacityStrategyCallback::GetAllTitleFromPeople(const std::vector<TDFaceDetectUnit>& peooples) {
  std::string titles_all;

  if (0 == peooples.size()) { ///< 无人脸模式
    titles_all = "";
  } else if (1 == peooples.size()) {   ///< 单人模式
    if (1 == peooples[0].have_perfect_face_ && peooples[0].perfect_face_1n_ptr_ && peooples[0].perfect_face_1n_ptr_->vec_faces.size() >0) {
      /// 播放详细人脸
      titles_all = peooples[0].perfect_face_1n_ptr_->vec_faces[0]->person_name;
    } else {
      /// 播放模糊人脸
      titles_all = GetTitleFromAgeGender(peooples[0].face_detect_info_ptr_->age, peooples[0].face_detect_info_ptr_->gender);
    }
  } else {
    /// 多人模式
    std::vector<std::string> vec_titles;
    for (int i=0; i< peooples.size(); ++i) {
      vec_titles.push_back(GetTitleFromAgeGender(peooples[i].face_detect_info_ptr_->age, peooples[i].face_detect_info_ptr_->gender));
    }
    /// 判断称呼是不是都一样
    bool is_the_same = true;
    for (int i =1; i< vec_titles.size(); ++i) {
      if (vec_titles[0] != vec_titles[i]) {
        is_the_same = false;
      }
    }
    if (is_the_same && vec_titles[0].length() >0) {
      titles_all = vec_titles[0] + "们";
    }
    
    /// 为了与公版统一，这里不处理
    titles_all = "";
  }
  return titles_all;
}

std::string LiteRobotCapacityStrategyCallback::GetTitleFromAgeGender(const int& age, const std::string& gender) {

  /*
  称呼（模糊人脸）
  1.无人脸：不加称呼
  2.单人：无年龄和性别                -> 你
  年龄<=15                      ->小朋友
  年龄>15 && <=40 + 男 ->帅哥
  年龄>15 && <=40 + 女  ->美女
  年龄>40 + 男                  ->先生
  年龄>40 +女                  ->女士
  3.多人：总计算单人称呼
  同一称呼： 后面加个“们”
  不同称呼：你们
  */
  if (gender.length() <=0 || 0 == age) {
    return "";
  }

  //if (age <=15) {   ///< 虹软人脸识别年龄不准，部分人会识别年龄偏低。这里改一下优化一下，原先的“小朋友”，与“帅哥”“美女”同一个称呼。
  //  return "小朋友";
  //} else 
  if (/*age >15 && */age <=40) {
    return "-1" == gender ? "美女" : "帅哥";
  } else {
    return "-1" == gender ? "女士" : "先生";
  }
  assert(false && "not get title from age gender");
  return "";
}

void LiteRobotCapacityStrategyCallback::NotifyCustomerLeave(const TDPeopleStatus& pre_people_status, std::vector<TDFaceDetectUnit>& pre_peoople, CallbackInterface *context) {

  /// 上送人离开事件
  DoEventNotifyService("customer_leave_event", "customer_leave_callback", "", context);
   
  /// 通知人工坐席人离开
  //TODO:add for linux
  /*std::string cstminfo = "{\"type\":\"artificial_auxiliary_event\","
                         "\"data\":{\"messageType\":\"c2noticeRemove\",\"messageSource\": null,\"messageContent\":"
                         "{\"level\":3,\"content\":\"\"},\"messageTarget\":\"\"}}";
  RequestService(STR_MODULE_ARTIFICIAL,GetUtility()->ConvertToString(CMD_ARTIFICIAL_AUXILIARY_HANGUP), "text", cstminfo, context);*/
}

void LiteRobotCapacityStrategyCallback::NotifyCustomerServe(const TDPeopleStatus& pre_people_status, std::vector<TDFaceDetectUnit>& peoples, CallbackInterface *context) {
  /// 这里只取第一个人的数据，当时是无人脸数据
  /// 为发兼容，参考模块 CustomerDetStrategyCallback
  TDFaceDetectInfoPtr person_blur_ptr = NULL;
  FaceCompareResultPtr person_detail_info_ptr = NULL;

  if (peoples.size() >0) {
    person_blur_ptr = peoples[0].face_detect_info_ptr_;
  }
  if (peoples.size() >0 && 1 ==peoples[0].have_perfect_face_ && peoples[0].perfect_face_1n_ptr_ && peoples[0].perfect_face_1n_ptr_->vec_faces.size() >0) {
    person_detail_info_ptr = peoples[0].perfect_face_1n_ptr_->vec_faces[0];
  }

  int tmp_age = 0;
  std::string tmp_gender ;
  if (person_detail_info_ptr) {
    tmp_age = atoi(person_detail_info_ptr->age.c_str());
    tmp_gender = person_detail_info_ptr->sex;
  } else if (person_blur_ptr) {
    tmp_age = person_blur_ptr->age;
    tmp_gender = person_blur_ptr->gender;
  }

  /// 上送人来了事件
  char cstminfo[400] = {'\0'};
  if (person_blur_ptr && 0 != tmp_age && tmp_gender.length() >0) {
    sprintf(cstminfo, "{\"age\":%d,\"gender\":\"%s\"}",
            tmp_age,
            "1" == tmp_gender ?"male":"female");
    DoEventNotifyService("customer_info_event", "customer_info_callback", cstminfo, context);
  }

  /// 通知人工坐席人来了
  //TODO:add for linux
  /*sprintf(cstminfo,"{\"type\":\"artificial_auxiliary_event\","
          "\"data\":{\"messageType\":\"c2notice\",\"messageSource\": null,\"messageContent\":"
          "{\"level\":3,\"content\":\"[人脸识别]姓名:%s,年龄:%d,性别:%s\"},\"messageTarget\":\"\"}}",
          (NULL == person_detail_info_ptr) ?"--":person_detail_info_ptr->person_name.c_str(),
          tmp_age,
          "1" == tmp_gender ?"男":("-1" == tmp_gender ?"女":"--"));
  RequestService(STR_MODULE_ARTIFICIAL,GetUtility()->ConvertToString(CMD_ARTIFICIAL_AUXILIARY_CALL), "text", cstminfo, context);*/
}
void LiteRobotCapacityStrategyCallback::NotifyArtificialSeatChangePeople(std::vector<TDFaceDetectUnit>& old_peooples, std::vector<TDFaceDetectUnit>& new_peooples, CallbackInterface *context) {
  NotifyCustomerServe(TDPeopleStatus_Have, new_peooples, context);
}

void LiteRobotCapacityStrategyCallback::HistoryServePeopleAdd(const std::vector<TDFaceDetectUnit>& peoples) {
  /// 添加到历史服务对象中，如果有详细人脸则根据详细人脸来判断，如果没有则根据trackid
  //for (unsigned int i = 0; i< peoples.size(); ++i)
  //{
  //  if(HistoryServePeopleHaveRecord(peoples[0])) {
  //    continue;
  //  }
  //  history_server_people_.push_back(peoples[0]);
  //}

  ///// 最多保存100个
  //while(history_server_people_.size() >100) {
  //  history_server_people_.pop_front();
  //}
  history_server_people_ = peoples;
  return;
}
void LiteRobotCapacityStrategyCallback::HistoryServePeopleClear() {
  history_server_people_.clear();
}
bool LiteRobotCapacityStrategyCallback::HistoryServePeopleHaveRecord(const TDFaceDetectUnit& people) {
  const TDFaceDetectUnit& face_unit_ret = people;
  FaceCompareResultPtr face_detail_ptr =NULL;
  if (1 == face_unit_ret.have_perfect_face_ && face_unit_ret.perfect_face_1n_ptr_ && face_unit_ret.perfect_face_1n_ptr_->vec_faces.size() >0) {
    /// 有详细人脸
    face_detail_ptr = face_unit_ret.perfect_face_1n_ptr_->vec_faces[0];
  }

  std::vector<TDFaceDetectUnit>::iterator it_find = history_server_people_.begin();
  for (it_find; it_find != history_server_people_.end(); ++it_find) {
    if (it_find->face_detect_info_ptr_->trackid == face_unit_ret.face_detect_info_ptr_->trackid) {
      break;
    }
    if (NULL == face_detail_ptr) {
      continue;
    }
    if (!(it_find->have_perfect_face_ && it_find->perfect_face_1n_ptr_ && it_find->perfect_face_1n_ptr_->vec_faces.size()>0)) {
      continue;
    }
    if (it_find->perfect_face_1n_ptr_->vec_faces[0]->person_name == face_detail_ptr->person_name) {
      break;   ///< find
    }
  }

  if (it_find != history_server_people_.end()) {
    return true;  ///< find
  }
  return false;
}

//////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
bool LiteRobotCapacityStrategyCallback::HaveTouchScreen() {
  /// 判断鼠标是否有用过，屏幕是否有被点击过
  POINT temp_point;
  if (::GetCursorPos(&temp_point)) {
    if (abs(last_time_user_clock_screen_point_.x - temp_point.x) + abs(last_time_user_clock_screen_point_.y - temp_point.y) > /*config_min_not_move_cursor_distance*/50
        && false == IsTeamviewDesktopConn()
       ) {
      /// 鼠标有动过
      last_time_user_clock_screen_point_ = temp_point;
      YSOS_LOG_INFO("检测到鼠标有动过 100%%有人");
      return true;
    }
  } else {
    YSOS_LOG_ERROR("获取不了鼠标的坐标");
    assert(false && "获取不了鼠标的坐标");
  }
  return false;
}

bool LiteRobotCapacityStrategyCallback::IsTeamviewDesktopConn() {
  //GetITMProcess gitprocess;
  //DWORD did = gitprocess.getProcID("TeamViewer_Desktop.exe");
  //return -1 != did ? true : false;
  return false;
}
#endif
//////////////////////////////////////////////////////////////////////////

}
