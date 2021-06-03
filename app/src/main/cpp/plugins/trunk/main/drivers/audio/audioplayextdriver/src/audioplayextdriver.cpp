/**
 *@file audioplayextdriver.cpp
 *@brief audio play ext driver
 *@version 1.0
 *@author l.j..
 *@date Created on: 2016-10-20 15:16:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

/// self header
#include "../include/audioplayextdriver.h"
#include "../include/play_voice.h"

/// boost headers
#include <boost/thread/thread_guard.hpp>

/// ysos headers
#include "../../../../../../core/trunk/main/public/include/sys_interface_package/bufferpool.h"
#include "../../../../../../core/trunk/main/public/include/sys_interface_package/callbackqueue.h"
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"
#include "../../../../../../core/trunk/main/public/include/sys_interface_package/ysos_struct.h"
#include "../../../../../../core/trunk/main/public/include/sys_interface_package/moduleinterface.h"
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basemoduleimpl.h"


namespace ysos {

#ifdef _WIN32
/// msg id for local thread process wim data message
#define WM_MSG_WINDOWSAUDIOCAPTUREDRIVE_WMOUTDONE     WM_USER+1000
#define WM_MSG_WINDOWSAUDIOCAPTUREDRIVE_THREADEXIT    WM_USER+1001

/**
 *@brief callback function. call it when audio have play finish
 *@param
 *@return
 */
typedef void (*AudioPlayCallBackDone)();

/**
 *@brief callback function. call it when audio status has switch
 *@param iEventId[Input] status event id
 *@return
 */
typedef void (*AudioPlayCallBackEvenFun)(unsigned int iEventId);

/**
 *@brief convert windows wave error type to tangdi error type
 *@param uWindowsErrorType windows wave error type
 *@return project error type define,please reference file "error.h"
 */
#ifdef CONVERWINDOWSWAVEERRORTYPETOTDERRORTYPE
extern UINT32 ConverWindowsWaveErrorTypeToTDErrorType(const UINT32 uWindowsErrorType);
#else
#define CONVERWINDOWSWAVEERRORTYPETOTDERRORTYPE
UINT32 ConverWindowsWaveErrorTypeToTDErrorType(const UINT32 uWindowsErrorType) {
  UINT32 result = YSOS_ERROR_FAILED;

  switch (uWindowsErrorType) {
  case MMSYSERR_NOERROR: {
    result = YSOS_ERROR_SUCCESS;
    break;
  }
  case MMSYSERR_INVALHANDLE: {
    result = YSOS_ERROR_INVALID_ARGUMENTS;
    break;
  }  ///< Specified device handle is invalid.
  case MMSYSERR_NODRIVER: {
    result = YSOS_ERROR_NOT_ENOUGH_RESOURCE;  ///< No device driver is present.
    break;
  }
  case MMSYSERR_BADDEVICEID: {
    result = YSOS_ERROR_INVALID_ARGUMENTS;  ///< Specified device identifier is out of range.
    break;
  }
  case MMSYSERR_NOMEM: {
    result = YSOS_ERROR_MEMORY_ALLOCATED_FAILED;  ///< Unable to allocate or lock memory.
    break;
  }
  case WAVERR_BADFORMAT: {
    result = YSOS_ERROR_LOGIC_ERROR;  ///< Attempted to open with an unsupported waveform-audio format
    break;
  }
  case WAVERR_STILLPLAYING: {
    result = YSOS_ERROR_LOGIC_ERROR;  ///< The buffer pointed to by the pwh parameter is still in the queue.
    break;
  }
  case WAVERR_UNPREPARED: {
    result = YSOS_ERROR_LOGIC_ERROR;  ///< The buffer pointed to by the pwh parameter hasn't been prepared.
    break;
  }
  default: {
    break;
  }
  }

  return result;
}
#endif

/**
 *@brief windows wave recorder call back function
 *       NOTICE:Applications should not call any system-defined functions from inside a callback function,
 *       except for EnterCriticalSection, LeaveCriticalSection, midiOutLongMsg, midiOutShortMsg, OutputDebugString,
 *       PostMessage, PostThreadMessage, SetEvent, timeGetSystemTime, timeGetTime, timeKillEvent, and timeSetEvent.
 *       Calling other wave functions will cause deadlock.
 *@param hwi Handle to the waveform-audio device associated with the callback function.
 *@param uMsg Waveform-audio input message. It can be one of the following messages.
 *@param dwInstance User instance data specified with waveInOpen.
 *@param dwParam1 Message parameter.
 *@param dwParam2 Message parameter.
 *@return
 */
void CALLBACK waveOutCallbackProc(
  HWAVEOUT   hwi,
  UINT      uMsg,
  DWORD_PTR dwInstance,
  DWORD_PTR dwParam1,
  DWORD_PTR dwParam2
) {
  AudioPlayExtDriver* paudio_driver = reinterpret_cast<AudioPlayExtDriver*>(dwInstance);
  switch (uMsg) {
  case WOM_OPEN: {
    YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.audioplayext"), "WOM_OPEN");
  }
  break;

  case WOM_DONE: { ///< 音频输出结束
    if (paudio_driver->is_reseting_close() || paudio_driver->is_reseting_stop()) { ///< 如果是close,则不再下发
      break;
    }
    WAVEHDR* pwavehdr = reinterpret_cast<WAVEHDR*>(dwParam1);
    PostThreadMessage(paudio_driver->process_callback_wave_threadid(), WM_MSG_WINDOWSAUDIOCAPTUREDRIVE_WMOUTDONE,
                      reinterpret_cast<WPARAM>(pwavehdr), reinterpret_cast<LPARAM>(hwi));
  }
  break;
  case WOM_CLOSE: {
    YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.audioplayext"), "WOM_CLOSE");
  }
  break;
  }
}

/**
 *@brief thread function for process windows api callback WIN_DATA message;
 *@param lParam pointer to WindowsAudioCaptureDriver object;
 *@return success return 0
 *        fail return no zero value
 */
UINT ThreadProcessWaveDone(LPARAM lParam) {
  AudioPlayExtDriver* pdriver  = reinterpret_cast<AudioPlayExtDriver*>(lParam);
  MSG msg;
  while (1) {
    if (PeekMessage(&msg, NULL, WM_MSG_WINDOWSAUDIOCAPTUREDRIVE_WMOUTDONE, WM_MSG_WINDOWSAUDIOCAPTUREDRIVE_THREADEXIT, PM_NOREMOVE)) {
      switch (msg.message) {
      case WM_MSG_WINDOWSAUDIOCAPTUREDRIVE_WMOUTDONE: {
        GetMessage(&msg, NULL, WM_MSG_WINDOWSAUDIOCAPTUREDRIVE_WMOUTDONE, WM_MSG_WINDOWSAUDIOCAPTUREDRIVE_WMOUTDONE);
        WAVEHDR* pwavehdr = reinterpret_cast<WAVEHDR*>(msg.wParam);
        HWAVEOUT hwaveout = reinterpret_cast<HWAVEOUT>(msg.lParam);
        //VoiceDataBlock voice_block;
        //pdriver->GetVoiceDataBlock(pwavehdr->lpData, voice_block);
        //if (voice_block.is_end_) {
        //}
        pdriver->ProcessWaveDone(pwavehdr);
      }
      break;

      case WM_MSG_WINDOWSAUDIOCAPTUREDRIVE_THREADEXIT: {
        YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.audioplayext"), "proc WM_MSG_WINDOWSAUDIOCAPTUREDRIVE_THREADEXIT");
        GetMessage(&msg, NULL, msg.message, msg.message);
        return 0;
      }
      }  ///< end switch
    }  ///< end if
    else {
      Sleep(1);
    }
  }  ///< end while
}

int AudioPlayExtDriver::ProcessWaveDone(WAVEHDR* pWavehdr) {
  int result = YSOS_ERROR_FAILED;

  do {
    MMRESULT mmret=waveOutUnprepareHeader(hwave_handle_, pWavehdr, sizeof(WAVEHDR));
    if (MMSYSERR_NOERROR != mmret) {
      YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.audioplayext"), "waveOutUnprepareHeader fail");
    }

    ///< 判断是否要向上层抛出事件
    VoiceDataBlock voice_block;
    if (YSOS_ERROR_SUCCESS != GetVoiceDataBlock(pWavehdr->lpData, voice_block)) {
      YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.audioplayext"), "GetVoiceDataBlock fail");
      break;
    }

    RemoveFormPlayList(pWavehdr->lpData);

    ///< 如果是最后一声音频，则才向上抛出事件
    if (true == voice_block.is_end_) {
      YSOS_LOG_DEBUG("================= 3");
      status_fun_("10024", "audio play end event");
      send_start_ = true;
      ///< 如果没有数据还需要再播放的，则表示当前没有再要播放的了，即停止播放了
      {
        boost::lock_guard<boost::mutex> lock_guard_voice_data(mutext_playsound_voice_datas_);
        if (playsound_voice_datas_.size() <= 0) {
          device_current_status_atomic_ = TDAPS_STOP;
        }
      }
    }

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  return result;
}
#else
#endif

DECLARE_PLUGIN_REGISTER_INTERFACE(AudioPlayExtDriver, DriverInterface);
AudioPlayExtDriver::AudioPlayExtDriver(const std::string &strClassName) : BaseDriverImpl(strClassName) {
  //hwave_handle_= NULL;
  //wformat_tag_ = WAVE_FORMAT_PCM;
  //cbsize_ = 0;
  //nsamples_per_sec_ = 16000;
  //nchannels_ = 1;
  //wbits_per_sample_ = 16;
  //nblock_align_ = (wbits_per_sample_ * nchannels_) / 8;
  //navg_bytes_per_sec_ = nblock_align_ * nsamples_per_sec_;
  uwave_buffer_data_size_pernum_ = 1024;
  uwave_buffer_num_ =4;
  //dwprocess_callback_wave_threadid_ =0;
  device_current_status_atomic_ = TDAPS_CLOSE;
  bis_reseting_stop_ = false;
  bis_reseting_close_ = false;
  send_start_ = true;
  logger_ = GetUtility()->GetLogger("ysos.audioplayext");
}

AudioPlayExtDriver::~AudioPlayExtDriver() {
}

int AudioPlayExtDriver::RealUnInitialize(void *param) {
  YSOS_LOG_DEBUG("in ~AudioPlayExtDriver UnInitialize");

  {
    YSOS_LOG_DEBUG("清除回调 播放设备 状态 队列");
    boost::lock_guard<boost::mutex> lockguard_eventcallback(mutex_event_callback_);
    vecevent_callback_.clear();
  }

  ///< 清除回调函数队列, 防止再次回调上层
  {
    YSOS_LOG_DEBUG("清除回调 音频播放完成件 函数队列");
    boost::lock_guard<boost::mutex> lock_guard_eventcallback(mutex_veproperity_onread_callback_);
    veproperity_onread_callback_.clear();
  }

  Close();

  {
    ///< 清空voice player status
    boost::lock_guard<boost::mutex> lock_guard_notify(mutex_list_baseinterface_event_notify_voiceplayerstatus_);
    list_baseinterface_event_notify_voiceplayerstatus_.clear();
  }
  {
    ///< 清空playdone
    boost::lock_guard<boost::mutex> lock_guard_notify(mutext_list_baseinterface_event_notify_playdone_);
    list_baseinterface_event_notify_playdone_.clear();
  }

  return YSOS_ERROR_SUCCESS;
}

int AudioPlayExtDriver::Read(BufferInterfacePtr pBuffer, DriverContexPtr context_ptr) {
  return YSOS_ERROR_SUCCESS;
}

int AudioPlayExtDriver::Write(BufferInterfacePtr input_buffer, BufferInterfacePtr output_buffer, DriverContexPtr context_ptr) {
  return YSOS_ERROR_SUCCESS;
}

int AudioPlayExtDriver::Ioctl(int iCtrlID, BufferInterfacePtr pInputBuffer, BufferInterfacePtr pOutputBuffer) {
  int result = YSOS_ERROR_FAILED;

  Properties property = static_cast<Properties>(iCtrlID);
  switch (property) {
  case CMD_ICC_START: { ///< 1 start record
    // return Start();
    result = YSOS_ERROR_SUCCESS;
  }
  break;

  case CMD_ICC_STOP: {
    if (NULL == pInputBuffer) {
      YSOS_LOG_DEBUG("error exectue CMD_ICC_STOP, the param is null ");
      break;
    }

    UINT8* pbuff = NULL;
    UINT32 buffer_size =0;
    pInputBuffer->GetBufferAndLength(&pbuff,&buffer_size);
    if (NULL == pbuff || buffer_size <= 0) {
      YSOS_LOG_DEBUG("error exectue CMD_ICC_STOP, the param is null ");
      break;
    }
    std::string alibity_name = reinterpret_cast<char*>(pbuff);
    YSOS_LOG_DEBUG(" call command CMD_ICC_STOP: ability:" << alibity_name);
    if (strcasecmp(alibity_name.c_str(), ABSTR_VOICEPLAY) == 0) {
      result = Stop();
    }
  }
  break;

  case CMD_INPUT_PLAY_AUDIO_DATA: {
    if (NULL == pInputBuffer) {
      YSOS_LOG_DEBUG("error ,CMD_INPUT_PLAY_AUDIO_DATA inputbuffer is null");
      break;
    }
    UINT8* wave_buffer = NULL;
    UINT32 buffer_size = 0;
    pInputBuffer->GetBufferAndLength(&wave_buffer, &buffer_size);
    AddToPlayList(wave_buffer, buffer_size, 0);
  }
  break;

  default: {
    YSOS_LOG_DEBUG("unsupport ability");
  }
  }  ///< end switch

  return result;
}

int AudioPlayExtDriver::Open(void *pParams) {
  YSOS_LOG_DEBUG("in AudioPlayExtDriver::Open [Enter]");
  int result = YSOS_ERROR_FAILED;

  do {
    boost::lock_guard<boost::mutex> lock_guard_driver(mutext_audio_driver_);

    if (TDAPS_OPEN == device_current_status_atomic_) {
      YSOS_LOG_DEBUG("当前已经是打开状态，返回成功");
      result = YSOS_ERROR_SUCCESS;
      break;
    }

    if (TDAPS_CLOSE != device_current_status_atomic_) {
      YSOS_LOG_DEBUG("打开失败，上一次打开没有被关闭");
      break;
    }
    YSOS_LOG_DEBUG("in AudioPlayExtDriver::Open [Check][0]");
#ifdef _WIN32
    WAVEFORMATEX wave_format;
    wave_format.wFormatTag = wformat_tag_;
    wave_format.nChannels = nchannels_;
    wave_format.nSamplesPerSec = nsamples_per_sec_;
    wave_format.nAvgBytesPerSec = navg_bytes_per_sec_;
    wave_format.nBlockAlign = nblock_align_;
    wave_format.wBitsPerSample = wbits_per_sample_;
    wave_format.cbSize = cbsize_;
    ///< 打开音频输出设备
    MMRESULT mmret =
      waveOutOpen(
        &hwave_handle_,  ///< Pointer to a buffer that receives a handle identifying the open waveform-audio input device. Use this handle to identify the device when calling other waveform-audio input functions. This parameter can be NULL if WAVE_FORMAT_QUERY is specified for fdwOpen.  //NOLINT
        WAVE_MAPPER,     ///< Identifier of the waveform-audio input device to open. It can be either a device identifier or a handle of an open waveform-audio input device. You can use the following flag instead of a device identifier.WAVE_MAPPER:The function selects a waveform-audio input device capable of recording in the specified format.  //NOLINT
        &wave_format,    ///< Pointer to a WAVEFORMATEX structure that identifies the desired format for recording waveform-audio data. You can free this structure immediately after waveInOpen returns.  //NOLINT
        reinterpret_cast<DWORD>(waveOutCallbackProc),  ///< Pointer to a fixed callback function, an event handle, a handle to a window, or the identifier of a thread to be called during waveform-audio recording to process messages related to the progress of recording. If no callback function is required, this value can be zero. For more information on the callback function, see waveInProc.  //NOLINT
        reinterpret_cast<DWORD>(this),                ///< User-instance data passed to the callback mechanism. This parameter is not used with the window callback mechanism.  //NOLINT
        CALLBACK_FUNCTION);                             ///< Flags for opening the device. The following values are defined.  //NOLINT
    switch (mmret) {
    case MMSYSERR_NOERROR:  {
      YSOS_LOG_DEBUG("waveOutOpen success");
    }
    break;
    case MMSYSERR_ALLOCATED:    ///< Specified resource is already allocated.
    case MMSYSERR_BADDEVICEID:  ///< Specified device identifier is out of range.
    case MMSYSERR_NODRIVER:     ///< No device driver is present.
    case MMSYSERR_NOMEM:        ///< Unable to allocate or lock memory.
    case WAVERR_BADFORMAT:      ///< Attempted to open with an unsupported waveform-audio format
      return ConverWindowsWaveErrorTypeToTDErrorType(mmret);
    default: {
      YSOS_LOG_DEBUG("waveOutOpen error result");
      return result;
    }
    }

    if (0 == dwprocess_callback_wave_threadid_) {
      HANDLE hthreadhandle = CreateThread(NULL, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(ThreadProcessWaveDone),
                                          this, CREATE_SUSPENDED, &dwprocess_callback_wave_threadid_);
      ResumeThread(hthreadhandle);
      CloseHandle(hthreadhandle);
      hthreadhandle = NULL;
    } else {
      YSOS_LOG_DEBUG("处理语音块线程已启动，不再启用");
    }
#else
    //TODO:add for linux
    result = device_voide_init();
    YSOS_LOG_DEBUG("in AudioPlayExtDriver::Open [Check][1]");
    if (0 == result) {
      YSOS_LOG_DEBUG("waveOutOpen success");
    } else {
      /* code */
      YSOS_LOG_ERROR("waveOutOpen failed !!! [result]=" << result);
    }
#endif
    YSOS_LOG_DEBUG("in AudioPlayExtDriver::Open out");
    device_current_status_atomic_ = TDAPS_OPEN;
    
    result = YSOS_ERROR_SUCCESS;
  } while (0);
  YSOS_LOG_DEBUG("in AudioPlayExtDriver::Open [Check][2]");
  return result;
}

void AudioPlayExtDriver::Close(void *pParams) {
  YSOS_LOG_DEBUG("in AudioPlayExtDriver::Close [Enter]");
  if (TDAPS_START == device_current_status_atomic_) {
    YSOS_LOG_DEBUG("当前设备在录音，先尝试stop");
    Stop();  ///< 注意，此函数中调用了 mutext_audio_driver
  }

  boost::lock_guard<boost::mutex> lock_guard_driver(mutext_audio_driver_);

  YSOS_LOG_DEBUG("in AudioPlayExtDriver::Close [Check][0]");
  if (TDAPS_CLOSE == device_current_status_atomic_) {
    YSOS_LOG_DEBUG("当前设备已关闭，不用再关闭，返回成功");
    return;
  }

  if (TDAPS_START == device_current_status_atomic_) {
    YSOS_LOG_DEBUG("关闭失败,当前设备是start 没有被stop");
    return;
  }
  YSOS_LOG_DEBUG("in AudioPlayExtDriver::Close [Check][1]");
  set_is_reseting_close(true);

#ifdef _WIN32  
  ///< The waveInReset function stops input on the given waveform-audio input device and resets the current position
  ///< to zero. All pending buffers are marked as done and returned to the application.
  ///< 经过测试，好像是所的有 All pending buffers are marked as done and returned to the application后才返回
  MMRESULT mmret = waveOutReset(hwave_handle_);
  set_is_reseting_close(false);
  ClearPlayList();
  if (MMSYSERR_NOERROR != mmret) {
    YSOS_LOG_DEBUG("waveOutReset failed");
  }
  YSOS_LOG_DEBUG("waveOutReset success");
  /// If there are input buffers that have been sent with the waveInAddBuffer function and that haven't been returned
  /// to the application, the close operation will fail. Call the waveInReset function to mark all pending buffers
  /// as done.
  mmret = waveOutClose(hwave_handle_);
  if (MMSYSERR_NOERROR != mmret) {
    YSOS_LOG_DEBUG("waveOutClose failed:" <<mmret);
    return;
  }
  hwave_handle_ = NULL;
  YSOS_LOG_DEBUG("waveOutClose succ");
  YSOS_LOG_DEBUG("向线程发送退出消息-begin");
  ///< 向线程发送退出消息
  if (0 != dwprocess_callback_wave_threadid_) {
    YSOS_LOG_DEBUG("向线程发送退出消息-end");
    PostThreadMessage(dwprocess_callback_wave_threadid_, WM_MSG_WINDOWSAUDIOCAPTUREDRIVE_THREADEXIT, NULL, NULL);
    dwprocess_callback_wave_threadid_ = NULL;
  }
#else
  //TODO:add for linux
  int ret = device_close();
  YSOS_LOG_DEBUG("in AudioPlayExtDriver::Close [Check][2]");
  set_is_reseting_close(false);
  ClearPlayList();
  if (0 != ret) {
    YSOS_LOG_DEBUG("waveOutReset failed");
  }
  YSOS_LOG_DEBUG("waveOutReset success");
  
#endif
  YSOS_LOG_DEBUG("in AudioPlayExtDriver::Close [Check][3]");
  device_current_status_atomic_ = TDAPS_CLOSE;
  return;
}

int AudioPlayExtDriver::Initialize(void *param) {
  return YSOS_ERROR_SUCCESS;
}

int AudioPlayExtDriver::GetProperty(int iTypeId, void *piType) {
  int result = YSOS_ERROR_FAILED;

  Properties property = static_cast<Properties>(iTypeId);
  switch (property) {
  case PROP_FUN_CALLABILITY: {
    FunObjectCommon2* pobject = reinterpret_cast<FunObjectCommon2*>(piType);
    if (NULL == pobject) {
      YSOS_LOG_DEBUG("error call property PROP_Fun_CallAbility, the param is null");
      break;
    }
    AbilityParam* pin = reinterpret_cast<AbilityParam*>(pobject->pparam1);
    AbilityParam* pout = reinterpret_cast<AbilityParam*>(pobject->pparam2);
    result = CallAbility(pin, pout);
    break;
  }

  default: {
    YSOS_LOG_DEBUG("unsupport ability");
  }
  }
  return result;
}

int AudioPlayExtDriver::SetProperty(int iTypeId, void *piType) {
  int result = YSOS_ERROR_FAILED;

  switch (iTypeId) {
  case PROP_INPUT_AUDIO_DATA_END: {
    result = SetPlayListEnd();
  }

  default: {
    YSOS_LOG_DEBUG("not support property,id:" << iTypeId);
    BaseDriverImpl::SetProperty(iTypeId, piType);
  }
  }  ///< end switch

  return result;
}

int AudioPlayExtDriver::CallAbility(AbilityParam* pInput, AbilityParam* pOut) {
  int result = YSOS_ERROR_FAILED;

  do {
    if (NULL == pInput || NULL == pOut) {
      YSOS_LOG_DEBUG("error call ablicity , input/out alibityparam is null");
      break;
    }

    YSOS_LOG_DEBUG("pOut->ability_name:" <<(pOut ? pOut->ability_name : "null") <<" pInput->data_type_name:"<< (pInput ? pInput->data_type_name : "null"));
    if (strcasecmp(ABSTR_VOICEPLAY, pOut->ability_name) ==0) {
      if (strcasecmp(DTSTR_STREAM_PCMX1X_P2, pInput->data_type_name) ==0) {
        UINT8* pindata = NULL;
        UINT32 buffer_size = 0;
        pInput->buf_interface_ptr->GetBufferAndLength(&pindata, &buffer_size);

        int isfinish = *pindata;

        ///<都是打断模式，字段无作用
        //int interrupt_type = atoi((char*)(pindata + 1)); ///< 打断类型 0：打断 1：不打断
        //YSOS_LOG_DEBUG("audio play get interrupt type = " << interrupt_type);

        if (buffer_size <= 2) {
          YSOS_LOG_DEBUG("error buffer data");
          break;
        }
        {
          ///< 加一个判断，是否需要清空以前的待播放的数据
          static bool need_clear = false;
          if (need_clear) {
            YSOS_LOG_DEBUG("play stop, clear pervious play data");
            Stop();
          }
          if (isfinish) {
            need_clear = true;
          } else {
            need_clear = false;
          }
        }
        AddToPlayList(pindata +2, buffer_size-2, isfinish); ///< 添加到队列后立即播放
        Start();
      } else if (strcasecmp(DTSTR_STREAM_PCMX1X, pInput->data_type_name) ==0) {
        UINT8* pindata = NULL;
        UINT32 buffer_size = 0;
        pInput->buf_interface_ptr->GetBufferAndLength(&pindata, &buffer_size);
        int isfinish = 0;
        AddToPlayList(pindata, buffer_size, isfinish);  ///< 添加到队列后立即播放
        Start();
      }
    } else {
      YSOS_LOG_DEBUG("error not support ability");
      break;
    }

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  return result;
}

int AudioPlayExtDriver::AddToPlayList(void* pData, int iDataLength, bool bIsEnd) {
  boost::lock_guard<boost::mutex> lock_guard_voice_data(mutext_playsound_voice_datas_);
  int result = YSOS_ERROR_FAILED;

  do {
    if (NULL == pData || iDataLength <= 0) {
      break;
    }

    try {
      VoiceDataBlock voiceblock;
      voiceblock.voice_buffer_ptr_.reset(new char[iDataLength]);
      memset(voiceblock.voice_buffer_ptr_.get(),0,iDataLength);
      memcpy(voiceblock.voice_buffer_ptr_.get(),pData,iDataLength);

      voiceblock.data_size_ = iDataLength;
      voiceblock.is_end_ = bIsEnd;

      voiceblock.wave_ptr_.reset(new WAVEHDR);
      memset(voiceblock.wave_ptr_.get(),0,sizeof(WAVEHDR));
      voiceblock.wave_ptr_->lpData = voiceblock.voice_buffer_ptr_.get();
      voiceblock.wave_ptr_->dwBufferLength = iDataLength;

      if (NULL == voiceblock.wave_ptr_ || NULL == voiceblock.voice_buffer_ptr_) {
        break;
      }
      playsound_voice_datas_.push_back(voiceblock);

      result = YSOS_ERROR_SUCCESS;
    } catch (...) {
      YSOS_LOG_DEBUG("AddToPlayList allow mem fail");
      break;
    }
  } while (0);

  return result;
}

int AudioPlayExtDriver::SetPlayListEnd() {
  boost::lock_guard<boost::mutex> lock_guard_voice_data(mutext_playsound_voice_datas_);
  int result = YSOS_ERROR_FAILED;

  do {
    if (playsound_voice_datas_.size() <= 0)
      break;

    std::list<VoiceDataBlock>::reverse_iterator rit_voice_block = playsound_voice_datas_.rbegin();
    (*rit_voice_block).is_end_ = true;

    result = YSOS_ERROR_SUCCESS;
  } while (0);

  return result;
}

int AudioPlayExtDriver::ClearPlayList(int iClearMode) {
  boost::lock_guard<boost::mutex> lock_guard_voice_data(mutext_playsound_voice_datas_);
  if (0 == iClearMode) { ///< clear all
    playsound_voice_datas_.clear();
  } else if (1 == iClearMode) { ///< 1 clear that not add to system wave buffer
    std::list<VoiceDataBlock>::iterator it_voicedata = playsound_voice_datas_.begin();
    for (it_voicedata; it_voicedata != playsound_voice_datas_.end(); /*++it_voicedata*/) {
      VoiceDataBlock* pvoice_datablock = &(*it_voicedata);
      if (true == pvoice_datablock->have_add_to_sysem_to_wave_out_write_) {
        ++ it_voicedata;
        continue;
      }
      it_voicedata = playsound_voice_datas_.erase(it_voicedata);
    }
  }

  return YSOS_ERROR_SUCCESS;
}

int AudioPlayExtDriver::GetVoiceDataBlock(void* pDataAddress, VoiceDataBlock& VoiceBlock) {
  boost::lock_guard<boost::mutex> lock_guard_voice_data(mutext_playsound_voice_datas_);

  int result = YSOS_ERROR_FAILED;
  std::list<VoiceDataBlock>::iterator it_voicedata = playsound_voice_datas_.begin();
  for (it_voicedata; it_voicedata != playsound_voice_datas_.end(); ++it_voicedata) {
    VoiceDataBlock* pvoice_datablock = &(*it_voicedata);
    if (pvoice_datablock->voice_buffer_ptr_.get() == pDataAddress) {
      VoiceBlock = *pvoice_datablock;
      result = YSOS_ERROR_SUCCESS;
      break;
    }
  }

  return result;
}

int AudioPlayExtDriver::RemoveFormPlayList(void* pDataAddress) {
  boost::lock_guard<boost::mutex> lock_guard_voice_data(mutext_playsound_voice_datas_);

  int result = YSOS_ERROR_FAILED;
  std::list<VoiceDataBlock>::iterator it_voicedata = playsound_voice_datas_.begin();
  for (it_voicedata; it_voicedata != playsound_voice_datas_.end(); ++it_voicedata) {
    VoiceDataBlock* pvoice_datablock = &(*it_voicedata);
    if (pvoice_datablock->voice_buffer_ptr_.get() == pDataAddress) {
      playsound_voice_datas_.remove(*pvoice_datablock);
      result = YSOS_ERROR_SUCCESS;
      break;
    }
  }

  return result;
}

int AudioPlayExtDriver::Start() {
  YSOS_LOG_DEBUG("in AudioPlayExtDriver::Start()");

  if (TDAPS_START != device_current_status_atomic_
      && TDAPS_OPEN != device_current_status_atomic_
      && TDAPS_STOP != device_current_status_atomic_) {
    YSOS_LOG_DEBUG("当前设备没有被打开，不能播放，返回失败,current status:"<< device_current_status_atomic_);
    return YSOS_ERROR_LOGIC_ERROR;
  }

  boost::lock_guard<boost::mutex> lock_guard_driver(mutext_audio_driver_);

#ifdef _WIN32
  if (NULL == hwave_handle_)
    return YSOS_ERROR_INVALID_ARGUMENTS;
  ///< 当心下面的 mutext_playsound_voice_datas_ 与上面 mutext_audio_driver_ 产生的互死锁
  boost::lock_guard<boost::mutex> lock_guard_voice_data(mutext_playsound_voice_datas_);

  ///< 如果当前要播放的有数据，则播放器为播放状态
  if (playsound_voice_datas_.size() > 0) {
    device_current_status_atomic_ = TDAPS_START;
  }

  std::list<VoiceDataBlock>::iterator it_playsound = playsound_voice_datas_.begin();
  MMRESULT mmret  = MMSYSERR_NOERROR;
  for (it_playsound = playsound_voice_datas_.begin(); it_playsound != playsound_voice_datas_.end(); ++it_playsound) {
    VoiceDataBlock* pvoice_block = &(*it_playsound);
    if (pvoice_block->have_waveoutprepareheader_)
      continue;

    ///< 将要输出的数据写入buffer
    mmret = ::waveOutPrepareHeader(hwave_handle_, pvoice_block->wave_ptr_.get(), sizeof(WAVEHDR));
    if (MMSYSERR_NOERROR != mmret) {
      YSOS_LOG_DEBUG("waveOutPrepareHeader fail");
      return ConverWindowsWaveErrorTypeToTDErrorType(mmret);
    }
    pvoice_block->have_waveoutprepareheader_ = true;
  }

  for (it_playsound = playsound_voice_datas_.begin(); it_playsound != playsound_voice_datas_.end(); ++it_playsound) {
    VoiceDataBlock* pvoice_block = &(*it_playsound);
    if (false == pvoice_block->have_waveoutprepareheader_)
      continue;
    if (pvoice_block->have_add_to_sysem_to_wave_out_write_)
      continue;

    ///< 将输出数据发送给输出设备
    mmret = ::waveOutWrite(hwave_handle_, pvoice_block->wave_ptr_.get(), sizeof(WAVEHDR));
    if (MMSYSERR_NOERROR != mmret) {
      YSOS_LOG_DEBUG("waveOutWrite fail");
      return ConverWindowsWaveErrorTypeToTDErrorType(mmret);
    }
    pvoice_block->have_add_to_sysem_to_wave_out_write_ = true;
  }

  if (send_start_) {
    status_fun_("10023", "audio play start event");
    send_start_ = false;
  }
#else
  {
    //TODO:add for linux
    ///< 当心下面的 mutext_playsound_voice_datas_ 与上面 mutext_audio_driver_ 产生的互死锁
    boost::lock_guard<boost::mutex> lock_guard_voice_data(mutext_playsound_voice_datas_);

    ///< 如果当前要播放的有数据，则播放器为播放状态
    if (playsound_voice_datas_.size() > 0) {
      device_current_status_atomic_ = TDAPS_START;
    }
    
    int ret = start_play_voice(); 
    if (0 != ret) {
        YSOS_LOG_ERROR("start_play_voice fail !!!");
    }
    YSOS_LOG_DEBUG("start_play_voice success");

    std::list<VoiceDataBlock>::iterator it_playsound = playsound_voice_datas_.begin();
    int mmret  = 0;
    for (it_playsound = playsound_voice_datas_.begin(); it_playsound != playsound_voice_datas_.end(); ++it_playsound) {
      VoiceDataBlock* pvoice_block = &(*it_playsound);
      if (pvoice_block->have_waveoutprepareheader_)
        continue;

      ///< 将要输出的数据写入buffer
      /*
      mmret = ::waveOutPrepareHeader(hwave_handle_, pvoice_block->wave_ptr_.get(), sizeof(WAVEHDR));
      if (MMSYSERR_NOERROR != mmret) {
        YSOS_LOG_DEBUG("waveOutPrepareHeader fail");
        return ConverWindowsWaveErrorTypeToTDErrorType(mmret);
      }*/
      pvoice_block->have_waveoutprepareheader_ = true;
    }

    for (it_playsound = playsound_voice_datas_.begin(); it_playsound != playsound_voice_datas_.end(); ++it_playsound) {
      VoiceDataBlock* pvoice_block = &(*it_playsound);
      if (false == pvoice_block->have_waveoutprepareheader_)
        continue;
      if (pvoice_block->have_add_to_sysem_to_wave_out_write_)
        continue;

      ///< 将输出数据发送给输出设备
      /*
      mmret = ::waveOutWrite(hwave_handle_, pvoice_block->wave_ptr_.get(), sizeof(WAVEHDR));
      if (MMSYSERR_NOERROR != mmret) {
        YSOS_LOG_DEBUG("waveOutWrite fail");
        return ConverWindowsWaveErrorTypeToTDErrorType(mmret);
      }*/
      play_voice(pvoice_block->wave_ptr_.get()->lpData, pvoice_block->wave_ptr_.get()->dwBufferLength);
      if (0 != mmret) {
        YSOS_LOG_DEBUG("waveOutWrite fail");
        return mmret;
      }
      pvoice_block->have_add_to_sysem_to_wave_out_write_ = true;
    }

    if (send_start_) {
      status_fun_("10023", "audio play start event");
      send_start_ = false;
    }
  }
#endif

  YSOS_LOG_DEBUG("in AudioPlayExtDriver::Start() done and success");
  return YSOS_ERROR_SUCCESS;
}

int AudioPlayExtDriver::Stop() {
  boost::lock_guard<boost::mutex> lock_guard_driver(mutext_audio_driver_);
  int result = YSOS_ERROR_FAILED;

  do {
    ClearPlayList(1); ///< 注意ClearPlayList 得在 waveOutReset后清理, 这里只清理那些不有加入到系统播放列表中数据

    if (TDAPS_STOP == device_current_status_atomic_) {
      YSOS_LOG_DEBUG("当前是stop状态，不需要再stop，返回成功");
      result = YSOS_ERROR_SUCCESS;
      break;
    }

    if (TDAPS_START != device_current_status_atomic_
        && TDAPS_OPEN != device_current_status_atomic_) {
      YSOS_LOG_DEBUG("当前设备没有start,返回失败,current statues:"<<device_current_status_atomic_);
      result = YSOS_ERROR_LOGIC_ERROR;
      break;
    }
#ifdef _WIN32
    if (NULL == hwave_handle_) {
      YSOS_LOG_DEBUG("error hwave handle is null");
      result = YSOS_ERROR_INVALID_ARGUMENTS;
      break;
    }

    set_is_reseting_stop(true);

    /// waveInStop:If there are any buffers in the queue, the current buffer will be marked as done (the dwBytesRecorded
    /// member in the header will contain the length of data), but any empty buffers in the queue will remain there.
    /// Calling this function when input is not started has no effect, and the function returns zero.
    MMRESULT mmret = waveOutReset(hwave_handle_);   ///< 重置输出设备，重置能够使输出设备全部buffer输出结束

    set_is_reseting_stop(false);

    ClearPlayList();  ///< 注意ClearPlayList 得在 waveOutReset后清理

    if (MMSYSERR_NOERROR != mmret) {
      break;
    }
    device_current_status_atomic_ = TDAPS_STOP;
    status_fun_("10024", "audio play end event");
    send_start_ = true;
#else
  {
    //TODO:add for linux
    set_is_reseting_stop(true);
     
    int ret = stop_play_voice(); 

    set_is_reseting_stop(false);

    ClearPlayList();  ///< 注意ClearPlayList 得在 waveOutReset后清理

    if (0 != ret) {
      YSOS_LOG_DEBUG("stop_play_voice failed !!!");
      break;
    }   
    YSOS_LOG_DEBUG("stop_play_voice success");
    device_current_status_atomic_ = TDAPS_STOP;
    status_fun_("10024", "audio play end event");
    send_start_ = true;
  }
#endif
    result = YSOS_ERROR_SUCCESS;
  } while (0);

  return result;
}

UINT32 AudioPlayExtDriver::device_current_status() {
  boost::lock_guard<boost::mutex> lock_guard_driver(mutext_audio_driver_);
  return device_current_status_atomic_;
}

bool AudioPlayExtDriver::is_reseting_stop() {
  boost::lock_guard<boost::mutex> lock_guard_reseting_stop(mutex_reseting_stop_);
  return bis_reseting_stop_;
}

void AudioPlayExtDriver::set_is_reseting_stop(bool bIsResetting) {
  boost::lock_guard<boost::mutex> lock_guard_reseting_stop(mutex_reseting_stop_);
  bis_reseting_stop_ = bIsResetting;
}

bool AudioPlayExtDriver::is_reseting_close() {
  boost::lock_guard<boost::mutex> lock_guard_reseting_close(mutex_reseting_close_);
  return bis_reseting_close_;
}

void AudioPlayExtDriver::set_is_reseting_close(bool bIsResetting) {
  boost::lock_guard<boost::mutex> lock_guard_reseting_close(mutex_reseting_close_);
  bis_reseting_close_ = bIsResetting;
}

int AudioPlayExtDriver::Pause() {
  return YSOS_ERROR_FAILED;
}

}