/**
 *@file autoselfstrategycallback.cpp
 *@brief the auto self strategy call back
 *@version 1.0
 *@author z.gg.
 *@date Created on: 2020-5-26 10:00:00
 *@copyright Copyright (c) 2020 YS Technology. All rights reserved.
 *
 */
/// Private Headers //  NOLINT
#include "../include/autoselfstrategycallback.h"

#include <stdio.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/locale.hpp>

#include "../../../public/include/common/commonenumdefine.h"

#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"
#include "../../../../../../core/trunk/main/public/include/os_hal_package/bufferutility.h"

#define INFINITE          0x7fffffff
#define MAX_MULTIPLIER    6
#ifdef _WIN32
const char *TASK_FILE           = "C:\\TDRobot\\YSOS\\data\\robottask\\task.xml";
#else
const char *TASK_FILE           = "../data/robottask/task.xml"; //WORK TASK DIRECTIOR
#endif

const char *TTS_MODULE          = "default@TtsExtModule";
const char *TTS_IOCTL_ID        = "8139";

const char *ROBOT_STATE           = "robot_state";
const char *ROBOT_STATE_NULL      = "null";
const char *ROBOT_CRUISE_SERVE    =  "cruise_serve";
const char *ROBOT_GO_HOME         = "go_home";
const char *ROBOT_CHARGING        = "charging";
const char *ROBOT_FAIL_CHARGE     = "failcharge";

const char *WORK_STATIC_SEARCH =  "WaitingService@TargetSearching@StaticSearching";
const char *WORK_LOCAL_SEARCH  =  "WaitingService@TargetSearching@LocalSearching";
const char *WORK_SPOT_SEARCH   =  "WaitingService@TargetSearching@SpotSearching";

enum ACTION_TYPE
{
    WORK_OFF      = 0,
    STATIC_SEARCH,
    LOCAL_SEARCH,
    SPOT_SEARCH
};

enum BATTERY_RANGE
{
    BATTERY_RANGE_VERY_LOW,
    BATTERY_RANGE_LOW,
    BATTERY_RANGE_MID,
    BATTERY_RANGE_HIGH
};

#ifdef _WIN32
#else
/**
 *原型：char *itoa( int value, char *string,int radix);
 *value：欲转换的数据；string：目标字符串的地址；radix：转换后的进制数，可以是10进制、16进制等。
 *功能：把一个整数转换成字符串
 **/
static void itoa( int value, char *str, int radix) {
   sprintf(str, "%d", value);
   return;
}
#endif

namespace ysos {
    DECLARE_PLUGIN_REGISTER_INTERFACE ( AutoSelfStrategyCallback,  CallbackInterface );  ///<  插件的入口，一定要加上 //  NOLINT
    AutoSelfStrategyCallback::AutoSelfStrategyCallback ( const std::string &strClassName ) : BaseStrategyCallbackImpl ( strClassName )
        , task_run_idx_ ( 0 )
        , today_task_num_ ( 0 )
        , today_task_ ( NULL )
        , task_day_()
        , nxt_work_day_()
        , breakDate_ ( "" )
        , data_ptr_ ( GetDataInterfaceManager()->GetData ( GLOBAL_DATA_KEY ) )
    {
        memset ( &nxt_work_day_, 0,  sizeof ( nxt_work_day_ ) );
        logger_ = GetUtility()->GetLogger ( "ysos.autoselfstrategy" );
         
        thread_run_self_ptr_ = NULL;
        atomic_thread_run_self_stop_flag_ =0;
    }

    AutoSelfStrategyCallback::~AutoSelfStrategyCallback (  void )
    {
        /*
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
        }*/
    }

    int AutoSelfStrategyCallback::Initialize(void *param)
    {
        YSOS_LOG_INFO("in Initialize");

        /*
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
            thread_run_self_ptr_.reset(new boost::thread(&AutoSelfStrategyCallback::ThreadFunRunSelf, this));
        }*/

        return BaseStrategyCallbackImpl::Initialize(param);
    }
    
    void AutoSelfStrategyCallback::ThreadFunRunSelf(CallbackInterface *context)
    {
        YSOS_LOG_INFO("ThreadFunRunSelf begin [Enter]");
        //TODO: 独立线程去处理时间计划任务
        //CallbackInterface *context;
        //while (true)
        {
            /* code */
            std::string cur_machine_state;
            data_ptr_->GetData("machine_state", cur_machine_state);
            
            if (cur_machine_state == "main@WaitingService" ) {
                if ( CheckTaskReadyBefore() ) {
                    //TODO:change state to main@WaitingService@BeforeWork
                    RequestService("default@TemplateModule", boost::lexical_cast<std::string>(CMD_TRANSFER_EVENT), "text", "{\"type\":\"time_before_work\",\"data\":{\"key\":\"value\"}}", context);
                } else if ( YSOS_ERROR_SUCCESS == getTaskReady() ) {
                    //TODO:change state to main@WaitingService@StartWork
                    if ( today_task_[task_run_idx_].action_type == WORK_OFF )
                    {
                        // case1: work off
                        RequestService("default@TemplateModule", boost::lexical_cast<std::string>(CMD_TRANSFER_EVENT), "text", "{\"type\":\"time_off_work\",\"data\":{\"key\":\"value\"}}", context);
                    }
                    else
                    {
                        std::string work_type;
                        switch ( today_task_[task_run_idx_].action_type )
                        {
                            case LOCAL_SEARCH:
                                {
                                    char param[64];
                                    sprintf ( param, "{\"still_rotate_name\":\"%s\"}", today_task_[task_run_idx_].action_place.c_str() );
                                    work_type = WORK_LOCAL_SEARCH;
                                    break;
                                }
                            case STATIC_SEARCH:
                                work_type = WORK_STATIC_SEARCH;
                                break;
                            case SPOT_SEARCH:
                                work_type = WORK_SPOT_SEARCH;
                                break;
                        }
                        // case2 :
                        RequestService("default@TemplateModule", boost::lexical_cast<std::string>(CMD_TRANSFER_EVENT), "text", "{\"type\":\"time_start_work\",\"data\":{\"work_type\":\"" + work_type + "\"}}", context);
                    }
                } else {
                    // special case: no task now
                    RequestService("default@TemplateModule", boost::lexical_cast<std::string>(CMD_TRANSFER_EVENT), "text", "{\"type\":\"time_start_work\",\"data\":{\"work_type\":\"WaitingService@TargetSearching@StaticSearching\"}}", context);
                }
            }
            
            
        }
        

        YSOS_LOG_INFO("ThreadFunRunSelf begin [End]");
        return;
    }
    int AutoSelfStrategyCallback::HandleMessage ( const std::string &event_name, BufferInterfacePtr input_buffer, CallbackInterface *context )
    {
        YSOS_LOG_DEBUG ( "HandleMessage: " << event_name );
        if ( data_ptr_ == NULL )
        {
            YSOS_LOG_ERROR ( "data_ptr_ is NULL!" );
            return YSOS_ERROR_FAILED;
        }
        //TODO: when event arrive, you can do action 
        // common process
        if ( event_name == "chassis_info_battery" )
        {
            // get task
            //TODO: check action time condition
            
        }
        else if (event_name == "network_status_event" )
        {
            ThreadFunRunSelf(context);
        }
        else if ( event_name == "task_updated_service" )
        {
            LoadTask ( boost::posix_time::microsec_clock::local_time().date() );
        }
        return YSOS_ERROR_SUCCESS;
    }

    int AutoSelfStrategyCallback::Initialized (  const std::string &key,  const std::string &value )
    {
        YSOS_LOG_DEBUG ( "Initialized " << key << ": " << value );
        int n_return = YSOS_ERROR_SUCCESS;
        if ( key == "robot_init_mode" )
        {
            LoadTask ( boost::posix_time::microsec_clock::local_time().date() );
        }
        else if ( key == "breakDate" )
        {
            breakDate_ = value;
        }
        return n_return;
    }

    int AutoSelfStrategyCallback::RequestService ( std::string service_name, std::string id,  std::string type, std::string json_value, CallbackInterface *context )
    {
        YSOS_LOG_DEBUG ( "RequestService " << service_name << " " << id << " " << type << " " << json_value );
        int n_return = YSOS_ERROR_SUCCESS;
        ReqIOCtlServiceParam req_service;
        req_service.id = id;
        req_service.service_name = service_name;
        if ( !type.empty() )
        {
            req_service.type = type;
        }
        if ( !json_value.empty() )
        {
            req_service.value = json_value;
        }
        n_return = DoIoctlService ( req_service,  context );
        if ( YSOS_ERROR_SUCCESS != n_return )
        {
            YSOS_LOG_DEBUG ( "execute DoIoctlService failed, n_return = " << n_return );
        }
        YSOS_LOG_DEBUG ( "RequestService done and success" );
        return n_return;
    }

    int AutoSelfStrategyCallback::LoadTask (  const boost::posix_time::ptime::date_type &day )
    {
        /* It is very bad not to read files in a new thread. However we do it first. */
        YSOS_LOG_DEBUG ( "LoadTask" );
        // record the task day
        task_day_ = day;
        // clear
        task_run_idx_ = 0;
        today_task_num_ = 0;
        if ( today_task_ != NULL )
        {
            delete []today_task_;
            today_task_ = NULL;
        }
        nxt_work_day_.tm_year = 0; // nxt_work_day_ is invaild
        // read and parse
        std::ifstream fi;
        fi.open ( TASK_FILE, std::ios_base::in );
        if ( !fi )
        {
            YSOS_LOG_WARN ( "Read the task file failed!" );
            return YSOS_ERROR_FAILED;
        }
        Json::Reader read;
        Json::Value value;
        bool rst = read.parse ( fi, value, false );
        fi.close();
        if ( !rst )
        {
            YSOS_LOG_WARN ( "Parse the task file failed!" );
            return YSOS_ERROR_FAILED;
        }
        int num = value.size();
        if ( num == 0 )
        {
            YSOS_LOG_WARN ( "Task num is invaild!" );
            return YSOS_ERROR_FAILED;
        }
        YSOS_LOG_INFO ( "Task num = " << num );
        // search today
        int i = -1;
        for ( int left = 0, right = num - 1;  left <= right; )
        {
            int year, month, day;
            int mid = ( left + right ) >> 1;
            std::string date = value[mid]["date"].asString();
            sscanf ( date.c_str(), "%d-%d-%d", &year, &month,  &day );
            boost::posix_time::ptime::date_type temp ( year, month, day );
            if ( temp == task_day_ )
            {
                YSOS_LOG_DEBUG ( "todya " << date );
                i = mid;
                break;
            }
            if ( temp < task_day_ )
            {
                left = mid + 1;
            }
            else
            {
                right = mid - 1;
            }
        }
        // read today tasks
        Json::Value task, tmp, tmp1;
        if ( i == -1 )
        {
            YSOS_LOG_WARN ( "Today task not found!" );
            //Today task no found , exec default task by wangjx 如果没有task则添加当前日期任务与明天任务，任务时间为默认时间与任务
            value.clear();
            Json::Value arrayObj1, arrayObj2;
            Json::Value item1, item2;
            Json::Value root;
            item1["action"]    = "LocalSearch";
            item1["place"]    = "yb";
            item1["time"]    = "07:20";
            arrayObj1.append ( item1 );
            item2["action"]    = "WorkOff";
            item2["place"]    = "-";
            item2["time"]    = "17:30";
            arrayObj1.append ( item2 );
            task.append ( arrayObj1 );
            tmp["date"] = to_iso_extended_string (  task_day_ );
            //YSOS_LOG_DEBUG("to_iso_extended_string(task_day_) " << to_iso_extended_string(task_day_) );
            tmp["task"] = arrayObj1;
            if ( breakDate_.length() > 0 ) //需要跳过休息日的
            {
                while ( 7 )  //一周7天，只循环7天就足以了
                {
                    boost::gregorian::day_iterator it ( task_day_,  1 );
                    ++it;
                    boost::posix_time::ptime::date_type temp (  it->year(), it->month(), it->day() );
                    int year = 0, month = 0, day = 0;
                    sscanf ( to_iso_extended_string ( temp ).c_str(),  "%d,%d,%d", &year, &month, &day );
                    if ( month == 1 || month == 2 )
                    {
                        month += 12;
                        year--;
                    }
                    //<!-- 是否跳过休息日 0:星期一 1：星期二 2：星期三 3：星期四 4：星期五 5：星期六 6：星期日-->
                    int iWeek = ( day + 2 * month + 3 * ( month + 1 ) / 5 + year + year / 4 - year / 100 + year / 400 ) % 7;
                    char str[25];

                    itoa ( iWeek, str, 10 );//把一个整数转换成字符串 

                    int pos = breakDate_.find ( str );
                    if ( pos == -1 )
                    {
                        tmp1["date"] = to_iso_extended_string ( temp );
                        break;
                    }
                    else
                    {
                        continue;
                    }
                }
            }
            else //无需跳过休息日
            {
                boost::gregorian::day_iterator it ( task_day_,  1 );
                ++it;
                boost::posix_time::ptime::date_type temp ( it->year(), it->month(), it->day() );
                tmp1["date"] = to_iso_extended_string ( temp );
            }
            //YSOS_LOG_DEBUG("to_iso_extended_string(temp) " << to_iso_extended_string(temp) );
            tmp1["task"] = arrayObj1;
            root.append ( tmp );
            root.append ( tmp1 );
            i = 0;
            value = root;
            //YSOS_LOG_DEBUG("root.toStyledString()= " << value.toStyledString() );
            task = value[i]["task"];
            //tmp["date"].append(task_day_.) ;
            //return YSOS_ERROR_FAILED;
        }
        else
        {
            task  = value[i]["task"];
        }
        int task_num = task.size();
        //YSOS_LOG_DEBUG("task_num " << task_num );
        if ( task_num == 0 )
        {
            YSOS_LOG_WARN ( "Toady task num is invaild!" );
            return YSOS_ERROR_FAILED;
        }
        today_task_ = new Task[task_num];
        if ( today_task_ == NULL )
        {
            YSOS_LOG_WARN ( "new task array failed!" );
            return YSOS_ERROR_FAILED;
        }
        today_task_num_ = 0;
        for ( int j = 0, &k = today_task_num_;  j < task_num; ++j )
        {
            std::string tm = task[j]["time"].asString();
            sscanf ( tm.c_str(), "%d:%d", &today_task_[k].action_hour,  &today_task_[k].action_minute );
            std::string act = task[j]["action"].asString();
            if ( act == "WorkOff" )
            {
                if ( k > 0  && today_task_[k - 1].action_type == WORK_OFF )
                {
                    YSOS_LOG_WARN ( "    redundant WorkOff" );
                    continue;
                }
                today_task_[k].action_type = WORK_OFF;
            }
            else if ( act == "StaticSearch" )
            {
                today_task_[k].action_type = STATIC_SEARCH;
            }
            else if ( act == "LocalSearch" )
            {
                today_task_[k].action_type = LOCAL_SEARCH;
                today_task_[k].action_place = task[j]["place"].asString();
                /*
                std::string strcharging;
                data_ptr_->GetData ( IS_CHARGING, strcharging );
                YSOS_LOG_DEBUG ( "charging: " << strcharging );
                std::string electchargestatus;
                data_ptr_->GetData ( CLECTCHARGE, electchargestatus );
                YSOS_LOG_DEBUG ( "electchargestatus: " << electchargestatus );
                int charging = atoi ( strcharging.c_str() );
                int electcharge = atoi ( electchargestatus.c_str() );
                if ( 1 == charging || 1 == electcharge )
                {
                    today_task_[k].action_place = "-";
                }
                else
                {
                    today_task_[k].action_place = task[j]["place"].asString();
                }*/
            }
            else if ( act == "SpotSearch" )
            {
                today_task_[k].action_type = SPOT_SEARCH;
            }
            else
            {
                YSOS_LOG_WARN ( "    unknown task " << act );
                continue;
            }
            YSOS_LOG_DEBUG ( "  got task: " << tm << " " << act << " " << today_task_[k].action_place );
            ++k;
        }
        // read next power-on
        /* unsigned int i=0;
            range_num = 0;
            while (range_num < DISTANCE_RANGE_MAXNUM && 2 == sscanf(value.c_str()+i,"%lf,%lf",&dis_range_[range_num],&wait_time_[range_num])) {
            range_num++;
            i=value.find('|',i);
            if(i==std::string::npos) {
            break;
            }
            i++;
            }*/
        if ( ++i >= num )
        {
            YSOS_LOG_WARN ( "no next task day!" );
        }
        else
        {
            //find next date power on
            if ( breakDate_.length() > 0 ) //有需要跳转的休息日
            {
                while ( i <= num )
                {
                    std::string strdate = value[i]["date"].asString();
                    //YSOS_LOG_DEBUG("strdate" << strdate <<" i=" << i );
                    sscanf ( strdate.c_str(), "%d-%d-%d", &nxt_work_day_.tm_year, &nxt_work_day_.tm_mon,  &nxt_work_day_.tm_mday );
                    //<!-- 是否跳过休息日 0:星期一 1：星期二 2：星期三 3：星期四 4：星期五 5：星期六 6：星期日-->
                    //根据日期判断星期几
                    if ( nxt_work_day_.tm_mon == 1 || nxt_work_day_.tm_mon == 2 )
                    {
                        nxt_work_day_.tm_mon += 12;
                        nxt_work_day_.tm_year--;
                    }
                    int iWeek = ( nxt_work_day_.tm_mday + 2 * nxt_work_day_.tm_mon + 3 * ( nxt_work_day_.tm_mon + 1 ) / 5 + nxt_work_day_.tm_year + nxt_work_day_.tm_year / 4 - nxt_work_day_.tm_year / 100 + nxt_work_day_.tm_year / 400 ) % 7;
                    //根据算出来的星期几，在Breakdate里查找有没有需要跳转的休息日
                    //<!-- 是否跳过休息日 0:星期一 1：星期二 2：星期三 3：星期四 4：星期五 5：星期六 6：星期日-->
                    char str[25];
                    itoa ( iWeek, str, 10 );
                    int pos = breakDate_.find ( str );
                    if ( pos == -1 )
                    {
                        Json::Value task = value[i]["task"];
                        if ( task.size() == 0 )
                        {
                            YSOS_LOG_WARN ( "next day task num is invaild!" );
                            nxt_work_day_.tm_year = 0;
                        }
                        else
                        {
                            std::string strtime = task[Json::Value::UInt (  0 )]["time"].asString();
                            sscanf ( strtime.c_str(), "%d:%d", &nxt_work_day_.tm_hour, &nxt_work_day_.tm_min );
                            YSOS_LOG_DEBUG ( "next power on: " << strdate <<  " " << strtime );
                        }
                        break;
                    }
                    else
                    {
                        ++i;
                        continue;
                    }
                }
            }
            else //无需要跳转的休息日
            {
                std::string strdate = value[i]["date"].asString();
                //YSOS_LOG_DEBUG("strdate" << strdate <<" i=" << i );
                sscanf ( strdate.c_str(), "%d-%d-%d", &nxt_work_day_.tm_year, &nxt_work_day_.tm_mon,  &nxt_work_day_.tm_mday );
                Json::Value task = value[i]["task"];
                if ( task.size() == 0 )
                {
                    YSOS_LOG_WARN ( "next day task num is invaild!" );
                    nxt_work_day_.tm_year = 0;
                }
                else
                {
                    std::string strtime = task[Json::Value::UInt (  0 )]["time"].asString();
                    sscanf ( strtime.c_str(), "%d:%d", &nxt_work_day_.tm_hour, &nxt_work_day_.tm_min );
                    YSOS_LOG_DEBUG ( "next power on: " << strdate <<  " " << strtime );
                }
            }
        }
        return YSOS_ERROR_SUCCESS;
    }

    bool timeCompare ( int cur_hour, int cur_minu, int tsk_hour, int tsk_minu )
    {
        if ( cur_hour > tsk_hour )
        {
            return true;
        }
        if ( cur_hour == tsk_hour )
        {
            return cur_minu >= tsk_minu;
        }
        return false;
    }

    int AutoSelfStrategyCallback::getTaskReady()
    {
        boost::posix_time::ptime local = boost::posix_time::microsec_clock::local_time();
        boost::posix_time::ptime::date_type today = local.date();
        if ( today != task_day_ )
        {
            LoadTask ( today );
        }
        if ( task_run_idx_ == today_task_num_ )
        {
            return YSOS_ERROR_FAILED;
        }
        int hour = ( int ) local.time_of_day().hours();
        int minu = ( int ) local.time_of_day().minutes();
        if ( !timeCompare ( hour, minu,  today_task_[task_run_idx_].action_hour, today_task_[task_run_idx_].action_minute ) )
        {
            return YSOS_ERROR_SKIP;
        }
        while ( task_run_idx_ + 1 < today_task_num_ )
        {
            if ( !timeCompare ( hour, minu,  today_task_[task_run_idx_ + 1].action_hour, today_task_[task_run_idx_ + 1].action_minute ) )
            {
                break;
            }
            YSOS_LOG_WARN ( "task skip " << task_run_idx_ );
            task_run_idx_++;
        }
        YSOS_LOG_DEBUG ( "task ready " << task_run_idx_ );
        return YSOS_ERROR_SUCCESS;
    }

    bool AutoSelfStrategyCallback::CheckTaskReadyBefore()
    {
        if ( today_task_num_ > 0 )
        {
            boost::posix_time::ptime local = boost::posix_time::microsec_clock::local_time();
            int hour = ( int ) local.time_of_day().hours();
            int minu = ( int ) local.time_of_day().minutes();
            if ( !timeCompare ( hour, minu,  today_task_[0].action_hour, today_task_[0].action_minute ) )
            {
                return true;
            }
        }
        return false;
    }

    bool AutoSelfStrategyCallback::TimeToRetry()
    {
        std::string strtimes;
        data_ptr_->GetData ( "charge_time", strtimes );
        int times = atoi ( strtimes.c_str() );
        if ( gohome_cnt_ >= times )
        {
            return false;
        }
        std::string strgap;
        data_ptr_->GetData ( "charge_gap", strgap );
        int gap = atoi ( strgap.c_str() );
        gap *= 1 << std::min ( MAX_MULTIPLIER, gohome_cnt_ - 1 );
        boost::posix_time::ptime curtime = boost::posix_time::microsec_clock::local_time();
        boost::posix_time::millisec_posix_time_system_config::time_duration_type time_elapse = curtime - last_failed_;
        int elaspse = time_elapse.total_seconds();
        YSOS_LOG_DEBUG ( "gap:" << gap << " elaspse:" <<  elaspse );
        return elaspse >= gap;
    }

    int AutoSelfStrategyCallback::GoHome (  CallbackInterface *context )
    {
        YSOS_LOG_DEBUG ( "GoHome" );
        NotifyCustomerLeave ( context );
        int rst = DoSwitchNotifyService ( "SuspendService@PowerManager@PrepareCharging",  context );
        if ( YSOS_ERROR_SUCCESS != rst )
        {
            YSOS_LOG_DEBUG ( "execute DoSwitchNotifyService failed" );
        }
        last_warning_spk_ =  boost::posix_time::microsec_clock::local_time();
        return rst;
    }

    int AutoSelfStrategyCallback::GoHomeWarning (  CallbackInterface *context )
    {
        boost::posix_time::ptime curtime = boost::posix_time::microsec_clock::local_time();
        boost::posix_time::millisec_posix_time_system_config::time_duration_type time_elapse = curtime - last_warning_spk_;
        int elaspse = time_elapse.total_seconds();
        if ( elaspse >= warning_gap_ )
        {
            RequestService ( TTS_MODULE, TTS_IOCTL_ID, "text",  warning_words_, context );
            last_warning_spk_ = curtime;
        }
        return YSOS_ERROR_SUCCESS;
    }

    int AutoSelfStrategyCallback::FailedToCharge (  CallbackInterface *context )
    {
        YSOS_LOG_DEBUG ( "FailedToCharge " << gohome_cnt_ );
        std::string strtimes;
        data_ptr_->GetData ( "charge_time", strtimes );
        int times = atoi ( strtimes.c_str() );
        if ( gohome_cnt_ >= times )
        {
            gohome_cnt_ = INFINITE;
            data_ptr_->SetData ( "fail_charge_ans", failed_to_charge_ );
        }
        else
        {
            last_failed_ = boost::posix_time::microsec_clock::local_time();
            data_ptr_->SetData ( "fail_charge_ans", wait_for_retry_ );
        }
        int rst = DoSwitchNotifyService ( "SuspendService@PowerManager@FailedToCharge",  context );
        if ( YSOS_ERROR_SUCCESS != rst )
        {
            YSOS_LOG_DEBUG ( "execute DoSwitchNotifyService failed" );
        }
        return rst;
    }

    int AutoSelfStrategyCallback::SucceedToCharge (  bool inservice, CallbackInterface *context )
    {
        YSOS_LOG_DEBUG ( "SucceedToCharge " << inservice );
        gohome_cnt_ = INFINITE;
        int rst;
        if ( inservice )
        {
            rst = RequestService ( TTS_MODULE, TTS_IOCTL_ID, "text", succeed_to_charge_, context );
            data_ptr_->SetData ( ROBOT_STATE, ROBOT_CHARGING );
            boost::this_thread::sleep ( boost::posix_time::milliseconds ( 3000 ) );
        }
        else
        {
            data_ptr_->SetData ( "on_charge_ans", succeed_to_charge_ );
            rst = DoSwitchNotifyService ( "SuspendService@PowerManager@OnCharging", context );
            if ( YSOS_ERROR_SUCCESS != rst )
            {
                YSOS_LOG_DEBUG ( "execute DoSwitchNotifyService failed" );
            }
        }
        return rst;
    }

    int AutoSelfStrategyCallback::LeaveCharger (  bool inservice, CallbackInterface *context )
    {
        YSOS_LOG_DEBUG ( "LeaveCharger " << inservice );
        int rst;
        if ( inservice )
        {
            rst = RequestService ( TTS_MODULE, TTS_IOCTL_ID, "text", leave_charger_, context );
            data_ptr_->SetData ( ROBOT_STATE,  ROBOT_FAIL_CHARGE );
            boost::this_thread::sleep ( boost::posix_time::milliseconds ( 3000 ) );
        }
        else
        {
            data_ptr_->SetData ( "fail_charge_ans", leave_charger_ );
            rst = DoSwitchNotifyService ( "SuspendService@PowerManager@FailedToCharge", context );
            if ( YSOS_ERROR_SUCCESS != rst )
            {
                YSOS_LOG_DEBUG ( "execute DoSwitchNotifyService failed" );
            }
        }
        return rst;
    }

    int AutoSelfStrategyCallback::StartWork ( std::string work, CallbackInterface *context )
    {
        YSOS_LOG_DEBUG ( "StartWork " << work );
        gohome_cnt_ = 0;
        int rst = DoSwitchNotifyService ( work, context );
        if ( YSOS_ERROR_SUCCESS != rst )
        {
            YSOS_LOG_DEBUG ( "execute DoSwitchNotifyService failed" );
        }
        return rst;
    }

}