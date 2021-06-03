/**   //NOLINT
  *@file commonenumdefine.h
  *@brief Definition of enum define
  *@version 0.1
  *@author wanfenfen
  *@date Created on: 2016:11:22   9:52
  *@copyright Copyright(c) 2016 YunShen Technology. All rights reserved.
  * 
  *@howto Usage:
  *@todo
  */
#ifndef YSOS_PLUGIN_COMMONENUMDEFINE_H  //NOLINT
#define YSOS_PLUGIN_COMMONENUMDEFINE_H  //NOLINT
/// Ysos Headers //  NOLINT
#include "../../../public/include/sys_interface_package/ysos_enum.h"

namespace ysos {
    enum PropertiesEx
    {
        /**
        *@brief basic properties
        */
        PROP_BUFFER                                = 691,    ///= YSOS_Platform_Property_End + 1,        ///< 能力分配缓冲内存  691
        PROP_CAPTURE_IMAGE                         = 692,

        PROP_Basic_Propery_End                     = 711,    ///= PROP_BUFFER + 20,                   ///< 691+20 = 711


        /// USE FOR COMMON [3000-8000]
        //PROP_BUFFER_LENGTH               = 3001,       ///< BUFFER LENGTH ,DATATYPE:UINT32
        //PROP_BUFFER_NUMBER,           ///< DATATYPE:UINT32
        PROP_MEMORY_BLOCK_ALIGN                   = 712,         ///= PROP_Basic_Propery_End + 1 ,      ///< BLOCK ALIGN (UNIT:BYTE), DATATYPE:UINT32
        PROP_FUN_CALLABILITY                      = 713,         ///< CALL FUNCTION ABILITY. NOTITY THAT THE PARAM IS OBJECT FUNOBJECTCOMMON2(PPARAM1 = &ABILITYPARAM, PPARAM2 = &ABILITYPARAM).

        /// PROP_CUSTOM_AUDIO_MODE,   ///<

        /**
        *@brief audio properties
        */
        PROP_AUDIO_BUFFER_READY                  = 714,
        PROP_AUDIO_BUFFER_BACK                   = 715,
        PROP_REGISTER_ABILITY_EVENT              = 716,              ///< module注册能力事件，由module传给driver
        PROP_UNREGISTER_ABILITY_EVENT            = 717,              ///< 取消能力事件通知注册， 与PROP_REGISTER_ABILITY_EVENT 相对应


        PROP_STATUS_OPEN                         = 718,              ///< 核心平台里设备的状态没有open，这里加一个
        PROP_STATUS_DEVICE_FAIL                  = 719,

        /// USE FOR DRIVER(8000,10000]
        PROP_WIDTH                              = 8000,
        PROP_HEIGHT                             = 8001,
        PROP_LEFT                               = 8002,
        PROP_TOP                                = 8003,
        PROP_MIRROR_MODE                        = 8004,
        PROP_DATA                               = 8005,
        PROP_JSON_DATA                          = 8006,
        PROP_CUSTOM_DATA                        = 8007,

        /// USE FOR AUDIIO CAPTURE DRIVER (10001,10020]
        PROP_VOICE_DATA_CALLBACK_FUN            = 10001, ///< VOICE DATA CALLBACK FUNCTION
        PROP_VOICE_ADD_BUFFER                   = 10002,
        PROP_VOICE_BUFFTERINTERFACE_CALLBACK    = 10003,
        PROP_VOICE_AUDIO_CAPTURE_STATUS_EVENT_FUN  = 10004,  ///< 录音机状态回调事件



        /// USE FOR AUDIO PLAY DRIVER (10020, 10040]
        //PROP_INPUT_AUDIO_DATA         =10021,       ///< INPUT DATA, WITH FUNCTION PARAM OBJECT FUNOBJECTCOMMON3(PPARAM1 = VOID*, PPARAM2= INT, PPARAM3= INT);
        PROP_INPUT_AUDIO_DATA_END               = 10022,     ///< NOTIFY DRIVER THE INTPUT AUDIO DATA IS FINISH, WITH FUNCTION PARAM NULL
        PROP_EVENT_AUDIO_PLAY_STATUS            = 10023,     ///< 当前播放器状态

        /// USE FOR MODULE(20000, 3000]
        /// USE FOR HCICUSTOMNLPMODULE(20000, 20100]
        PROP_FUN_GETCUSTOMNLP       = 20001,  ///< CALL FUN HCICUSTOMNLPMODULE::GETCUSTOMNLP, WITH FUNCTION PARAMS OBJECT FUNOBJECTCOMMON2(PPARAM1 = &STD::STRING, PPARAM2 = &STD::STRING).

        /// USE FOR IFLYNLPMODULE (20100, 20200]
        PROP_FUN_GET_NLP_PARAMS       = 20101,  ///< CALL FUN IFLYNLPMODULE::GETNLPPARAMS, WIFH ONE FUNCTION PARSMS (PPARAM1 = &STD::STRING)

        /// USE FOR AUDIO PLAY MODULE(20201, 20400]
        PROP_INPUT_SOUND              = 20202 /*= BASEMODULEIMPL_COMMAND_END + 1*/, ///< SET INPUT SOUND, FOR DSOUND INPUT DATA
        PROP_AUDIOPLAY_MODULE_END,    // DISCARD, USE PROP_INPUT_AUDIO_DATA_END
        //PROP_INPUT_AUDIO_DATA_END, // USING, REFRENCE FROM AUDIO PLAY DRIVER

        //////////////////////////////////////////////////////////////////////////////
        /**
        *@brief chassis properties, using range (20400, 20800]
        */
        PROP_CHASSIS_SYSTEM_INFO                      = 20401,    ///< 底盘系统信息
        PROP_CHASSIS_TYPE_INFO                        = 20402,    ///< 底盘类型
        PROP_CHASSIS_GET_MOVE_MODE                    = 20403,    ///< 获取行走模式

        PROP_CHASSIS_PROPERTY_END                     = 20800,    ///< 底盘属性添加于此行之上

        //////////////////////////////////////////////////////////////////////////////
        /// (20800, 21000] for PowerManager Driver/Module
        PROP_POWERMGR_SERIAL_PORT                    = 20801,     ///< 串口端口
        PROP_POWERMGR_SERIAL_BAUDRATE                = 20802,     ///< 串口波特率
        PROP_POWERMGR_SERIAL_PARITY                  = 20803,     ///< 串口Parity
        PROP_POWERMGR_SERIAL_BYTESIZE                = 20804,     ///< 串口bytesize
        PROP_POWERMGR_SERIAL_STOPBITS                = 20805,     ///< 串口stopbits
        PROP_POWERMGR_SERIAL_READ_TIMEOUT            = 20806,     ///< 串口stopbits

        PROP_POWERMGR_BATTERY_INFO                   = 20810,     ///< 电量信息
        PROP_POWERMGR_POWEROFF_INFO                  = 20811,     ///< 关机信息
        PROP_POWERMGR_REBOOT_DOWN                    = 20812,     ///< 重启下半身
        PROP_POWERMGR_CHARGE_INFO                    = 20813,     ///<线充状态
        #if 0
        PROP_POWERMGR_WATCHDOG_ENABLE                = 20820,     ///< 使能看门狗
        PROP_POWERMGR_WATCHDOG_INTERVAL              = 20821,     ///< 喂狗时间间隔，单位：秒
        PROP_POWERMGR_WATCHDOG_REBOOTMODE            = 20822,     ///< 看门狗重启模式
        PROP_POWERMGR_QUERYBATTERY_INTERVAL          = 20823,     ///< 查询电池电量的时间间隔，这个时间要大于喂狗时间间隔，单位：毫秒
        #endif
        PROP_POWERMGR_PROPERTY_END                   = 21000,
        /// USE FOR CALLBACK (30000, 40000]

        //use for video module(20900, 21000]
        PROP_ADD_VIDEO_BUFFER                        = 20900,
        PROP_GET_CAMERA_OPEN_STATUS_ACCORD_ID        = 20901,     ///< 获取摄像头 打开/关闭状态， 1 打开，0关闭
        PROP_GET_CAMERA_OPEN_STATUS_ACCORD_NAME      = 20902,     ///< 重载获取摄像头 打开/关闭状态， 1 打开，0关闭
        PROP_CAMERA_TAKE_A_PICTURE                   = 20903,     ///< 人脸拍照，获取当前摄像头base64图片数据
        PROP_ROTATE_ANGLE                            = 20904,     ///< 视频旋转角度

        //use for face camparision (21100, 21200]
        PROP_FACE_COMPARISON_11_V1                   = 20100,     ///< 人脸1比1比对
        PROP_FACE_COMPARISON_1N_V1                   = 20101,     ///< 人脸1比n比对
        PROP_FACE_REGISTER                           = 20102,     ///< 人脸注册
        PROP_FACE_MATTING                            = 20103,     ///< 人脸抠图
        PROP_FACE_MATTING_FILE                       = 20104,     ///< 人脸抠图
        PROP_FACE_SELECT_ONE                         = 20105,     ///< 选择一个人脸作为最佳人脸
        PROP_FACE_DETECT                             = 20106,     ///< face detect
        PROP_SPEECH_RECOGNITION                      = 20107,     ///< 语音识别
        PROP_FACE1N_DATA_TYPE                        = 20108,     ///< 人脸识别1比n，使用到的数据格式， 0 人脸特征，1 图片地址，2 图片Base64
        PROP_FACE_RECOGNIZE                          = 20109,     ///< 获取人脸识别结果
        PROP_FACE_DETECT_ASYNC                       = 20110,     ///< 人脸检测，检测后通知识别线程就返回
        PROP_FACE_MIN_SCORE                          = 20111,     ///< 人脸检测最低分数
        PROP_FACE_RECOGNIZE_INTERVAL                 = 20112,     ///< 人脸识别时间间隔
        PROP_MIN_RECOGNIZE_SCORE                     = 20113,     ///< 人脸识别最小分数
        PROP_FACE_DETECTOR                           = 20114,     ///< 人脸检测器配置
        PROP_FACE_RECOGNIZER                         = 20115,     ///< 人脸识别器配置
        PROP_RECOGNIZE_SERVER                        = 20116,     ///< 人脸识别服务器地址
        PROP_ORG_ID                                  = 20117,     ///< 机构号
        PROP_TERM_ID                                 = 20118,     ///< 终端号
        PROP_MIN_FACE_WIDTH                          = 20119,     ///< 人脸的最小宽度
        PROP_MIN_FACE_HEIGHT                         = 20120,     ///< 人脸的最小高度
        PROP_MIN_LOCAL_RECOGNIZE_SCORE               = 20121,     ///< 本地人脸识别最小分数
        PROP_ENABLE_LOCAL_RECOGNIZE                  = 20122,     ///< 启用本地人脸识别
        PROP_FACE_COMPARE                            = 20123,     ///< 获取人脸比较结果
        PROP_FACE_DETECT_RECOGNITION                 = 20124,     ///< 人脸检测和识别

        PROP_FACE_COMPARISON_1N_V1_CCB               = 20150,     ///< 人脸1比n比对(建设银行)

        //use for face RealSense (21200, 21300]
        PROP_REALSENSE_SET_HORIZONTAL_REVISE         = 21201,
        PROP_REALSENSE_SET_VERTICAL_ANGLE            = 21202,     ///< 摄像头上扬角度

        //use for financial business (21300, 21400]
        PROP_FINANCIAL_HAS_REQUEST                   = 21301,
        PROP_FINANCIAL_REQUEST_DATA                  = 21302,
        PROP_FINANCIAL_REQUEST                       = 21303,
    };

    enum IoCtlCommand
    {
        /// use for common [0, 10000]
        CMD_ICC_SET_OUTPUTBUF                       = 8132, ///=YSOS_Platform_Control_Cmd_End + 1,   ///< 8132
        CMD_ICC_START                               = 8133,
        CMD_ICC_STOP                                = 8134,
        CMD_ICC_PAUSE                               = 8135,

        CMD_GET_DATA                                = 8136,
        CMD_SET_DATA                                = 8137,

        CMD_CAPTURE_IMAGE                           = 8138,                                          ///< 8138

        CMD_CUSTOM_MODE                             = 8139,                                          ///< 添加定制语音

        CMD_SET_JSON_DATA                           = 8140,                                          ///< 8140 设置Json Data
        CMD_SET_CUSTOM_DATA                         = 8141,                                          ///< 8141 设置定制数据

        CMD_SET_IMAGE_WIDTH_HEIGHT                  = 8142,

        CMD_PERSON_IS_ADMINISTRATER                 = 8143,                                          ///< 8143 当前客户是否为管理员

        /// use for driver(10000,20000]
        CMD_DRIVER_UNINITIALIZE                       = 10001,      ///< 通知driver 释放资源

        /// use for audiio capture driver (10000,10020]


        /// use for audio play driver (10020, 10040]
        CMD_DELAY                                     = 10020,  ///<   //  NOLINT,delay to play next audio, in m-sec
        CMD_INTERRUPT                                 = 10021, /// playing can be interrupted or not
        CMD_PLAY_CTRL_END                             = 10022,
        CMD_INPUT_PLAY_AUDIO_DATA                     = 10023,
        /// use for module(20000, 30000]

        //////////////////////////////////////////////////////////////////////////////
        /**
        *@brief chassis command, using range (20000, 20400]
        */
        CMD_CHASSIS_CONNECT                           = 20001,
        CMD_CHASSIS_DISCONNECT                        = 20002,
        CMD_CHASSIS_RECONNECT                         = 20003,
        CMD_CHASSIS_RESTART                           = 20004,
        CMD_CHASSIS_SET_DEVICE_INFO                   = 20005,
        CMD_CHASSIS_GET_DEVICE_INFO                   = 20006,
        CMD_CHASSIS_ENABLE_HEART_BEAT                 = 20007,
        CMD_CHASSIS_DISABLE_HEART_BEAT                = 20008,
        CMD_CHASSIS_POWER_OFF                         = 20009,
        CMD_CHASSIS_MAP_INITIALIZE                    = 20010,
        CMD_CHASSIS_SET_MAP_INFO                      = 20011,
        CMD_CHASSIS_GET_MAP_INFO                      = 20012,
        CMD_CHASSIS_SET_VIRTUAL_OBSTACLE              = 20013,
        CMD_CHASSIS_GET_VIRTUAL_OBSTACLE              = 20014,
        CMD_CHASSIS_SET_PREDEFINED_POSITION           = 20015,
        CMD_CHASSIS_GET_PREDEFINED_POSITION           = 20016,
        CMD_CHASSIS_SET_OBSTACLE_DETECTION_RANGE      = 20017,
        CMD_CHASSIS_GET_OBSTACLE_DETECTION_RANGE      = 20018,
        CMD_CHASSIS_SET_MOVE_SPEED                    = 20019,
        CMD_CHASSIS_GET_MOVE_SPEED                    = 20020,
        CMD_CHASSIS_SEND_HEART_BEAT                   = 20021,
        CMD_CHASSIS_GET_DEVICE_STATUS                 = 20022,
        CMD_CHASSIS_GET_HEALTH_STATUS                 = 20023,
        CMD_CHASSIS_GET_LOCALIZATION_STATUS           = 20024,
        CMD_CHASSIS_GET_NAVIGATION_STATUS             = 20025,
        CMD_CHASSIS_SET_SENSOR_DATA                   = 20026,
        CMD_CHASSIS_GET_SENSOR_DATA                   = 20027,
        CMD_CHASSIS_SET_OBSTACLE_DATA                 = 20028,
        CMD_CHASSIS_GET_OBSTACLE_DATA                 = 20029,
        CMD_CHASSIS_GO_HOME                           = 20030,
        CMD_CHASSIS_START_MOVE_BY_DIRECTION           = 20031,
        CMD_CHASSIS_START_MOVE_TO                     = 20032,
        CMD_CHASSIS_START_MOVE_BY_PATH                = 20033,
        CMD_CHASSIS_PAUSE_MOVE                        = 20034,
        CMD_CHASSIS_RESUME_MOVE                       = 20035,
        CMD_CHASSIS_STOP_MOVE                         = 20036,
        CMD_CHASSIS_START_RECORD_PATH                 = 20037,
        CMD_CHASSIS_STOP_RECORD_PATH                  = 20038,
        CMD_CHASSIS_START_ROTATE_TO                   = 20039,
        CMD_CHASSIS_START_ROTATE                      = 20040,

        CMD_CHASSIS_START_MOVE_STILL                  = 20041,
        CMD_CHASSIS_START_MOVE_TO_POINT               = 20042,
        CMD_CHASSIS_START_MOVE_CRUISE                 = 20043,
        CMD_CHASSIS_START_MOVE_RANDOM                 = 20044,
        CMD_CHASSIS_MOVE_TO_CAMERA_POINT              = 20045,
        CMD_CHASSIS_MOVE_RELATIVELY                   = 20046,

        CMD_CHASSIS_GET_ROTATE_STATUS                 = 20055,
        CMD_CHASSIS_CHECK_ERROR_STATUS                = 20056,
        CMD_CHASSIS_GET_NEAREST_LASER_INFO_IN_ANGLE_RANGE = 20057,
        CMD_CHASSIS_IS_POSITION_NEAR                  = 20058,

        CMD_CHASSIS_GET_GREET_STATUS_CODE             = 20060,
        CMD_CHASSIS_START_GREET                       = 20061,
        CMD_CHASSIS_PAUSE_GREET                       = 20062,
        CMD_CHASSIS_RESUME_GREET                      = 20063,
        CMD_CHASSIS_STOP_GREET                        = 20064,
        CMD_CHASSIS_SET_GREET_PARAM                   = 20065,

        CMD_CHASSIS_COMMAND_END                       = 20400,  ///< 底盘属性添加于此行之上


        /// (20800, 21000] for PowerManager Driver/Module
        CMD_POWERMGR_CLOSE_UPBODY_DOWNLOCK           = 20801, ///< 上半身关机，下半身锁定    //NOLINT
        CMD_POWERMGR_CLOSE_ALLBODY                   = 20802, ///< 全身关机                  //NOLINT
        CMD_POWERMGR_CLOSE_UPBODY                    = 20803, ///< 上半身关机                //NOLINT
        CMD_POWERMGR_POWER_SET_MODE_VOLT             = 20804, ///< 设置查询电量为读电压模式  //NOLINT
        CMD_POWERMGR_POWER_SET_MODE_BATTERY          = 20805, ///< 设置查询电量为读电池模式  //NOLINT
        CMD_POWERMGR_POWER_QUERY_BATTERY             = 20806, ///< 查询电量                  //NOLINT
        CMD_POWERMGR_POWER_QUERY_MODE                = 20807, ///< 查询电量模式，电压或电池  //NOLINT
        CMD_POWERMGR_SYNCH_CURSYS_TIME               = 20808, ///< 同步当前系统时间          //NOLINT
        CMD_POWERMGR_SYNCH_OPENSYS_TIME              = 20809, ///< 同步开机时间              //NOLINT
        CMD_POWERMGR_SYNCH_CLOSESYS_TIME             = 20810, ///< 同步关机时间              //NOLINT
        CMD_POWERMGR_SYNCH_CANCEL_OPENSYS            = 20811, ///< 取消同步开机              //NOLINT
        CMD_POWERMGR_SYNCH_CANCEL_CLOSESYS           = 20812, ///< 取消同步关机              //NOLINT
        CMD_POWERMGR_REBOOT_UPBODY                   = 20813, ///< 重启上半身                //NOLINT
        CMD_POWERMGR_REBOOT_DOWNBODY                 = 20814, ///< 重启下半身                //NOLINT
        CMD_POWERMGR_REBOOT_ALLBODY                  = 20815, ///< 重启全身身                //NOLINT
        CMD_POWERMGR_WATCHDOG_CLOSE                  = 20816, ///< 关闭看门狗                //NOLINT
        CMD_POWERMGR_WATCHDOG_OPEN                   = 20817, ///< 开启看门狗， 数据：左两个字节为时间间隔，单位为秒，最右边一个字节为重启 //NOLINT
        CMD_POWERMGR_WATCHDOG_WATCH                  = 20818, ///< 喂狗                     //NOLINT
        CMD_POWERMGR_WATCHDOG_QUERY                  = 20819, ///< 查询看门狗状态           //NOLINT
        CMD_POWERMGR_NOTIFY_CHARGING                 = 20820, ///< 通知正在充电             //NOLINT
        CMD_POWERMGR_NOTIFY_NOT_CHARGING             = 20821, ///< 通知没在充电             //NOLINT
        CMD_POWERMGR_THREAD_STOP                     = 20822, ///< 策略层调用命令前关闭module层线程（临时应对）   //NOLINT
        CMD_POWERMGR_THREAD_START                    = 20823, ///< 策略层调用命令后重启module层线程（临时应对）   //NOLINT

        CMD_POWERMGR_COMMAND_END                     = 21000,

        /// (25000, 28000] for hardware Driver/Module
        //< (25000, 25100] 串口模块
        CMD_PORT_READ                                = 25001,        //< 读数据
        CMD_PORT_NEGREAD                             = 25002,        //< 被动读数据
        CMD_PORT_WRITE                               = 25003,        //< 写数据
        CMD_PORT_RESET                               = 25004,        //< 重置串口
        CMD_PORT_OTHER                               = 25005,        //< 其他

        //< (25100, 25200] crt310004读卡器
        CMD_READ                                     = 25101,        //< 读命令
        CMD_WRITE                                    = 25102,        //< 写命令
        CMD_RESET                                    = 25103,        //< 重置读卡器
        CMD_CARD_IN                                  = 25104,        //< 进卡命令
        CMD_CARD_EJECT                               = 25105,        //< 退卡命令
        CMD_CARD_RETAIN                              = 25106,        //< 吞卡命令
        CMD_CARD_POS                                 = 25107,        //< 获取卡位置
        CMD_CARD_TYPE                                = 25108,        //< 获取卡类型
        CMD_COLD_RESET                               = 25109,        //< 接触式IC卡冷复位
        CMD_WARM_RESET                               = 25110,        //< 接触式IC卡热复位
        CMD_POWER_ON                                 = 25111,        //< 接触式IC卡上电
        CMD_POWER_OFF                                = 25112,        //< 接触式IC卡下电
        CMD_CPU_CAPDU                                = 25113,        //< 接触式IC卡C-APDU命令
        CMD_RF_RESET                                 = 25114,        //< 非接式IC卡复位
        CMD_RF_CPU_CAPDU                             = 25115,        //< 非接式IC卡C-APDU命令
        CMD_CAPDU                                    = 25117,        //< IC卡C-APDU命令
        CMD_PBOC_PSE                                 = 25118,        //< PBOC应用选择
        CMD_PBOC_PRO                                 = 25119,        //< PBOC交易初始化
        CMD_PBOC_EXAUTH                              = 25120,        //< PBOC外部认证
        CMD_PBOC_LOG                                 = 25121,        //< PBOC交易日志
        CMD_PBOC_QCLOG                               = 25122,        //< PBOC圈存日志
        CMD_PBOC_TAG                                 = 25123,        //< PBOC标签值
        CMD_GET_READY                                = 25124,        //< 进卡前准备
        CMD_READ_MAG                                 = 25125,        //< 读磁卡
        CMD_READ_IC                                  = 25126,        //< 读IC卡
        CMD_WRITE_IC                                 = 25127,        //< 写IC卡
        CMD_CALLBACK_PSE                             = 25128,        //< 上抛应用信息
        CMD_CALLBACK_PRO                             = 25129,        //< 上抛初始化信息
        CMD_CALLBACK_LOG                             = 25130,        //< 上抛交易日志信息
        CMD_CALLBACK_QCLOG                           = 25131,        //< 上抛圈存日志信息
        CMD_CALLBACK_TAG                             = 25132,        //< 上抛标签信息
        CMD_CALLBACK_READ                            = 25133,        //< 上抛信息
        CMD_CALLBACK_AUTO                            = 25134,        //< 自动选择
        CMD_PBOC_TAG_ACTIVE                          = 25135,        //< PBOC标签值
        //< t10em2 电动读卡器
        CMD_T10EM2_RESET                             = 25136,        //< 复位读卡器
        CMD_T10EM2_ENTER                             = 25137,        //< 设置进卡
        CMD_T10EM2_EJECT                             = 25138,        //< 弹卡
        CMD_T10EM2_ICNO                              = 25139,        //< 读取健康卡号
        CMD_T10EM2_IC                                = 25140,        //< 读取健康卡信息
        CMD_T10EM2_MAG                               = 25141,        //< 读取医联卡
        CMD_T10EM2_SEC                               = 25142,        //< 读取社保卡
        CMD_T10EM2_TYPE                              = 25143,        //< 检测卡类型
        CMD_T10EM2_HAVE                              = 25144,        //< 上抛有卡信息
        CMD_CALLBACK_CARD                            = 25145,        //< 上抛卡信息
        //< 梁山县人民医院读卡器
        CMD_READER_AUTH                              = 25146,        //< 安全认证
        CMD_READER_RESET                             = 25147,        //< 复位卡机
        CMD_READER_GET_VERSION                       = 25148,        //< 获取版本号
        CMD_READER_GET_SERIAL                        = 25149,        //< 获取序列号
        CMD_READER_DETECT_CARD                       = 25150,        //< 检测卡类型
        CMD_READER_ENTER_CARD                        = 25151,        //< 进卡控制
        CMD_READER_CARD_POSITON                      = 25152,        //< 停卡位置设置
        CMD_READER_GET_STATUS                        = 25153,        //< 读取卡机状态
        CMD_READER_MOVE_CARD                         = 25154,        //< 移卡
        CMD_READER_READ                              = 25155,        //< 读磁道数据
        CMD_READER_WRITE                             = 25156,        //< 写磁道数据
        CMD_READER_CARD_RESET                        = 25157,        //< 卡上电并复位
        CMD_READER_APDU                              = 25158,        //< 发送APDU命令
        CMD_READER_POWER_OFF                         = 25159,        //< 卡下电
        CMD_READER_RF_SERIAL                         = 25160,        //< 获取RF卡序列号
        CMD_READER_READ_APPOINT                      = 25161,        //< 读指定位置数据
        CMD_READER_WRITE_APPOINT                     = 25162,        //< 写指定位置数据
        CMD_READER_CHANGE_PWD                        = 25163,        //< 修改卡密码
        CMD_READER_SET_LOG                           = 25164,        //< 设置日志信息
        CMD_READER_GET_ERROR                         = 25165,        //< 获取错误信息
        CMD_READER_SET_BAUD                          = 25166,        //< 设置波特率
        CMD_READER_M1_READ                           = 25167,        //< M1卡读取扇区块数据
        CMD_READER_M1_WRITE                          = 25168,        //< M1卡写入数据块数据
        //< crt310004读卡器新增
        CMD_CARDIN_SET                               = 25190,        //< 进卡控制
        CMD_CALLBACK_310_STATUS                      = 25191,        //< 上抛状态信息
        CMD_READ_M1                                  = 25192,        //< 读M1卡

        //< (25200, 25300] zt598密码键盘
        CMD_PINPAD_OPEN                              = 25201,        //< 打开键盘
        CMD_PINPAD_CLOSE                             = 25202,        //< 关闭键盘
        CMD_PINPAD_LOAD_MASKEY                       = 25203,        //< 加载主密钥
        CMD_PINPAD_LOAD_WORKEY                       = 25204,        //< 加载工作密钥
        CMD_PINPAD_RESET                             = 25205,        //< 重置密码键盘
        CMD_PINPAD_GETMAC                            = 25206,        //< MAC加密
        CMD_PINPAD_READPIN                           = 25207,        //< 获取PINBLOCK
        CMD_PINPAD_PASSWORD                          = 25208,        //< 获取用户按键
        CMD_CALLBACK_GETMAC                          = 25209,        //< 上抛MAC数据
        CMD_CALLBACK_READPIN                         = 25210,        //< 上抛PINBLOCK数据
        CMD_CALLBACK_PINAUTO                         = 25211,        //< 自动选择
        //< 凯明杨密码键盘
        CMD_KMY_AUTH                                 = 25250,        //< 安全认证
        CMD_KMY_OPEN                                 = 25251,        //< 打开设备
        CMD_KMY_CLOSE                                = 25252,        //< 关闭设备
        CMD_KMY_GET_VERSION                          = 25253,        //< 获取版本号
        CMD_KMY_GET_SERIAL                           = 25254,        //< 获取序列号
        CMD_KMY_FACTORY_SET                          = 25255,        //< 恢复出厂设置
        CMD_KMY_SET_MODE                             = 25256,        //< 设置模式
        CMD_KMY_MASKEY                               = 25257,        //< 加载主密钥
        CMD_KMY_WORKEY                               = 25258,        //< 加载工作密钥
        CMD_KMY_ACTIVE_KEY                           = 25259,        //< 激活密钥
        CMD_KMY_START_PIN                            = 25260,        //< 启动加密
        CMD_KMY_GET_PIN                              = 25261,        //< 获取pinblock
        CMD_KMY_GET_MAC                              = 25262,        //< 计算mac值
        CMD_KMY_DATA_COMPUTE                         = 25263,        //< 数据运算
        CMD_KMY_TEXT_MODE                            = 25264,        //< 明文模式
        CMD_KMY_WRITE_DATA                           = 25265,        //< 指定位置写入数据
        CMD_KMY_READ_DATA                            = 25266,        //< 指定位置读取数据
        CMD_KMY_SET_LOG                              = 25267,        //< 设置日志信息
        CMD_KMY_GET_ERROR                            = 25268,        //< 获取错误信息


        //< (25300, 25400] 签到模块
        CMD_SIGN_IN                                  = 25301,        //< 设备签到

        //< (25400, 25500] idtrym9000mini身份证读卡器
        CMD_IDTRYM_IDDATA                            = 25401,        //< 获取身份证信息
        CMD_IDTRYM_RESET                             = 25402,        //< 重置读卡器
        CMD_IDTRYM_MFCPUSAM_POWERON                  = 25403,        //< 非接触式IC卡上电复位
        CMD_IDTRYM_DATAWITHPATH                      = 25404,        //< 获取身份证信息
        CMD_CALLBACK_IDDATA                          = 25405,        //< 上抛身份证信息
        CMD_CALLBACK_IDTRYMAUTO                      = 25406,        //< 自动选择

        //< (25500, 25600] et58热敏打印机
        CMD_ET58_DYHH                                = 25501,        //< 打印并换行
        CMD_ET58_SPZB                                = 25502,        //< 水平制表
        CMD_ET58_DYHC                                = 25503,        //< 打印并回车
        CMD_ET58_JZ                                  = 25504,        //< 进纸
        CMD_ET58_TZ                                  = 25505,        //< 退纸
        CMD_PRINTER_STATUS                           = 25506,        //< 实时传送状态
        CMD_ET58_ZFHJJ                               = 25507,        //< 设置字符行间距2mm
        CMD_ET58_ZFYJJ                               = 25508,        //< 设置字符右间距
        CMD_ET58_ZFDYMS                              = 25509,        //< 设置字符打印模式
        CMD_ET58_HJJ                                 = 25510,        //< 设置行间距为n点行（n/203寸）
        CMD_ET58_INIT                                = 25511,        //< 打印机初始化
        CMD_PRINTER_CUT                              = 25512,        //< 自动切纸
        CMD_ET58_DYWZ                                = 25513,        //< 设置打印位置
        CMD_ET58_DYJZ                                = 25514,        //< 打印并进纸n字符行
        CMD_ET58_YXBK                                = 25515,        //< 允许字符倍宽打印
        CMD_ET58_QXBK                                = 25516,        //< 取消字符倍宽打印
        CMD_ET58_SPZBZ                               = 25517,        //< 水平制表值
        CMD_ET58_SDDT                                = 25518,        //< 设定点图命令
        CMD_ET58_JRHZDY                              = 25519,        //< 进入汉字打印方式
        CMD_ET58_TCHZDY                              = 25520,        //< 退出汉字打印方式
        CMD_ET58_XZZFDY                              = 25521,        //< 选择字符打印模式
        CMD_ET58_HZXHX                               = 25522,        //< 设定汉字的下划线
        CMD_ET58_HZZYJJ                              = 25523,        //< 设置汉字的左右间距
        CMD_ET58_HZ4BDY                              = 25524,        //< 设置/取消汉字4倍打印
        CMD_ET58_XZBQ                                = 25525,        //< 下载标签
        CMD_ET58_BQDY                                = 25526,        //< 标签打印
        CMD_ET58_DYND                                = 25527,        //< 设定打印浓度
        CMD_ET58_ROM                                 = 25528,        //< 询问ROM内容加总的核对码
        CMD_ET58_GJBB                                = 25529,        //< 查询固件版本
        CMD_ET58_TMKD                                = 25530,        //< 设置条码宽度
        CMD_ET58_TMGD                                = 25531,        //< 设置条形码高度
        CMD_ET58_DYTM                                = 25532,        //< 打印条形码
        CMD_ET58_WDATA                               = 25533,        //< 写入打印数据
        CMD_ET58_PRINT                               = 25534,        //< 打印数据
        CMD_CALLBACK_ET58CSZT                        = 25535,        //< 上抛状态数据
        CMD_CALLBACK_ET58ROM                         = 25536,        //< 上抛ROM内容
        CMD_CALLBACK_ET58GJBB                        = 25537,        //< 上抛固件版本
        CMD_CALLBACK_ET58AUTO                        = 25538,        //< 自动选择
        //< 福彩打印机
        CMD_PRINT_LOTTERY                            = 25539,        //< 打印彩票
        CMD_LOTTERY_CHANGE_PAPER                     = 25540,        //< 换纸
        CMD_CALLBACK_SUCCEED                         = 25541,        //< 上抛打印成功
        CMD_CALLBACK_FAILED                          = 25542,        //< 上抛打印失败
        //< 美松
        CMD_MS_PRINT                                 = 25543,        //< 打印数据（文字、二维码）
        CMD_MS_SET                                   = 25544,        //< 设置字体参数

        //< (25600,25700]机械手臂
        CMD_ARM_RESET                                = 25601,        //< 舵机复位
        CMD_ARM_POSE                                 = 25602,        //< 手臂手势
        CMD_ARM_GET_VER                              = 25603,        //< 获取运动控制器版本号
        CMD_ARM_SET_SERVO                            = 25604,        //< 设置舵机角度
        CMD_ARM_SEARCH_LAST_ANGLE                    = 25605,        //< 查询上一次舵机角度
        CMD_ARM_GET_CURRENT_ANGLE                    = 25606,        //< 获取当前舵机角度
        CMD_ARM_TO_LAST_ANGLE                        = 25607,        //< 旋转至设置角度
        CMD_CALLBACK_ARMGETVER                       = 25608,        //< 上抛版本号
        CMD_CALLBACK_ARMAUTO                         = 25609,        //< 自动选择

        //< (25700, 25800] 定时器模块
        CMD_CALLBACK_TIMEAUTO                        = 25701,        //< 自动选择

        //< (25800, 25900] 头部
        CMD_HEAD_POSE                                = 25801,        //< 头部动作
        CMD_HEAD_NOD                                 = 25802,        //< 头部点头
        CMD_HEAD_SHAKE                               = 25803,        //< 头部摇头
        CMD_HEAD_TO_LEFT                             = 25804,        //< 向左转头
        CMD_HEAD_TO_RIGHT                            = 25805,        //< 向右转头
        CMD_CALLBACK_HEADGETVER                      = 25806,        //< 上抛版本号
        CMD_CALLBACK_HEADAUTO                        = 25807,        //< 自动选择

        //< (25900, 26000] 百度机器人
        CMD_BAIDU_ACTION                             = 25901,        //< 执行手势动作
        CMD_BAIDU_SET_ANGLE_SPEED                    = 25902,        //< 设置手臂角度和速度
        CMD_BAIDU_EXECUTE_APPOINT                    = 25903,        //< 执行手臂手势
        CMD_BAIDU_SWING                              = 25904,        //< 执行摆臂动作
        CMD_BAIDU_SET_SWING                          = 25905,        //< 设置摆臂
        CMD_BAIDU_EXECUTE_SWING                      = 25906,        //< 执行摆臂命令
        CMD_BAIDU_STOP                               = 25907,        //< 停止摆臂
        CMD_BAIDU_GET_ANGLE                          = 25908,        //< 获取手臂角度
        CMD_BAIDU_GET_STATUS                         = 25909,        //< 获取手臂状态
        CMD_BAIDU_SET_CHEST                          = 25910,        //< 设置胸口灯
        CMD_BAIDU_SET_SHOULDER                       = 25911,        //< 设置肩膀灯
        CMD_BAIDU_SET_FINGER                         = 25912,        //< 设置手指灯
        CMD_BAIDU_SET_CHASSIS                        = 25913,        //< 设置底盘氛围灯
        CMD_CALLBACK_BAIDUGETVER                     = 25914,        //< 上抛版本号
        CMD_CALLBACK_BAIDUGETANGLE                   = 25915,        //< 上抛角度信息
        CMD_CALLBACK_BAIDUGETSTATUS                  = 25916,        //< 上抛状态信息
        CMD_CALLBACK_BAIDUAUTO                       = 25917,        //< 自动选择

        //< (26000, 26100] msd347打印机
        CMD_MSD347_INIT                              = 26001,        //< 初始化打印机
        CMD_MSD347_CLEAN                             = 26002,        //< 清空打印机缓存数据
        CMD_MSD347_PRINTMARKCUT                      = 26003,        //< 检测黑标并切纸
        CMD_MSD347_SETLINESPACE                      = 26004,        //< 设置行间距
        CMD_MSD347_SETALIGNMENT                      = 26005,        //< 设置对齐方式
        CMD_MSD347_SETMARGIN                         = 26006,        //< 设置左边距
        CMD_MSD347_PRINTPDF417                       = 26007,        //< 打印pdf417码
        CMD_MSD347_PRINTQRCODE                       = 26008,        //< 打印二维码
        CMD_MSD347_PRINTANDWRAP                      = 26009,        //< 打印并换行
        CMD_MSD347_ENLARGE                           = 26010,        //< 放大字符
        CMD_MSD347_BOLD                              = 26011,        //< 设置/取消粗体打印
        CMD_MSD347_STATUSFEEDBACK                    = 26012,        //< 实时状态反馈
        CMD_MSD347_PRINT                             = 26013,        //< 打印数据
        CMD_MSD347_CUT                               = 26014,        //< 全切纸
        CMD_MSD347_BITMAP                            = 26015,        //< 打印图片
        CMD_CALLBACK_MSD347STATUS                    = 26016,        //< 上抛状态数据
        CMD_CALLBACK_MSD347AUTO                      = 26017,        //< 自动选择

        //< (26100, 26200] k203发卡机
        CMD_K203_GETVER                              = 26101,        //< 读取动态库版本信息
        CMD_K203_RESET                               = 26102,        //< 复位读卡器
        CMD_K203_SETBAUD                             = 26103,        //< 设置卡机波特率
        CMD_K203_CHECKPOSITION                       = 26104,        //< 查询卡片在卡机内的位置
        CMD_K203_CHECKSTATUS                         = 26105,        //< 查看传感器状态
        CMD_K203_ENTER                               = 26106,        //< 使能进卡
        CMD_K203_SETENTER                            = 26107,        //< 设置进卡
        CMD_K203_EOT                                 = 26108,        //< 取消设置
        CMD_K203_MOVE                                = 26109,        //< 移动卡片
        CMD_K203_EJECT                               = 26110,        //< 弹卡
        CMD_K203_AUTOTESTIC                          = 26111,        //< 自动检测接触式IC
        CMD_K203_SIMCOLDRESET                        = 26112,        //< 激活sim卡（固定电压5V）
        CMD_K203_SIMOFF                              = 26113,        //< sim卡下电
        CMD_K203_SIMT0APDU                           = 26114,        //< T = 0的sim卡命令传送
        CMD_K203_SIMT1APDU                           = 26115,        //< T = 1的sim卡命令传送
        CMD_K203_SIMSELECT                           = 26116,        //< sim卡座选择
        CMD_K203_SIMWARMRESET                        = 26117,        //< 激活sim卡（电压可选）
        CMD_K203_SIMSEND                             = 26118,        //< sim卡发卡
        CMD_K203_READ                                = 26119,        //< 读卡
        CMD_CALLBACK_K203AUTO                        = 26120,        //< 自动选择

        //< (26200, 26300] 配置模块
        CMD_CONFIG_MODIFY                            = 26201,        //< 修改数据
        CMD_CONFIG_ADD                               = 26202,        //< 增加数据
        CMD_CONFIG_ANAL                              = 26203,        //< 解析数据
        CMD_CONFIG_CALLBACKINFO                      = 26204,        //< 上抛解析数据
        CMD_CONFIG_AUTO                              = 26205,        //< 自动选择
        CMD_CONFIG_LOG                               = 26206,        //< 读取并上抛日志文件信息

        //< (26300, 26400] 分辨率模块
        CMD_SCREEN_INFO                              = 26301,        //< 查询屏幕分辨率
        CMD_SCREEN_MODIFY                            = 26302,        //< 修改屏幕分辨率
        CMD_SCREEN_CALLBACKINFO                      = 26303,        //< 上抛分辨率信息
        CMD_SCREEN_CALLBACKAUTO                      = 26304,        //< 自动选择
        CMD_SCREEN_DELCACHE                          = 26305,        //< 删除ie缓存文件夹

        //< (26500, 26600] 语音模块
        CMD_AUDIO_CAPTURE_RUN                        = 26501,        //< 开启语音识别
        CMD_AUDIO_CAPTURE_STOP                       = 26502,        //< 关闭语音识别

        //< (26600, 26700] http请求模块
        CMD_HTTP_SERVICE                             = 26601,        //< HTTP请求服务
        CMD_HTTP_SET_URL                             = 26602,        //< 设置url
        CMD_HTTP_SET_HEAD                            = 26603,        //< 设置Htttp的头部内容
        CMD_HTTP_SET_CONTENT                         = 26604,        //< 设置Htttp的Content
        CMD_HTTP_REQUEST                             = 26605,        //< 上抛HTTP返回结果
        CMD_HTTP_AUTO                                = 26606,        //< 自动选择

        //< (26800, 26900]
        CMD_MEDICAL_INSURANCE_LOGIN                  = 26801,        //< 登录医保接口
        CMD_MEDICAL_INSURANCE_LOGIN_RESPONSE         = 26802,        //< 登录医保接口(返回响应),上抛返回结果
        CMD_MEDICAL_INSURANCE_AUTO                   = 26803,        //< 自动选择

        //< (26700, 26800]梁山县自助终端收银系统
        CMD_UMS_INIT                                 = 26701,        //< 初始化UMS系统
        CMD_UMS_READ_CARD                            = 26702,        //< 读卡
        CMD_UMS_GET_PIN                              = 26703,        //< 开始获取pin密文
        CMD_UMS_TRANS_CARD                           = 26704,        //< 发送交易
        CMD_UMS_EJECT_CARD                           = 26705,        //< 退卡
        CMD_UMS_END                                  = 26706,        //< 结束操作
        CMD_UMS_GET_PRESS                            = 26707,        //< 获取键值
        CMD_UMS_GET_PINWORD                          = 26708,        //< 获取pin密文

        /// (25000, 28000] for hardware Driver/Module End

        //< (28000, 28100] 语音模块
        CMD_AUDIO_ASR                                = 28001,        //< 科大ASR(string形式)
        CMD_AUDIO_JSON_ASR                           = 28002,        //< 科大ASR(json形式)
        CMD_AUDIO_JSON_NLP                           = 28003,        //< NLP(json形式)
        CMD_AUDIO_ASR_MODE                           = 28004,        //< 科大ASR模式
        CMD_AUDIO_ASR_BEGIN                          = 28005,        //< 开始语音识别
        CMD_AUDIO_ASR_END                            = 28006,        //< 结束语音识别
        CMD_AUDIO_CONTEXT_CLEAR                      = 28007,        //< 清除语义上下文
        CMD_AUDIO_PARAMETER_SET                      = 28008,        //< 前端点击转发事件
        CMD_AUDIO_AUTO                               = 28100,        //< 自动选择
        CMD_AUDIO_ASR_ACCENT                         = 28101,        //< 科大ASR简体中文方言
        CMD_AUDIO_TTS_MODE                           = 28102,        //< 科大TTS模式
        CMD_AUDIO_TTS_SPEAKER                        = 28103,        //< 科大TTS发音人

        //< (29000, 29100]] 金融业务模块
        CMD_FINANCIAL_REQUEST_YE001                  = 29001,        //< 金融业务请求（余额查询）

        //< (29100, 29200]] C1C2通信模块
        CMD_ARTIFICIAL_AUXILIARY_OPEN                  = 29101,        //< 发送
        CMD_ARTIFICIAL_AUXILIARY_CLOSE                 = 29102,        //< 接收
        CMD_ARTIFICIAL_AUXILIARY_SEND                  = 29103,        //< 发送
        CMD_ARTIFICIAL_AUXILIARY_RECV                  = 29104,        //< 接收
        CMD_ARTIFICIAL_AUXILIARY_LOGIN                 = 29105,        //< 登录
        CMD_ARTIFICIAL_AUXILIARY_LOGOUT                = 29106,        //< 登出
        CMD_ARTIFICIAL_AUXILIARY_CALL                  = 29107,        //< 呼叫
        CMD_ARTIFICIAL_AUXILIARY_HANGUP                = 29108,        //< 挂断
        CMD_ARTIFICIAL_AUXILIARY_PASS                  = 29109,        //< 透传


        /// use for callback (30000, 40000]
        CMD_FACE_RCGN_1_1_SET_BASE                     = 30001,        //< 人脸1比1基数设置
        CMD_FACE_RCGN_1_1_CLEAR_BASE                   = 30002,        //< 人脸1比1基数清除
        CMD_FACE_RCGN_1_1_CMP_BY_ID                    = 30003,        //< 人脸1比1比对

        // 前端视频采集
        CMD_WEB_VIDEO_CAPTURE                          = 30004,

        // 二维码扫描
        CMD_QRCODE_SCAN                                = 30005,
        CMD_ET58_QRCODE                                = 25545,
    };

    /**
      *@brief 事件及状态枚举,如果是事件，以EVENT开头，如果是状态以STATUS开头
      */
    enum EventStatus
    {

        //播放器事件及状态
        /// use for audio play driver (10020, 10040]
        EVENT_AUDIO_PALYER_STATUS                   = 10020,     ///< 录音机状态事件
        STATUS_AUDIO_PALYER_STATUS_CLOSE            = 10021,
        STATUS_AUDIO_PALYER_STATUS_CLOSE_OPEN       = 10022,
        STATUS_AUDIO_PALYER_STATUS_CLOSE_START      = 10023,
        STATUS_AUDIO_PALYER_STATUS_CLOSE_STOP       = 10024,
        STATUS_AUDIO_PALYER_STATUS_CLOSE_ERROR      = 10025,

        EVENT_AUDIO_PALY_DONE                       = 10030  ///< 录音机播放完成事件

    };

    /**
     *读卡器属性类型//NOLINT
    */
    enum CardReaderProperty
    {
        PROPERTY_PORT,                ///< 串口号        //NOLINT
        PROPERTY_BAUDRATE,            ///< 波特率        //NOLINT
        PROPERTY_TYPE,                ///< 进卡类型      //NOLINT
        PROPERTY_ALL                  ///< 全部属性      //NOLINT
    };

    /**
     *串口属性设置//NOLINT
    */
    enum SerialPortProperty
    {
        PROPERTY_PORTS,                ///< 串口号        //NOLINT
        PROPERTY_BAUDRATES,            ///< 波特率        //NOLINT
        PROPERTY_PARITY,               ///< 奇偶校验      //NOLINT
        PROPERTY_STOPBITS,             ///< 停止位        //NOLINT
        PROPERTY_BYTESIZE,             ///< 数据位        //NOLINT
        PROPERTY_MODE,                 ///< 异步方式      //NOLINT
        PROPERTY_ALLS                  ///< 全部属性      //NOLINT
    };

    /**
     *手势种类设置//NOLINT
    */
    enum ArmPoseType
    {
        EnterPose,                    ///< 进入手势
        LeavePose,                    ///< 离开手势
        HandShakePose,                ///< 握手手势
        TouchPose,                    ///< 摸后脑勺手势
        PleasePose,                   ///< 请进手势
        QuietnessPose                 ///< 安静手势
    };

    ///定义数据类型：文本类型，数据流，对象
#define DTSTR_NULL      "null"       ///< 数据为空
#define DTSTR_TEXT_PURE "text_pure"  ///< 纯文本，不带任何格式，以'\0'结尾
#define DTSTR_TEXT_JSON "text_json"  ///< json格式的文本，以'\0'结尾
#define DTSTR_TEXT_XML  "text_xml"  ///< xml格式的文本，以'\0'结尾
#define DTSTR_TEXT_JSON_EXTRACT "text_json_extract@x"   ///< 输入数据格式是json,但需要提取@后面指定的段中的数据作为有效数据


#define DTSTR_STREAM_PCMX1X               "stream_pcm@x@1@x"  ///< 流数据，音频数据pcm
#define DTSTR_STREAM_PCMX1X_P2          "stream_pcm@x@1@x@p2"  ///< 流数据，音频数据pcm,但在音频数据的前面会有2字节的控制说明符（第1字节 表示是否是最后一个音频数据，第2字节保留）
#define DTSTR_STREAM_FRAME_RGB24      "stream_frame_rgb@24"   ///< 视频帧数据,rgb24位方案
#define DTSTR_STREAM_FRAME_RGB24_P6   "stream_frame_rgb@24@p6"   ///< 视频帧数据，rgb24位方案，帧数据前面会有8字节的控制说明符（第1字节表示是否是最后一个视频数据，第2字节表示通道数，第3,4字节表示宽度，第5，6字节表示长度）
#define DTSTR_STREAM_BINARY           "stream_binary"            ///< 二进制流数据


#define DTSTR_OBJECT_FaceDetectInfoV1_P1    "object_facedetectinfo1@p1"   ///< 对象数据，结构为 FaceDetectInfo1, 在对象数据的前面会有1个字节表示有多少个对象
#define DTSTR_OBJECT_FaceRegisterResult1nV1_P1 "object_faceregisterresult1nv1@p1"   ///< 对象数据，结构为FaceRegisterResult1nV1，在对象数据有前面会有1上字节表示有多少个对象


    ///定义能力类型
    //语音模块相关能力
#define ABSTR_NULL                     "null"    ///< 无能力
#define ABSTR_ASR                      "asr"    ///< 语音识别能力，只支持转写，不支持语义理解
#define ABSTR_ASRNLP                "asrnlp"    ///< 语音识别能力，支持转写，支持语义理解
#define ABSTR_NLP                      "nlp"    ///< 语义理解
#define ABSTR_TTS                      "tts"    ///< 语音合成
#define ABSTR_VOICERECORD      "voicerecord"    ///< 录音功能
#define ABSTR_VOICEPLAY          "voiceplay"    ///< 声音播放功能

    //视频相关的功能
#define ABSTR_VIDEOCAPTURE    "videocapture"    ///< 摄像头帧数据捕获
#define ABSTR_FACEDETECT      "facedetect"      ///< 人脸检测
#define ABSTR_FACECOMPARISION11  "facecomparision11"   ///< 人脸比对 1 比1
#define ABSTR_FACECOMPARISION1N  "facecomparision1n"   ///< 人脸比对 1 比n
#define ABSTR_FACERECOGNITION  "facerecognition"       ///< face recognition
#define ABSTR_FACEDETECT_RECOGNITION  "facedetect_recogniton"   ///< 人脸检测和识别


    //RealSense相关功能
#define ABSTR_REALSENSE_TRACKINGBODY   "trackingbody"  ///< 人体跟踪
#define ABSTR_REALSENSE_VIDEOCAPTURE   "videocapture"  ///< 摄像头帧数据


    //机械手臂相关信息
#define SERVO_OVER_TIME 1000 ///< 读取串口超时时间

}

#endif
