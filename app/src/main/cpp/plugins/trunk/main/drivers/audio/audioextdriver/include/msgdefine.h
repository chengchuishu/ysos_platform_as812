#ifndef XFASR_MSGDEFINE_H  //NOLINT
#define XFASR_MSGDEFINE_H

/// 定义语音识别消息处理消息类型
#define MSG_REQ_GET_XFRESULT 100
#define MSG_REQ_STARTASREX 101
#define MSG_REQ_STOPASR 102
#define MSG_REQ_OPEN_AUDIO_CAPTURE_DEVICE 103  ///< 打开录音设备
#define MSG_REQ_CLOSE_AUDIO_CAPTURE_DVICE 104  ///< 关闭录音设备
#define MSG_REQ_NLP       105   /// TODO 应该支持多线程


/// 定义TTS消息处理类型
#define MSG_REQ_TTS_STARTTTS 150
#define MSG_REQ_TTS_STOPTTS  151
#define MSG_REQ_TTS_GETAUDIO 152

//定义一些其他
//#define MSG_REQ_OCX_OPTIMIZATION    180 ///< 定义一些OCX模块的优化工作

/// 定义返回时消息类型
//#define MSG_RES_ASR_ERROR                          WM_USER+201
//#define MSG_RES_ASR_TIMEOUT                        WM_USER+202
#define MSG_RES_ARSRETURNEX                          WM_USER+203
#define MSG_RES_ARSRETURN                            WM_USER+204   /// 为了兼容保留
#define MSG_RES_NLP_RESULT                           WM_USER+205

#define MSG_RES_TTS_PLAYDONE                         WM_USER+251
//#define MSG_RES_ARSRETURNEX WM_USER+204

#define MSG_RES_AUDIOCAPTURE_EVENT                   WM_USER + 300    ///< 录音机状态事件（打开设备，录音中。。。）
#define MSG_RES_ASRSTATUS_EVENT                      WM_USER + 301    ///< 语音识别状态事件（侦听中，识别中。。。）
#define MSG_RES_CHECK_DEVICE_RESULT_EVENT            WM_USER + 302    ///< 当前设备状态事件（正常，异常。。。）
#define MSG_RES_AUDIOPLAY_EVENT                      WM_USER + 303    ///< 播放器状态事件（播放中，播放完成。。。）
#define MSG_RES_AUDIOCAPTURE_EVENT_VOLUME_STRENGTH   WM_USER + 304    ///< 录音机录音强度事件


/// 定义消息的优先级
#define MSG_PRIORITY_MIN ThreadMsgPriorityMin
#define MSG_PRIORITY_COMMON ThreadMsgPriorityCommon
#define MSG_PRIORITY_MAX ThreadMsgPriorityMax

#define MSG_REQ_GET_XFRESULT_PRIORITY MSG_PRIORITY_MIN
#define MSG_REQ_STARTASR_PRIORITY MSG_PRIORITY_COMMON
#define MSG_REQ_STOPASR_PRIORITY (MSG_PRIORITY_COMMON-1)  ///< STOP 的优先级要高于 STARTASR
#define MSG_REQ_OPENDEVICE_PRIORITY (MSG_PRIORITY_COMMON)
#define MSG_REQ_CLOSEDVICE_PRIORITY (MSG_PRIORITY_COMMON-2)  ///< CLOSEDEVICE 的优先级要高于 STOP
#define MSG_REQ_NLP_PRIORITY (MSG_PRIORITY_COMMON)
#define MSG_REQ_TTS_GETAUDIO_PRIORITY (ThreadMsgPriorityMin)

#define MSG_REQ_TTS_STARTTTS_PRIORITY (MSG_PRIORITY_COMMON)
#define MSG_REQ_TTS_STOPTTS_PRIORITY (MSG_PRIORITY_COMMON -1)  /// STOP 的优先级要高于start

//#define MSG_REQ_OCX_OPTIMIZATION_PRIORITY (MSG_PRIORITY_COMMON)

//#define MSG_RES_ASR_ERROR_PRIORITY MSG_PRIORITY_COMMON
//#define MSG_RES_ASR_TIMEOUT_PRIORITY MSG_PRIORITY_COMMON
#define MSG_RES_ARSRETURNEX_PRIORITY (MSG_PRIORITY_COMMON)
#define MSG_RES_NLP_RESULT_PRIORITY (MSG_PRIORITY_COMMON)

#define MSG_RES_TTS_PLAYDONE_PRIORITY (MSG_PRIORITY_COMMON)

#endif
