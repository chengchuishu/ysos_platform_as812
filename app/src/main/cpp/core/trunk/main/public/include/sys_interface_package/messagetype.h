/**
 *@file MessageType.h
 *@brief Definition of MessageType
 *@version 0.1
 *@author XuLanyue
 *@date Created on: 2016-04-26 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef CHP_MESSAGE_TYPE_H          //NOLINT
#define CHP_MESSAGE_TYPE_H          //NOLINT


//  请注意: 千位指示了优先级级别//NOLINT

//  system message
#define YSOS_SYSTEM_MESSAGE_BASE                    10000
#define YSOS_SYSTEM_MESSAGE_POWER_OFF               (YSOS_SYSTEM_MESSAGE_BASE + 2000)
#define YSOS_SYSTEM_MESSAGE_RESTART                 (YSOS_SYSTEM_MESSAGE_BASE + 2001)
#define YSOS_SYSTEM_MESSAGE_EXCEPTION               (YSOS_SYSTEM_MESSAGE_BASE + 2002)

//  application message
#define YSOS_APP_MESSAGE_BASE                       20000
#define YSOS_APP_MESSAGE_TRANSACTION_CANCEL         (YSOS_APP_MESSAGE_BASE + 3000)
#define YSOS_APP_MESSAGE_TRANSACTION_STOP           (YSOS_APP_MESSAGE_BASE + 3001)
#define YSOS_APP_MESSAGE_TRANSACTION_PAUSE          (YSOS_APP_MESSAGE_BASE + 3002)

//  agent message
#define YSOS_AGENT_MESSAGE_BASE                     30000
#define YSOS_AGENT_MESSAGE_HEART_BEAT_REQ           31000     //  客户端心跳请求, C->S
#define YSOS_AGENT_MESSAGE_HEART_BEAT_RSP           31001     //  对客户端心跳请求的响应, S->C
#define YSOS_AGENT_MESSAGE_GET_SERVICE_LIST_REQ     31002     //  客户端枚举服务请求, C->S
#define YSOS_AGENT_MESSAGE_GET_SERVICE_LIST_RSP     31003     //  对客户端枚举服务请求的响应, S->C
#define YSOS_AGENT_MESSAGE_WRITE_LOG_REQ            31004     //  客户端写日志请求, C->S
#define YSOS_AGENT_MESSAGE_WRITE_LOG_RSP            31005     //  对客户端写日志请求的响应, S->C
#define YSOS_AGENT_MESSAGE_REG_EVENT_REQ            31006     //  客户端注册事件请求, C->S
#define YSOS_AGENT_MESSAGE_REG_EVENT_RSP            31007     //  对客户端注册事件请求的响应, S->C
#define YSOS_AGENT_MESSAGE_UNREG_EVENT_REQ          31008     //  客户端反注册事件请求, C->S
#define YSOS_AGENT_MESSAGE_UNREG_EVENT_RSP          31009     //  对客户端反注册事件请求的响应, S->C
#define YSOS_AGENT_MESSAGE_IOCTL_REQ                31010     //  客户端IOCtl服务请求, C->S
#define YSOS_AGENT_MESSAGE_IOCTL_RSP                31011     //  对客户端IOCtl服务请求的响应, S->C
#define YSOS_AGENT_MESSAGE_LOAD_CONFIG_REQ          31012     //  客户端装载配置请求, C->S
#define YSOS_AGENT_MESSAGE_LOAD_CONFIG_RSP          31013     //  对客户端装载配置请求的响应, S->C
#define YSOS_AGENT_MESSAGE_GET_SERVICE_STATE_REQ    31014     //  客户端获取服务状态请求, C->S
#define YSOS_AGENT_MESSAGE_GET_SERVICE_STATE_RSP    31015     //  对客户端获取服务状态请求的响应, S->C
#define YSOS_AGENT_MESSAGE_SWITCH_REQ               31016     //  对页面切换服务的请求, C->S
#define YSOS_AGENT_MESSAGE_SWITCH_RSP               31017     //  对页面切换服务的响应, S->C
#define YSOS_AGENT_MESSAGE_INIT_REQ                 31018     //  客户端初始化请求, C->S
#define YSOS_AGENT_MESSAGE_INIT_RSP                 31019     //  对客户端初始化请求的响应, S->C
#define YSOS_AGENT_MESSAGE_C_WAVE_FAREWELL_REQ      31020     //  客户端挥手请求, C->S
#define YSOS_AGENT_MESSAGE_C_WAVE_FAREWELL_RSP      31021     //  对客户端挥手请求的响应, S->C
#define YSOS_AGENT_MESSAGE_S_WAVE_FAREWELL_REQ      31022     //  服务端挥手请求, S->C
#define YSOS_AGENT_MESSAGE_S_WAVE_FAREWELL_RSP      31023     //  对服务端挥手请求的响应, C->S
#define YSOS_AGENT_MESSAGE_SWITCH_NOTIFY_RSQ        31024     //  对页面切换通知服务的请求, C->S
#define YSOS_AGENT_MESSAGE_SWITCH_NOTIFY_RSP        31025     //  对页面切换通知服务的响应, S->C
#define YSOS_AGENT_MESSAGE_READY_RSQ                31026     //  对页面准备完毕服务的请求, C->S
#define YSOS_AGENT_MESSAGE_READY_RSP                31027     //  对页面准备完毕服务的响应, S->C
#define YSOS_AGENT_MESSAGE_EVENT_NOTIFY_RSQ         31028     //  事件通知服务的请求, C->S
#define YSOS_AGENT_MESSAGE_EVENT_NOTIFY_RSP         31029     //  事件通知服务的响应, S->C
#define YSOS_AGENT_MESSAGE_GET_DATA_RSQ             31030     //  获取数据服务的请求, C->S
#define YSOS_AGENT_MESSAGE_GET_DATA_RSP             31031     //  获取数据服务的响应, S->C
#define YSOS_AGENT_MESSAGE_SET_DATA_RSQ             31032     //  设置数据服务的请求, C->S
#define YSOS_AGENT_MESSAGE_SET_DATA_RSP             31033     //  设置数据服务的响应, S->C
#define YSOS_AGENT_MESSAGE_CUSTOM_EVENT_RSQ         31034     //  设置自定义事件服务的请求, C->S
#define YSOS_AGENT_MESSAGE_CUSTOM_EVENT_RSP         31035     //  设置自定义事件服务的响应, S->C
#define YSOS_AGENT_MESSAGE_UNINIT_RSQ               31036     //  客户端注销请求, C->S
#define YSOS_AGENT_MESSAGE_UNINIT_RSP               31037     //  对客户端注销请求的响应, S->C
/// 北京赛特斯专属
#define YSOS_AGENT_MESSAGE_INIT_CONNECTION_REQ      31038     //  客户端初始化请求, C->S
#define YSOS_AGENT_MESSAGE_INIT_CONNECTION_RSP      31039     //  对客户端初始化请求的响应, S->C
#define YSOS_AGENT_MESSAGE_UNINIT_CONNECTION_REQ    31040     //  客户端注销请求, C->S
#define YSOS_AGENT_MESSAGE_UNINIT_CONNECTION_RSP    31041     //  对客户端注销请求的响应, S->C

#define YSOS_AGENT_SERVICE_NAME_HEART_BEAT           "Heartbeat"
#define YSOS_AGENT_SERVICE_NAME_SERVICE_LIST         "GetServiceList"
#define YSOS_AGENT_SERVICE_NAME_WRITE_LOG            "WriteLog"
#define YSOS_AGENT_SERVICE_NAME_REG_EVENT            "RegServiceEvent"
#define YSOS_AGENT_SERVICE_NAME_UNREG_EVENT          "UnregServiceEvent"
#define YSOS_AGENT_SERVICE_NAME_IOCTL                "IOCtl"
#define YSOS_AGENT_SERVICE_NAME_LOAD_CONFIG          "LoadConfig"
#define YSOS_AGENT_SERVICE_NAME_GET_SERVICE_STATE    "GetServiceState"
#define YSOS_AGENT_SERVICE_NAME_SWITCH               "Switch"
#define YSOS_AGENT_SERVICE_NAME_INIT                 "Init"
#define YSOS_AGENT_SERVICE_NAME_SWITCH_NOTIFY        "SwitchNotify"
#define YSOS_AGENT_SERVICE_NAME_READY                "Ready"
#define YSOS_AGENT_SERVICE_NAME_EVENT_NOTIFY         "EventNotify"
#define YSOS_AGENT_SERVICE_NAME_GET_DATA             "GetData"
#define YSOS_AGENT_SERVICE_NAME_SET_DATA             "SetData"
#define YSOS_AGENT_SERVICE_NAME_CUSTOM_EVENT         "CustomEvent"
#define YSOS_AGENT_SERVICE_NAME_UNINIT               "Uninit"
/// 北京赛特斯专属
#define YSOS_AGENT_SERVICE_NAME_INIT_CONNECTION      "initConnection"
#define YSOS_AGENT_SERVICE_NAME_UNINIT_CONNECTION    "uninitConnection"

#endif  // CHP_MESSAGE_TYPE_H       //NOLINT
