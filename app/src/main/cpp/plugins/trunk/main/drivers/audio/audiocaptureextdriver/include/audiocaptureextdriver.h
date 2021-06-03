/**
 *@file audiocaptureextdriver.h
 *@brief nlpext interface
 *@version 1.0
 *@author l.j..
 *@date Created on: 2016-10-20 13:48:23
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef AUDIOCAPTRUEEXT_INTERFACE_H_  //NOLINT
#define AUDIOCAPTRUEEXT_INTERFACE_H_  //NOLINT

#ifdef _WIN32
#include <Windows.h>
#include <mmsystem.h>  ///< 系统录音头文件
#else
#endif

#include <assert.h>

#include <vector>  ///< TODO 放到前面会报错： Found C system header after C++ system header.  //NOLINT

#include <boost/shared_ptr.hpp>  //NOLINT
#include <boost/thread/thread.hpp>  //NOLINT
#include <boost/shared_array.hpp>  //NOLINT
#include <boost/thread/mutex.hpp>   //NOLINT
#include <boost/atomic/atomic.hpp>
 
/// ysos headers
#include "../../../../../../core/trunk/main/public/include/sys_interface_package/common.h"
#include "../../../../../../core/trunk/main/public/include/sys_interface_package/bufferpool.h"  //NOLIN"
#include "../../../../../../core/trunk/main/public/include/core_help_package/baseinterfaceimpl.h"
#include "../../../../../../core/trunk/main/public/include/os_hal_package/driverinterface.h"
#include "../../../../../../core/trunk/main/protect/include/os_hal_package/datainterfacemanager.h"

/// private headers
#include "../../../public/include/common/commonenumdefine.h"
#include "../../../public/include/common/commonstructdefine.h"

#include "../include/linuxrec.h"

#ifdef _WIN32
//#pragma comment(lib, "winmm")  ///< 系统录音库
#else
#define E_SR_NOACTIVEDEVICE		1
#define E_SR_NOMEM				    2
#define E_SR_INVAL				    3
#define E_SR_RECORDFAIL		    4
#define E_SR_ALREADY			    5

#define DEFAULT_FORMAT		\
{\
	WAVE_FORMAT_PCM,	\
	1,			\
	16000,			\
	32000,			\
	2,			\
	16,			\
	sizeof(WAVEFORMATEX)	\
}

enum sr_audsrc
{
	SR_MIC,	/* write data from mic */
	SR_USER	/* write data from user by calling API */
};

struct speech_rec {
	enum sr_audsrc aud_src;  /* from mic or manual  stream write */
	//struct speech_rec_notifier notif;
	const char * session_id;
	int ep_stat;
	int rec_stat;
	int audio_status;
	struct recorder *recorder;
	volatile int state;
	//char * session_begin_params;
};

/* wave data block header */
typedef struct wavehdr_tag {
    char*          lpData;                 /* pointer to locked data buffer */
    unsigned long  dwBufferLength;         /* length of data buffer */
    unsigned long  dwBytesRecorded;        /* used for input only */
    //DWORD_PTR   dwUser;                 /* for client's use */
    unsigned long  dwFlags;                /* assorted flags (see defines) */
    unsigned long  dwLoops;                /* loop control counter */
    //struct wavehdr_tag FAR *lpNext;     /* reserved for driver */
    struct wavehdr_tag *lpNext;     /* reserved for driver */
    //DWORD_PTR   reserved;               /* reserved for driver */
} WAVEHDR;
#endif

namespace ysos {

typedef boost::shared_ptr<WAVEHDR> WAVEHDRPtr;
typedef boost::shared_array<char> MySharedCharBuffer;

// struct AudioCaptureDriverCallBack {
//  void* pcall_fun_address_;
//  std::string strparam;
//};
/**
  *@brief AudioCaptureExtDriver for windows.
  */
//class AudioCaptureExtDriver {
class YSOS_EXPORT AudioCaptureExtDriver: public DriverInterface, public BaseInterfaceImpl {
  DISALLOW_COPY_AND_ASSIGN(AudioCaptureExtDriver)
  DECLARE_PROTECT_CONSTRUCTOR(AudioCaptureExtDriver)
  DECLARE_CREATEINSTANCE(AudioCaptureExtDriver)
public:
  // enum PropertiesEx {
  //  BufferLength = 0,       ///< buffer length ,datatype:UINT32
  //  BufferNumber,           ///< datatype:UINT32
  //  MemoryBlockAlign  ,      ///< block align (unit:Byte), datatype:UINT32
  //  VoiceDataCallbackFun      ///< voice data callback function
  // };
  enum AudioCaptureStatus {
    TDACS_CLOSE =0,
    TDACS_OPEN =1,
    TDACS_START =2,
    TDACS_STOP =3,
  };
  enum AudioCaptureStatusEx {
    TDACS_DEVICE_FAIL = 10, // 设备连不上
  };
  //AudioCaptureExtDriver::AudioCaptureExtDriver();
  virtual ~AudioCaptureExtDriver();

  //////////////////////////////////////////////////////////////////////////
  /**
   *@brief inherit from DriverInterface;
   *@param pBuffer[Input/Output]
   *@return success return YSOS_ERROR_SUCCESS，
   *        fail return one of the error types
   */
  //virtual int Read(BufferInterfacePtr pBuffer);
  virtual int Read(BufferInterfacePtr pBuffer, DriverContexPtr context_ptr=NULL);

  /**
   *@brief inherit from DriverInterface;
   *@param pBuffer[Input/Output]
   *@return success return YSOS_ERROR_SUCCESS
   *        fail return one of the error types
   */
  //virtual int Write(BufferInterfacePtr pBuffer);
  virtual int Write(BufferInterfacePtr input_buffer, BufferInterfacePtr output_buffer=NULL, DriverContexPtr context_ptr=NULL);

  /**
   *@brief inherit from DriverInterface ,setting input or output buffer
   *@param iCtrlID[Input]
   *       0 set output buffer, relate param pOutputBuffer
   *       1 start record
   *       2 stop record
   *@param pInputBuffer[Input/Output]
   *@param pOutputBuffer[Input/Output]
   *@return success return YSOS_ERROR_SUCCESS，
   *        fail return one of the error types
   */
  virtual int Ioctl(int iCtrlID, BufferInterfacePtr pInputBuffer, BufferInterfacePtr pOutputBuffer);

  /**
   *@brief inherit from BaseInterface
   *@return success return YSOS_ERROR_SUCCESS,
   *        fail return one of(YSOS_ERROR_LOGIC_ERROR)
   */
  virtual int RealInitialize(void *param=NULL);

      /**
  *@brief 在系统退出时，会依次调Module、Callback和Driver的uninitialize的,在Close后
   *@param 
   *@return   
   */
  virtual int RealUnInitialize(void *param=NULL);

  /**
   *@brief inherit from BaseInterface
   *@param iTypeId[Input] property id
   *@param piType[Output] property value
   *@return success return YSOS_ERROR_SUCCESS,
   *        fail return one of (YSOS_ERROR_INVALID_ARGUMENTS)
   */
  virtual int GetProperty(int iTypeId, void *piType);

  /**
   *@brief inherit from BaseInterface
   *@param iTypeId[Input] property id
   *       if iTypeId = VoiceDataCallbackFun, Then piType is a function pointer like void (*AudioCaptureDriverOnReadCallback)(BufferInterfacePtr pOutputBuffer);
   *@param piType[Input] property value
   *@return success return YSOS_ERROR_SUCCESS,
   *        fail return (YSOS_ERROR_INVALID_ARGUMENTS)
   */
  virtual int SetProperty(int iTypeId, void *piType);

  //////////////////////////////////////////////////////////////////////////
  /**
   *@brief inherit from DriverInterface
   *@      only open device(not include start recorder)
   *@param pParams[Input]
   *@return success return YSOS_ERROR_SUCCESS,
   *        fail return (YSOS_ERROR_MEMORY_ALLOCATED_FAILED,YSOS_ERROR_LOGIC_ERROR)
   */
  virtual int Open(void *pParams);

  /**
   *@brief inherit from DriverInterface
   */
  virtual void Close(void *pParams = NULL);

  //virtual void AddEventRecordeVoiceDataCallBack(void* pCallbackFun);
  /**
  *@brief start recorder
  *@return success return YSOS_ERROR_SUCCESS
  *        fail return one of(YSOS_ERROR_INVALID_ARGUMENTS,YSOS_ERROR_LOGIC_ERROR,YSOS_ERROR_MEMORY_ALLOCATED_FAILED,YSOS_ERROR_FAILED)
  *                    return YSOS_ERROR_NOT_ENOUGH_RESOURCE if no capture driver
  */
  virtual int Start();

  /**
   *@brief 如果当前设备是关闭的，则先关闭再打开，如果设备是可用的，则直接打开
   *@param 
   *@return   
   */
   virtual int StartEx();
  /**
   *@brief stop recorder
   *@return success return YSOS_ERROR_SUCCESS
   *        fail return one of(YSOS_ERROR_INVALID_ARGUMENTS,YSOS_ERROR_LOGIC_ERROR,YSOS_ERROR_MEMORY_ALLOCATED_FAILED,YSOS_ERROR_FAILED)
   */
  virtual int Stop();

  UINT32 device_current_status();
  inline bool is_reseting();
  inline void set_is_reseting(bool bIsResetting);
 // inline void set_wave_handle(HWAVEIN hwi);

  //void AddEventStatusCallBack(void* pFun);
  //void AddEventVolumeStrengthCallBack(void* pFun);
  
  /**
   *@brief 是否显示当前音量
   *@param bShowOrFalse[Input] 是否显示，如果为1表示会回调音量事件，如果为0则不会回调音量事件
   *@return 
   */
  //void SetShowVolumeStrength(bool bShowOrFalse);

  //virtual int GetAudioCaptureVoiceFormat(int& format_tag, int& channels, int& samples_per_seconds, int& avg_bytes_per_second, int& block_align, int& bits_per_sample, int& cbsize);
  //virtual int GetAudioCaptureVoiceFormat(WAVEFORMATEX& voice_format);

  //void SetCurVolume(DWORD dwValue);
 protected:
   //////////////////////////////////////////////////////////////////////////
   virtual int CallAbility(AbilityParam* pInput, AbilityParam* pOut);
   virtual int AbilityVoiceRecord_STREAM_PCMX1X(AbilityParam* input_param, AbilityParam* output_param);

   //////////////////////////////////////////////////////////////////////////
  //friend void CALLBACK waveInCallbackProc(HWAVEIN hwi, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);

  /**
   *@brief 在异常情况下，关闭设备，释放资源。
   *       异常情况：突然设备无效被拔掉
   *@param 
   *@return   
   */
  //int CloseWithException();


  /**
  *@brief get method, data member:hwave_handle_
  *@return success return recorder
  *        fail return NULL
  */
  /*inline HWAVEIN wave_handle() const {
    return hwave_handle_;
  }*/

  /**
  *@brief get method, data member:dwprocess_callback_wave_threadid_
  *@return success return recorder call back thread
  *        fail return NULL
  */
  //inline DWORD process_callback_wave_threadid() const {
  //  return dwprocess_callback_wave_threadid_;
  //}


  /**
   *@brief thread function for wave recorder call back
   *@param lParam[Input] WindowsAudioCaptureDriver* address point
   *@return success return 0
   *        fail return non zero value
   */
  //friend UINT ThreadProcessCallbackWaveData(LPARAM lParam);

  /**
   *@brief process recorder call back wave data
   *@param pWavehdr[Input] have recorder wave date header address
   *@return 0 success ,-1 fail
   */
  //virtual int ProcessWaveCallbackData(WAVEHDR* pWavehdr);

  //void CallBackEventStatus(int iEventID);
  //void CallBackEventVolumeStrength(char* pData, UINT32 uDataLength);
  
  //////////////////////////////////////////////////////////////////////////
  int PrepareData(WAVEHDR* pWavehdr);
  //void wait_for_rec_stop(struct recorder *rec, unsigned int timeout_ms);
  //void end_sr_on_error(struct speech_rec *sr, int errcode);
  //int sr_write_audio_data(struct speech_rec *sr, char *data, unsigned int len);
  //void iat_cb(char *data, unsigned long len, void *user_para);
  //int sr_init_ex(struct speech_rec *sr, enum sr_audsrc aud_src, record_dev_id devid);

  //////////////////////////////////////////////////////////////////////////
 private:
  //boost::atomic<int> device_current_status_atomic_;   ///< default is TDACS_CLOSE
  std::vector<void*> veproperity_onread_callback_;          ///< wave data need call back funcation address list
  boost::mutex mutex_veproperity_onread_callback_;
  //std::vector<AudioCaptureDriverCallBack> veproperity_onread_callback_;          ///< wave data need call back funcation address list
  //HWAVEIN hwave_handle_;                    ///< wave handle of open recorder device,default value 0
  unsigned int dwprocess_callback_wave_threadid_;  ///< thread id of windows recorder thread call back ,default value:0
  void* event_syn_thread_process_callback_wave_exit;   ///< event for synchronize dwprocess_callback_wave_threadi exist;
  boost::atomic<int> thread_process_callback_wave_exit_flag_;   ///< 是否退出线程标记，如果是1表示出，默认是0
  std::vector<MySharedCharBuffer> vewave_output_buffer_;   ///< pointer to data buffer(not inclue prefix), the buffer generate by outside
  std::vector<WAVEHDRPtr> vewave_output_buffer_headaddress_;
  UINT32 uwave_buffer_data_size_pernum_;   ///< data buffer size (not include wave headeer), default value:1024;
  UINT32 uwave_buffer_num_;                ///< buffer number,synchronize with vecwave_output_buffer,default value 4;

  /// windows wave format param
  /// Waveform-audio format type. Format tags are registered with Microsoft Corporation for many compression algorithms.
  /// A complete list of format tags can be found in the Mmreg.h header file. For one- or two-channel PCM data,
  /// this value should be WAVE_FORMAT_PCM. When this structure is included in a WAVEFORMATEXTENSIBLE structure,
  /// this value must be WAVE_FORMAT_EXTENSIBLE.
  /// default value: WAVE_FORMAT_PCM;
  unsigned short wformat_tag_;

  /// windows wave format param
  /// Number of channels in the waveform-audio data. Monaural data uses one channel and stereo data uses two channels.
  /// default value:1
  unsigned short nchannels_;

  /// windows wave format param
  /// Sample rate, in samples per second (hertz). If wFormatTag is WAVE_FORMAT_PCM,
  /// then common values for nSamplesPerSec are 8.0 kHz, 11.025 kHz, 22.05 kHz, and 44.1 kHz.
  /// For non-PCM formats, this member must be computed according to the manufacturer's specification of the format tag.
  /// default value:16000
  unsigned int nsamples_per_sec_;

  /// windows wave format param
  /// Required average data-transfer rate, in bytes per second, for the format tag.
  /// If wFormatTag is WAVE_FORMAT_PCM, nAvgBytesPerSec should be equal to the product of nSamplesPerSec and nBlockAlign.
  /// For non-PCM formats, this member must be computed according to the manufacturer's specification of the format tag.
  /// default value:nblock_align_ * nsamples_per_sec_
  unsigned int navg_bytes_per_sec_;

  /// windows wave format param
  /// Block alignment, in bytes. The block alignment is the minimum atomic unit of data for the wFormatTag format type.
  /// If wFormatTag is WAVE_FORMAT_PCM or WAVE_FORMAT_EXTENSIBLE, nBlockAlign must be equal to the product of nChannels
  /// and wBitsPerSample divided by 8 (bits per byte). For non-PCM formats, this member must be computed according to
  /// the manufacturer's specification of the format tag.
  /// Software must process a multiple of nBlockAlign bytes of data at a time. Data written to and read from a device
  /// must always start at the beginning of a block. For example, it is illegal to start playback of PCM data in the middle
  /// of a sample (that is, on a non-block-aligned boundary).
  /// default value:(wbits_per_sample_ * nchannels_) / 8
  unsigned short nblock_align_;

  /// windows wave format param
  /// Bits per sample for the wFormatTag format type. If wFormatTag is WAVE_FORMAT_PCM, then wBitsPerSample should be
  /// equal to 8 or 16. For non-PCM formats, this member must be set according to the manufacturer's specification of
  /// the format tag. If wFormatTag is WAVE_FORMAT_EXTENSIBLE, this value can be any integer multiple of 8 and represents
  /// the container size, not necessarily the sample size; for example, a 20-bit sample size is in a 24-bit container.
  /// Some compression schemes cannot define a value for wBitsPerSample, so this member can be 0.
  /// default value:16
  unsigned short wbits_per_sample_;

  /// windows wave format param
  /// Size, in bytes, of extra format information appended to the end of the WAVEFORMATEX structure. This information
  /// can be used by non-PCM formats to store extra attributes for the wFormatTag. If no extra information is required
  /// by the wFormatTag, this member must be set to 0. For WAVE_FORMAT_PCM formats (and only WAVE_FORMAT_PCM formats),
  /// this member is ignored. When this structure is included in a WAVEFORMATEXTENSIBLE structure, this value must be at
  /// least 22.
  /// default value:0
  unsigned short cbsize_;

  /// todo not used now 对外提供的函数一律要加锁
  boost::mutex mutext_audio_driver_;              ///< mutext for public call
  bool bis_reseting_;   ///<  当前是否在reset操作。不是很好的解决方案，
  boost::mutex mutex_reseting_;   ///< mutex for variant bis_reseting

  std::vector<void*> vecevent_status_callback_;
  boost::mutex mutex_event_status_callback_;

  boost::atomic<int> close_counter_;  ///< 执行关闭设备的计数器，用于判断麦克风设备是否被外部拔出

  boost::mutex mutex_event_volume_strength_callback_;
  std::vector<void*> vecevent_volume_strength_calalback_;
  boost::atomic<int> show_volume_strength_atomic_;   ///<  是否显示当前的音量强度

 // WAVEFORMATEX wave_format_clone; ///< 对上面所有语音参数的一个备份,只有打开设备成功后才会有值。
  //add for linux //boost::mutex mutex_list_have_wave_data_;
  //add for linux //std::list<WAVEHDR*> list_have_wave_data_;   ///< 有wave数据的列表， 为了支持外部调用，而不是由自己触发

  bool capture_switch_;
  //add for linux //std::string pcm_dir_;
  //add for linux //FILE* pcm_file_;

  DataInterfacePtr data_ptr_;                       ///< 内存指针

};

typedef boost::shared_ptr<AudioCaptureExtDriver> AudioCaptureDriverPtr;

}  /// end of namespace ysos
#endif  /// <AUDIOCAPTRUEEXT_INTERFACE_H_   //NOLINT
