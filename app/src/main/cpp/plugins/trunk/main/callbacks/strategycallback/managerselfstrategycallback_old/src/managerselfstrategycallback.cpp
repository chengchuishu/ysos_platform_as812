/**
 *@file managerselfstrategycallback.cpp
 *@brief manager self strategy call back
 *@version 1.0
 *@author l.j..
 *@date Created on: 2016-10-20 15:16:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
/// Private Headers //  NOLINT
#include "../include/managerselfstrategycallback.h"

#include <stdio.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

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

const char *TTS_MODULE          =  "default@TtsExtModule";
const char *TTS_IOCTL_ID        = "8139";

const char *POWERMGR_MODULE     = "default@PowerManagerModule";
const char *CHASSIS_MODULE      = "default@ChassisModule1";
const char *ARTIFICIAL_MODULE   = "default@ArtificialAuxiliaryModule";

const char *WORK_STATE          = "work_state";
const char *ELECTRICITY         = "electricity";
const char *CLECTCHARGE         = "electcharge";
const char *IS_CHARGING         = "is_charging";
const char *IS_EMERGENCYSTOP    = "is_emergencystop";
const char *IS_BATTERY_LOW      = "is_battery_low";

const char *ROBOT_STATE           = "robot_state";
const char *ROBOT_STATE_NULL      = "null";
const char *ROBOT_CRUISE_SERVE    =  "cruise_serve";
const char *ROBOT_GO_HOME         = "go_home";
const char *ROBOT_CHARGING        = "charging";
const char *ROBOT_FAIL_CHARGE     = "failcharge";

const char *CUSTOMER_STATUS          =  "customer_status";
const char *SERVER_STATUS_IDLE       = "0";

const char *CRUISE_MODE          = "cruise_mode";
const char *LAST_CRUISE_STATUS   = "last_cruise_status";
const char *CURISE_ANS           = "curise_ans";

const char *ROBOT_LOCAL_SEARCH    =  "local_search";

const char *CHASSIS_OUTPUT_STATUS_CHARGING =  "charging";
const char *CHASSIS_OUTPUT_STATUS_BATTERY  =  "battery";

const char *MOVE_STATE          = "task_status";
const char *MOVE_STATE_UNREACH  = "unreach";
const char *MOVE_STATE_WARNING  = "warning";

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
    BATTERY_RANGE_HIGH,
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
    DECLARE_PLUGIN_REGISTER_INTERFACE ( ManagerSelfStrategyCallback,  CallbackInterface );  ///<  插件的入口，一定要加上 //  NOLINT
    ManagerSelfStrategyCallback::ManagerSelfStrategyCallback ( const std::string &strClassName ) : BaseStrategyCallbackImpl ( strClassName )
        , battery_ ( 0xff )
        , battery_range_ ( 0xff )
        , charging_ ( 0xff )
        , mngself_ ( false )
        , poweroff_ ( false )
        , last_retry_ ( false )
        , gohome_cnt_ ( INFINITE )
        , last_failed_()
        , warning_gap_ ( 10 )
        , last_warning_spk_()
        , warning_words_ ( "宝宝要回家，大家可否避让一下" )
        , succeed_to_charge_ ( "宝宝到家补充能量了" )
        , wait_for_retry_ ( "前方高能出没，让我稍作调整再出发" )
        , failed_to_charge_ ( "充电桩无法到达" )
        , leave_charger_ ( "宝宝离开充电桩了" )
        , ignore_charge_ ( "宝宝已经在充电了" )
        , start_to_work_ ( "新的一天大家好，宝宝开始服务了" )
        , power_full_ ( "宝宝充满活力啦" )
        , task_run_idx_ ( 0 )
        , today_task_num_ ( 0 )
        , today_task_ ( NULL )
        , task_day_()
        , nxt_work_day_()
        , breakDate_ ( "" )
        , data_ptr_ ( GetDataInterfaceManager()->GetData ( GLOBAL_DATA_KEY ) )
    {
        memset ( &nxt_work_day_, 0,  sizeof ( nxt_work_day_ ) );
        logger_ = GetUtility()->GetLogger ( "ysos.managerselfstrategy" );
    }

    ManagerSelfStrategyCallback::~ManagerSelfStrategyCallback (  void )
    {
    }

    int ManagerSelfStrategyCallback::HandleMessage ( const std::string &event_name, BufferInterfacePtr input_buffer, CallbackInterface *context )
    {
        YSOS_LOG_DEBUG ( "HandleMessage: " <<
                         event_name );
        if ( data_ptr_ == NULL )
        {
            YSOS_LOG_ERROR ( "data_ptr_ is NULL!" );
            return YSOS_ERROR_FAILED;
        }
        // sign state
        /*
        std::string signin;
        data_ptr_->GetData ( "signin_result", signin );
        if ( signin != "0" )
        {
            YSOS_LOG_DEBUG ( "signin is not finish: " <<  signin );
            return YSOS_ERROR_FAILED;
        }*/
        // robot state
        std::string robot_state;
        data_ptr_->GetData ( ROBOT_STATE, robot_state );
        YSOS_LOG_DEBUG ( "robot state: " << robot_state );
        std::string electchargestatus;
        data_ptr_->GetData ( CLECTCHARGE, electchargestatus );
        YSOS_LOG_DEBUG ( "electchargestatus: " << electchargestatus );
        /*if (strcharging.empty()) {
        	today_task_[k].action_place = task[j]["place"].asString();
        }*/
        int electcharge = atoi ( electchargestatus.c_str() );
        // common process
        if ( event_name == "chassis_info_battery" )
        {
            // work state
            std::string work_state;
            data_ptr_->GetData ( WORK_STATE, work_state );
            YSOS_LOG_DEBUG ( "work state: " << work_state );
            // customer
            std::string customer_status;
            data_ptr_->GetData ( CUSTOMER_STATUS,  customer_status );
            YSOS_LOG_DEBUG ( "customer status: " << customer_status );
            // 是否在充电状态
            switch ( UpdateChargeInfo ( context ) )
            {
                case YSOS_ERROR_SUCCESS:
                    if ( charging_ == 0 && electcharge == 0 )
                    {
                        if ( robot_state == ROBOT_CHARGING )
                        {
                            LeaveCharger ( ( customer_status !=  SERVER_STATUS_IDLE ), context );
                        }
                    }
                    else
                    {
                        if ( robot_state == ROBOT_GO_HOME || robot_state == ROBOT_FAIL_CHARGE )
                        {
                            SucceedToCharge ( ( customer_status !=  SERVER_STATUS_IDLE ), context );
                        }
                    }
                // no break here
                case YSOS_ERROR_FAILED:
                    return YSOS_ERROR_SUCCESS;
                default:
                    break;
            }
            // 获取电量数据
            {
                int rst = UpdateBatteryInfo ( context );
                if ( rst == YSOS_ERROR_FAILED )
                {
                    return YSOS_ERROR_SUCCESS;
                }
                if ( battery_range_ == BATTERY_RANGE_VERY_LOW || battery_range_ == BATTERY_RANGE_LOW  && customer_status == SERVER_STATUS_IDLE )
                {
                    if ( charging_ == 0 )
                    {
                        if ( robot_state != ROBOT_GO_HOME && robot_state != ROBOT_FAIL_CHARGE )
                        {
                            gohome_cnt_ = 1;
                            GoHome ( context );
                            return YSOS_ERROR_SUCCESS;
                        }
                    }
                    else
                    {
                        if ( robot_state != ROBOT_CHARGING )
                        {
                            if ( customer_status == SERVER_STATUS_IDLE )
                            {
                                SucceedToCharge ( false, context );
                            }
                            else
                            {
                                data_ptr_->SetData ( ROBOT_STATE,  ROBOT_CHARGING );
                            }
                            return YSOS_ERROR_SUCCESS;
                        }
                    }
                }
                else if ( rst == YSOS_ERROR_SUCCESS  && battery_range_ == BATTERY_RANGE_HIGH )
                {
                    if ( work_state == "1" && robot_state == ROBOT_CHARGING && electcharge == 0 )
                    {
                        if ( customer_status == SERVER_STATUS_IDLE )
                        {
                            std::string work;
                            data_ptr_->GetData ( LAST_CRUISE_STATUS, work );
                            if ( work.empty() )
                            {
                                YSOS_LOG_INFO ( "  no last work type" );
                                work = WORK_STATIC_SEARCH;
                            }
                            data_ptr_->SetData ( CURISE_ANS, power_full_ );
                            StartWork ( work, context );
                        }
                        else
                        {
                            data_ptr_->SetData ( ROBOT_STATE, ROBOT_CRUISE_SERVE );
                        }
                        return YSOS_ERROR_SUCCESS;
                    }
                }
            }
            // 获取急停按钮状态
            {
                UpdateEmergencyStopInfo ( context );
            }
            if ( robot_state == ROBOT_FAIL_CHARGE && ( battery_range_ == BATTERY_RANGE_VERY_LOW || customer_status == SERVER_STATUS_IDLE ) )
            {
                if ( TimeToRetry() )
                {
                    ++gohome_cnt_;
                    GoHome ( context );
                    return YSOS_ERROR_SUCCESS;
                }
            }
            // fix specical case: the front application restart
            if ( work_state.empty() )
            {
                task_run_idx_ = 0;
            }
            // get task
            int tsk_rst = getTaskReady();
            if ( work_state.empty() )
            {
                data_ptr_->SetData ( WORK_STATE, "1" );
                // specical case: power on after work off
                if ( today_task_[task_run_idx_].action_type ==  WORK_OFF )
                {
                    YSOS_LOG_DEBUG ( "skip the last work off" );
                    task_run_idx_++;
                    tsk_rst = YSOS_ERROR_SKIP;
                }
            }
            if ( tsk_rst == YSOS_ERROR_SUCCESS && customer_status == SERVER_STATUS_IDLE )
            {
                if ( today_task_[task_run_idx_].action_type == WORK_OFF )
                {
                    // work off
                    task_run_idx_++;
                    data_ptr_->SetData ( WORK_STATE, "2" );
                    if ( charging_ == 0 )
                    {
                        if ( robot_state != ROBOT_GO_HOME )
                        {
                            gohome_cnt_ = 1;
                            GoHome ( context );
                        }
                    }
                    else
                    {
                        if ( robot_state != ROBOT_CHARGING )
                        {
                            SucceedToCharge ( false, context );
                        }
                    }
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
                                data_ptr_->SetData ( ROBOT_LOCAL_SEARCH, param );
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
                    task_run_idx_++;
                    data_ptr_->SetData ( WORK_STATE, "1" );
                    if ( battery_range_ >= BATTERY_RANGE_MID )
                    {
                        if ( robot_state == ROBOT_STATE_NULL )
                        {
                            data_ptr_->SetData ( CURISE_ANS, start_to_work_ );
                        }
                        StartWork ( work_type, context );
                    }
                    else
                    {
                        data_ptr_->SetData ( LAST_CRUISE_STATUS, work_type );
                    }
                }
                return YSOS_ERROR_SUCCESS;
            }
            if ( robot_state == ROBOT_STATE_NULL )
            {
                // special case: no task now
                data_ptr_->SetData ( CURISE_ANS, start_to_work_ );
                StartWork ( WORK_STATIC_SEARCH, context );
                return YSOS_ERROR_SUCCESS;
            }
            if ( work_state == "2" && !mngself_ && ( charging_ || gohome_cnt_ == INFINITE ) )
            {
                // work off, then auto power off
                ManagerSelf ( context );
                mngself_ = true;
                return YSOS_ERROR_SUCCESS;
            }
        }
        // go home event
        else if ( event_name == "go_home_chassis_event" )
        {
            if ( charging_ == 1 )
            {
                RequestService ( TTS_MODULE, TTS_IOCTL_ID, "text", ignore_charge_, context );
            }
            else
            {
                gohome_cnt_ = 1;
                GoHome ( context );
            }
        }
        else if ( event_name == "chassis_charge_moving" )
        {
            if ( robot_state == ROBOT_GO_HOME )
            {
                std::string move_state;
                if ( YSOS_ERROR_SUCCESS == ParserMoveInfo ( input_buffer, move_state ) )
                {
                    if ( move_state == MOVE_STATE_UNREACH )
                    {
                        FailedToCharge ( context );
                    }
                    else if ( move_state == MOVE_STATE_WARNING )
                    {
                        GoHomeWarning ( context );
                    }
                }
            }
        }
        else if ( event_name == "powermgr_power_off" )
        {
            if ( !poweroff_ )
            {
                if ( charging_ == 1 )
                {
                    RequestService ( CHASSIS_MODULE, GetUtility()->ConvertToString ( CMD_CHASSIS_POWER_OFF ), "text", "", context );
                    RequestService ( POWERMGR_MODULE,  GetUtility()->ConvertToString (  CMD_POWERMGR_NOTIFY_CHARGING ), "text", "", context );
                }
                else
                {
                    RequestService ( POWERMGR_MODULE, GetUtility()->ConvertToString ( CMD_POWERMGR_NOTIFY_NOT_CHARGING ), "text", "", context );
                }
                poweroff_ = true;
            }
        }
        else if ( event_name == "task_updated_service" )
        {
            LoadTask ( boost::posix_time::microsec_clock::local_time().date() );
        }
        return YSOS_ERROR_SUCCESS;
    }

    int ManagerSelfStrategyCallback::Initialized (  const std::string &key,  const std::string &value )
    {
        YSOS_LOG_DEBUG ( "Initialized " << key << ": " << value );
        int n_return = YSOS_ERROR_SUCCESS;
        if ( key == "robot_init_mode" )
        {
            LoadTask ( boost::posix_time::microsec_clock::local_time().date() );
        }
        else if ( key == "warning_gap" )
        {
            warning_gap_ = atoi ( value.c_str() );
        }
        else if ( key == "warning_spk_gap" )
        {
            warning_words_ = value;
        }
        else if ( key == "succeed_to_charge" )
        {
            succeed_to_charge_ = value;
        }
        else if ( key == "wait_for_retry" )
        {
            wait_for_retry_ = value;
        }
        else if ( key == "failed_to_charge" )
        {
            failed_to_charge_ = value;
        }
        else if ( key == "leave_charger" )
        {
            leave_charger_ = value;
        }
        else if ( key == "ignore_charge" )
        {
            ignore_charge_ = value;
        }
        else if ( key == "start_to_work" )
        {
            start_to_work_ = value;
        }
        else if ( key == "power_full" )
        {
            power_full_ = value;
        }
        else if ( key == "breakDate" )
        {
            breakDate_ = value;
        }
        return n_return;
    }

    int ManagerSelfStrategyCallback::RequestService ( std::string service_name, std::string id,  std::string type, std::string json_value, CallbackInterface *context )
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

    int ManagerSelfStrategyCallback::LoadTask (  const boost::posix_time::ptime::date_type &day )
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
                std::string strcharging;
                data_ptr_->GetData ( IS_CHARGING, strcharging );
                YSOS_LOG_DEBUG ( "charging: " << strcharging );
                std::string electchargestatus;
                data_ptr_->GetData ( CLECTCHARGE, electchargestatus );
                YSOS_LOG_DEBUG ( "electchargestatus: " << electchargestatus );
                /*if (strcharging.empty()) {
                	today_task_[k].action_place = task[j]["place"].asString();
                }*/
                int charging = atoi ( strcharging.c_str() );
                int electcharge = atoi ( electchargestatus.c_str() );
                if ( 1 == charging || 1 == electcharge )
                {
                    today_task_[k].action_place = "-";
                }
                else
                {
                    today_task_[k].action_place = task[j]["place"].asString();
                }
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

    int ManagerSelfStrategyCallback::getTaskReady()
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

    bool ManagerSelfStrategyCallback::TimeToRetry()
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

    int ManagerSelfStrategyCallback::UpdateChargeInfo ( CallbackInterface *context )
    {
        std::string strcharging;
        data_ptr_->GetData ( IS_CHARGING, strcharging );
        YSOS_LOG_DEBUG ( "charging: " << strcharging );
        if ( strcharging.empty() )
        {
            return YSOS_ERROR_FAILED;
        }
        int charging = atoi ( strcharging.c_str() );
        if ( charging_ == charging )
        {
            return YSOS_ERROR_SKIP;
        }
        charging_ = charging;
        DoEventNotifyService ( "charging_info_event", "charging_info_callback", strcharging, context );
        return YSOS_ERROR_SUCCESS;
    }

    int ManagerSelfStrategyCallback::UpdateBatteryInfo (  CallbackInterface *context )
    {
        std::string strbattery;
        data_ptr_->GetData ( ELECTRICITY, strbattery );
        YSOS_LOG_DEBUG ( "battery: " << strbattery );
        if ( strbattery.empty() )
        {
            return YSOS_ERROR_FAILED;
        }
        int battery = atoi ( strbattery.c_str() );
        if ( battery > 100 )
        {
            return YSOS_ERROR_SKIP;
        }
        if ( battery_ == battery )
        {
            return YSOS_ERROR_SKIP;
        }
        battery_ = battery;
        DoEventNotifyService ( "battery_info_event", "battery_info_callback", strbattery, context );
        // battery range
        int range;
        do
        {
            int threshold;
            std::string strthreshold;
            data_ptr_->GetData ( "battery_high",  strthreshold );
            threshold = atoi ( strthreshold.c_str() );
            if ( battery >= threshold )
            {
                range = BATTERY_RANGE_HIGH;
                break;
            }
            data_ptr_->GetData ( "battery_low",  strthreshold );
            threshold = atoi ( strthreshold.c_str() );
            if ( battery >= threshold )
            {
                range = BATTERY_RANGE_MID;
                break;
            }
            data_ptr_->GetData ( "battery_very_low",  strthreshold );
            threshold = atoi ( strthreshold.c_str() );
            if ( battery >= threshold )
            {
                range = BATTERY_RANGE_LOW;
                break;
            }
            range = BATTERY_RANGE_VERY_LOW;
        } while ( 0 );
        YSOS_LOG_DEBUG ( "battery_range: " << range );
        if ( battery_range_ == range )
        {
            return YSOS_ERROR_SKIP;
        }
        battery_range_ = range;
        data_ptr_->SetData ( IS_BATTERY_LOW, ( battery_range_ <= BATTERY_RANGE_LOW ) ? "1" :  "0" );
        return YSOS_ERROR_SUCCESS;
    }

    int ManagerSelfStrategyCallback::UpdateEmergencyStopInfo ( CallbackInterface *context )
    {
        YSOS_LOG_DEBUG ( "UpdateEmergencyStopInfo: BEGIN"  );
        std::string stremergencystop;
        data_ptr_->GetData ( IS_EMERGENCYSTOP,  stremergencystop );
        YSOS_LOG_DEBUG ( "IS_EMERGENCYSTOP: " << stremergencystop );
        if ( stremergencystop.empty() )
        {
            return YSOS_ERROR_FAILED;
        }
        int emergencystop = atoi ( stremergencystop.c_str() );
        YSOS_LOG_DEBUG ( "emergencystop: " << emergencystop );
        if ( emergency_stop_ == emergencystop )
        {
            return YSOS_ERROR_SKIP;
        }
        emergency_stop_ = emergencystop;
        DoEventNotifyService ( "emergency_stop_event", "emergency_stop_callback", stremergencystop,  context );
        YSOS_LOG_DEBUG ( "UpdateEmergencyStopInfo: END"  );
        return YSOS_ERROR_SUCCESS;
    }

    int ManagerSelfStrategyCallback::ParserMoveInfo (  BufferInterfacePtr input_buffer,  std::string &move_state )
    {
        const char *buffer =  reinterpret_cast<const char *> ( GetBufferUtility()->GetBufferData ( input_buffer ) );
        if ( buffer == NULL )
        {
            YSOS_LOG_ERROR ( "buffer is NULL!" );
            return YSOS_ERROR_FAILED;
        }
        YSOS_LOG_DEBUG ( "input_buffer: " << buffer );
        Json::Reader js_reader;
        Json::Value js_value;
        if ( !js_reader.parse ( buffer, js_value,  false ) )
        {
            YSOS_LOG_ERROR ( "ParserMoveInfo failed!" );
            return YSOS_ERROR_FAILED;
        }
        js_value = js_value["data"];
        if ( js_value.isMember ( MOVE_STATE ) )
        {
            move_state = js_value[MOVE_STATE].asString();
            return YSOS_ERROR_SUCCESS;
        }
        return YSOS_ERROR_FAILED;
    }

    int ManagerSelfStrategyCallback::GoHome (  CallbackInterface *context )
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

    int ManagerSelfStrategyCallback::GoHomeWarning (  CallbackInterface *context )
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

    int ManagerSelfStrategyCallback::FailedToCharge (  CallbackInterface *context )
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

    int ManagerSelfStrategyCallback::SucceedToCharge (  bool inservice, CallbackInterface *context )
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

    int ManagerSelfStrategyCallback::LeaveCharger (  bool inservice, CallbackInterface *context )
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

    int ManagerSelfStrategyCallback::StartWork ( std::string work, CallbackInterface *context )
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

    int ManagerSelfStrategyCallback::ManagerSelf ( CallbackInterface *context )
    {
        int n_return = YSOS_ERROR_SUCCESS;
        // stop the module thread
        RequestService ( POWERMGR_MODULE,  GetUtility()->ConvertToString (  CMD_POWERMGR_THREAD_STOP ), "text", "", context );
        do
        {
            Json::Value js_value;
            Json::FastWriter js_writer;
            std::string str_date;
            // next work time
            std::tm tm_work_on;
            if ( task_run_idx_ < today_task_num_ )
            {
                tm_work_on.tm_year = ( int ) task_day_.year();
                tm_work_on.tm_mon = ( int ) task_day_.month();
                tm_work_on.tm_mday = ( int ) task_day_.day();
                tm_work_on.tm_hour = today_task_[task_run_idx_].action_hour;
                tm_work_on.tm_min = today_task_[task_run_idx_].action_minute;
            }
            else
            {
                tm_work_on = nxt_work_day_;
            }
            // caculate the power off
            boost::posix_time::ptime poweroff = boost::posix_time::microsec_clock::local_time();
            poweroff += boost::posix_time::minutes ( 2 );
            if ( tm_work_on.tm_year != 0 )
            {
                // caculate the power on
                boost::posix_time::ptime poweron ( boost::gregorian::date ( tm_work_on.tm_year,  tm_work_on.tm_mon, tm_work_on.tm_mday ),
                                                   boost::posix_time::time_duration (  tm_work_on.tm_hour, tm_work_on.tm_min, 0 ) );
                if ( poweron <= poweroff )
                {
                    YSOS_LOG_DEBUG ( "power on soon, skip" );
                    break;
                }
                // set power on
                js_value["year"] = ( int ) poweron.date().year();
                js_value["month"] = ( int ) poweron.date().month();
                js_value["day"] = ( int ) poweron.date().day();
                js_value["hour"] = poweron.time_of_day().hours();
                js_value["minute"] = poweron.time_of_day().minutes();
                js_value["second"] = poweron.time_of_day().seconds();
                str_date = js_writer.write ( js_value );
                str_date = GetUtility()->ReplaceAllDistinct ( str_date, "\\r\\n", "" );
                n_return = RequestService ( POWERMGR_MODULE, GetUtility()->ConvertToString ( CMD_POWERMGR_SYNCH_OPENSYS_TIME ), "text",  str_date, context );
                if ( YSOS_ERROR_SUCCESS != n_return )
                {
                    break;
                }
            }
            // set power off
            boost::this_thread::sleep_for (  boost::chrono::milliseconds ( 350 ) );
            js_value.clear();
            js_value["year"] = ( int ) poweroff.date().year();
            js_value["month"] = ( int )  poweroff.date().month();
            js_value["day"] = ( int ) poweroff.date().day();
            js_value["hour"] = poweroff.time_of_day().hours();
            js_value["minute"] =  poweroff.time_of_day().minutes();
            js_value["second"] = poweroff.time_of_day().seconds();
            str_date = js_writer.write ( js_value );
            str_date = GetUtility()->ReplaceAllDistinct ( str_date, "\\r\\n", "" );
            n_return = RequestService ( POWERMGR_MODULE, GetUtility()->ConvertToString ( CMD_POWERMGR_SYNCH_CLOSESYS_TIME ), "text", str_date, context );
            if ( YSOS_ERROR_SUCCESS != n_return )
            {
                break;
            }
            YSOS_LOG_DEBUG ( "ManagerSelf done and success" );
        } while ( 0 );
        // resume the module thread
        boost::this_thread::sleep_for ( boost::chrono::milliseconds ( 350 ) );
        RequestService ( POWERMGR_MODULE, GetUtility()->ConvertToString (  CMD_POWERMGR_THREAD_START ), "text", "",  context );
        return n_return;
    }

    void ManagerSelfStrategyCallback::NotifyCustomerLeave ( CallbackInterface *context )
    {
        /*std::string cstminfo =
            "{\"type\":\"artificial_auxiliary_event\","
            "\"data\":{\"messageType\":\"c2noticeRemove\",\"messageSource\": null,\"messageContent\":"
            "{\"level\":3,\"content\":\"\"},\"messageTarget\":\"\"}}";
        RequestService ( ARTIFICIAL_MODULE, GetUtility()->ConvertToString (  CMD_ARTIFICIAL_AUXILIARY_HANGUP ), "text", cstminfo, context );*/
    }

}