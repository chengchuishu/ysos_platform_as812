/**
 *@file audioplayextdriver.h
 *@brief the audio play ext call back
 *@version 1.0
 *@author l.j..
 *@date Created on: 2016-10-20 15:16:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

#ifndef AUDIOPLAYEXT_INTERFACE_H_  //NOLINT
#define AUDIOPLAYEXT_INTERFACE_H_  //NOLINT

#ifdef _WIN32
/// stl headers
#include <Windows.h>
#include <mmsystem.h>  ///< 系统录音头文件
#else
#endif

#include <assert.h>
#include <vector>  ///< TODO 放到前面会报错： Found C system header after C++ system header.  //NOLINT
#include <list>

/// boost headers
#include <boost/shared_ptr.hpp>  //NOLINT
#include <boost/thread/thread.hpp>  //NOLINT
#include <boost/shared_array.hpp>  //NOLINT
#include <boost/thread/mutex.hpp>   //NOLINT
#include <boost/atomic/atomic.hpp>

/// ysos headers
#include "../../../../../../core/trunk/main/public/include/sys_interface_package/common.h"
#include "../../../../../../core/trunk/main/public/include/sys_interface_package/bufferpool.h"  //NOLINT
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basedriverimpl.h"

/// private headers
#include "../../../public/include/common/commonenumdefine.h"
#include "../../../public/include/common/commonstructdefine.h"

//#pragma comment(lib, "winmm")  ///< 系统录音库

#ifdef _WIN32
#else
/* wave data block header */
typedef struct wavehdr_tag {
    char*          lpData;                 /* pointer to locked data buffer */
    unsigned long  dwBufferLength;         /* length of data buffer */
    unsigned long  dwBytesRecorded;        /* used for input only */
    //DWORD_PTR   dwUser;                 /* for client's use */
    unsigned long  dwFlags;                /* assorted flags (see defines) */
    unsigned long  dwLoops;                /* loop control counter */
    //struct wavehdr_tag FAR *lpNext;     /* reserved for driver */
    //DWORD_PTR   reserved;               /* reserved for driver */
} WAVEHDR;
#endif

namespace ysos {

typedef boost::shared_ptr<WAVEHDR> WAVEHDRPtr;
typedef boost::shared_array<char> MySharedCharBuffer;

struct VoiceDataBlock {
  WAVEHDRPtr wave_ptr_;
  MySharedCharBuffer voice_buffer_ptr_;
  unsigned int data_size_;
  bool is_end_;
  bool have_add_to_sysem_to_wave_out_write_;  /// have add to system play list(have down waveOutWrite oper)
  bool have_waveoutprepareheader_;
 public:
  VoiceDataBlock() {
    wave_ptr_ = NULL;
    voice_buffer_ptr_ = NULL;
    data_size_ = 0;
    is_end_ =false;
    have_add_to_sysem_to_wave_out_write_ = false;
    have_waveoutprepareheader_ = false;
  }

  bool operator == (const VoiceDataBlock& OtherVoiceDataBlock) {
    if (wave_ptr_ == OtherVoiceDataBlock.wave_ptr_
        && voice_buffer_ptr_ == OtherVoiceDataBlock.voice_buffer_ptr_)
      return true;

    return false;
  }
};

class YSOS_EXPORT AudioPlayExtDriver : public BaseDriverImpl {
  DISALLOW_COPY_AND_ASSIGN(AudioPlayExtDriver)
  DECLARE_PROTECT_CONSTRUCTOR(AudioPlayExtDriver)
  DECLARE_CREATEINSTANCE(AudioPlayExtDriver)
 public:
  enum AudioPlayStatus {
    TDAPS_CLOSE =0,
    TDAPS_OPEN =1,
    TDAPS_START =2,
    TDAPS_STOP =3,
    TDAPS_ERROR = 10
  };

  virtual ~AudioPlayExtDriver();

  /**
   *@brief inherit from DriverInterface;
   *@param pBuffer[Input/Output]
   *@return success return YSOS_ERROR_SUCCESS，
   *        fail return one of the error types
   */
  virtual int Read(BufferInterfacePtr pBuffer, DriverContexPtr context_ptr=NULL);

  /**
   *@brief inherit from DriverInterface;
   *@param pBuffer[Input/Output]
   *@return success return YSOS_ERROR_SUCCESS
   *        fail return one of the error types
   */
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

  /**
   *@brief inherit from BaseInterface
   *@return success return YSOS_ERROR_SUCCESS,
   *        fail return one of(YSOS_ERROR_LOGIC_ERROR)
   */
  virtual int Initialize(void *param=NULL);

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

 private:
  virtual int CallAbility(AbilityParam* pInput, AbilityParam* pOut);
/*
  friend void CALLBACK waveOutCallbackProc(
    HWAVEOUT   hwi,
    UINT      uMsg,
    DWORD_PTR dwInstance,
    DWORD_PTR dwParam1,
    DWORD_PTR dwParam2
  );
  friend UINT ThreadProcessWaveDone(LPARAM lParam);
*/

  /**
   *@brief stop recorder
   *@return success return YSOS_ERROR_SUCCESS
   *        fail return one of(YSOS_ERROR_INVALID_ARGUMENTS,YSOS_ERROR_LOGIC_ERROR,YSOS_ERROR_MEMORY_ALLOCATED_FAILED,YSOS_ERROR_FAILED)
   */
  virtual int Stop();

  virtual int AddToPlayList(void* pData, int iDataLength, bool bIsEnd);

  /**
   *@brief set current play list endest data is the end data block
   *@param
   *@return
   */
  virtual int SetPlayListEnd();

  /**
   *@brief
   *@param iClearMode[Input] 0 clear all, 1 clear that not add to system wave buffer
   *@return
   */
  virtual int ClearPlayList(int iClearMode =0);

  virtual int GetVoiceDataBlock(void* pDataAddress, VoiceDataBlock& VoiceBlock);

  virtual int RemoveFormPlayList(void* pDataAddress);

  virtual int Start();

  virtual int Pause();

  int ProcessWaveDone(WAVEHDR* pWavehdr);

  /**
  *@brief get method, data member:dwprocess_callback_wave_threadid_
  *@return success return recorder call back thread
  *        fail return NULL
  */
 /*
  inline DWORD process_callback_wave_threadid() const {
    return dwprocess_callback_wave_threadid_;
  }*/

  inline bool is_reseting_stop();

  inline void set_is_reseting_stop(bool bIsResetting);

  inline bool is_reseting_close();

  inline void set_is_reseting_close(bool bIsResetting);

  /**
  *@brief get method, data member:hwave_handle_
  *@return success return recorder
  *        fail return NULL
  */
  /*inline HWAVEOUT wave_handle() const {
    return hwave_handle_;
  }*/

  UINT32 device_current_status();

 private:
  boost::atomic<int> device_current_status_atomic_;   ///< default is TDACS_CLOSE
  std::vector<void*> veproperity_onread_callback_;          ///< wave data need call back funcation address list
  boost::mutex mutex_veproperity_onread_callback_;
  //HWAVEOUT hwave_handle_;                    ///< wave handle of open recorder device,default value 0
  //DWORD dwprocess_callback_wave_threadid_;  ///< thread id of windows recorder thread call back ,default value:0
  std::list<VoiceDataBlock> playsound_voice_datas_;
  boost::mutex mutext_playsound_voice_datas_;

  UINT32 uwave_buffer_data_size_pernum_;   ///< data buffer size (not include wave headeer), default value:1024;
  UINT32 uwave_buffer_num_;                ///< buffer number,synchronize with vecwave_output_buffer,default value 4;

  /// windows wave format param
  /// Waveform-audio format type. Format tags are registered with Microsoft Corporation for many compression algorithms.
  /// A complete list of format tags can be found in the Mmreg.h header file. For one- or two-channel PCM data,
  /// this value should be WAVE_FORMAT_PCM. When this structure is included in a WAVEFORMATEXTENSIBLE structure,
  /// this value must be WAVE_FORMAT_EXTENSIBLE.
  /// default value: WAVE_FORMAT_PCM;
  //WORD wformat_tag_;

  /// windows wave format param
  /// Number of channels in the waveform-audio data. Monaural data uses one channel and stereo data uses two channels.
  /// default value:1
  //WORD nchannels_;

  /// windows wave format param
  /// Sample rate, in samples per second (hertz). If wFormatTag is WAVE_FORMAT_PCM,
  /// then common values for nSamplesPerSec are 8.0 kHz, 11.025 kHz, 22.05 kHz, and 44.1 kHz.
  /// For non-PCM formats, this member must be computed according to the manufacturer's specification of the format tag.
  /// default value:16000
 // DWORD nsamples_per_sec_;

  /// windows wave format param
  /// Required average data-transfer rate, in bytes per second, for the format tag.
  /// If wFormatTag is WAVE_FORMAT_PCM, nAvgBytesPerSec should be equal to the product of nSamplesPerSec and nBlockAlign.
  /// For non-PCM formats, this member must be computed according to the manufacturer's specification of the format tag.
  /// default value:nblock_align_ * nsamples_per_sec_
  //DWORD navg_bytes_per_sec_;

  /// windows wave format param
  /// Block alignment, in bytes. The block alignment is the minimum atomic unit of data for the wFormatTag format type.
  /// If wFormatTag is WAVE_FORMAT_PCM or WAVE_FORMAT_EXTENSIBLE, nBlockAlign must be equal to the product of nChannels
  /// and wBitsPerSample divided by 8 (bits per byte). For non-PCM formats, this member must be computed according to
  /// the manufacturer's specification of the format tag.
  /// Software must process a multiple of nBlockAlign bytes of data at a time. Data written to and read from a device
  /// must always start at the beginning of a block. For example, it is illegal to start playback of PCM data in the middle
  /// of a sample (that is, on a non-block-aligned boundary).
  /// default value:(wbits_per_sample_ * nchannels_) / 8
  //WORD nblock_align_;

  /// windows wave format param
  /// Bits per sample for the wFormatTag format type. If wFormatTag is WAVE_FORMAT_PCM, then wBitsPerSample should be
  /// equal to 8 or 16. For non-PCM formats, this member must be set according to the manufacturer's specification of
  /// the format tag. If wFormatTag is WAVE_FORMAT_EXTENSIBLE, this value can be any integer multiple of 8 and represents
  /// the container size, not necessarily the sample size; for example, a 20-bit sample size is in a 24-bit container.
  /// Some compression schemes cannot define a value for wBitsPerSample, so this member can be 0.
  /// default value:16
  //WORD wbits_per_sample_;

  /// windows wave format param
  /// Size, in bytes, of extra format information appended to the end of the WAVEFORMATEX structure. This information
  /// can be used by non-PCM formats to store extra attributes for the wFormatTag. If no extra information is required
  /// by the wFormatTag, this member must be set to 0. For WAVE_FORMAT_PCM formats (and only WAVE_FORMAT_PCM formats),
  /// this member is ignored. When this structure is included in a WAVEFORMATEXTENSIBLE structure, this value must be at
  /// least 22.
  /// default value:0
  //WORD cbsize_;

  ///< todo not used now 对外提供的函数一律要加锁
  boost::mutex mutext_audio_driver_;              ///< mutext for public call
  bool bis_reseting_stop_;   ///<  当前是否在reset操作。不是很好的解决方案，
  boost::mutex mutex_reseting_stop_;   ///< mutex for variant bis_reseting
  bool bis_reseting_close_;
  boost::mutex mutex_reseting_close_;

  std::vector<void*> vecevent_callback_;
  boost::mutex mutex_event_callback_;

  std::list<ModuleInterface*> list_baseinterface_event_notify_voiceplayerstatus_;  ///< 对module传出当前播放设备状态事件
  boost::mutex mutex_list_baseinterface_event_notify_voiceplayerstatus_;
  std::list<ModuleInterface*> list_baseinterface_event_notify_playdone_;
  boost::mutex mutext_list_baseinterface_event_notify_playdone_;

  bool send_start_;

};

typedef boost::shared_ptr<AudioPlayExtDriver> AudioPlayDriverPtr;

}  /// end of namespace ysos
#endif  ///< AUDIOCAPTRUEEXT_INTERFACE_H_   //NOLINT

