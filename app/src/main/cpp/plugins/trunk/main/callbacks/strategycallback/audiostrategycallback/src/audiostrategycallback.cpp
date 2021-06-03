/**
 *@file audiostrategycallback.cpp
 *@brief audio strategy call back
 *@version 1.0
 *@author l.j..
 *@date Created on: 2016-10-20 15:16:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
/// self headers //  NOLINT
#include "../include/audiostrategycallback.h"

/// boost headers
#include <boost/filesystem.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

/// ysos headers
#include "../../../../../../core/trunk/main/public/include/os_hal_package/bufferutility.h"
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/manager_declare.h"
#include "../../../../../../core/trunk/main/public/include/sys_interface_package/protocolinterface.h"

/// private headers
#include "../../../public/include/common/commonenumdefine.h"

namespace ysos {
    DECLARE_PLUGIN_REGISTER_INTERFACE(AudioStrategyCallback, CallbackInterface);  ///<  插件的入口，一定要加上 //  NOLINT
    AudioStrategyCallback::AudioStrategyCallback(const std::string &strClassName):BaseStrategyCallbackImpl(strClassName) {
        logger_ = GetUtility()->GetLogger("ysos.audiostrategy");
        menu_code_.clear();
        menu_code_["001600"] = "OnService@AccountsService@InsertBankCard";
        menu_code_["001601"] = "OnService@AccountsService@EnterPassword";
        menu_code_["001602"] = "OnService@AccountsService@DisplayResult";
        menu_code_["002004"] = "OnService@InformationCollectionService@InformationCollectionServiceIndex";
        menu_code_["004001"] = "OnService@WeatherService@WeatherServiceIndex";
        menu_code_["005001"] = "OnService@MusicService@MusicServiceIndex";
        menu_code_["009001"] = "OnService@TrailerService@TrailerServiceIndex";
        menu_code_["013000"] = "OnService@ExchangeRateService@ExchangeRateServiceIndex";
        menu_code_["028000"] = "OnService@SettingsService@SettingsServiceIndex";
        menu_code_["006000"] = "OnService@FinanceService@FinanceServiceIndex";
        menu_code_["006006"] = "OnService@FinanceService@FinanceServiceDetail";
        menu_code_["000100"] = "OnService@RechargeService@SelectAmount";
        menu_code_["000101"] = "OnService@RechargeService@EnterPhoneNumber";
        menu_code_["000102"] = "OnService@RechargeService@SelectPaymentMethod";
        menu_code_["000103"] = "OnService@RechargeService@InsertBankCard";
        menu_code_["000104"] = "OnService@RechargeService@EnterPassword";
        menu_code_["000105"] = "OnService@RechargeService@DisplayResult";
        menu_code_["000400"] = "OnService@RepayService@InsertCreditCard";
        menu_code_["000406"] = "OnService@RepayService@EnterCreditCardNumber";
        menu_code_["000409"] = "OnService@RepayService@RepaymentInstructions";
        menu_code_["000401"] = "OnService@RepayService@CheckCreditCardNumber";
        menu_code_["000402"] = "OnService@RepayService@EnterRepaymentAmount";
        menu_code_["000407"] = "OnService@RepayService@CheckRepaymentInfo";
        menu_code_["000408"] = "OnService@RepayService@CheckMobilePhoneNumber";
        menu_code_["000403"] = "OnService@RepayService@InsertBankCard";
        menu_code_["000404"] = "OnService@RepayService@EnterPassword";
        menu_code_["000405"] = "OnService@RepayService@DisplayResult";
        menu_code_["017000"] = "OnService@ReservationService@ReservationServiceIndex";
        menu_code_["017001"] = "OnService@ReservationService@ReservationServiceDebitInfo";
        menu_code_["017002"] = "OnService@ReservationService@ReservationServiceOnlineInfo";
        menu_code_["017003"] = "OnService@ReservationService@ReservationServicePhoneInfo";
        menu_code_["018000"] = "OnService@FoundService@FoundServiceIndex";
        menu_code_["001700"] = "OnService@PaymentService@PaymentServiceIndex";
        menu_code_["001701"] = "OnService@PaymentService@PaymentServiceXWaterInput";
        menu_code_["001711"] = "OnService@PaymentService@PaymentServiceXElectricInput";
        menu_code_["001721"] = "OnService@PaymentService@PaymentServiceXGasInput";
        menu_code_["001712"] = "OnService@PaymentService@PaymentServiceInformation";
        menu_code_["001713"] = "OnService@PaymentService@PaymentServicePayments";
        menu_code_["001714"] = "OnService@PaymentService@PaymentServiceQRcodes";
        menu_code_["001715"] = "OnService@PaymentService@PaymentServiceOK";
        YSOS_LOG_DEBUG("menu_code_.size = " << menu_code_.size());
        notify_event_.clear();
        notify_event_["accountskeeper"] = "account_manager_callback";
        notify_event_["report"] = "debit_card_loss_callback";
        notify_event_["transfer"] = "repay_callback";
        notify_event_["transferin"] = "repay_internal_callback";
        notify_event_["transferout"] = "repay_outer_callback";
        notify_event_["finance"] = "investment_callback";
        notify_event_["delibao"] = "treasure_callback";
        notify_event_["fundQuery"] = "fund_callback";
        notify_event_["credit"] = "credit_card_callback";
        notify_event_["repay"] = "credit_card_repay_callback";
        notify_event_["ledger"] = "credit_card_account_query_callback";
        notify_event_["check"] = "credit_card_detail_query_callback";
        notify_event_["accounts"] = "account_inquiry_callback";
        notify_event_["remainder"] = "account_balance_query_callback";
        notify_event_["records"] = "account_detail_query_callback";
        notify_event_["homepage"] = "service_index_callback";
        notify_event_["exit"] = "exit_callback";
        notify_event_["currentdeposit"] = "current_deposit_callback";
        notify_event_["fixedeposit"] = "fixed_deposit_callback";
        notify_event_["print"] = "print_callback";
        notify_event_["over"] = "over_callback";
        notify_event_["currentflow"] = "current_flow_callback";
        notify_event_["historicalflow"] = "historical_flow_callback";
        notify_event_["query"] = "query_callback";
        notify_event_["back"] = "back_callback";
        notify_event_["next"] = "next_callback";
        YSOS_LOG_DEBUG("notify_event_.size = " << notify_event_.size());
    }

    AudioStrategyCallback::~AudioStrategyCallback(void) {
    }

    int AudioStrategyCallback::HandleMessage(const std::string &event_name, BufferInterfacePtr input_buffer, CallbackInterface *context) {
        int n_return = YSOS_ERROR_FAILED;
        lock_.Lock();
        do {
            data_ptr_ = GetDataInterfaceManager()->GetData(GLOBAL_DATA_KEY);
            if (!data_ptr_) {
                YSOS_LOG_DEBUG("get data_ptr failed");
                break;
            }
#ifdef _DEBUGVIEW
            ::OutputDebugString((std::string("ysos.audiostrategy , aevent_name:") + event_name).c_str());
#endif
            if ("artificial_auxiliary_event" == event_name) {
                if (NULL == input_buffer) {
                    break;
                }
                UINT8 *buffer;
                UINT32 buffer_size;
                input_buffer->GetBufferAndLength(&buffer, &buffer_size);
                std::string input_data((char *)buffer);
                YSOS_LOG_DEBUG("input_data = " << input_data);
                if (input_data.empty()) {
                    break;
                }
#ifdef _DEBUGVIEW
                ::OutputDebugString((std::string("ysos.audiostrategy , audiostrategycallback receive event artificial_auxiliary_event:") + input_data).c_str());
#endif
                YSOS_LOG_DEBUG(" audiostrategycallback receive event artificial_auxiliary_event:" << input_data);
                Json::Value json_input;
                Json::Value json_robot;
                Json::Reader json_reader;
                Json::FastWriter json_writer;
                json_reader.parse(input_data, json_input, true);
                std::string type = json_input["data"]["messageType"].asString();
                //YSOS_LOG_DEBUG("type = " << type);
                std::string content;
                if (type.empty()) {
                    break;
                }
                else if ("robotMove" == type) {
                    if (Json::stringValue == json_input["data"]["messageContent"].type()) {
                        content = json_input["data"]["messageContent"].asString();
                    }
                    else if (Json::intValue == json_input["data"]["messageContent"].type()) {
                        int id = json_input["data"]["messageContent"].asInt();
                        std::stringstream stream;
                        stream << id;
                        content = stream.str();
                    }
                    else {
                        YSOS_LOG_DEBUG("messageContent type = " << json_input["data"]["messageContent"].type());
                    }
                }
                else if ("nlp" == type) {
                    json_robot = json_input["data"]["messageContent"]["robot"];
                }
                else if ("testWalkToAddPos" == type || "postAllContent" == type || "repeaterSubproblem" == type) {
                    Json::Value json_data = json_input["data"]["messageContent"];
                    content = json_writer.write(json_data);
                    content = GetUtility()->ReplaceAllDistinct ( content, "\\r\\n", "" );
                }
                else {
                    content = json_input["data"]["messageContent"].asString();
                }
                //YSOS_LOG_DEBUG("type = " << type << "; content = " << content);
                std::string ioctl_id = "";
                if ("voice_broadcast" == type) { ///< 语音播报
                    n_return = RequestService("default@TtsExtModule", "8139", "", content, context);
                    json_robot["answer_best"] = content;
                    json_robot["service"] = "chat";
                    std::string menu_result = json_writer.write(json_robot);
                    menu_result = GetUtility()->ReplaceAllDistinct ( menu_result, "\\r\\n", "" );
                    YSOS_LOG_DEBUG("menu_result = " << menu_result);
                    n_return = DoEventNotifyService("Answer_Type_Event", "Answer_Type_Event", menu_result, context);
                    if (YSOS_ERROR_SUCCESS != n_return) {
                        YSOS_LOG_DEBUG("execute EventNotify failed, n_return = " << n_return);
                        n_return = YSOS_ERROR_FAILED;
                        break;
                    }
                }
                else if ("voice_analysis" == type) {   ///< 语义分析
                    //n_return = RequestService("default@AsrExtModule", "28001", "", content, context);
                    Json::Value json_data;
                    json_data["text"] = content;
                    content = json_writer.write(json_data);
                    content = GetUtility()->ReplaceAllDistinct ( content, "\\r\\n", "" );
                    YSOS_LOG_DEBUG("get voice_analysis to NlpExtModule, content = " << content);
                    n_return = RequestService("default@NlpExtModule", "28003", "", content, context);
                }
                else if ("ZASR" == type) {   ///< 识别控制
                    if ("0" == content) { ///< 关闭
                        ioctl_id = "26502";
                    }
                    else if ("1" == content) {   ///< 打开
                        ioctl_id = "26501";
                    }
                    n_return = RequestService("default@AudioCaptureExtModule", ioctl_id, "", "", context);
                }
                else if ("ZTTS" == type) {   ///< 播报控制
                }
                else if ("faceid" == type) {   ///< 表情切换
                    std::string answer = content + "#";
                    std::string service = "face";
                    n_return = EventNotify(answer, service, "face_emotion_callback", context);
                }
                else if ("menuid" == type) {   ///< 业务切换
                    //std::string state = "";
                    //std::map<std::string, std::string>::iterator itor = menu_code_.find(content);
                    //if (itor == menu_code_.end()) {
                    //  YSOS_LOG_DEBUG("no match state, content = " << content);
                    //  break;
                    //}
                    //state = itor->second;
                    //YSOS_LOG_DEBUG("state = " << state);
                    //n_return = DoSwitchNotifyService(state, context);
                    //if (YSOS_ERROR_SUCCESS != n_return) {
                    //  YSOS_LOG_DEBUG("menuid DoSwitchNotifyService error, n_return = " << n_return);
                    //  break;
                    //}
                    ///< 医疗组需求
                    json_robot["answer_best"] = "好的*" + content;
                    json_robot["service"] = "hisOpenApp";
                    std::string menu_result = json_writer.write(json_robot);
                    menu_result = GetUtility()->ReplaceAllDistinct ( menu_result, "\\r\\n", "" );
                    YSOS_LOG_DEBUG("menu_result = " << menu_result);
                    n_return = DoEventNotifyService("Answer_Type_Event", "Answer_Type_Event", menu_result, context);
                    if (YSOS_ERROR_SUCCESS != n_return) {
                        YSOS_LOG_DEBUG("execute EventNotify failed, n_return = " << n_return);
                        n_return = YSOS_ERROR_FAILED;
                        break;
                    }
                }
                else if ("appIndex" == type) {   ///< 返回首页
                    n_return = DoSwitchNotifyService("OnService", context);
                    if (YSOS_ERROR_SUCCESS != n_return) {
                        YSOS_LOG_DEBUG("menuid DoSwitchNotifyService error, n_return = " << n_return);
                        break;
                    }
                }
                else if ("ZXFM" == type) {   ///< 模式控制
                    if ("0" == content) { ///< 人工模式
                        ioctl_id = "26502";
                        data_ptr_->SetData("ZXFM", "2");
                        //n_return = DoEventNotifyService("artifical_info_event", "artifical_info_callback", "2", context);
                    }
                    else if ("1" == content) {   ///< 托管模式
                        ioctl_id = "26501";
                        data_ptr_->SetData("ZXFM", "1");
                        //n_return = DoEventNotifyService("artifical_info_event", "artifical_info_callback", "1", context);
                    }
                    //n_return = RequestService("default@AudioCaptureExtModule", ioctl_id, "", "", context);
                }
                else if ("robotMove" == type) {   ///< 移动
                    std::string ioctl_value = "";
                    if ("2" == content || "4" == content || "6" == content) { ///< 停止
                        ioctl_id = "20036";
                        n_return = RequestService("default@ChassisModule1", ioctl_id, "", ioctl_value, context);
                    }
                    else if ("0" == content) {   ///< 静止模式
                        n_return = DoSwitchNotifyService("WaitingService@TargetSearching@StaticSearching", context);
                        if (YSOS_ERROR_SUCCESS != n_return) {
                            YSOS_LOG_DEBUG("menuid DoSwitchNotifyService error, n_return = " << n_return);
                            break;
                        }
                    }
                    else if ("1" == content) {   ///< 向前走
                        ioctl_id = "20031";
                        ioctl_value = "{\"direction\":\"+\",\"continuous\":true}";
                        n_return = RequestService("default@ChassisModule1", ioctl_id, "", ioctl_value, context);
                    }
                    else if ("3" == content) {   ///< 向左转
                        ioctl_id = "20031";
                        ioctl_value = "{\"rotate\":\"+\",\"continuous\":true}";
                        n_return = RequestService("default@ChassisModule1", ioctl_id, "", ioctl_value, context);
                    }
                    else if ("5" == content) {   ///< 向右转
                        ioctl_id = "20031";
                        ioctl_value = "{\"rotate\":\"-\",\"continuous\":true}";
                        n_return = RequestService("default@ChassisModule1", ioctl_id, "", ioctl_value, context);
                    }
                    else if ("7" == content) {   ///< 定点巡航
                        //ioctl_id = "28001";
                        //ioctl_value = "定点巡航";
                        //n_return = RequestService("default@AsrExtModule", ioctl_id, "", ioctl_value, context);
                        n_return = DoSwitchNotifyService("WaitingService@TargetSearching@SpotSearching", context);
                        if (YSOS_ERROR_SUCCESS != n_return) {
                            YSOS_LOG_DEBUG("menuid DoSwitchNotifyService error, n_return = " << n_return);
                            break;
                        }
                    }
                    else if ("8" == content) {   ///< 随机行走
                        ioctl_id = "28001";
                        ioctl_value = "随机行走";
                        n_return = RequestService("default@AsrExtModule", ioctl_id, "", ioctl_value, context);
                    }
                    else if ("11" == content) {   ///< 充电
                        //ioctl_id = "28001";
                        //ioctl_value = "自助充电";
                        //n_return = RequestService("default@AsrExtModule", ioctl_id, "", ioctl_value, context);
                        n_return = DoSwitchNotifyService("SuspendService@PowerManager@PrepareCharging", context);
                        if (YSOS_ERROR_SUCCESS != n_return) {
                            YSOS_LOG_DEBUG("menuid DoSwitchNotifyService error, n_return = " << n_return);
                            break;
                        }
                    }
                }
                else if ("robotMoveByVideoClick" == type) {   ///< 点击行走
                    n_return = RequestService("default@ChassisModule1", "20045", "", content, context);
                }
                else if ("testWalkToAddPos" == type) {   ///< 地图移动
                    n_return = RequestService("default@ChassisModule1", "20042", "", content, context);
                }
                else if ("nlp" == type) {   ///< 快速检索
                    content = json_robot["speakContent"].asString();
                    if (!content.empty()) {
                        n_return = RequestService("default@TtsExtModule", "8139", "", content, context);
                    }
                    Json::Value json_command = json_robot["commands"];
                    int command_num = json_command.size();
                    if (0 != command_num) {
                        for (int i=0; i<command_num; i++) {
                            std::string command = json_command[i].asString();
                            if ("chat" == command) {
                                continue;
                            }
                            std::string answer;
                            std::string::size_type pos = command.find("faceid_");
                            if (std::string::npos != pos) {
                                std::string face_id = command.substr(pos+7);
                                answer = face_id + "#";
                                std::string service = "face";
                                EventNotify(answer, service, "face_emotion_callback", context);
                                continue;
                            } ///< face_id
                            YSOS_LOG_DEBUG("command = " << command);
                            std::map<std::string, std::string>::iterator it = notify_event_.find(command);
                            if (it != notify_event_.end()) {
                                std::string event_name = it->second;
                                YSOS_LOG_DEBUG("event_name = " << event_name);
                                EventNotify(answer, command, event_name, context);
                            }
                            //pos = command.find("menuid_");
                            //if (std::string::npos != pos) {
                            //  std::string menu_id = command.substr(pos+7);
                            //  YSOS_LOG_DEBUG("menu_id = " << menu_id);
                            //  std::string state = "";
                            //  std::map<std::string, std::string>::iterator itor = menu_code_.find(menu_id);
                            //  if (itor == menu_code_.end()) {
                            //    YSOS_LOG_DEBUG("no match state, menu_id = " << menu_id);
                            //    continue;
                            //  }
                            //  state = itor->second;
                            //  YSOS_LOG_DEBUG("state = " << state);
                            //  n_return = DoSwitchNotifyService(state, context);
                            //  if (YSOS_ERROR_SUCCESS != n_return) {
                            //    YSOS_LOG_DEBUG("menuid DoSwitchNotifyService error, n_return = " << n_return);
                            //    continue;
                            //  }
                            //} ///< menu_id
                        } ///< for
                    }
                }
                else if ("getYsosToken" == type) {
                    int pos = 0;
                    std::string cur_state;
                    data_ptr_->GetData("machine_state", cur_state);
                    YSOS_LOG_DEBUG("cur_state = " << cur_state);
                    Json::Value state_info;
                    state_info["robotState"] = cur_state;
                    state_info["stateLevel"] = 0;
                    Json::Value state_set;
                    state_set[pos] = state_info;
                    Json::Value robot_state_set;
                    robot_state_set["robotStateSet"] = state_set;
                    std::string token_id;
                    data_ptr_->GetData("token_id", token_id);
                    YSOS_LOG_DEBUG("token_id = " << token_id);
                    robot_state_set["newToken"] = token_id;
                    robot_state_set["lifeTime"] = 570000;
                    robot_state_set["platForm"] = "1";
                    Json::Value json_message;
                    json_message["messageType"] = "TokenAndScene";
                    json_message["messageContent"] = robot_state_set;
                    json_robot["data"] = json_message;
                    std::string send_data = json_writer.write(json_robot);
                    send_data = GetUtility()->ReplaceAllDistinct ( send_data, "\\r\\n", "" );
                    YSOS_LOG_DEBUG("send_data = " << send_data);
                    ioctl_id = "29103";
                    n_return = RequestService("default@ArtificialAuxiliaryModule", ioctl_id, "", send_data, context);
                }
                else if ("postAllContent" == type) {
                    Json::Value json_data = json_input["data"]["messageContent"];
                    Json::Value scene_set = json_data["sceneSet"];
                    int zero = 0;
                    scene_set = scene_set[zero];
                    std::string service = scene_set["commands"][zero].asString();
                    if (service.empty()) {
                        service = "chat";
                    }
                    std::string question = scene_set["question"].asString();
                    YSOS_LOG_DEBUG("question = " << question);
                    std::string speak_content = scene_set["speakContent"].asString();
                    YSOS_LOG_DEBUG("speak_content = " << speak_content);
                    Json::Value nlp_value;
                    nlp_value["rc"] = "0";
                    nlp_value["rg"] = "0";
                    nlp_value["text"] = question;
                    nlp_value["service"] = service;
                    nlp_value["time_interval"] = 0;
                    nlp_value["showContent"] = scene_set["showContent"];
                    nlp_value["recommendQs"] = scene_set["recommendQs"];
                    nlp_value["parameterSet"] = scene_set["parameterSet"];
                    nlp_value["commands"] = scene_set["commands"];
                    nlp_value["answer_best"] = speak_content;
                    nlp_value["interrupt_type"] = 0;
                    std::string nlp_result = json_writer.write(nlp_value);
                    nlp_result = GetUtility()->ReplaceAllDistinct ( nlp_result, "\\r\\n", "" );
                    YSOS_LOG_DEBUG("nlp_result = " << nlp_result);
                    n_return = DoEventNotifyService("OnYSOSResult", "OnYSOSResult", nlp_result, context);
                    if (YSOS_ERROR_SUCCESS != n_return) {
                        YSOS_LOG_DEBUG("execute EventNotify failed, n_return = " << n_return);
                        n_return = YSOS_ERROR_FAILED;
                        break;
                    }
                    if (!speak_content.empty()) {
                        RequestService("default@TtsExtModule", "8139", "", speak_content, context);
                    }
                }
                else if ("repeaterSubproblem" == type) {
                    Json::Value json_data = json_input["data"]["messageContent"];
                    Json::Value scene_set = json_data["sceneSet"];
                    int zero = 0;
                    scene_set = scene_set[zero];
                    std::string service = scene_set["commands"][zero].asString();
                    if (service.empty()) {
                        service = "chat";
                    }
                    std::string question = scene_set["question"].asString();
                    YSOS_LOG_DEBUG("question = " << question);
                    std::string speak_content = scene_set["speakContent"].asString();
                    YSOS_LOG_DEBUG("speak_content = " << speak_content);
                    Json::Value nlp_value;
                    nlp_value["rc"] = "0";
                    nlp_value["rg"] = "0";
                    nlp_value["text"] = question;
                    nlp_value["service"] = service;
                    nlp_value["time_interval"] = 0;
                    nlp_value["showContent"] = scene_set["showContent"];
                    nlp_value["recommendQs"] = scene_set["recommendQs"];
                    nlp_value["parameterSet"] = scene_set["parameterSet"];
                    nlp_value["commands"] = scene_set["commands"];
                    nlp_value["answer_best"] = speak_content;
                    nlp_value["interrupt_type"] = 0;
                    std::string nlp_result = json_writer.write(nlp_value);
                    nlp_result = GetUtility()->ReplaceAllDistinct ( nlp_result, "\\r\\n", "" );
                    YSOS_LOG_DEBUG("nlp_result = " << nlp_result);
                    n_return = DoEventNotifyService("OnYSOSResult", "OnYSOSResult", nlp_result, context);
                    if (YSOS_ERROR_SUCCESS != n_return) {
                        YSOS_LOG_DEBUG("execute EventNotify failed, n_return = " << n_return);
                        n_return = YSOS_ERROR_FAILED;
                        break;
                    }
                    if (!speak_content.empty()) {
                        RequestService("default@TtsExtModule", "8139", "", speak_content, context);
                    }
                }
                else if ("checkTerraceVersion" == type) {
                } ///< repeaterSubproblem
            } ///< artificial_auxiliary_event
            else if ("artificial_auxiliary_send_event" == event_name) {
                UINT8 *buffer;
                UINT32 buffer_size;
                input_buffer->GetBufferAndLength(&buffer, &buffer_size);
                std::string input_data((char *)buffer);
                YSOS_LOG_INFO("artificial_auxiliary_send_event input_data = " << input_data);
                if (input_data.empty()) {
                    YSOS_LOG_ERROR("artificial_auxiliary_send_event input_data is null");
                    break;
                }
                RequestService("default@ArtificialAuxiliaryModule",GetUtility()->ConvertToString(CMD_ARTIFICIAL_AUXILIARY_PASS), "", input_data, context);
            }
            n_return = YSOS_ERROR_SUCCESS;
        } while (0);
        lock_.Unlock();
        return n_return;
    }

    int AudioStrategyCallback::Initialized(const std::string &key, const std::string &value) {
        int n_return = YSOS_ERROR_SUCCESS;
        YSOS_LOG_DEBUG("menu_code_.size = " << menu_code_.size());
        YSOS_LOG_DEBUG("menu_code_.size = " << menu_code_.size());
        return n_return;
    }

    int AudioStrategyCallback::RequestService(std::string service_name, std::string id, std::string type, std::string json_value, CallbackInterface *context) {
        YSOS_LOG_DEBUG("RequestService execute");
        int n_return = YSOS_ERROR_SUCCESS;
        YSOS_LOG_DEBUG("RequestService service_name = " << service_name);
        YSOS_LOG_DEBUG("RequestService id = " << id);
        YSOS_LOG_DEBUG("RequestService text = " << type);
        YSOS_LOG_DEBUG("RequestService json_value = " << json_value);
#ifdef _DEBUGVIEW
        ::OutputDebugString((std::string("ysos.audiostrategy , RequestService, service_name:") + service_name + " id:" + id + "  type:" + type + " json_value:" + json_value).c_str());
#endif
        do {
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
                n_return = YSOS_ERROR_FAILED;
                break;
            }
            YSOS_LOG_DEBUG("RequestService done and success");
        } while (0);
        return n_return;
    }

    int AudioStrategyCallback::EventNotify(const std::string &answer, const std::string &service, const std::string &event_name, CallbackInterface *context) {
        YSOS_LOG_DEBUG("EventNotify execute");
        int n_return = YSOS_ERROR_SUCCESS;
        do {
            Json::Value json_value;
            json_value["rc"] = "0";
            json_value["rg"] = "0";
            json_value["answer_best"] = "";
            json_value["answer_old"] = answer;
            json_value["service"] = service;
            json_value["text"] = "";
            json_value["interrupt_type"] = 0;
            json_value["time_interval"] = 10;
            json_value["eventName"] = "SwitchNotify";
            Json::FastWriter json_writer;
            std::string result_value = json_writer.write(json_value);
            result_value = GetUtility()->ReplaceAllDistinct ( result_value, "\\r\\n", "" );
            std::string notify_event_name = event_name;
            std::string notify_callback_name= event_name;
            if (notify_event_name == "face_emotion_callback") {
                notify_event_name = "face_emotion_event";
            }
            YSOS_LOG_DEBUG("result_value = " << result_value);
            YSOS_LOG_DEBUG("notify_event_name: " << notify_event_name << " notify_callback_name: " << notify_callback_name);
            n_return = DoEventNotifyService(notify_event_name, notify_callback_name, result_value, context);
#ifdef _DEBUGVIEW
            ::OutputDebugString((std::string("ysos.audiostrategy , EventNotify:") + result_value).c_str());
#endif
            if (YSOS_ERROR_SUCCESS != n_return) {
                YSOS_LOG_DEBUG("execute EventNotify failed, n_return = " << n_return);
                n_return = YSOS_ERROR_FAILED;
                break;
            }
            YSOS_LOG_DEBUG("EventNotify done and success");
        } while (0);
        return n_return;
    }

}