/**
*@file websocket.cpp
*@brief Implement of gaussian chassis driver socket connect
*@version 0.9.0.0
*@author Wangxg
*@date Created on: 2016-12-15 14:40:00
*@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
* 
*/

/// Myself Headers
#include "../include/websocket.h"

/// YSOS Headers
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"

// #define  STATUS_MYNULL        "MYNULL"
#define  STATUS_CONNECTING    "Connecting"
#define  STATUS_OPEN          "Open"
#define  STATUS_FAILED        "Failed"
#define  STATUS_CLOSE         "Closed"

#define  SERVER_NA            "N/A"
#define  SERVER_NAME          "Server"

#define INIT_SEND_INFO        "Hello"

/// Device Status
#define DEVICE_STATUS_BATTERY         "battery"
#define DEVICE_STATUS_CHARGE          "charge"
#define DEVICE_STATUS_CHARGER         "charger"
#define DEVICE_STATUS_CHARGER_STATUS  "chargerStatus"
#define DEVICE_STATUS_SPEED           "speed"
#define DEVICE_STATUS_MILLAGE         "millage"
#define DEVICE_STATUS_MILEAGE         "mileage"
#define DEVICE_STATUS_EMERGENCYSTOP   "emergencyStop"

/// Health Status
#define HEALTH_STATUS_CONNECT_MCU      "MCUConnection"
#define HEALTH_STATUS_CONNECT_LASER    "laserConnection"
#define HEALTH_STATUS_CONNECT_ROUTER   "routerConnection"

/// Greet Status
#define GTEET_STATUS_CODE              "statusCode"

#define NOTICE_TYPE_LEVEL               "noticeTypeLevel"
#define NOTICE_TYPE                     "noticeType"

#define NOTICE_TYPE_INFO               "INFO"
#define NOTICE_TYPE_INFO_PLANING       "PLANNING"
#define NOTICE_TYPE_INFO_HEADING       "HEADING"
#define NOTICE_TYPE_INFO_UNREACHABLE   "UNREACHABLE"
#define NOTICE_TYPE_WARN               "WARN"
#define NOTICE_TYPE_WARN_LOCAL_FAIL    "LOCALIZATION_FAILED"
#define NOTICE_TYPE_WARN_GOAL_NOSAFE   "GOAL_NOT_SAFE"
#define NOTICE_TYPE_WARN_CLOSEOBSTACLE "TOO_CLOSE_TO_OBSTACLES"

static log4cplus::Logger g_logger = GetUtility()->GetLogger("ysos.chassis");;

/////////////////////////////////////////////////////////////////////////////
///////////// connection_metadata class /////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

connection_metadata::connection_metadata(int id, websocketpp::connection_hdl hdl, std::string uri,void *param_ptr) {
    m_id = id;
    m_hdl = hdl;
    m_status = STATUS_CONNECTING;
    m_uri = uri;
    m_server = SERVER_NA;
    m_move_latest_status = NOTICE_TYPE_INFO_PLANING;
    m_battery_status = -1;
    m_speed_status_ = 0.0;
    m_charge_status = -1;
    m_emergencystopstatus_ = false;
    m_bconnect_mcu_ = false;
    m_bconnect_laser_ = false;
    m_bconnect_router_ = false;
    m_greet_status_code_ = 0;
    m_error_status_code_ = 0;
    // param_ptr_ = param_ptr;
}

connection_metadata::~connection_metadata(void) {
}

void connection_metadata::on_open(client *client_ptr, websocketpp::connection_hdl hdl) {
    if(NULL == client_ptr ) {
        YSOS_LOG_ERROR_CUSTOM(g_logger, "Connection Client Pointer Exception!");
        return;
    }
    YSOS_LOG_DEBUG_CUSTOM(g_logger, "Now open socket");
    websocketpp::lib::error_code errcode;
    m_status = STATUS_OPEN;
    client::connection_ptr conptr = client_ptr->get_con_from_hdl(hdl);
    if(conptr) {
        m_server = conptr->get_response_header(SERVER_NAME);
    }
    /// send first message when on open to start web socket
    client_ptr->send(hdl, INIT_SEND_INFO, websocketpp::frame::opcode::text, errcode);
}

void connection_metadata::on_fail(client *client_ptr, websocketpp::connection_hdl hdl) {
    if(NULL == client_ptr ) {
        YSOS_LOG_ERROR_CUSTOM(g_logger, "Connection Client Pointer Exception!");
        return;
    }
    YSOS_LOG_DEBUG_CUSTOM(g_logger, "Open socket fail");
    m_status = STATUS_FAILED;
    client::connection_ptr conptr = client_ptr->get_con_from_hdl(hdl);
    if(conptr) {
        m_server = conptr->get_response_header(SERVER_NAME);
        m_error_reason = conptr->get_ec().message();
    }
}

void connection_metadata::on_close(client *client_ptr, websocketpp::connection_hdl hdl) {
    if(NULL == client_ptr ) {
        YSOS_LOG_ERROR_CUSTOM(g_logger, "Connection Client Pointer Exception!");
        return;
    }
    YSOS_LOG_DEBUG_CUSTOM(g_logger, "Now close socket");
    m_status = STATUS_CLOSE;
    client::connection_ptr conptr = client_ptr->get_con_from_hdl(hdl);
    if (conptr ) {
        std::stringstream s;
        s << "close code:" << conptr->get_remote_close_code() << "("
          << websocketpp::close::status::get_string(conptr->get_remote_close_code())
          << "), close reason: " << conptr->get_remote_close_reason();
        m_error_reason = s.str();
    }
}

void connection_metadata::on_message_for_device(websocketpp::connection_hdl, client::message_ptr msg) {
    if (msg && msg->get_opcode() == websocketpp::frame::opcode::text) {
        std::string strresult = msg->get_payload();
        // YSOS_LOG_DEBUG_CUSTOM(g_logger, "on_message_for_device " <<strresult);
        Json::Reader js_reader;
        Json::Value js_value;
        if(!js_reader.parse(strresult,js_value,false)) {
            YSOS_LOG_WARN_CUSTOM(g_logger, "Get Device Status Exception:" <<strresult);
            return;
        }
        if(true != js_value.isMember(DEVICE_STATUS_BATTERY) ||
                true != js_value.isMember(DEVICE_STATUS_CHARGE)  ||
                true != js_value.isMember(DEVICE_STATUS_SPEED)   ||
                true != js_value.isMember(DEVICE_STATUS_EMERGENCYSTOP)) {
            YSOS_LOG_WARN_CUSTOM(g_logger, "Get Device Status Exception:" <<strresult);
            return;
        }
        int ibattery = js_value[DEVICE_STATUS_BATTERY].asInt();
        bool bcharge = js_value[DEVICE_STATUS_CHARGE].asBool();
        if (true == js_value.isMember(DEVICE_STATUS_CHARGER) ) {
            int bcharger = js_value[DEVICE_STATUS_CHARGER].asInt();
            if (bcharger == 3 || bcharger == 4 || bcharger == 5) {
                bcharge = true;
            }
        }
        /*if (true == js_value.isMember(DEVICE_STATUS_CHARGER_STATUS) ) {
            bcharge =  js_value[DEVICE_STATUS_CHARGER_STATUS].asBool();;
        }*/
        float fspeed = (float)js_value[DEVICE_STATUS_SPEED].asDouble();
        bool bemergencystop = js_value[DEVICE_STATUS_EMERGENCYSTOP].asBool();
        set_battery_status(ibattery);
        set_charge_status(bcharge);
        set_speed_status(fspeed);
        set_emergencystopstatus_status(bemergencystop);
        YSOS_LOG_DEBUG_CUSTOM(g_logger, "The battery status is:" <<ibattery<<",the charge status is:"<<bcharge);
    }
    else {
    }
}

void connection_metadata::on_message_for_navigation(websocketpp::connection_hdl, client::message_ptr msg) {
    if (msg && msg->get_opcode() == websocketpp::frame::opcode::text) {
        std::string strresult = msg->get_payload();
        if(strresult.empty()) {
            YSOS_LOG_WARN_CUSTOM(g_logger, "Get navigation empty!");
            return;
        }
        // YSOS_LOG_DEBUG_CUSTOM(g_logger, "on_message_for_navigation " <<strresult);
        Json::Reader js_reader;
        Json::Value js_value;
        if (!js_reader.parse(strresult,js_value,false)) {
            YSOS_LOG_WARN_CUSTOM(g_logger, "Get navigation Exception:" <<strresult);
            return;
        }
        if(true != js_value.isMember(NOTICE_TYPE_LEVEL) || true != js_value.isMember(NOTICE_TYPE) ) {
            YSOS_LOG_WARN_CUSTOM(g_logger, "Get navigation Exception:" <<strresult);
            return;
        }
        std::string strnoticetype = js_value[NOTICE_TYPE_LEVEL].asString();
        std::string strlatest_status = js_value[NOTICE_TYPE].asString();
        YSOS_LOG_DEBUG_CUSTOM(g_logger, "navigation: " <<strnoticetype<<" "<<strlatest_status);
        set_move_latest_status(strlatest_status);
#if TEST_PLAN_TIME
        if(0==strnoticetype.compare(NOTICE_TYPE_INFO)) {
            if ( 0 == strlatest_status.compare(NOTICE_TYPE_INFO_PLANING) ) {
                time_last_ = boost::posix_time::microsec_clock::universal_time();
            }
            else if ( 0 == strlatest_status.compare(NOTICE_TYPE_INFO_HEADING) ) {
                boost::posix_time::ptime time_now = boost::posix_time::microsec_clock::universal_time();
                boost::posix_time::millisec_posix_time_system_config::time_duration_type time_elapse;
                time_elapse = time_now - time_last_;
                YSOS_LOG_DEBUG_CUSTOM(g_logger, "The path planning time is:" <<time_elapse.ticks()<<"us");
            }
#endif
        }
        else {
        }
    }
    void connection_metadata::on_message_for_health( websocketpp::connection_hdl, client::message_ptr msg ) {
        if (msg && msg->get_opcode() == websocketpp::frame::opcode::text) {
            std::string strresult = msg->get_payload();
            // YSOS_LOG_DEBUG_CUSTOM(g_logger, "on_message_for_health " <<strresult);
            if(strresult.empty()) {
                YSOS_LOG_WARN_CUSTOM(g_logger, "Get health empty!");
                return;
            }
            if (strresult[0]=='\"') {
                unsigned int i,j,k=0;
                for(i=1; i<strresult.size() && strresult[i]!='{'; i++);
                for(j=strresult.size()-2; j>=0 && strresult[j]!='}'; j--);
                for(; i<=j; i++) {
                    // to simplify the process, only change \" to "
                    if(strresult[i]!='\\') {
                        strresult[k++]=strresult[i];
                    }
                }
                strresult[k]='\0';
            }
            Json::Reader js_reader;
            Json::Value js_value;
            if (! js_reader.parse(strresult,js_value,false) ) {
                YSOS_LOG_WARN_CUSTOM(g_logger, "Get Device Health Status Exception:" <<strresult);
                return;
            }
            if(true != js_value.isMember(HEALTH_STATUS_CONNECT_MCU) ||
                    true != js_value.isMember(HEALTH_STATUS_CONNECT_LASER) ||
                    true != js_value.isMember(HEALTH_STATUS_CONNECT_ROUTER)) {
                YSOS_LOG_WARN_CUSTOM(g_logger, "Get Device Health Status Exception:" <<strresult);
                return;
            }
            bool bconnectmcu = js_value[HEALTH_STATUS_CONNECT_MCU].asBool();
            bool bconnectlaser = js_value[HEALTH_STATUS_CONNECT_LASER].asBool();
            bool bconnectrouter = js_value[HEALTH_STATUS_CONNECT_ROUTER].asBool();
            set_mcu_status(bconnectmcu);
            set_laser_status(bconnectlaser);
            set_router_status(bconnectrouter);
            YSOS_LOG_DEBUG_CUSTOM(g_logger, "The health status is:" <<bconnectmcu<<","<<bconnectlaser<<","<<bconnectrouter);
        }
        else {
        }
    }
    void connection_metadata::on_message_for_greet(websocketpp::connection_hdl, client::message_ptr msg) {
        if (msg && msg->get_opcode() == websocketpp::frame::opcode::text) {
            std::string strresult = msg->get_payload();
            if(strresult.empty()) {
                YSOS_LOG_WARN_CUSTOM(g_logger, "Get greet empty!");
                return;
            }
            // YSOS_LOG_DEBUG_CUSTOM(g_logger, "on_message_for_greet " <<strresult);
            Json::Reader js_reader;
            Json::Value js_value;
            if (! js_reader.parse(strresult,js_value,false) ) {
                YSOS_LOG_WARN_CUSTOM(g_logger, "Get Status Code Exception:" <<strresult);
                return;
            }
            if(true != js_value.isMember(GTEET_STATUS_CODE)) {
                YSOS_LOG_WARN_CUSTOM(g_logger, "Get Status Code Exception:" <<strresult);
                return;
            }
            int statusCode = js_value[GTEET_STATUS_CODE].asInt();
            YSOS_LOG_DEBUG_CUSTOM(g_logger, "The status code is:"<<statusCode);
            if (statusCode>=1100 && statusCode<=1105) {
                set_greet_status_code(statusCode);
            }
            else if (statusCode==1005 || statusCode==1006 || statusCode==1010) {
                set_error_status_code(statusCode);
            }
            else {
            }
        }
        else {
        }
    }
    void connection_metadata::set_move_latest_status(std::string move_status) {
        boost::lock_guard<boost::mutex> lock_guard_member(m_mutex);
        m_move_latest_status = move_status;
    }
    std::string connection_metadata::get_move_latest_status() {
        boost::lock_guard<boost::mutex> lock_guard_member(m_mutex);
        return m_move_latest_status;
    }
    void connection_metadata::clear_latest_status() {
        set_move_latest_status(NOTICE_TYPE_INFO_PLANING);
        return;
    }
    void connection_metadata::set_battery_status(int battery_status) {
        m_battery_status = battery_status;
    }
    int connection_metadata::get_battery_status() {
        return m_battery_status;
    }
    void connection_metadata::set_charge_status(bool charge_status) {
        m_charge_status = static_cast<int>(charge_status);
    }
    bool connection_metadata::get_charge_status() {
        return (m_charge_status == 1);
    }
    websocketpp::connection_hdl connection_metadata::get_hdl() const {
        return m_hdl;
    }
    int connection_metadata::get_id() const {
        return m_id;
    }
    std::string connection_metadata::get_status() const {
        return m_status;
    }
    void connection_metadata::set_mcu_status( bool mcu_status ) {
        m_bconnect_mcu_ = mcu_status;
    }
    bool connection_metadata::get_mcu_status() {
        return m_bconnect_mcu_;
    }
    void connection_metadata::set_laser_status( bool laser_status ) {
        m_bconnect_laser_ = laser_status;
    }
    bool connection_metadata::get_laser_status() {
        return m_bconnect_laser_;
    }
    void connection_metadata::set_router_status( bool router_status ) {
        m_bconnect_router_ = router_status;
    }
    bool connection_metadata::get_router_status() {
        return m_bconnect_router_;
    }
    void connection_metadata::set_speed_status( float speed_status ) {
        m_speed_status_ = speed_status;
    }
    float connection_metadata::get_speed_status() {
        return m_speed_status_;
    }
    void connection_metadata::set_emergencystopstatus_status( bool emergencystopstatus ) {
        m_emergencystopstatus_ = emergencystopstatus;
    }
    bool connection_metadata::get_emergencystopstatus_status() {
        return m_emergencystopstatus_;
    }
    void connection_metadata::set_greet_status_code(int status_code) {
        m_greet_status_code_ = status_code;
    }
    int connection_metadata::get_greet_status_code() {
        return m_greet_status_code_;
    }
    void connection_metadata::set_error_status_code(int status_code) {
        m_error_status_code_ = status_code;
    }
    int connection_metadata::get_error_status_code() {
        return m_error_status_code_;
    }
    /////////////////////////////////////////////////////////////////////////////
    ///////////// websocket_endpoint class //////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////
    websocket_endpoint::websocket_endpoint () {
        m_endpoint.clear_access_channels(websocketpp::log::alevel::all);
        m_endpoint.clear_error_channels(websocketpp::log::elevel::all);
        m_endpoint.init_asio();
        m_endpoint.start_perpetual();
        m_thread = websocketpp::lib::make_shared<websocketpp::lib::thread>(&client::run, &m_endpoint);
    }
    websocket_endpoint::~websocket_endpoint() {
        m_endpoint.stop_perpetual();
        for (con_list::const_iterator it = m_connection_list.begin(); it != m_connection_list.end(); ++it) {
            if(STATUS_OPEN != it->second->get_status() ) {
                YSOS_LOG_DEBUG_CUSTOM(g_logger, "The Connect Status is not OPEN!The Status is " <<(it->second->get_status()).c_str());
                continue;
            }
            YSOS_LOG_DEBUG_CUSTOM(g_logger, "Closing connection " << it->second->get_id());
            websocketpp::lib::error_code errcode;
            m_endpoint.close(it->second->get_hdl(), websocketpp::close::status::going_away, "", errcode);
            if (errcode) {
                YSOS_LOG_ERROR_CUSTOM(g_logger, "Error closing connection "<< it->second->get_id() << ":" << errcode.message());
            }
        }
        m_thread->join();
    }
    int websocket_endpoint::connect(int new_id,std::string const &uri,void *param_ptr) {
        websocketpp::lib::error_code errcode;
        client::connection_ptr conptr = m_endpoint.get_connection(uri, errcode);
        if(errcode || NULL == conptr) {
            YSOS_LOG_DEBUG_CUSTOM(g_logger, "Connect initialization error: " << errcode.message());
            return -1;
        }
        connection_metadata::ptr metadata_ptr = websocketpp::lib::make_shared<connection_metadata>(new_id, conptr->get_handle(), uri,param_ptr);
        m_connection_list[new_id] = metadata_ptr;
        if(WEBSOCKET_DEVICE_ID == new_id) {
            conptr->set_open_handler(websocketpp::lib::bind(
                                         &connection_metadata::on_open,
                                         metadata_ptr,
                                         &m_endpoint,
                                         websocketpp::lib::placeholders::_1
                                     ));
            conptr->set_fail_handler(websocketpp::lib::bind(
                                         &connection_metadata::on_fail,
                                         metadata_ptr,
                                         &m_endpoint,
                                         websocketpp::lib::placeholders::_1
                                     ));
            conptr->set_close_handler(websocketpp::lib::bind(
                                          &connection_metadata::on_close,
                                          metadata_ptr,
                                          &m_endpoint,
                                          websocketpp::lib::placeholders::_1
                                      ));
            conptr->set_message_handler(websocketpp::lib::bind(
                                            &connection_metadata::on_message_for_device,
                                            metadata_ptr,
                                            websocketpp::lib::placeholders::_1,
                                            websocketpp::lib::placeholders::_2
                                        ));
        }
        else if(WEBSOCKET_NAVIGATION_ID == new_id) {
            conptr->set_open_handler(websocketpp::lib::bind(
                                         &connection_metadata::on_open,
                                         metadata_ptr,
                                         &m_endpoint,
                                         websocketpp::lib::placeholders::_1
                                     ));
            conptr->set_fail_handler(websocketpp::lib::bind(
                                         &connection_metadata::on_fail,
                                         metadata_ptr,
                                         &m_endpoint,
                                         websocketpp::lib::placeholders::_1
                                     ));
            conptr->set_close_handler(websocketpp::lib::bind(
                                          &connection_metadata::on_close,
                                          metadata_ptr,
                                          &m_endpoint,
                                          websocketpp::lib::placeholders::_1
                                      ));
            conptr->set_message_handler(websocketpp::lib::bind(
                                            &connection_metadata::on_message_for_navigation,
                                            metadata_ptr,
                                            websocketpp::lib::placeholders::_1,
                                            websocketpp::lib::placeholders::_2
                                        ));
        }
        else if(WEBSOCKET_SYSHEALTH_ID == new_id) {
            conptr->set_open_handler(websocketpp::lib::bind(
                                         &connection_metadata::on_open,
                                         metadata_ptr,
                                         &m_endpoint,
                                         websocketpp::lib::placeholders::_1
                                     ));
            conptr->set_fail_handler(websocketpp::lib::bind(
                                         &connection_metadata::on_fail,
                                         metadata_ptr,
                                         &m_endpoint,
                                         websocketpp::lib::placeholders::_1
                                     ));
            conptr->set_close_handler(websocketpp::lib::bind(
                                          &connection_metadata::on_close,
                                          metadata_ptr,
                                          &m_endpoint,
                                          websocketpp::lib::placeholders::_1
                                      ));
            conptr->set_message_handler(websocketpp::lib::bind(
                                            &connection_metadata::on_message_for_health,
                                            metadata_ptr,
                                            websocketpp::lib::placeholders::_1,
                                            websocketpp::lib::placeholders::_2
                                        ));
        }
        else if(WEBSOCKET_GREET_ID == new_id) {
            conptr->set_open_handler(websocketpp::lib::bind(
                                         &connection_metadata::on_open,
                                         metadata_ptr,
                                         &m_endpoint,
                                         websocketpp::lib::placeholders::_1
                                     ));
            conptr->set_fail_handler(websocketpp::lib::bind(
                                         &connection_metadata::on_fail,
                                         metadata_ptr,
                                         &m_endpoint,
                                         websocketpp::lib::placeholders::_1
                                     ));
            conptr->set_close_handler(websocketpp::lib::bind(
                                          &connection_metadata::on_close,
                                          metadata_ptr,
                                          &m_endpoint,
                                          websocketpp::lib::placeholders::_1
                                      ));
            conptr->set_message_handler(websocketpp::lib::bind(
                                            &connection_metadata::on_message_for_greet,
                                            metadata_ptr,
                                            websocketpp::lib::placeholders::_1,
                                            websocketpp::lib::placeholders::_2
                                        ));
        }
        m_endpoint.connect(conptr);
        return new_id;
    }
    void websocket_endpoint::close(int id, websocketpp::close::status::value code, std::string reason) {
        websocketpp::lib::error_code errcode;
        con_list::iterator metadata_it = m_connection_list.find(id);
        if (metadata_it == m_connection_list.end()) {
            YSOS_LOG_WARN_CUSTOM(g_logger, "No connection found with id " << id);
            return;
        }
        m_endpoint.close(metadata_it->second->get_hdl(), code, reason, errcode);
        if (errcode) {
            YSOS_LOG_ERROR_CUSTOM(g_logger, "Close connect error: " << errcode.message());
        }
    }
    void websocket_endpoint::send(int id, std::string message) {
        websocketpp::lib::error_code errcode;
        con_list::iterator metadata_it = m_connection_list.find(id);
        if (metadata_it == m_connection_list.end()) {
            YSOS_LOG_WARN_CUSTOM(g_logger, "No connection found with id " << id);
            return;
        }
        m_endpoint.send(metadata_it->second->get_hdl(), message, websocketpp::frame::opcode::text, errcode);
        if (errcode) {
            YSOS_LOG_ERROR_CUSTOM(g_logger, "Send message error: " << errcode.message());
            return;
        }
    }
    connection_metadata::ptr websocket_endpoint::get_metadata(int id) const {
        con_list::const_iterator metadata_it = m_connection_list.find(id);
        if (metadata_it == m_connection_list.end()) {
            return connection_metadata::ptr();
        }
        else {
            return metadata_it->second;
        }
    }