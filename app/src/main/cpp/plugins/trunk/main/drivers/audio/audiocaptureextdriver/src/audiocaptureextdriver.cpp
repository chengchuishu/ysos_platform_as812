/*@version 1.0
  *@author l.j..
  *@date Created on: 2016-10-20 13:48:23
  *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
  * 
*/
/// self header
#include "../include/audiocaptureextdriver.h"

#include <boost/thread/thread_guard.hpp>
#include <boost/filesystem.hpp>

/// ysos headers
#include "../../../../../../core/trunk/main/public/include/sys_interface_package/bufferpool.h"
#include "../../../../../../core/trunk/main/public/include/sys_interface_package/callbackqueue.h"
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basemoduleimpl.h"

namespace ysos {

#ifdef _WIN32
typedef void (*AudioCaptureCallBackDataFun)(char* pData, UINT32 uDataLength);
typedef void (*AudioCaptureCallBackEventStatusFun)(unsigned int iEventId);
typedef void (*AudioCaptureCallBackEventVolumeStrengthFun)(int iStrengthValue);

/// msg id for local thread process wim data message
#define WM_MSG_WINDOWSAUDIOCAPTUREDRIVE_WMDATA     WM_USER+2000
#define WM_MSG_WINDOWSAUDIOCAPTUREDRIVE_WMCLOSE    WM_USER+2001
#define WM_MSG_WINDOWSAUDIOCAPTUREDRIVE_THREADEXIT WM_USER+2002

static UINT32 ConverWindowsWaveErrorTypeToTDErrorType(const UINT32 uWindowsErrorType) {
  int result = YSOS_ERROR_FAILED;

  do {
    switch (uWindowsErrorType) {
    case MMSYSERR_NOERROR: {
      result = YSOS_ERROR_SUCCESS;
      break;
    }
    case MMSYSERR_INVALHANDLE: {
      result =  YSOS_ERROR_INVALID_ARGUMENTS;
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
      result = YSOS_ERROR_FAILED;
      break;
    }
    }
  } while (0);

  return result;
}
//#endif

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
void CALLBACK waveInCallbackProc(
  HWAVEIN   hwi,
  UINT      uMsg,
  DWORD_PTR dwInstance,
  DWORD_PTR dwParam1,
  DWORD_PTR dwParam2
) {
  AudioCaptureExtDriver* paudio_driver = reinterpret_cast<AudioCaptureExtDriver*>(dwInstance);
  switch (uMsg) {
  case WIM_OPEN: {
    YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.audiocaptureext"), "wim_open");
  }
  break;

  case WIM_DATA: {
    WAVEHDR* pwavehdr = reinterpret_cast<WAVEHDR*>(dwParam1);
    bool bis_resetting = paudio_driver->is_reseting();
    if (true == bis_resetting) {
      YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.audiocaptureext"), "close rest操作，不再下发数据，不让数据块作任何处理，同时让回调数据快速返回");
      return;
    }
    YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.audiocaptureext"), "wim_date,bufferle " << pwavehdr->dwBufferLength << ",bytesrecord: " << pwavehdr->dwBytesRecorded);
    ///< warning ,please don't process WAVEHDR* data here,in order to avoid deadloak.
    PostThreadMessage(paudio_driver->process_callback_wave_threadid(), WM_MSG_WINDOWSAUDIOCAPTUREDRIVE_WMDATA,
                      reinterpret_cast<WPARAM>(pwavehdr), reinterpret_cast<LPARAM>(hwi));
  }
  break;

  case WIM_CLOSE: {
    YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.audiocaptureext"), "wim_close");
    ///< 如果是设备被拔出，也会上报此事件
    PostThreadMessage(paudio_driver->process_callback_wave_threadid(), WM_MSG_WINDOWSAUDIOCAPTUREDRIVE_WMCLOSE, 0, 0);
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
UINT ThreadProcessCallbackWaveData(LPARAM lParam) {
  AudioCaptureExtDriver* pdriver  = reinterpret_cast<AudioCaptureExtDriver*>(lParam);
  assert(NULL != pdriver);
  MSG msg;
  while (/*1*/0 == pdriver->thread_process_callback_wave_exit_flag_) {
    if (PeekMessage(&msg, NULL, WM_MSG_WINDOWSAUDIOCAPTUREDRIVE_WMDATA, WM_MSG_WINDOWSAUDIOCAPTUREDRIVE_THREADEXIT, PM_NOREMOVE)) {
      switch (msg.message) {
      case WM_MSG_WINDOWSAUDIOCAPTUREDRIVE_WMDATA: {
        GetMessage(&msg, NULL, WM_MSG_WINDOWSAUDIOCAPTUREDRIVE_WMDATA, WM_MSG_WINDOWSAUDIOCAPTUREDRIVE_WMDATA);
        WAVEHDR* pwavehdr = reinterpret_cast<WAVEHDR*>(msg.wParam);
        HWAVEIN hwavein = reinterpret_cast<HWAVEIN>(msg.lParam);
        pdriver->ProcessWaveCallbackData(pwavehdr);
      }
      break;
      case WM_MSG_WINDOWSAUDIOCAPTUREDRIVE_WMCLOSE: {
        YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.audiocaptureext"), "proc WM_MSG_WINDOWSAUDIOCAPTUREDRIVE_WMCLOSE");
        SetEvent(pdriver->event_syn_thread_process_callback_wave_exit);  ///< 事件通知此线程退出了
        pdriver->CloseWithException();
        return 0;
      }
      case WM_MSG_WINDOWSAUDIOCAPTUREDRIVE_THREADEXIT: {
        YSOS_LOG_DEBUG_CUSTOM(GetUtility()->GetLogger("ysos.audiocaptureext"), "proc WM_MSG_WINDOWSAUDIOCAPTUREDRIVE_THREADEXIT");
        GetMessage(&msg, NULL, msg.message, msg.message);
        SetEvent(pdriver->event_syn_thread_process_callback_wave_exit);  ///< 事件通知此线程退出了
        return 0;  /// exit from the thread
      }
      }  ///< end switch
    }  ///< end if
    else {
      Sleep(1);
    }
  }  ///< end while

  SetEvent(pdriver->event_syn_thread_process_callback_wave_exit);  ///< 事件通知此线程退出了
  return 0;  /// exit from the thread
}



int AudioCaptureExtDriver::ProcessWaveCallbackData(WAVEHDR* pWavehdr) {
  int result = YSOS_ERROR_FAILED;
  YSOS_LOG_DEBUG("AudioCaptureExtDriver::ProcessWaveCallbackData in");

  do {
    if (NULL == pWavehdr) {
      YSOS_LOG_DEBUG("pWavehdr is null");
      break;
    }

    /// step 1.flag the current memory not for record;
    /// The waveInUnprepareHeader function cleans up the preparation performed by the waveInPrepareHeader function.
    /// This function must be called after the device driver fills a buffer and returns it to the application.
    /// You must call this function before freeing the buffer.
    MMRESULT mmret = MMSYSERR_NOERROR;
    {
      /// 防止录音的时候关闭设备
      boost::lock_guard<boost::mutex> lock_guard_driver(mutext_audio_driver_);
      if (TDACS_CLOSE == device_current_status_atomic_) {
        YSOS_LOG_DEBUG("当前设备已关闭，不再把buffer加入系统录音队列中");
        result = YSOS_ERROR_SUCCESS;
        break;
      }
      mmret = waveInUnprepareHeader(wave_handle(), pWavehdr, sizeof(WAVEHDR));
    }
    switch (mmret) {
    case MMSYSERR_NOERROR: {
      YSOS_LOG_DEBUG("waveInUnprepareHeader success");
      ///< 如果当前不是 运行状态，则不再往上抛出（比如是open,stop,close 状态)
      if (/*TDACS_OPEN*/TDACS_START != device_current_status_atomic_) {
        YSOS_LOG_DEBUG("in ProcessWaveCallbackData 当前不是运行状态，则不再往上抛出,当前状态：" << device_current_status_atomic_);
        break;
      }
    }
    break;
    case MMSYSERR_INVALHANDLE:   ///< Specified device handle is invalid.
    case MMSYSERR_NODRIVER:      ///< No device driver is present.
    case MMSYSERR_NOMEM:         ///< Unable to allocate or lock memory.
    case WAVERR_STILLPLAYING:    ///< The buffer pointed to by the pwh parameter is still in the queue.
    default: {
      YSOS_LOG_DEBUG("waveInUnprepareHeader failed");
    }
    break;
    }  ///< end switch

    if (!pcm_dir_.empty()) {
      static int count = 0;
      if (NULL == pcm_file_) {
        std::string cur_time = boost::posix_time::to_iso_string(boost::posix_time::second_clock::local_time());
        cur_time.replace(cur_time.find('T'), 1, "_");
        std::string file_name = pcm_dir_ + cur_time + ".pcm";
        YSOS_LOG_DEBUG("pcm file name = " << file_name);
        pcm_file_ = fopen(file_name.c_str(), "wb");
        if (NULL == pcm_file_) {
          YSOS_LOG_DEBUG("open file failed");
        }
      }
      if (NULL != pcm_file_) {
        if (pWavehdr->dwBytesRecorded > 0) {
          fwrite(pWavehdr->lpData, pWavehdr->dwBytesRecorded, 1, pcm_file_);
          ++count;
          if (1000 <= count) {
            YSOS_LOG_DEBUG("big enough and close current file");
            fclose(pcm_file_);
            pcm_file_ = NULL;
            count = 0;
          }
        } else {
          YSOS_LOG_DEBUG("pWavehdr->dwBytesRecorded no data");
        }
      }
    }

    YSOS_LOG_DEBUG("ready waveInPrepareHeader");
    {
      boost::lock_guard<boost::mutex> lock_guard_wave_data(mutex_list_have_wave_data_);
      list_have_wave_data_.push_back(pWavehdr);
    }

    YSOS_LOG_DEBUG("AudioCaptureExtDriver::ProcessWaveCallbackData done and success");
    result = YSOS_ERROR_SUCCESS;
  } while (0);

  return result;
}
#else
    //TODO:add for linux---- define field
    boost::atomic<int> device_current_status_atomic_;
    boost::mutex mutext_audio_driver_;              ///< mutext for public call
    
    boost::mutex mutex_list_have_wave_data_;
    std::list<WAVEHDR*> list_have_wave_data_;   ///< 有wave数据的列表， 为了支持外部调用，而不是由自己触发
    
    std::string pcm_dir_;
    FILE* pcm_file_;

    //add for linux
    struct recorder *rec_;

/* after stop_record, there are still some data callbacks */
void wait_for_rec_stop(struct recorder *rec, unsigned int timeout_ms)
{
	while (!is_record_stopped(rec)) {
		usleep(1*1000);
		if (timeout_ms != (unsigned int)-1)
			if (0 == timeout_ms--)
				break;
	}
}

int sr_write_audio_data(char *data, unsigned int len)
{
	int ret = 0;
	
	if (!data || !len)
		return 0;
  
  //TODO:add for linux  -- send data to next nodule
	//
  int result = YSOS_ERROR_FAILED;
  //printf("AudioCaptureExtDriver::sr_write_audio_data in\n");
  do {
 
    {
      /// 防止录音的时候关闭设备
      boost::lock_guard<boost::mutex> lock_guard_driver(mutext_audio_driver_);
      if (0 == device_current_status_atomic_) {
        //printf("当前设备已关闭，不再把buffer加入系统录音队列中\n");
        result = YSOS_ERROR_SUCCESS;
        break;
      }
    }
    if (2 != device_current_status_atomic_) {
        //printf("in ProcessWaveCallbackData 当前不是运行状态，则不再往上抛出,当前状态：[device_current_status_atomic_] =[%d]\n", (int)device_current_status_atomic_);
        result = YSOS_ERROR_SUCCESS;
        break;
    }
    //TODO:add for linux
    WAVEHDR* pWavehdr = new WAVEHDR();
    pWavehdr->lpData = data;
    pWavehdr->dwBytesRecorded = len;//sizeof(data);
    //save to pcm files
    if (!pcm_dir_.empty()) {
      static int count = 0;
      if (NULL == pcm_file_) {
        std::string cur_time = boost::posix_time::to_iso_string(boost::posix_time::second_clock::local_time());
        cur_time.replace(cur_time.find('T'), 1, "_");
        std::string file_name = pcm_dir_ + cur_time + ".pcm";
        printf("pcm file name = [%s]\n", file_name.c_str());
        pcm_file_ = fopen(file_name.c_str(), "wb");
        if (NULL == pcm_file_) {
          printf("open file failed\n");
        }
      }
      if (NULL != pcm_file_) {
        if (pWavehdr->dwBytesRecorded > 0) {
          fwrite(pWavehdr->lpData, pWavehdr->dwBytesRecorded, 1, pcm_file_);
          ++count;
          if (1000 <= count) {
            printf("big enough and close current file\n");
            fclose(pcm_file_);
            pcm_file_ = NULL;
            count = 0;
          }
        } else {
          printf("pWavehdr->dwBytesRecorded no data\n");
        }
      }
    }

    //printf("ready waveInPrepareHeader\n");
    {
      boost::lock_guard<boost::mutex> lock_guard_wave_data(mutex_list_have_wave_data_);
      list_have_wave_data_.push_back(pWavehdr);
    }

    //printf("AudioCaptureExtDriver::ProcessWaveCallbackData done and success\n");
    result = YSOS_ERROR_SUCCESS;
  } while (0);

	return result;
}

/* the record call back */
void iat_cb(char *data, unsigned long len, void *user_para)
{
	int errcode;
  //printf("iat_cb---->sr_write_audio_data in [Enter]\n");
	if(len == 0 || data == NULL) {
    printf("[Warn]iat_cb---->len == 0 || data == NULL\n");
    return;
  }	
  	
	errcode = sr_write_audio_data(data, len);
  //printf("iat_cb---->sr_write_audio_data  [errcode]=[%d]\n", errcode);
	if (errcode) {
    printf("[Warn]iat_cb---->sr_write_audio_data-->errcode=[%d]\n", errcode);
		stop_record(rec_);
		return;
	}
  //printf("iat_cb---->sr_write_audio_data in [End]\n");
}


#endif

int AudioCaptureExtDriver::PrepareData(WAVEHDR* pWavehdr) {
  int result = YSOS_ERROR_FAILED;

  do {
    /// 防止录音的时候关闭设备
    boost::lock_guard<boost::mutex> lock_guard_driver(mutext_audio_driver_);
    if (TDACS_CLOSE == device_current_status_atomic_) {
      YSOS_LOG_DEBUG("当前设备已关闭，不再把buffer加入系统录音队列中");
      result = YSOS_ERROR_SUCCESS;
      break;
    }
#ifdef _WIN32
    int mmret = waveInPrepareHeader(wave_handle(), pWavehdr, sizeof(WAVEHDR));
    if (MMSYSERR_NOERROR != mmret) {
      YSOS_LOG_DEBUG("waveInPrepareHeader fail");
      result = ConverWindowsWaveErrorTypeToTDErrorType(mmret);
      break;
    }
    mmret = waveInAddBuffer(wave_handle(), pWavehdr, sizeof(WAVEHDR));
    if (MMSYSERR_NOERROR != mmret) {
      YSOS_LOG_DEBUG("waveInAddBuffer fail");
      result = ConverWindowsWaveErrorTypeToTDErrorType(mmret);
      break;
    }
#else
    //TODO:add for linux
    
#endif
  } while (0);

  return result;
}

DECLARE_PLUGIN_REGISTER_INTERFACE(AudioCaptureExtDriver, DriverInterface);
AudioCaptureExtDriver::AudioCaptureExtDriver(const std::string &strClassName) : BaseInterfaceImpl(strClassName) {
  logger_ = GetUtility()->GetLogger("ysos.audiocaptureext");
  //hwave_handle_= NULL;
  wformat_tag_ = WAVE_FORMAT_PCM;
  cbsize_ = 0;
  nsamples_per_sec_ = 16000;
  nchannels_ = 1;
  wbits_per_sample_ = 16;
  nblock_align_ = (wbits_per_sample_ * nchannels_) / 8;
  navg_bytes_per_sec_ = nblock_align_ * nsamples_per_sec_;
  uwave_buffer_data_size_pernum_ = 1024 * 2/*32000*/;///< (1 秒内 即1秒内有 15.625个数据块, 16000*2B/(1024B*2) = 15.625
  uwave_buffer_data_size_pernum_ = nsamples_per_sec_*wbits_per_sample_/8.0*nchannels_/(1000.0/100.0); //100ms 
  //uwave_buffer_data_size_pernum_ = 16000*16/8.0*1/(1000.0/100.0); //100ms 
  uwave_buffer_num_ = 10;  ///< 最多缓存1秒的数据
  //dwprocess_callback_wave_threadid_ = 0;
  device_current_status_atomic_ = TDACS_CLOSE;
  close_counter_ = 0;
  show_volume_strength_atomic_ = 0;
  event_syn_thread_process_callback_wave_exit = NULL;
  thread_process_callback_wave_exit_flag_ = 0;
  pcm_dir_ = "";
  pcm_file_ = NULL;
  capture_switch_ = true;
}

int AudioCaptureExtDriver::RealInitialize(void *param) {
  return YSOS_ERROR_SUCCESS;
}


AudioCaptureExtDriver::~AudioCaptureExtDriver() {
}

int AudioCaptureExtDriver::RealUnInitialize(void *param) {

  ///< 清除回调函数队列, 防止再次回调上层
  {
    YSOS_LOG_DEBUG("清除回调 音频录音设备 状态 队列");
    boost::lock_guard<boost::mutex> lockguard_eventcallback(mutex_event_status_callback_);
    vecevent_status_callback_.clear();
  }

  {
    YSOS_LOG_DEBUG("清除回调 音频录音数据 函数队列");
    boost::lock_guard<boost::mutex> lock_guard_callback(mutex_veproperity_onread_callback_);
    veproperity_onread_callback_.clear();
  }

  {
    show_volume_strength_atomic_ = 0;
    YSOS_LOG_DEBUG("清除回调 录音音量强度 函数队列");
    boost::lock_guard<boost::mutex> lock_guard_callback(mutex_event_volume_strength_callback_);
    vecevent_volume_strength_calalback_.clear();
  }

  Close();
#ifdef _WIN32 
  {
    ///< 等待处理数据的线程退出，不要写在Close()函数里，要不然会死锁
    thread_process_callback_wave_exit_flag_ =1;
    if (event_syn_thread_process_callback_wave_exit) {
      int thread_wait_ret =WaitForSingleObject(event_syn_thread_process_callback_wave_exit, 5000);  ///<等3minute
      if (WAIT_OBJECT_0 != thread_wait_ret) {
        YSOS_LOG_ERROR("error, the thread can't exit in 5000 miliseconds");
      }
    }
  }
#else
    //TODO:add for linux---- no relative thread
#endif
  YSOS_LOG_DEBUG("uninitialize logname:" << logic_name_);
  return YSOS_ERROR_SUCCESS;
}

int AudioCaptureExtDriver::Open(void *pParams) {
  YSOS_LOG_DEBUG("AudioCaptureExtDriver::Open in [Enter]");

  int result = YSOS_ERROR_FAILED;
  do {
    boost::lock_guard<boost::mutex> lock_guard_driver(mutext_audio_driver_);
    if (TDACS_OPEN == device_current_status_atomic_) {
      YSOS_LOG_DEBUG("当前已经是打开状态，返回成功");
      result = YSOS_ERROR_SUCCESS;
      break;
    }
    YSOS_LOG_DEBUG("AudioCaptureExtDriver::Open [Check][0]");
    if (TDACS_CLOSE != device_current_status_atomic_) {
      YSOS_LOG_DEBUG("打开失败，上一次打开没有被关闭");
      break;
    }
    YSOS_LOG_DEBUG("AudioCaptureExtDriver::Open [Check][1]");
    if (!pcm_dir_.empty()) {
      YSOS_LOG_DEBUG("pcm_dir_ = " << pcm_dir_);
      if (!boost::filesystem::exists(pcm_dir_)) {
        if (!boost::filesystem::create_directories(pcm_dir_)) {
          YSOS_LOG_DEBUG("create directory failed");
          break;
        }
      }
    }
    YSOS_LOG_DEBUG("AudioCaptureExtDriver::Open [Check][2]");
    data_ptr_ = GetDataInterfaceManager()->GetData(GLOBAL_DATA_KEY);
    if (!data_ptr_) {
      YSOS_LOG_DEBUG("get data_ptr failed");
      break;
    }
    YSOS_LOG_DEBUG("AudioCaptureExtDriver::Open [Check][3]");
    {
      vewave_output_buffer_.clear();
      for (UINT32 ui =0; ui < uwave_buffer_num_; ++ui) {
        MySharedCharBuffer sharebuf(new char[uwave_buffer_data_size_pernum_]);
        vewave_output_buffer_.push_back(sharebuf);
      }

      vewave_output_buffer_headaddress_.clear();
      uwave_buffer_num_ = vewave_output_buffer_.size();
      for (UINT32 i = 0; i < uwave_buffer_num_; ++i) {
        boost::shared_ptr<WAVEHDR> wavehdr_ptr(new WAVEHDR);
        memset(wavehdr_ptr.get(), 0, sizeof(WAVEHDR));
        vewave_output_buffer_headaddress_.push_back(wavehdr_ptr);
        vewave_output_buffer_headaddress_[i]->dwBufferLength = uwave_buffer_data_size_pernum_;  ///< data buffer length
        vewave_output_buffer_headaddress_[i]->lpData = reinterpret_cast<char*>(vewave_output_buffer_[i].get());  ///< locate the data address
      }  ///< end for
#ifdef _WIN32
      if (0 == dwprocess_callback_wave_threadid_) {
        thread_process_callback_wave_exit_flag_ =0;
        HANDLE hthreadhandle = CreateThread(NULL, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(ThreadProcessCallbackWaveData),
                                            this, CREATE_SUSPENDED, &dwprocess_callback_wave_threadid_);
        event_syn_thread_process_callback_wave_exit = CreateEvent(NULL, TRUE, FALSE, NULL);
        ResumeThread(hthreadhandle);
        CloseHandle(hthreadhandle);
        hthreadhandle = NULL;
      } else {
        YSOS_LOG_DEBUG("处理语音块线程已启动，不再启用");
      }
#else
    //TODO:add for linux ---- no create thread process
#endif
    }
    YSOS_LOG_DEBUG("AudioCaptureExtDriver::Open [Check][4]");
#ifdef _WIN32
    WAVEFORMATEX wave_format;
    wave_format.wFormatTag = wformat_tag_; ///< WAVE_FORMAT_PCM
    wave_format.nChannels = nchannels_; ///< 1
    wave_format.nSamplesPerSec = nsamples_per_sec_; ///< 16000   每秒采集数据块的个数
    wave_format.nAvgBytesPerSec = navg_bytes_per_sec_; ///< nblock_align_ * nsamples_per_sec_   音频数据每秒转化率(一秒生成的实际数据大小)
    wave_format.nBlockAlign = nblock_align_; ///< (wbits_per_sample_ * nchannels_) / 8   一个数据块的字节数
    wave_format.wBitsPerSample = wbits_per_sample_; ///< 16
    wave_format.cbSize = cbsize_; ///< 0
    MMRESULT mmret =
      waveInOpen(
        &hwave_handle_,  ///< Pointer to a buffer that receives a handle identifying the open waveform-audio input device. Use this handle to identify the device when calling other waveform-audio input functions. This parameter can be NULL if WAVE_FORMAT_QUERY is specified for fdwOpen.  //NOLINT
        WAVE_MAPPER,     ///< Identifier of the waveform-audio input device to open. It can be either a device identifier or a handle of an open waveform-audio input device. You can use the following flag instead of a device identifier.WAVE_MAPPER:The function selects a waveform-audio input device capable of recording in the specified format.  //NOLINT
        &wave_format,    ///< Pointer to a WAVEFORMATEX structure that identifies the desired format for recording waveform-audio data. You can free this structure immediately after waveInOpen returns.  //NOLINT
        reinterpret_cast<DWORD>(waveInCallbackProc),  ///< Pointer to a fixed callback function, an event handle, a handle to a window, or the identifier of a thread to be called during waveform-audio recording to process messages related to the progress of recording. If no callback function is required, this value can be zero. For more information on the callback function, see waveInProc.  //NOLINT
        reinterpret_cast<DWORD>(this),                ///< User-instance data passed to the callback mechanism. This parameter is not used with the window callback mechanism.  //NOLINT
        CALLBACK_FUNCTION);                           ///< Flags for opening the device. The following values are defined.  //NOLINT
    switch (mmret) {
    case MMSYSERR_NOERROR: {
      YSOS_LOG_DEBUG("waveInOpen success");
    }
    break;
    case MMSYSERR_ALLOCATED:    ///< Specified resource is already allocated.
    case MMSYSERR_BADDEVICEID:  ///< Specified device identifier is out of range.
    case MMSYSERR_NODRIVER:     ///< No device driver is present.
    case MMSYSERR_NOMEM:        ///< Unable to allocate or lock memory.
    case WAVERR_BADFORMAT:      ///< Attempted to open with an unsupported waveform-audio format
      YSOS_LOG_DEBUG("waveInOpen fail : " << mmret);
      //CallBackEventStatus(TDACS_DEVICE_FAIL);
      return ConverWindowsWaveErrorTypeToTDErrorType(mmret);
      break;
    default: {
      YSOS_LOG_DEBUG("waveInOpen fail : " << mmret);
      //CallBackEventStatus(TDACS_DEVICE_FAIL);
      return YSOS_ERROR_FAILED;
      break;
    }
    }

    for (UINT32 i = 0; i < uwave_buffer_num_; i++) {
      MMRESULT mmret = waveInPrepareHeader(hwave_handle_, vewave_output_buffer_headaddress_[i].get(), sizeof(WAVEHDR));
      switch (mmret) {
      case  MMSYSERR_NOERROR:
        break;
      case MMSYSERR_INVALHANDLE:  ///< Specified device handle is invalid.
      case MMSYSERR_NODRIVER:     ///< No device driver is present.
      case MMSYSERR_NOMEM:        ///< Unable to allocate or lock memory.
        YSOS_LOG_DEBUG("waveInPrepareHeader fail");
        return ConverWindowsWaveErrorTypeToTDErrorType(mmret);
      default: {
        YSOS_LOG_DEBUG("waveInPrepareHeader fail");
        return YSOS_ERROR_FAILED;
      }
      } ///< end switch

      /// The waveInAddBuffer function sends an input buffer to the given waveform-audio input device.
      /// When the buffer is filled, the application is notified
      mmret = waveInAddBuffer(hwave_handle_, vewave_output_buffer_headaddress_[i].get(), sizeof(WAVEHDR));
      switch (mmret) {
      case MMSYSERR_NOERROR :
        break;
      case MMSYSERR_INVALHANDLE:  ///< Specified device handle is invalid.
      case MMSYSERR_NODRIVER:     ///< No device driver is present.
      case MMSYSERR_NOMEM:        ///< Unable to allocate or lock memory.
      case WAVERR_UNPREPARED:     ///< The buffer pointed to by the pwh parameter hasn't been prepared.
        YSOS_LOG_DEBUG("waveInAddBuffer fail");
        return ConverWindowsWaveErrorTypeToTDErrorType(mmret);
      default: {
        YSOS_LOG_DEBUG("waveInAddBuffer fail");
        return YSOS_ERROR_FAILED;
      }
      break;
      }  ///< end switch
    }  ///< end for
#else
    //TODO:add for linux  -- open record instance
    YSOS_LOG_DEBUG("AudioCaptureExtDriver::Open [Check][4][0]");
    

    struct speech_rec *sr;
    //memset(sr, 0, sizeof(struct speech_rec));
    //sr->state = 0;
    //sr->aud_src = SR_MIC;
    //sr->ep_stat = 0;
    //sr->rec_stat = 0;
    //sr->audio_status = 1;

    int errcode = create_recorder(&rec_, iat_cb, (void*)sr);
		if (rec_ == NULL || errcode != 0) {
			printf("create recorder failed: %d\n", errcode);
      YSOS_LOG_DEBUG("waveInOpen fail : " << errcode);
			errcode = -E_SR_RECORDFAIL;
			if (rec_) {
		    destroy_recorder(rec_);
		    rec_ = NULL;
	    }
      break;
		}
    YSOS_LOG_DEBUG("waveInOpen success");
		YSOS_LOG_DEBUG("AudioCaptureExtDriver::Open [Check][4][1]");
	  
    WAVEFORMATEX wavfmt = DEFAULT_FORMAT; 
		errcode = open_recorder(rec_, get_default_input_dev(), &wavfmt);
		if (errcode != 0) {
			printf("recorder open failed: %d\n", errcode);
			errcode = -E_SR_RECORDFAIL;
			if (rec_) {
		    destroy_recorder(rec_);
		    rec_ = NULL;
	    }
      break;
		}
    YSOS_LOG_DEBUG("AudioCaptureExtDriver::Open [Check][4][2]");
#endif
    YSOS_LOG_DEBUG("AudioCaptureExtDriver::Open [Check][5]");
    device_current_status_atomic_ = TDACS_OPEN;
    //CallBackEventStatus(TDACS_OPEN);
    result = YSOS_ERROR_SUCCESS;
  } while (0);

  YSOS_LOG_DEBUG("AudioCaptureExtDriver::Open out [End]");
  return result;
}

void AudioCaptureExtDriver::Close(void *pParams) {
  YSOS_LOG_DEBUG("AudioCaptureExtDriver::Close [Enter]");
  do {
    if (TDACS_START == device_current_status_atomic_) {
      YSOS_LOG_DEBUG("当前设备在录音，先尝试stop");
      Stop();  ///< 注意，此函数中调用了 mutext_audio_driver
    }
    YSOS_LOG_DEBUG("AudioCaptureExtDriver::Close [Check][0]");
    boost::lock_guard<boost::mutex> lock_guard_driver(mutext_audio_driver_);
    
    YSOS_LOG_DEBUG("向线程发送退出消息-begin");
#ifdef _WIN32
    ///< 向线程发送退出消息
    if (0 != dwprocess_callback_wave_threadid_) {  ///< close thread
      YSOS_LOG_DEBUG("向线程发送退出消息-end");
      PostThreadMessage(dwprocess_callback_wave_threadid_, WM_MSG_WINDOWSAUDIOCAPTUREDRIVE_THREADEXIT, NULL, NULL);
      dwprocess_callback_wave_threadid_ = NULL;
    }
#else
    //TODO:add for linux
#endif

    YSOS_LOG_DEBUG("in WindowsAudioCaptureDriver::Close");
    if (TDACS_CLOSE == device_current_status_atomic_) {
      YSOS_LOG_DEBUG("当前设备已关闭，不用再关闭，返回成功");
      break;
    }
    YSOS_LOG_DEBUG("AudioCaptureExtDriver::Close [Check][1]");
    ///< 只有在stop情况下才能关闭
    if (TDACS_START == device_current_status_atomic_) {
      YSOS_LOG_DEBUG("关闭失败,当前设备是start 没有被stop");
      break;
    }
    YSOS_LOG_DEBUG("AudioCaptureExtDriver::Close [Check][2]");
    set_is_reseting(true);

#ifdef _WIN32
    ///< The waveInReset function stops input on the given waveform-audio input device and resets the current position
    ///< to zero. All pending buffers are marked as done and returned to the application.
    ///< 经过测试，好像是所的有 All pending buffers are marked as done and returned to the application 后才返回
    MMRESULT mmret = waveInReset(hwave_handle_);
    set_is_reseting(false);
    switch (mmret) {
    case MMSYSERR_NOERROR : {
      YSOS_LOG_DEBUG("waveInReset succ");
    }
    break;
    case MMSYSERR_INVALHANDLE:   ///< MMSYSERR_INVALHANDLE
    case MMSYSERR_NODRIVER:      ///< No device driver is present.
    case MMSYSERR_NOMEM:         ///< Unable to allocate or lock memory.
    default: {
      YSOS_LOG_DEBUG("waveInReset failed, 关闭失败");
      return;
    }
    break;
    }

    /// If there are input buffers that have been sent with the waveInAddBuffer function and that haven't been returned
    /// to the application, the close operation will fail. Call the waveInReset function to mark all pending buffers
    /// as done.
    mmret = waveInClose(hwave_handle_);
    switch (mmret) {
    case MMSYSERR_NOERROR: {
      YSOS_LOG_DEBUG("waveInClose succ");
      hwave_handle_ = NULL;
    }
    break;
    case MMSYSERR_INVALHANDLE:  ///< Specified device handle is invalid.
    case MMSYSERR_NODRIVER:     ///< No device driver is present.
    case MMSYSERR_NOMEM:        ///< Unable to allocate or lock memory.
    case WAVERR_STILLPLAYING:   ///< There are still buffers in the queue.
    default: {
      YSOS_LOG_DEBUG("waveInClose failed: " << mmret);
      return;
    }
    break;
    }
#else
    //TODO:add for linux
    if (rec_) {
		  if(!is_record_stopped(rec_))
			  stop_record(rec_);
		    close_recorder(rec_);
		    destroy_recorder(rec_);
		    rec_ = NULL;
	  }
#endif
    YSOS_LOG_DEBUG("AudioCaptureExtDriver::Close [Check][3]");
    device_current_status_atomic_ = TDACS_CLOSE;
    //CallBackEventStatus(TDACS_CLOSE);

    //if (NULL != pcm_file_) {
    //  fclose(pcm_file_);
    //  pcm_file_ = NULL;
    //}
  } while (0);
  YSOS_LOG_DEBUG("AudioCaptureExtDriver::Close [End]");
  return;
}

int AudioCaptureExtDriver::Start() {
  int result = YSOS_ERROR_FAILED;
  YSOS_LOG_DEBUG("AudioCaptureExtDriver::Start [Enter]");

  do {
    boost::lock_guard<boost::mutex> lock_guard_driver(mutext_audio_driver_);
    YSOS_LOG_DEBUG("AudioCaptureExtDriver::Start in");

    if (TDACS_START == device_current_status_atomic_) {
      YSOS_LOG_DEBUG("当前是录音状态 同时句柄有效，不需要再start,返回成功");
      result = YSOS_ERROR_SUCCESS;
      break;
    }
    YSOS_LOG_DEBUG("AudioCaptureExtDriver::Start [Check][0]");
    if (TDACS_OPEN != device_current_status_atomic_
        && TDACS_STOP != device_current_status_atomic_) {
      YSOS_LOG_DEBUG("当前设备没有被打开，不能录音，返回失败,current status: " << device_current_status_atomic_);
      break;
    }
    YSOS_LOG_DEBUG("AudioCaptureExtDriver::Start [Check][1]");
#ifdef _WIN32
    if (NULL == hwave_handle_) {
      result = YSOS_ERROR_INVALID_ARGUMENTS;
      break;
    }

    /// start recorder
    MMRESULT mmret = waveInStart(hwave_handle_);
    switch (mmret) {
    case MMSYSERR_NOERROR: {  ///< success
      YSOS_LOG_DEBUG("waveInStart success");
      device_current_status_atomic_ = TDACS_START;
      YSOS_LOG_DEBUG("set device_current_status_: " << device_current_status_atomic_);
      //CallBackEventStatus(TDACS_START);
      return YSOS_ERROR_SUCCESS;
    }

    case MMSYSERR_NODRIVER: {    ///< No device driver is present.
      //CallBackEventStatus(TDACS_DEVICE_FAIL);
      return YSOS_ERROR_NOT_ENOUGH_RESOURCE;
    }

    case MMSYSERR_INVALHANDLE:  ///< Specified device handle is invalid.
    case MMSYSERR_NOMEM:        ///< Unable to allocate or lock memory.
      YSOS_LOG_DEBUG(" waveInStart fail: " << mmret);
      return ConverWindowsWaveErrorTypeToTDErrorType(mmret);

    default:
      YSOS_LOG_DEBUG(" waveInStart fail2");
    }
#else
    //TODO:add for linux
    /** start for recorder **/
    int ret = start_record(rec_);
		if (ret != 0) {
			printf("start record failed: %d\n", ret);
      YSOS_LOG_DEBUG("start record failed: [ret]=" << ret);
			return YSOS_ERROR_FAILED;
		}
    printf("start record success ret: [%d]\n", ret);
    device_current_status_atomic_ = TDACS_START;
    YSOS_LOG_DEBUG("set device_current_status_: " << device_current_status_atomic_);
    return YSOS_ERROR_SUCCESS;
#endif
  } while (0);
    YSOS_LOG_DEBUG("AudioCaptureExtDriver::Start [End]");
  return YSOS_ERROR_FAILED;
}

int AudioCaptureExtDriver::StartEx() {
  int result = YSOS_ERROR_FAILED;
  YSOS_LOG_DEBUG("AudioCaptureExtDriver::StartEx [Enter]");
  do {
    ///< warning 这里面不要使用锁
    result = Start();
    if (YSOS_ERROR_NOT_ENOUGH_RESOURCE == result) {
      YSOS_LOG_DEBUG("waveInStart fail MMSYSERR_NODRIVER,关闭当前驱动并再打开");
      Close();
      Open(NULL);
      result = Start();
    }
  } while (0);
  YSOS_LOG_DEBUG("AudioCaptureExtDriver::StartEx [End]");
  return result;
}


int AudioCaptureExtDriver::Stop() {
  int result = YSOS_ERROR_FAILED;

  do {
    boost::lock_guard<boost::mutex> lock_guard_driver(mutext_audio_driver_);
    YSOS_LOG_DEBUG("AudioCaptureExtDriver::Stop in [Enter]");

    if (TDACS_STOP == device_current_status_atomic_) {
      YSOS_LOG_DEBUG("当前是stop状态，不需要再stop，返回成功");
      result = YSOS_ERROR_SUCCESS;
      break;
    }
    YSOS_LOG_DEBUG("AudioCaptureExtDriver::Stop [Check][0]");
    if (TDACS_START != device_current_status_atomic_
        && TDACS_OPEN != device_current_status_atomic_) {
      YSOS_LOG_DEBUG("当前设备没有start,返回失败,current statues: " << device_current_status_atomic_);
      break;
    }
    YSOS_LOG_DEBUG("AudioCaptureExtDriver::Stop [Check][1]");
#ifdef _WIN32
    if (NULL == hwave_handle_) {
      YSOS_LOG_DEBUG("hwave_handle_ is null");
      break;
    }

    /// waveInStop:If there are any buffers in the queue, the current buffer will be marked as done (the dwBytesRecorded
    /// member in the header will contain the length of data), but any empty buffers in the queue will remain there.
    /// Calling this function when input is not started has no effect, and the function returns zero.
    MMRESULT mmret = waveInStop(hwave_handle_);
    switch (mmret) {
    case MMSYSERR_NOERROR: { ///< stop succ
      YSOS_LOG_DEBUG("waveInStop succ");
      device_current_status_atomic_ = TDACS_STOP;
      //CallBackEventStatus(TDACS_STOP);
      return YSOS_ERROR_SUCCESS;
    }

    case MMSYSERR_NODRIVER: { ///< No device driver is present.
      YSOS_LOG_DEBUG("waveInStop fail ,MMSYSERR_NODRIVER 设备被拔出，返回stop成功");
      device_current_status_atomic_ = TDACS_STOP;
      //CallBackEventStatus(TDACS_DEVICE_FAIL);
      return YSOS_ERROR_SUCCESS;
    }

    case MMSYSERR_INVALHANDLE:  ///< Specified device handle is invalid.
    //case MMSYSERR_NODRIVER:     ///< No device driver is present.
    case MMSYSERR_NOMEM:        ///< Unable to allocate or lock memory.
      YSOS_LOG_DEBUG("waveInStop fail : " << mmret);
      return ConverWindowsWaveErrorTypeToTDErrorType(mmret);

    default: {
      YSOS_LOG_DEBUG("waveInStop fail");
      return YSOS_ERROR_FAILED;
    }
    }
#else
    //TODO:add for linux
    YSOS_LOG_DEBUG("AudioCaptureExtDriver::Stop [Check][1][0]");
    device_current_status_atomic_ = TDACS_STOP;
    /** stop for recorder **/
    int ret = stop_record(rec_);
    if (ret != 0) {
        printf("Stop stop_record failed! \n");
        return YSOS_ERROR_FAILED;
    }
    printf("Stop stop_record success [ret]=[%d] \n", ret);

    result = YSOS_ERROR_SUCCESS;
    
    YSOS_LOG_DEBUG("AudioCaptureExtDriver::Stop [Check][1][1]");
    wait_for_rec_stop(rec_, (unsigned int)-1);
    YSOS_LOG_DEBUG("AudioCaptureExtDriver::Stop [Check][1][2]");
    
#endif
  YSOS_LOG_DEBUG("AudioCaptureExtDriver::Stop [Check][2]");
  } while (0);
  YSOS_LOG_DEBUG("AudioCaptureExtDriver::Stop [End]");
  return result;
}

UINT32 AudioCaptureExtDriver::device_current_status() {
  boost::lock_guard<boost::mutex> lock_guard_driver(mutext_audio_driver_);
  return device_current_status_atomic_;
}

inline bool AudioCaptureExtDriver::is_reseting() {
  boost::lock_guard<boost::mutex> lock_guard_is_reseting(mutex_reseting_);
  return bis_reseting_;
}

inline void AudioCaptureExtDriver::set_is_reseting(bool bIsResetting) {
  boost::lock_guard<boost::mutex> lock_guard_is_reseting(mutex_reseting_);
  bis_reseting_ = bIsResetting;
}

#ifdef _WIN32
int AudioCaptureExtDriver::CloseWithException() {

  boost::lock_guard<boost::mutex> lock_guard_driver(mutext_audio_driver_);
  hwave_handle_ = NULL;
  YSOS_LOG_DEBUG("向线程发送退出消息-begin");
  ///< 向线程发送退出消息
  if (0 != dwprocess_callback_wave_threadid_) {  ///< close thread
    YSOS_LOG_DEBUG("向线程发送退出消息-end");
    PostThreadMessage(dwprocess_callback_wave_threadid_, WM_MSG_WINDOWSAUDIOCAPTUREDRIVE_THREADEXIT, NULL, NULL);
    dwprocess_callback_wave_threadid_ = NULL;
  }
  device_current_status_atomic_ = TDACS_CLOSE;
  //CallBackEventStatus(TDACS_CLOSE);

  return YSOS_ERROR_SUCCESS;
}

int AudioCaptureExtDriver::GetAudioCaptureVoiceFormat(int& format_tag, int& channels, int& samples_per_seconds, int& avg_bytes_per_second, int& block_align, int& bits_per_sample, int& cbsize) {
  boost::lock_guard<boost::mutex> lock_guard_driver(mutext_audio_driver_);
  return YSOS_ERROR_SUCCESS;
}

int AudioCaptureExtDriver::GetAudioCaptureVoiceFormat(WAVEFORMATEX& voice_format) {
  boost::lock_guard<boost::mutex> lock_guard_driver(mutext_audio_driver_);

  voice_format.wFormatTag = wformat_tag_;
  voice_format.nChannels = nchannels_;
  voice_format.nSamplesPerSec = nsamples_per_sec_;
  voice_format.nAvgBytesPerSec = navg_bytes_per_sec_;
  voice_format.nBlockAlign = nblock_align_;
  voice_format.wBitsPerSample = wbits_per_sample_;
  voice_format.cbSize = cbsize_;

  return YSOS_ERROR_SUCCESS;
}

void AudioCaptureExtDriver::AddEventRecordeVoiceDataCallBack(void* pCallbackFun) {
  boost::lock_guard<boost::mutex> lock_guard_callback_datafun(mutex_veproperity_onread_callback_);

  {
    ///<  查找一下，如果已加入过，则不再加入
    std::vector<void*>::iterator it = veproperity_onread_callback_.begin();
    for (it ; it != veproperity_onread_callback_.end(); it++) {
      if (*it == pCallbackFun) {
        ///< 已经加入过，不再加入
        return;
      }
    }
  }

  veproperity_onread_callback_.push_back(pCallbackFun);
}

void AudioCaptureExtDriver::AddEventStatusCallBack(void* pFun) {
  boost::lock_guard<boost::mutex> lockguard_eventcallback(mutex_event_status_callback_);
  vecevent_status_callback_.push_back(pFun);
}

void AudioCaptureExtDriver::AddEventVolumeStrengthCallBack(void* pFun) {
  boost::lock_guard<boost::mutex> lockguard_eventcallback(mutex_event_volume_strength_callback_);
  vecevent_volume_strength_calalback_.push_back(pFun);
}


void AudioCaptureExtDriver::CallBackEventStatus(int iEventID) {
  YSOS_LOG_DEBUG("in AudioCaptureExtDriver::CallBackEvent: " << iEventID);
  boost::lock_guard<boost::mutex> lockguard_eventcallback(mutex_event_status_callback_);
  ///< 只有状态切换的时候才callback
  static int ilatest_eventid = -1;
  if (ilatest_eventid == iEventID)
    return;
  ilatest_eventid = iEventID;
  std::vector<void*>::iterator it_event = vecevent_status_callback_.begin();
  for (it_event; it_event != vecevent_status_callback_.end(); ++it_event) {
    AudioCaptureCallBackEventStatusFun pfun = reinterpret_cast<AudioCaptureCallBackEventStatusFun>(*it_event);
    pfun(iEventID);
  }
}


void AudioCaptureExtDriver::SetShowVolumeStrength(bool bShowOrFalse) {
  show_volume_strength_atomic_ = bShowOrFalse ? 1: false;
}


void AudioCaptureExtDriver::CallBackEventVolumeStrength(char* pData, UINT32 uDataLength) {
  if (0 == show_volume_strength_atomic_)
    return;

  assert(pData && uDataLength >0);
  if (NULL == pData || uDataLength <=0)
    return;

  ///< 160ms 接收一次，只抛出一次，在单个页面中测试是不卡的，但在程序里不知道会怎么样，
  int imax_strength =0;
  int itempstrength =0;
  ///< 计算音量强度, 这里只计算最大音量
  short* pshortdata =reinterpret_cast<short*>(pData);
  for (DWORD i=1; i<(uDataLength/2); i++) {
    itempstrength = (int)(/*pData[i]*/ pshortdata[i] * /*rcBK.Height() */100/ 0xffff);
    itempstrength = abs(itempstrength);  ///< 使用绝对值
    if (imax_strength < itempstrength)
      imax_strength = itempstrength;   ///<  要注意的是，值的范围是 [0 -49]
  }

  ///< 为了避免很卡，这里320ms响应一次
  static int iall_max_strength =0;
  static int icalltime =0;
  ++icalltime;

  if (iall_max_strength < imax_strength)
    iall_max_strength = imax_strength;

  if (icalltime <2) { ///< 320ms响应一次 160*2
    return;
  } else {
    icalltime =0;
  }

  {
    boost::lock_guard<boost::mutex> lockguard_eventcallback(mutex_event_volume_strength_callback_);

    std::vector<void*>::iterator it_event = vecevent_volume_strength_calalback_.begin(); ///< 向回调函数回调音量强度
    for (it_event; it_event != vecevent_volume_strength_calalback_.end(); ++it_event) {
      AudioCaptureCallBackEventVolumeStrengthFun pfun = reinterpret_cast<AudioCaptureCallBackEventVolumeStrengthFun>(*it_event);
      pfun(/*imax_strength*/iall_max_strength);
    }
  }
  iall_max_strength=0;

}
#else
    //TODO:add for linux
#endif

//////////////////////////////////////////////////////////////////////////
int AudioCaptureExtDriver::Read(BufferInterfacePtr pBuffer, DriverContexPtr context_ptr) {
  return YSOS_ERROR_SUCCESS;
}

int AudioCaptureExtDriver::Write(BufferInterfacePtr input_buffer, BufferInterfacePtr output_buffer, DriverContexPtr context_ptr) {
  return YSOS_ERROR_SUCCESS;
}

int AudioCaptureExtDriver::Ioctl(int iCtrlID, BufferInterfacePtr pInputBuffer, BufferInterfacePtr pOutputBuffer) {
  int result = YSOS_ERROR_FAILED;

  Properties property = static_cast<Properties>(iCtrlID);
  switch (property) {
  case CMD_ICC_START: {
    result = Start();
  }
  break;

  case CMD_ICC_STOP: {
    result = Stop();
  }
  break;

  case CMD_AUDIO_CAPTURE_RUN: {
    YSOS_LOG_DEBUG("command PROP_RUN in");
    capture_switch_ = true;
    result = YSOS_ERROR_SUCCESS;
  }
  break;

  case CMD_AUDIO_CAPTURE_STOP: {
    YSOS_LOG_DEBUG("command PROP_STOP in");
    capture_switch_ = false;
    result = YSOS_ERROR_SUCCESS;
  }
  break;

  default: {
    YSOS_LOG_DEBUG("error, ioctrl not support,iCtrlID id:" << iCtrlID);
    break;
  }
  }  ///< end switch

  return result;
}


int AudioCaptureExtDriver::GetProperty(int iTypeId, void *piType) {
  int result = YSOS_ERROR_FAILED;

  Properties property = static_cast<Properties>(iTypeId);
  switch (property) {
  case /*BufferLength*/PROP_BUFFER_LENGTH: {  ///< Buffer_Length：Buffer size per block
    UINT32* pudata = reinterpret_cast<UINT32*>(piType);
    *pudata = uwave_buffer_data_size_pernum_;
    result = YSOS_ERROR_SUCCESS;
    break;
  }

  case /*BufferNumber*/PROP_BUFFER_NUMBER: {  ///< Buffer_Number：Buffers block num
    UINT32* pudata = reinterpret_cast<UINT32*>(piType);
    *pudata = uwave_buffer_num_;
    result = YSOS_ERROR_SUCCESS;
    break;
  }

  case /*MemoryBlockAlign*/PROP_MEMORY_BLOCK_ALIGN: {  ///< MemoryBlockAlign.get need memory block Align
    UINT32* pudata = reinterpret_cast<UINT32*>(piType);
    *pudata = nblock_align_;
    result = YSOS_ERROR_SUCCESS;
    break;
  }

  case PROP_FUN_CALLABILITY: {
    FunObjectCommon2* pobject = reinterpret_cast<FunObjectCommon2*>(piType);
    if (NULL == pobject) {
      YSOS_LOG_DEBUG("error call property PROP_Fun_CallAbility, the param is null");
    }
    AbilityParam* pin = reinterpret_cast<AbilityParam*>(pobject->pparam1);
    AbilityParam* pout = reinterpret_cast<AbilityParam*>(pobject->pparam2);
    result = CallAbility(pin, pout);
    break;
  }

  default: {
    YSOS_LOG_DEBUG("not support property, id: " << property);
    break;
  }
  }

  return result;
}

int AudioCaptureExtDriver::SetProperty(int iTypeId, void *piType) {
  int result = YSOS_ERROR_FAILED;

  Properties property = static_cast<Properties>(iTypeId);
  switch (property) {
  case /*BufferNumber*/PROP_BUFFER_NUMBER: {  ///< Buffer_Number
    UINT32* pdata = reinterpret_cast<UINT32*>(piType);
    uwave_buffer_num_ = *pdata;
    result = YSOS_ERROR_SUCCESS;
    break;
  }

  case /*BufferLength*/PROP_BUFFER_LENGTH: {  ///< Buffer_Length
    assert(piType);
    UINT32* pdata = reinterpret_cast<UINT32*>(piType);
    uwave_buffer_data_size_pernum_ = *pdata;
    result = YSOS_ERROR_SUCCESS;
    break;
  }

  case /*MemoryBlockAlign*/PROP_MEMORY_BLOCK_ALIGN: {  ///< MemoryBlockAlign
    assert(piType);
    UINT32* pdata = reinterpret_cast<UINT32*>(piType);
    nblock_align_ = *pdata;
    result = YSOS_ERROR_SUCCESS;
    break;
  }

  case /*VoiceDataCallbackFun*/PROP_VOICE_DATA_CALLBACK_FUN: {  ///< VoiceDataCallBackFun：register call drive call back function:AudioCaptureDriverCallback
    veproperity_onread_callback_.push_back(piType);
    result = YSOS_ERROR_SUCCESS;
    break;
  }

  case PROP_INPUT_SOUND: {
    std::string* dir = reinterpret_cast<std::string*>(piType);
    pcm_dir_ = *dir;
    result = YSOS_ERROR_SUCCESS;
    break;
  }

  default: {
    YSOS_LOG_DEBUG("not support property，id:" << iTypeId);
    break;
  }
  }  ///< end switch

  return result;
}

int AudioCaptureExtDriver::CallAbility(AbilityParam* pInput, AbilityParam* pOut) {
  int result = YSOS_ERROR_FAILED;

  do {
    if (NULL == pOut) {
      YSOS_LOG_DEBUG("error call ablicity , input/out alibityparam is null");
      break;
    }
    if (strcasecmp(ABSTR_VOICERECORD, pOut->ability_name) ==0) {
      if (strcasecmp(DTSTR_STREAM_PCMX1X, pOut->data_type_name) ==0) {
        result = AbilityVoiceRecord_STREAM_PCMX1X(pInput, pOut);
      }
    } else {
      YSOS_LOG_DEBUG("error not support ability");
    }
  } while (0);

  return result;
}

int AudioCaptureExtDriver::AbilityVoiceRecord_STREAM_PCMX1X(AbilityParam* input_param, AbilityParam* output_param) {
  int result = YSOS_ERROR_FAILED;
  YSOS_LOG_DEBUG("AudioCaptureExtDriver::AbilityVoiceRecord_STREAM_PCMX1X in [Enter]");
  do {
    if (NULL == output_param->buf_interface_ptr) {
      YSOS_LOG_DEBUG("output_param->buf_interface_ptr is null");
      break;
    }
    YSOS_LOG_DEBUG("AudioCaptureExtDriver::AbilityVoiceRecord_STREAM_PCMX1X [Check][0]");
    // out put the data
    UINT8* pout = NULL;
    UINT32 pout_size =0;
    output_param->buf_interface_ptr->GetBufferAndLength(&pout, &pout_size);
    {
      UINT32 max_length = 0, prefix_length =0;
      output_param->buf_interface_ptr->GetMaxLength(&max_length);
      output_param->buf_interface_ptr->GetPrefixLength(&prefix_length);
      pout_size = max_length - prefix_length;
    }
    YSOS_LOG_DEBUG("AudioCaptureExtDriver::AbilityVoiceRecord_STREAM_PCMX1X [Check][1]");
    {
      std::string artificail_flag = "0";
      //data_ptr_->GetData("ZXFM", artificail_flag);
      YSOS_LOG_DEBUG("artificail_flag = " << artificail_flag);

      ///< 把已录到的数据 放到callback interface中，同时再给系统去录音
      boost::lock_guard<boost::mutex> lock_guard_wave_data(mutex_list_have_wave_data_);
      if (list_have_wave_data_.size() > 0) {
        WAVEHDR* pwave_data = *list_have_wave_data_.begin();
        if (pout_size < pwave_data->dwBufferLength) {
          YSOS_LOG_ERROR("error, pout_size is too small, pout_size: " << pout_size << "pwave_data->dwBytesRecorded: " << pwave_data->dwBytesRecorded);
        }
        YSOS_LOG_DEBUG("AudioCaptureExtDriver::AbilityVoiceRecord_STREAM_PCMX1X [Check][2]");
        if (pout_size >= pwave_data->dwBytesRecorded) {
          //if (capture_switch_ && "2" != artificail_flag) { ///< 全人工
          YSOS_LOG_DEBUG("AudioCaptureExtDriver::AbilityVoiceRecord_STREAM_PCMX1X [Check][3]");
          if (capture_switch_) { ///< 半人工
            //memcpy_s(pout, pout_size, pwave_data->lpData, pwave_data->dwBytesRecorded);
            memcpy(pout, pwave_data->lpData, pwave_data->dwBytesRecorded);
            output_param->buf_interface_ptr->SetLength(pwave_data->dwBytesRecorded);
          } else {
            output_param->buf_interface_ptr->SetLength(0);
          }
          YSOS_LOG_DEBUG("AudioCaptureExtDriver::AbilityVoiceRecord_STREAM_PCMX1X [Check][4]");
          list_have_wave_data_.pop_front();
          PrepareData(pwave_data);
          YSOS_LOG_DEBUG("AudioCaptureExtDriver::AbilityVoiceRecord_STREAM_PCMX1X [Check][5]");
          result = YSOS_ERROR_SUCCESS;
          break;
        } else {
          YSOS_LOG_DEBUG("pout_size not enough");
          break;
        }
        YSOS_LOG_DEBUG("AudioCaptureExtDriver::AbilityVoiceRecord_STREAM_PCMX1X [Check][6]");
      } else {
        result = YSOS_ERROR_SKIP;  ///< 让callbackup skip
        YSOS_LOG_DEBUG("AudioCaptureExtDriver::AbilityVoiceRecord_STREAM_PCMX1X [Check][7]");
        break;
      }
    }
    YSOS_LOG_DEBUG("AudioCaptureExtDriver::AbilityVoiceRecord_STREAM_PCMX1X [Check][8]");
  } while (0);
  YSOS_LOG_DEBUG("AudioCaptureExtDriver::AbilityVoiceRecord_STREAM_PCMX1X [Check][9]");
  YSOS_LOG_DEBUG("AudioCaptureExtDriver::AbilityVoiceRecord_STREAM_PCMX1X [End]");
  return result;
}

}