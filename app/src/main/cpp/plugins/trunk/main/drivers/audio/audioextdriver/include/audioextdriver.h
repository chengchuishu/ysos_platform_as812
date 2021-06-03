/**
 *@file audioextdriver.h
 *@brief audioext interface
 *@version 1.0
 *@author l.j..
 *@date Created on: 2016-10-20 13:48:23
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

#ifndef AUDIOEXT_INTERFACE_H_  //NOLINT
#define AUDIOEXT_INTERFACE_H_  //NOLINT

#ifdef _WIN32
/// windows headers
#include <Windows.h>
#include <assert.h>
#else
#include <cassert>
#endif

/// stl headers
#include <vector>  ///< TODO 放到前面会报错： Found C system header after C++ system header.  //NOLINT

/// boost headers
#include <boost/shared_ptr.hpp>
#include <boost/thread/thread.hpp>
#include <boost/shared_array.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/atomic.hpp>

/// ysos headers
#include "../../../../../../core/trunk/main/public/include/sys_interface_package/common.h"
#include "../../../../../../core/trunk/main/public/include/sys_interface_package/bufferpool.h"
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basedriverimpl.h"

/// private headers
#include "../../../public/include/common/commonenumdefine.h"
#include "../../../public/include/common/commonstructdefine.h"
#include "../include/scenesinterface.h"
#include "../include/XFAsr.h"

namespace ysos {

/**
  *@brief AudioExtDriver for windows.
  */
class YSOS_EXPORT AudioExtDriver: public BaseDriverImpl {
  DISALLOW_COPY_AND_ASSIGN(AudioExtDriver)
  DECLARE_PROTECT_CONSTRUCTOR(AudioExtDriver)
  DECLARE_CREATEINSTANCE(AudioExtDriver)

 public:
  //friend int ThreadStartAsrRecordingWithInterruption(const int iCurrentStartAsrCount, AudioExtDriver* pCXFAsr, const int iOwnerid,const int lFlag);

  virtual ~AudioExtDriver();

  /**
   *@brief 从底层驱动中读取数据  // NOLINT
   *@param pBuffer[Output]： 读取的数据缓冲  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Read(BufferInterfacePtr pBuffer, DriverContexPtr context_ptr=NULL);

  /**
   *@brief 写数据到底层驱动中  // NOLINT
   *@param pBuffer[Input]： 写入的数据缓冲  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Write(BufferInterfacePtr input_buffer, BufferInterfacePtr output_buffer=NULL, DriverContexPtr context_ptr=NULL);

  /**
   *@brief 控制驱动状态/配置参数到驱动/读取状态等  // NOLINT
   *@param iCtrlID[Input]： ID  // NOLINT
   *@param pInputBuffer[Input]： 写入的数据缓冲  // NOLINT
   *@param pOutputBuffer[Output]： 读出的数据缓冲  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Ioctl(int iCtrlID, BufferInterfacePtr pInputBuffer, BufferInterfacePtr pOutputBuffer);

  /**
   *@brief 打开底层驱动，并传入配置参数  // NOLINT
   *@param pParams[Input]： 驱动所需的打开参数  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Open(void *pParams = NULL);

  /**
   *@brief 打开底层驱动，并传入配置参数  // NOLINT
   *@param ： 无  // NOLINT
   *@return： 无  // NOLINT
   */
  virtual void Close(void *pParams = NULL);

  /**
   *@brief 获取接口的属性值  // NOLINT
   *@param type_id[Input]： 属性的ID  // NOLINT
   *@param type[Input/Output]：属性值的值  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int GetProperty(int iTypeId, void *piType);

  /**
   *@brief 设置接口的属性值  // NOLINT
   *@param type_id[Input]： 属性的ID  // NOLINT
   *@param type[input]： 属性值的值  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int SetProperty(int iTypeId, void *piType);

 protected:
  /**
  *@brief  配置文件中设置的Key、Value对，会在Initialized函数中调用 //  NOLINT
  *@param key 配置参数中的Key  // NOLINT
  *@param value 与Key对应的Value  // NOLINT
  *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
  */
  virtual int Initialized(const std::string &key, const std::string &value);

 protected:
  virtual int CallAbility(AbilityParam* pInput, AbilityParam* pOut);
  virtual int AbilityASR_PCM2Textpure(AbilityParam* pInput, AbilityParam* pOut);
  virtual int AbilityASRNLP_PCM2Json(AbilityParam* pInput, AbilityParam* pOut);
  virtual int AbilityNLP_Text2Json(AbilityParam* pInput, AbilityParam* pOut);
  virtual int AbilityNlp_Json2Json(AbilityParam* pInput, AbilityParam* pOut);
  virtual int AbilityTTS_Text2PCMP2(AbilityParam* pInput, AbilityParam* pOut);
  virtual int AbilityTTS_Text2PCM(AbilityParam* pInput, AbilityParam* pOut);

  /**
   *@brief
   *@param extra_params[Input] 通过@切分字符串得到的数组
   *@return
   */
  virtual int AbilityTTS_JsonExtract2PCMP2(AbilityParam* pInput, std::vector<std::string>& extra_params, AbilityParam* pOut);

  /**
  *@brief 注册或取消注册 module的事件接收
  *@param is_register[Input] 是否是注册
  *@param module_interface[Input] module的指针
  *@param ability_name[Input] 能力名称，不能为null, 如果为空字符串同时为取消注册，则取消注册所有的事件
  *@return
  */
  virtual int RegisterOrUnregisterAbilityEvent(bool is_register, BaseInterface* module_interface, std::string* ability_name);

 private:
  static CXFAsr cxfasr_;  ///< 这里设为静态变量，一个进程只有一个对象

  FILE *tts_file_;
  std::string tts_pcm_dir_;
};

typedef boost::shared_ptr<AudioExtDriver> AudioCaptureDriverPtr;

}  //< namespace ysos
#endif  //< AUDIOEXT_INTERFACE_H_
