/**
  *@file powermanagerdriver.cpp
  *@brief Definition of PowerManagerDriver for windows.
  *@version 0.1
  *@author wangxg
  *@date Created on: 2016/11/22   17:00
  *@copyright Copyright(c) 2016 YunShen Technology. All rights reserved.
  *
*/

/// Private Headers
#include "../include/powermanagerdriver.h"

/// Public Headers
#include "../../../public/include/common/commonenumdefine.h"
#include "../../../public/include/common/commonstructdefine.h"

/// YSOS Headers
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"


#ifndef MAX_SEND_LEN
#define MAX_SEND_LEN (1000+6)
#endif

#ifndef FRAME_TAIL_FLAG
#define FRAME_TAIL_FLAG 0x03
#endif

#ifndef MIN
#define MIN(x,y) ((x)<(y)?(x):(y))
#endif

#ifdef  WIN32
#define SLEEP(a)      Sleep(a)
#else
#define SLEEP(a)      (boost::this_thread::sleep_for(boost::chrono::milliseconds(a)))
#endif

#define RECEIVE_ONE_TIMEOUT     50
#define RECEIVE_EMPTY_TIMEOUT   400
#define RECEIVE_EMPTY_SLEEP     50

#define _DEBUG 1

#ifdef _DEBUG
std::string bytestohexstring(char* bytes,int bytelength) {
  std::string str("");
  std::string str2("0123456789abcdef");
  for (int i=0; i<bytelength; i++) {
    int b;
    b = 0x0f&(bytes[i]>>4);
    char s1 = str2.at(b);
    str.append(1,str2.at(b));
    b = 0x0f & bytes[i];
    str.append(1,str2.at(b));
    char s2 = str2.at(b);
  }
  return str;
}
#endif

namespace ysos {

DECLARE_PLUGIN_REGISTER_INTERFACE(PowerManagerDriver, DriverInterface);
PowerManagerDriver::PowerManagerDriver(const std::string &strClassName): BaseDriverImpl(strClassName) {
  comport_port_ = 1;
  comport_baudrate_ = 9600;
  comport_parity_ = 0;
  comport_bytesize_ = 8;
  comport_stopbits_ = 0;
  comport_read_timeout_ = 10*1000;
  logger_ = GetUtility()->GetLogger("ysos.powermgr");
  return;
}

PowerManagerDriver::~PowerManagerDriver() {
  return;
}

int PowerManagerDriver::Initialize(void *param/*=NULL*/) {
  return BaseDriverImpl::Initialize(param);
}

int PowerManagerDriver::GetProperty(int itypeid, void *itypeptr) {
  if (NULL == itypeptr) {
    YSOS_LOG_ERROR("PowerManagerDriver GetProperty Function Parameter Error!");
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }

  UINT32 *typeptr = reinterpret_cast<UINT32*>(itypeptr);
  Properties propertyID = static_cast<Properties>(itypeid);

  switch (propertyID) {
  case PROP_POWERMGR_SERIAL_PORT:
    *typeptr = comport_port_ ;
    break;
  case PROP_POWERMGR_SERIAL_BAUDRATE:
    *typeptr = comport_baudrate_;
    break;
  case PROP_POWERMGR_SERIAL_PARITY:
    *typeptr = comport_parity_;
    break;
  case PROP_POWERMGR_SERIAL_BYTESIZE:
    *typeptr = comport_bytesize_;
    break;
  case PROP_POWERMGR_SERIAL_STOPBITS:
    *typeptr = comport_stopbits_ ;
    break;
  case PROP_POWERMGR_SERIAL_READ_TIMEOUT:
    if (YSOS_ERROR_SUCCESS != GetReadTimeOut(typeptr)) {
      return YSOS_ERROR_FAILED;
    }
    break;
  default:
    YSOS_LOG_INFO("PowerManagerDriver GetProperty Function Not Support This Method!");
    return YSOS_ERROR_NOT_SUPPORTED;
  }  ///< end switch

  return YSOS_ERROR_SUCCESS;
}

int PowerManagerDriver::SetProperty(int itypeid, void *itypeptr) {
  if (NULL == itypeptr) {
    YSOS_LOG_ERROR("PowerManagerDriver SetProperty Function Parameter Error!");
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }
  Properties propertyID = static_cast<Properties>(itypeid);
  UINT32 *typeptr = reinterpret_cast<UINT32*>(itypeptr);

  switch (propertyID) {
  case PROP_POWERMGR_SERIAL_PORT:
    comport_port_ = *typeptr;
    break;
  case PROP_POWERMGR_SERIAL_BAUDRATE:
    comport_baudrate_ = *typeptr;
    break;
  case PROP_POWERMGR_SERIAL_PARITY:
    comport_parity_ = *typeptr;
    break;
  case PROP_POWERMGR_SERIAL_BYTESIZE:
    comport_bytesize_ = *typeptr;
    break;
  case PROP_POWERMGR_SERIAL_STOPBITS:
    comport_stopbits_ = *typeptr;
    break;
  case PROP_POWERMGR_SERIAL_READ_TIMEOUT:
    if (YSOS_ERROR_SUCCESS != SetReadTimeOut(*typeptr)) {
      return YSOS_ERROR_FAILED;
    }
    break;
  default:
    YSOS_LOG_INFO("PowerManagerDriver SetProperty Function Not Support This Method!");
    return YSOS_ERROR_NOT_SUPPORTED;
  }  ///< end switch

  return YSOS_ERROR_SUCCESS;
}

int PowerManagerDriver::Open(void *paramsptr /*= NULL*/) {
  if (NULL == paramsptr) {
    return Open(comport_port_,comport_baudrate_,(UINT8)comport_parity_,(UINT8)comport_bytesize_,(UINT8)comport_stopbits_);
  }
  //TODO: add for linux
#ifdef _WIN32
  int *portptr = reinterpret_cast<int*>(paramsptr);

  if (comport_pm_.IsOpen()) {
    Close(NULL);
    SLEEP(500);
  }
  if (comport_pm_.Open(*portptr)) {
    YSOS_LOG_DEBUG("PowerManagerDriver Open Success!");
    SetReadTimeOut(comport_read_timeout_);
    return YSOS_ERROR_SUCCESS;
  }

  data_ptr_ = GetDataInterfaceManager()->GetData(GLOBAL_DATA_KEY);
  if (!data_ptr_) {
    YSOS_LOG_DEBUG("get data_ptr_ failed");
    return YSOS_ERROR_FAILED;
  }
#else
  //TODO: need to add somethding action for linux
  YSOS_LOG_DEBUG("PowerManagerDriver Open Success!");
  return YSOS_ERROR_SUCCESS;
#endif

  YSOS_LOG_DEBUG("PowerManagerDriver Open Fail!");
  return YSOS_ERROR_FAILED;
}

int PowerManagerDriver::Open(UINT32 iport, UINT32 ibaudrate, UINT8 btparity , UINT8 btbytesize , UINT8 btstopbits) {
#ifdef _WIN32
  if (comport_pm_.IsOpen()) {
    Close(NULL);
    SLEEP(500);
  }
  if (comport_pm_.Open((DWORD)iport,(DWORD)ibaudrate,(BYTE)btparity,(BYTE)btbytesize,(BYTE)btstopbits)) {
    YSOS_LOG_DEBUG("PowerManagerDriver Open Success!");
    SetReadTimeOut(comport_read_timeout_);
    return YSOS_ERROR_SUCCESS;
  }
#else
  //TODO: need to add somethding action for linux
  YSOS_LOG_DEBUG("PowerManagerDriver Open Success!");
  return YSOS_ERROR_SUCCESS;
#endif
  YSOS_LOG_DEBUG("PowerManagerDriver Open Fail!");

  return YSOS_ERROR_FAILED;
}

void PowerManagerDriver::Close(void *paramsptr /*= NULL*/) {
#ifdef _WIN32
  SetReadTimeOut(0);
  comport_pm_.Close();
#else
  //TODO: need to add somethding action for linux
#endif
  YSOS_LOG_DEBUG("PowerManagerDriver Close!");

  return;
}

int PowerManagerDriver::Read(BufferInterfacePtr bufferptr, DriverContexPtr contextptr) {
  UINT8 *recvbufferptr = NULL ;
  UINT8 tmpbufferptr[40] ,tmpbufferptr1[40];
  UINT32 irecvbufferlength = 0;
  UINT32 new_len = 0;
  int ireadsize = 0;
  bool bendflag = false;
  boost::posix_time::ptime time_begin,time_now,time_end;
  bufferptr->GetBufferAndLength(&recvbufferptr, &irecvbufferlength);
  bufferptr->GetMaxLength(&irecvbufferlength);
  if (NULL == bufferptr) {
    YSOS_LOG_ERROR("PowerManagerDriver Read Function Parameter Error!");
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }
#ifdef _WIN32
  if (!comport_pm_.IsOpen()) {
    YSOS_LOG_ERROR("PowerManagerDriver Read Function Serial Not Open!");
    return YSOS_ERROR_FAILED;
  }
  if (tmpbufferptr == NULL || irecvbufferlength <= 0) {
    YSOS_LOG_ERROR("PowerManagerDriver Read Function Receive Buffer Error!");
    return YSOS_ERROR_FAILED;
  }
  time_begin = boost::posix_time::microsec_clock::universal_time();
  while (true) {
    //int iread = comport_pm_.Read(recvbufferptr+ireadsize, 1);
    int iread = comport_pm_.Read(tmpbufferptr+ ireadsize, 1);
    if (1 == iread) { ///< 读取成功
      //if (FRAME_TAIL_FLAG == recvbufferptr[ireadsize]) {
      if (FRAME_TAIL_FLAG == tmpbufferptr[ireadsize]) {
        time_end = boost::posix_time::microsec_clock::universal_time();
        bendflag = true;
      } else {
        bendflag = false;
      }
      if (++ireadsize == irecvbufferlength) {
        YSOS_LOG_ERROR("PowerManagerDriver Receive Buffer Not Enough!Buffer Size:"<<irecvbufferlength);
        break;
      }
    } else { ///< 读取失败
      time_now = boost::posix_time::microsec_clock::universal_time();
      if (bendflag) {     ///< 判断是否结束
        if ((int)(time_now-time_end).total_milliseconds() > RECEIVE_ONE_TIMEOUT) { ///< 接收到0x03后50ms没有接收到数据,则认为接收结束
          break;
        }
      }

      if (ireadsize == 0) {
        /// 空白内容超时时间
        if ((int)(time_now-time_begin).total_milliseconds() > RECEIVE_EMPTY_TIMEOUT) {
          break;
        }
        /// 否则等待一下
        SLEEP(RECEIVE_EMPTY_SLEEP);
      }
    }
  } ///< While结束
  if (ireadsize > 3) {
    JudgeData(tmpbufferptr, (UINT32)ireadsize, tmpbufferptr1, &new_len);
    memcpy_s(recvbufferptr, new_len, tmpbufferptr1, new_len);
  } else {
    memcpy_s(recvbufferptr,(UINT32) ireadsize, tmpbufferptr, ireadsize);
  }
  YSOS_LOG_DEBUG("PowerManagerDriver Read Function Receive Buffer Length:"<<new_len);
#ifdef _DEBUG
  if (new_len > 0) {
    std::string all_recv_command = bytestohexstring((char*)recvbufferptr, new_len);
    YSOS_LOG_DEBUG("PowerManagerDriver Receive Data :"<<all_recv_command.c_str());
  }
#endif

#else
  //TODO: need to add somethding action for linux
#endif
  bufferptr->SetLength(new_len);
  return new_len;
}

int PowerManagerDriver::Write(BufferInterfacePtr inputbuffer, BufferInterfacePtr outputbuffer, DriverContexPtr contextptr) {
  if (inputbuffer == NULL) {
    YSOS_LOG_ERROR("PowerManagerDriver Write Function Parameter Error!");
    return YSOS_ERROR_INVALID_ARGUMENTS;
  }
#ifdef _WIN32
  if (!comport_pm_.IsOpen()) {
    YSOS_LOG_ERROR("PowerManagerDriver Read Function Serial Not Open!");
    return YSOS_ERROR_FAILED;
  }

  UINT8 *sendbufferptr = NULL;
  UINT32 isendbufferlength = 0;
  inputbuffer->GetBufferAndLength(&sendbufferptr, &isendbufferlength);
  YSOS_LOG_DEBUG("PowerManagerDriver Send Data Length:"<<isendbufferlength);

#ifdef _DEBUG
  if (isendbufferlength > 0) {
    std::string all_send_command = bytestohexstring((char*)sendbufferptr, isendbufferlength);
    YSOS_LOG_DEBUG("PowerManagerDriver Send Data :"<<all_send_command.c_str());
  }
#endif

  int iret = comport_pm_.Write(sendbufferptr, isendbufferlength);
  if (iret != 0) {
    YSOS_LOG_ERROR("PowerManagerDriver Write Function Send Data Fail! " << iret);
    return YSOS_ERROR_FAILED;
  }

#else
  //TODO: need to add somethding action for linux
#endif
  return YSOS_ERROR_SUCCESS;
}


void PowerManagerDriver::JudgeData(UINT8 *data, UINT32 len, UINT8 *new_data, UINT32 *new_len) {
#ifdef _WIN32
  char sz_temp[40];
  int i_len = 2;
  sz_temp[0] = data[0];
  sz_temp[1] = data[1];
  if (data == NULL || new_data == NULL) {
    YSOS_LOG_ERROR("data is null");
    return ;
  }
  for (UINT32 i = 2; i < len - 1; ++i) {
    if (data[i] == 0x1b && data[i+1] == 0x00) {
      sz_temp[i_len] = 0x1b;
      i = i+1;
    }
    if (data[i] == 0x1b && data[i+1] == 0xe7) {
      sz_temp[i_len] = 0x02;
      i = i+1;

    }
    if (data[i] == 0x1b && data[i+1] == 0xe8) {
      sz_temp[i_len] = 0x03;
      i = i+1;
    } else {
      sz_temp[i_len] = data[i];
    }
    ++i_len;
  }
  sz_temp[i_len] = 0x03;
  ++i_len;
  *new_len = i_len;
  memcpy_s(new_data, i_len, sz_temp, i_len);
#else
  //TODO: need to add somethding action for linux
#endif
}

void PowerManagerDriver::ReBootDown() {
	YSOS_LOG_DEBUG("reboot down begin");
#ifdef _WIN32  
	comport_pm_.Lock();
	comport_pm_.ClearError();
	comport_pm_.Purge();
	comport_pm_.Input().Clear();
	comport_pm_.Output().Clear();
	char send_data[] = {0x02, 0x01, 0x06, 0x41, 0x03};
	int i_ret = comport_pm_.Write(send_data, 6);
	if (i_ret != 0) {
		YSOS_LOG_ERROR("PowerManagerDriver Write ReBootDown Send Data Fail! " << i_ret);
		data_ptr_->SetData("chassis_connect", "1");
	}
	data_ptr_->SetData("chassis_connect", "0");
#else
  //TODO: need to add somethding action for linux
  data_ptr_->SetData("chassis_connect", "0");
#endif
}

int PowerManagerDriver::Ioctl(int ictrlid, BufferInterfacePtr inputbufferptr, BufferInterfacePtr outputbufferptr) {
  YSOS_LOG_ERROR("PowerManagerDriver::Ioctl " << ictrlid);
  int nretresult = YSOS_ERROR_SUCCESS;
#ifdef _WIN32
  try {
    if (!comport_pm_.IsOpen()) {
      YSOS_LOG_ERROR("PowerManagerDriver Ioctl Function Serial Not Open!");
      return YSOS_ERROR_FAILED;
    }

    if (ictrlid == PROP_POWERMGR_REBOOT_DOWN) {
      ReBootDown();
      return YSOS_ERROR_SUCCESS;
    }

    comport_pm_.Lock();
    comport_pm_.ClearError();

    if ((ictrlid&0x01) == 0x01) {
      comport_pm_.Purge();
      comport_pm_.Input().Clear();
      comport_pm_.Output().Clear();

      nretresult = Write(inputbufferptr);
    }
    if ((ictrlid&0x02) == 0x02) {
      if (Read(outputbufferptr) > 0) {
        nretresult = YSOS_ERROR_SUCCESS;
      }
    }

    comport_pm_.Unlock();
    return nretresult;

  } catch (...) {
    YSOS_LOG_ERROR("PowerManagerDriver Ioctl Function Exception!");
  }
#else
  //TODO: need to add somethding action for linux
  if (ictrlid == PROP_POWERMGR_REBOOT_DOWN) {
      //ReBootDown();
      return YSOS_ERROR_SUCCESS;
  }
  return YSOS_ERROR_SUCCESS;
#endif
  return YSOS_ERROR_FAILED;
}

int PowerManagerDriver::SetReadTimeOut(UINT32 itimeout) {
#ifdef _WIN32
  if (!comport_pm_.IsOpen()) {
    YSOS_LOG_ERROR("PowerManagerDriver SetReadTimeOut Function Serial Not Open!");
    return YSOS_ERROR_FAILED;
  }
  LPCOMMTIMEOUTS commtimeoutptr = comport_pm_.GetTimeouts(NULL);
  if (NULL!= commtimeoutptr) {
    YSOS_LOG_DEBUG("PowerManagerDriver GetTimeouts Success!Now Timeout:"<<commtimeoutptr->ReadTotalTimeoutConstant);
    commtimeoutptr->ReadTotalTimeoutConstant = comport_read_timeout_;
    if (comport_pm_.SetTimeouts(commtimeoutptr)) {
      YSOS_LOG_DEBUG("PowerManagerDriver SetReadTimeOut Success!");
      return YSOS_ERROR_SUCCESS;
    }
  }
#else
  //TODO: need to add somethding action for linux
  return YSOS_ERROR_SUCCESS;
#endif
  return YSOS_ERROR_FAILED;
}

int PowerManagerDriver::GetReadTimeOut(UINT32 *itimeout_ptr) {
#ifdef _WIN32
  if (!comport_pm_.IsOpen()) {
    YSOS_LOG_ERROR("PowerManagerDriver SetReadTimeOut Function Serial Not Open!");
    return YSOS_ERROR_FAILED;
  }

  LPCOMMTIMEOUTS commtimeoutptr = comport_pm_.GetTimeouts(NULL);
  if (NULL!= commtimeoutptr) {
    comport_read_timeout_ = commtimeoutptr->ReadTotalTimeoutConstant;
    if (NULL!=itimeout_ptr) {
      *itimeout_ptr = comport_read_timeout_;
    }
    return YSOS_ERROR_SUCCESS;
  }
#else
  //TODO: need to add somethding action for linux
  return YSOS_ERROR_SUCCESS;
#endif
  YSOS_LOG_ERROR("PowerManagerDriver SetReadTimeOut Function Fail!");
  return YSOS_ERROR_FAILED;
}

}
