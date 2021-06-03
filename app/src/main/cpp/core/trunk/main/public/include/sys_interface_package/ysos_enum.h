/**
 *@file MessageType.h
 *@brief Definition of MessageType
 *@version 0.1
 *@author XuLanyue
 *@date Created on: 2016-04-26 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef SIP_YSOS_ENUM_H_          //NOLINT
#define SIP_YSOS_ENUM_H_          //NOLINT

namespace ysos {

/**
 *@brief used for get/set property
 */
enum YSOS_Platform_Property {
  /**
   * BaseInterface Property
  */
  PROP_NAME                                 =0,    ///< 名字
  PROP_TYPE                                 =1,    ///< 类型
  PROP_UUID                                 =2,    ///< 唯一标识符
  PROP_VERSION                              =3,  ///<  获取、设置模块的版本号 //  NOLINT

  PROP_MODULE                               =4,  ///<  module_interface  //  NOLINT
  PROP_CALLBACK                             =5, ///<  callback_interface  //  NOLINT
  PROP_DRIVER                               =6,   ///<  driver_interface   //  NOLINT
  PROP_STRATEGY                             =7, ///<  strategy_interface  //  NOLINT
  PROP_AGENT                                =8,  ///<  agent_interface  //  NOLINT
  PROP_PROTOCOL                             =9,  ///<  protocol_interface  //  NOLINT
  PROP_MODULE_LINK                          =10,  ///<  module_link_interface  //  NOLINT
  PROP_CONNECTION                           =11,   ///<  connection_interface   // NOLINT

  PROP_Base_Interface_end                   =50, /// =PROP_NAME+50,   ///<  50 //  NOLINT

  /**
   * ControlInterface Property
  */

  PROP_Control_Interface_End                =100, ///PROP_Base_Interface_end+50,  ///<  100 //  NOLINT

  /**
   * DriverInterface Property
  */

  PROP_Driver_Interface_End                 =150, ///=PROP_Control_Interface_End+50,  ///<  150 //  NOLINT

  /**
   * ModuleInterface Property
  */
  PROP_SOURCE                               =151, ///= PROP_Driver_Interface_End + 1,           ///< 0个prev, >=1个next  33  151//NOLINT
  PROP_TRANSFORM                            =152,       ///< 1个prev, >=1个next  //NOLINT
  PROP_DESTINATION                          =153,     ///< 1个prev, 0个next  //NOLINT
  PROP_REPEATER                             =154,       ///< >=1个prev, >=0个next;优先级最高, 禁止修改数据  //NOLINT

  PROP_IN_DATA_TYPES                        =155,    ///< 能力输入数据类型，可确定协议格式
  PROP_OUT_DATA_TYPES                       =156,   ///< 能力输出数据类型，可确定协议格式
  PROP_SELF_ALLOCATOR                       =157,  ///< 能力是否需要自己分配器
  PROP_BUFFER_PREFIX_LENGTH                 =158, ///< 能力分配缓冲需要的前缀长度
  PROP_BUFFER_LENGTH                        =159,        ///< 能力分配缓冲需要的最大长度
  PROP_BUFFER_DATA_REVISE                   =160,   ///< 如能力共享分配器，下级能力是否修改数据
  PROP_BUFFER_NUMBER                        =161,        ///< 能力需要最大缓冲个数
  PROP_MAX_CAPBILITY                        =162,        ///< 能力可同步处理的最大计算个数

  PROP_CLOSE                                =163,             ///<  45 //  NOLINT
  PROP_STOP                                 =164,
  PROP_PAUSE                                =165,
  PROP_RUN                                  =166,

  PROP_NEXTCALLBACKQUEUE                    =167,   ///<  当前Module的NextCallbackQueue //  NOLINT
  PROP_PREVCALLBACKQUEUE                    =168,   ///< 当前Module的PreCallbackQueue
  PROP_STATE_COUNT                          =169,     ///<  当前Module的mode_state_count值 //  NOLINT
  PROP_MODULE_TYPE                          =170,     ///<  设置Module的类型 //  NOLINT
  PROP_BUFFER_POOL                          =171,     ///<  获取Buffer Pool Ptr //  NOLINT
  PROP_NEXTCALLBACK_ERROR                   =172, ///<  记录出错的NextCallback所对应的Module地址 //  NOLINT
  PROP_BUFFER_PREFIX                        =173,      ///<  根据Module Data Info 设置Buffer的Prefix  //  NOLINT
  //PROP_MODULE_LINK ,        ///<  增加所属ModuleLink的逻辑名 //  NOLINT
  /// 记录当前Module对象在ModuleLink中的Level，该值是uint_32类型，它有如下几种表示方式：
  /// 1 当Module是普通Module时，它的高16位和低16位相同，都是Level的值，如：0x00010001
  /// 2 当Module是Repeater时，它的高16位，是前一个Module的Level，低16位是下一个Module的Level，如0x00020003
  /// 3 当Module是Repeater是ModuleLink中的最后一个模块时，它的低16位是0，如0x00040000
  PROP_MODULE_LINK_LEVEL                    =174,

  PROP_PREV_MODULE                          =175,

  PROP_THREADDATA                           =176,
  PROP_THREAD_NOTIFY                        =177,  ///<  通知事件，通知线程条件已满足，可以继续执行 //  NOLINT
  //PROP_THREADCALLBACK,

  PROP_MYLINK                               =178,

  PROP_STATUS_EVENT                         =179,    ///< Module的状态事件信息

  PROP_LOCK_MODULE                          =180,    ///< lock module, not to permit to run stop and so on
  PROP_UNLOCK_MODULE                        =181,    ///< lock module, permit to run stop and so on

  PROP_NOTIFY_STATUS_EVENT                  =182,    ///< Module的上报状态事件信息

  PROP_Module_Interface_End                 =250, ///=PROP_Driver_Interface_End+100,

  /**
   * ModuleLinkInterface Property
  */
  PROP_MODULE_LINK_IS_INHERENT               =251,                 ///< 此ModuleLink为固有的
  PROP_MODULE_LINK_INFO_TYPE_ALL             =252,               ///< 整个Link的信息,忽略Repeator的信息
  PROP_MODULE_LINK_INFO_TYPE_LEAF            =253,              ///< Link中所有的Destination的信息
  PROP_MODULE_LINK_INFO_TYPE_LAYER           =254,             ///< Link的某一层的所有节点信息,忽略Repeator的信息
  PROP_MODULE_LINK_INFO_TYPE_EDGE            =255,              ///< Link中某条边的节点信息,忽略Repeator的信息

  /// 试图获取Link中某条边上的Repeater的信息
  /// 输入输出参数为RepeaterIncludeModule*
  /// 依据所传的是modulename还是edgename(两个modulename以"|"分隔)
  /// 若对应的Repeater不存在则试图创建一个临时的Repeater
  PROP_MODULE_LINK_INFO_TYPE_EDGE_REPEATER   =256,
  PROP_MODULE_LINK_INFO_TYPE_REPEATER        =257,          ///< Link中所有Repeater的信息

  /// Link在初始化中Open后立即Run, 对应于Link配置文件中的autorun
  PROP_MODULE_LINK_INFO_AUTORUN              =258,

  /// Link的深度, Module的最大Level, Source的Level为1.
  PROP_MODULE_LINK_INFO_DEPTH                =259,

  PROP_MODULE_LINK_INTERFACE_END             =300, ///=PROP_Module_Interface_End+50,
  /**
   * CallbackInterface Property
   */
  PROP_OWNER_ID                              =301, ///= PROP_MODULE_LINK_INTERFACE_END + 1, ///< Callback所属的ID
  PROP_UNIQUE_KEY                            =302,   ///<  获取属于该Callback的唯一Key //  NOLINT

  PROP_Callback_Interface_End                =330, ///=PROP_MODULE_LINK_INTERFACE_END+30,

  /**
   * ProtocolInterface Property
  */
  PROP_ENCODE_TYPE                          =331, ///= PROP_Callback_Interface_End + 1,
  PROP_DECODE_TYPE                          =332,
  PROP_NORMAL                               =333,
  PROP_REQUEST                              =334,
  PROP_GET                                  =335,
  PROP_POST                                 =336,
  PROP_RESPONSE                             =337,

  PROP_Protocol_Interface_End               =360, ///=PROP_Callback_Interface_End+30,

  /**
   * StrategyInterface Property
   */
  PROP_IOINFO_CALLBACK                      =361,/// = PROP_Protocol_Interface_End + 1,
  PROP_FIRST_OPERATOR                       =362,
  PROP_SECOND_OPERATOR                      =363,
  PROP_FIRST_VALUE                          =364,
  PROP_SECOND_VALUE                         =365,
  PROP_EXPRESSION                           =366,
  PROP_TRANSITION_INFO_ADD                  =367,
  PROP_TRANSITION_INFO_REMOVE               =368,
  PROP_IS_MATCHED                           =369,

  //PROP_NAME,
  PROP_LOGIC                                =370,
  PROP_IOINFO                               =371,
  PROP_TRANSITION_INFO_CALLBACK             =372,
  //PROP_IOINFO_CALLBACK,
  //PROP_STRATEGY,
  PROP_CLEAR_RESULT                         =373,

  PROP_SWITCH                               =374,

  PROP_PARENT                               =375,
  PROP_PARENT_NAME                          =376,
  PROP_PARENT_STATE_NAME                    =377,
  PROP_CHILD                                =378,
  PROP_MANAGER                              =379,
  //PROP_STRATEGY,
  PROP_MECHANISM                            =380,
  PROP_STATE                                =381,      // 切换State，或获取当前State
  PROP_CURRENT_STATE_NAME                   =382,

  PROP_SWITCH_REQ                           =383,
  PROP_REG_SERVICE_EVENT_REQ                =384,
  PROP_UN_REG_SERVICE_EVENT_REQ             =385,
  PROP_READY_REQ                            =386,
  PROP_IOCTL_REQ                            =387,
  PROP_EVENT_NOTIFY_REQ                     =388,
  PROP_SWITCH_NOTIFY_REQ                    =389,

  PROP_MSG                                  =390,                     ///<  添加或获取消息数据 //  NOLINT
  PROP_CMD                                  =391,                     ///<  添加或获取命令数据 //  NOLINT
  PROP_STATE_SWITCH_INFO                    =392,
  //PROP_AGENT,                   ///<  register agent //  NOLINT
  PROP_UNREG_AGENT                          =393,             ///< 从状态机中反注册Agent  //  NOLINT

  PROP_GET_EVENT_CALLBACK                   =394,  ///< 通过EventName获得对应的Agent注册的Callback Name  // NOLINT

  PROP_SWITCH_REQ_WITHOUT_DISPATCH         =395,
  PROP_REG_SERVICE_EVENT_REQ_WITHOUT_DISPATCH       =396,
  PROP_UN_REG_SERVICE_EVENT_REQ_WITHOUT_DISPATCH    =397,
  PROP_READY_REQ_WITHOUT_DISPATCH          =398,
  PROP_IOCTL_REQ_WITHOUT_DISPATCH          =399,

  PROP_Strategy_Interface_End                = 410, ///=PROP_Protocol_Interface_End+50,

  /**
   * AgentInterface Property
  */
  PROP_CLIENT_IP                             =411, ///= PROP_Strategy_Interface_End + 1,
  PROP_CLIENT_PORT                           =412,
  PROP_SERVER_IP                             =413,
  PROP_SERVER_PORT                           =414,
  PROP_SYNC_MODE                             =415,
  PROP_ASYNC_MODE                            =416,
  PROP_ON_PROCESS_MESSAGE                    =417,
  PROP_ON_SENT_MESSAGE                       =418,
  PROP_ON_CONNECT_PLATFROM                   =419,

  PROP_Agent_Interface_End                   =440, ///=PROP_Strategy_Interface_End+30,

  /**
   * SessionInterface Property
  */
  //PROP_MANAGER = PROP_Agent_Interface_End + 1,     ///< 设置所属的AppManager
  PROP_REDIRECT                             =441, ///= PROP_Agent_Interface_End + 1,    ///<  本地重定向 //  NOLINT
  PROP_DOSERVICE                            =442,  ///<  DoServiceCallback //  NOLINT
  PROP_DISPATCHSERVICE                      =443,  ///<  DispatchServiceCallback //  NOLINT
  PROP_ERROR                                =444,      ///<  platform sdk error //  NOLINT

  PROP_Session_Interface_End                =470,  ///=PROP_Agent_Interface_End+30,

  /**
   * TransportInterface/ConnectionInterface Property
  */
  PROP_INFORMATION                          =471, ///= PROP_Session_Interface_End + 1,                ///< Transport Information
  PROP_ON_ACCEPT                            =472,                                                   ///< On Transport Accept 异步回调，通过SetProperity设置
  PROP_ON_CONNECT                           =473,                                                  ///< On Transport Connect 异步回调，通过SetProperity设置
  PROP_ON_DISCONNECT                        =474,                                               ///< On Transport Disconnect 异步回调，通过SetProperity设置
  PROP_ON_READ                              =475,                                                     ///< On Read 异步回调，通过SetProperity设置
  PROP_ON_WRITE                             =476,                                                    ///< On Write 异步回调，通过SetProperity设置
  //PROP_PROTOCOL,                                                    ///< The protocol needed by this transport
  PROP_MODE                                 =477,                                                        ///< 同步/异步模式，通过SetProperity设置
  //PROP_CALLBACK,                                                    ///< 设置Callback属性
  PROP_HANDLECONNECT                        =478,                                                ///< 客户端模式下调用HandleConnect
  PROP_READ                                 =479,                                                         ///< 设置ConnectionCallbackInterface对象，只支持ServerTerminal模式
  PROP_INVALID_CONNECTION                   =480,                                           ///< 设置无效链接，支持Server/ServerTerminal模式
  PROP_ERROR_CODE                           =481,                                                   ///< 获取对应的error code
  PROP_NEW_SESSIONID                        =482,                                                ///< 插入session id和connection指针匹配表，支持Server/ServerTerminal模式
  PROP_SESSIONID_CONNECTIONPTR              =483,                                      ///< 更新session id和connection指针匹配表，支持Server模式
  PROP_SESSIONID                            =484,                                                    ///< 设置/获取session id
  PROP_SPECIFIED_SESSIONID                  =485,                                          ///< 获取指定的session id
  PROP_READ_HEARTBEAT                       =486,                                               ///< 添加读取心跳机制消息队列
  PROP_WRITE_HEARTBEAT                      =487,                                              ///< 添加写入心跳机制消息队列
  PROP_HEARTBEAT                            =488,                                                    ///< 执行心跳机制

  /**
   * TransportCallbackInterface/ConnectionCallbackInterface/RPCCallbackInterface Property
  */
  PROP_CLIENT                              =489,                                                      ///< 客户端，负责向服务器请求连接
  PROP_SERVER                              =490,                                                      ///< 服务器端，启动Listen，等待客户端端连接请求
  PROP_SERVER_TERMINAL                     =491,                                             ///< 客户端请求后，服务器端会产生一个端点与客户端端通信
  PROP_SESSIONPTR                          =492,                                                  ///< appmanager连接到指定服务器后，绑定session指针

  /**
   * YSOSSDKInterface Property
  */
  PROP_JSON_RPC                             =493,                                                    ///< 设置jsonrpc属性， 用于本地或者远程通信
  PROP_PLATFORM_RPC                         =494,                                                ///< 设置平台rpc属性，用于ocx与平台通信
  PROP_PLATFORM                             =495,                                                    ///< 设置平台属性，用于启动平台

  PROP_Transport_Interface_End              =500, ///=PROP_Session_Interface_End+30,

  /**
   * BufferInterface Property
   */
  // PROP_BUFFER_POOL = PROP_Transport_Interface_End + 1,

  PROP_Buffer_Interface_End                =530, ///=PROP_Transport_Interface_End+30,

  /**
   * MessageInterface Property
   */
  PROP_MESSAGE_ANY                         =531, ///= PROP_Buffer_Interface_End + 1,

  /// remove flag
  PROP_PM_NO_REMOVE                        =532,
  PROP_PM_REMOVE                           =533,

  /// find flag
  PROP_FROM_ID                             =534,
  PROP_FROM_SOURCE                         =535,
  PROP_FROM_DESTINATION                    =536,

  PROP_Message_Interface_End               =560, ///=PROP_Buffer_Interface_End+30,

  /**
   * StreamInterface Property
  */
  PROP_SEEK_SET                           =561, ///= PROP_Message_Interface_End + 1,
  PROP_SEEK_CUR                           =562,
  PROP_SEEK_END                           =563,
  PROP_ASCII                              =564,   ///<  ascii, gb2312. etc  //  NOLINT
  PROP_UTF8                               =565,    ///<  utf-8 //  NOLINT
  PROP_UTF16                              =566,   ///<  unicode //  NOLINT
  PROP_LOCK_TYPE_BASE                     =567,
  PROP_LOCK_TYPE_FAKE                     =568,
  PROP_LOCK_TYPE_LIGHT                    =569,
  PROP_LOCK_TYPE_WEIGHT                   =570,
  //PROP_ERROR,
  PROP_NOT_SPECIFIED                      =571,
  PROP_EMPTY                              =572,
  //PROP_NORMAL,
  PROP_FULL                               =573,

  PROP_ROOT_NAME                          =574,

  PROP_Stream_Interface_End               =590, ///=PROP_Message_Interface_End+30,

  /**
   * OS Types
  */
  PROP_WINDOWS                           =591, ///= PROP_Stream_Interface_End + 1,
  PROP_LINUX                             =592,
  PROP_ANDROID                           =593,
  PROP_IOS                               =594,

  /**
   * Data Types
  */
  PROP_JSON                              =595,
  PROP_XML                               =596,

  YSOS_Platform_Property_End             =690, ///=PROP_Stream_Interface_End+100,
};

/**
 *@brief used for ioctl
 */
enum YSOS_Platform_Cmd {
  /**
   * BaseInterface Command
   */

  CMD_Base_Interface_End                    = 10,
  /**
   * ControlInterface Command
   */
  CMD_OPEN                                  =11,
  CMD_STOP                                  =12,
  CMD_RUN                                   =13,
  CMD_FLUSH                                 =14,
  CMD_CLOSE                                 =15,
  CMD_PAUSE                                 =16,

  CMD_Control_Interface_End                 =20, //= CMD_Base_Interface_End + 10,

  /**
   * DriverInterface Command
   */

  CMD_Driver_Interface_End                 =30, //= CMD_Control_Interface_End + 10,

  /**
   * ModuleInterface Command
   */
  CMD_NORMAL                               =31, ///<  普通的COMMAND, 具体实现在中 //  NOLINT

  CMD_Module_Interface_End                 =80, ///= CMD_Driver_Interface_End + 50,

  /**
   * ModuleLinkInterface Command
   */
  CMD_MODULE_LINK_ADD_EDGE                 =81,                     ///< 添加边到Repeater
  CMD_MODULE_LINK_REMOVE_EDGE              =82,                  ///< 从Repeater移除边

  /// 对指定实例名的Module进行指定的控制
  /// 以一个PropertyInfo的实例的指针作为Ioctl的param.
  /// 在该PropertyInfo的实例中, id为要进行的控制,
  /// str_id为要被控制的Module的实例名
  CMD_MODULE_LINK_CTRL_MODULE              =83,

  /// 从ModuleLink移除指定的Repeater, 恢复ModuleLink到插入前的状态
  /// 以一个PropertyInfo的实例的指针作为Ioctl的param.
  /// 在该PropertyInfo的实例中, id为要移除的Repeater的level
  /// 若level为0则移除所有的临时Repeater.
  CMD_MODULE_LINK_REMOVE_REPEATER          =84,

  CMD_MODULE_LINK_ADD_INHERENT_REPEATER    =85,        ///< 添加固有的Repeater到Link.

  CMD_MODULE_LINK_INTERFACE_END            =130, ///= CMD_Module_Interface_End + 50,

  /**
   * CallbackInterface Command
   */

  CMD_Callback_Interface_End               =160, ///= CMD_MODULE_LINK_INTERFACE_END + 30,

  /**
   * ProtocolInterface Command
   */

  CMD_Protocol_Interface_End              =190, /// = CMD_Callback_Interface_End + 30,

  /**
   * StrategyInterface Command
   */
  CMD_AGENT_DISPATCH                      =191,
  CMD_MECHANISM                           =192,
  CMD_GET_SERVICE_LIST                    =193,
  CMD_GET_MECHANISIM_STATE                =194,
  CMD_CHANGE_STATE_MACHINE                =195,
  CMD_GET_STATE_SWITCH_INFO               =196,
  CMD_GET_CURRENT_STATE_NAME              =197,
  CMD_CLEAR_MSG                           =198,           ///< clear msg_queue_
  CMD_CLEAR_CMD                           =199,           ///<  clear cmd_queue_  //  NOLINT

  /*CMD_AGENT_DISPATCH*///,
  //CMD_CLEAR_MSG,           ///< clear msg_queue_
  //CMD_CLEAR_CMD,           ///<  clear cmd_queue_  //  NOLINT
  //CMD_GET_SERVICE_LIST,    ///<  get all service supported by strategy  //  NOLINT
  //CMD_GET_MECHANISIM_STATE,   ///<  get mechanism state //  NOLINT
  //CMD_MECHANISM,

  CMD_Strategy_Interface_End              =240, ///= CMD_Protocol_Interface_End + 50,

  /**
   * AgentInterface Command
   */

  CMD_Agent_Interface_End                 =270, ///= CMD_Strategy_Interface_End + 30,

  /**
  * TransportInterface Command
  */

  CMD_Transport_Interface_End            =300, ///= CMD_Agent_Interface_End + 30,

  /**
   * BufferInterface Command
   */

  CMD_Buffer_Interface_End               =330, ///= CMD_Transport_Interface_End + 30,

  /**
   * MessageInterface Command
   */

  CMD_Message_Interface_End             =360, ///= CMD_Buffer_Interface_End + 30,

  /**
   * StreamInterface Command
   */

  CMD_Stream_Interface_End              =390, ///= CMD_Message_Interface_End + 30,


  YSOS_Platform_Normal_Cmd_End          =131,  ///!!!!!!!!!!!!!!= CMD_NORMAL + 100,



  CMD_CONTROL                           =8031,  ///=CMD_NORMAL+8000,   ///<  大于CONTROL_COMMAND控制命令，命令信息在TeamParam里，具体实现在RealIoctl(三个参数) //  NOLINT

  CMD_CONTROL_MODULE                    =32,    ///!!!!!!!!!!!!!!!!!!!= CMD_NORMAL + 1,
  CMD_REPEATINFO                        =8032,  ///=   CMD_CONTROL  + 1,
  CMD_TRANSITION_ADD                    =8033,
  CMD_TRANSITION_REMOVE                 =8034,
  CMD_MAPPINGPAIR_ADD                   =8035,
  CMD_MAPPINGPAIR_REMOVE                =8036,
  CMD_MAPPINGPAIR_REMOVE_ALL            =8037,


  /// Command End //  NOLINT
  YSOS_Platform_Control_Cmd_End        =8131,  ///= CMD_CONTROL + 100,
};
}

#endif  // SIP_YSOS_ENUM_H_       //NOLINT
