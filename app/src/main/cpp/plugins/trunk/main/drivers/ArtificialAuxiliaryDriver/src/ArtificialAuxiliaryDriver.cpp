/*@version 1.0
  *@author l.j..
  *@date Created on: 2016-10-20 13:48:23
  *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
  * 
*/

/// self Headers //  NOLINT
#include "../include/ArtificialAuxiliaryDriver.h"

/// stl Headers
#include <stdlib.h>

/// boost headers
#include <boost/thread/thread_guard.hpp>
#include <boost/chrono.hpp>

/// ysos headers
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"

/// private headers
#ifdef _WIN32
#pragma comment(lib,"ws2_32.lib")
#else
  unsigned long GetTickCount()
  {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
  }
#endif

namespace ysos {
DECLARE_PLUGIN_REGISTER_INTERFACE(ArtificialAuxiliaryDriver, DriverInterface);  ///<  插件的入口，一定要加上 //  NOLINT
ArtificialAuxiliaryDriver::ArtificialAuxiliaryDriver(const std::string &strClassName) : BaseInterfaceImpl(strClassName) {
  artificialauxiliary_open_ = false;
  logger_ = GetUtility()->GetLogger("ysos.ArtificialAuxiliary");
}

ArtificialAuxiliaryDriver::~ArtificialAuxiliaryDriver(void) {
}

int ArtificialAuxiliaryDriver::SetProperty(int type_id, void *type) {
  YSOS_LOG_DEBUG("driver SetProperty execute");
  int n_return = YSOS_ERROR_SUCCESS;

  YSOS_LOG_DEBUG("driver SetProperty done");
  return n_return;
}

int ArtificialAuxiliaryDriver::Read(BufferInterfacePtr pBuffer, DriverContexPtr context_ptr/*=NULL*/) {
  YSOS_LOG_DEBUG("driver Read execute");
  int n_return = YSOS_ERROR_SUCCESS;

  do {
    UINT8* buffer;
    UINT32 buffer_size;
    n_return = pBuffer->GetBufferAndLength(&buffer, &buffer_size);
    if (YSOS_ERROR_SUCCESS != n_return) {
      break;
    }
  } while (0);

  YSOS_LOG_DEBUG("driver Read done");
  return n_return;
}

int ArtificialAuxiliaryDriver::Write(BufferInterfacePtr input_buffer, BufferInterfacePtr output_buffer/*=NULL*/, DriverContexPtr context_ptr/*=NULL*/) {
  YSOS_LOG_DEBUG("driver Write execute");
  int n_return = YSOS_ERROR_SUCCESS;

  do {
    UINT8* buffer;
    UINT32 buffer_size;
    n_return = input_buffer->GetBufferAndLength(&buffer, &buffer_size);
    if (YSOS_ERROR_SUCCESS != n_return) {
      break;
    }
  } while (0);

  YSOS_LOG_DEBUG("driver Write done");
  return n_return;
}

int ArtificialAuxiliaryDriver::Ioctl(int iCtrlID, BufferInterfacePtr pInputBuffer, BufferInterfacePtr pOutputBuffer) {
  YSOS_LOG_DEBUG("driver Ioctl execute");
  int n_return = YSOS_ERROR_SUCCESS;

  do {
    switch (iCtrlID) {
    case CMD_ARTIFICIAL_AUXILIARY_SEND: {
      YSOS_LOG_DEBUG("command CMD_ARTIFICIAL_AUXILIARY_SEND");
      n_return = IoctlSend(pInputBuffer, pOutputBuffer);
      break;
    }
    case CMD_ARTIFICIAL_AUXILIARY_RECV: {
      YSOS_LOG_DEBUG("command CMD_ARTIFICIAL_AUXILIARY_RECV");
      if (!artificialauxiliary_open_) {
        YSOS_LOG_DEBUG("socket closed");
        n_return = YSOS_ERROR_FAILED;
        break;
      }
      n_return = IoctlRecv(pInputBuffer, pOutputBuffer);
      break;
    }
    default: {
      YSOS_LOG_DEBUG("invalid command");
      n_return = YSOS_ERROR_FAILED;
      break;
    }
    } //switch
  } while (0);

  YSOS_LOG_DEBUG("driver Ioctl done");
  return n_return;
}

unsigned int ArtificialAuxiliaryDriver::ThreadProcessConnect(){
  int time_begin = GetTickCount()-5000;
  int time_end = GetTickCount();
  int time_interval = time_end - time_begin;
  while(1){
	  boost::this_thread::interruption_point();

	time_end = GetTickCount();
	time_interval = time_end - time_begin;
    if (!artificialauxiliary_open_ && time_interval>=5000) {
		YSOS_LOG_DEBUG("driver do Connectting");

	  int ret = Connect();
      if (YSOS_ERROR_SUCCESS == ret) {
        artificialauxiliary_open_ = true;
		YSOS_LOG_INFO("driver Connect Success");
      }else{
		YSOS_LOG_ERROR("driver Connect failed");
	  }

	  time_begin=time_end;
	}
	//Sleep(1000);
	boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
  }

  YSOS_LOG_INFO("driver thread connectting exit");

  return 0;
}

int ArtificialAuxiliaryDriver::Open(void *pParams) {
  YSOS_LOG_DEBUG("driver Open execute");
  int n_return = YSOS_ERROR_SUCCESS;

  do {
    if (artificialauxiliary_open_) {
      YSOS_LOG_DEBUG("ArtificialAuxiliary open already");
      break;
    }
	else
	{
		if (!thread_ptr_){
		  thread_ptr_.reset(new boost::thread(boost::bind(&ArtificialAuxiliaryDriver::ThreadProcessConnect, this)));
		}
	}
  } while (0);

  YSOS_LOG_DEBUG("driver Open done");
  return YSOS_ERROR_SUCCESS;
}

void ArtificialAuxiliaryDriver::Close(void *pParams /* = nullptr */) {
  YSOS_LOG_DEBUG("driver Close execute");

  if(thread_ptr_) {
    thread_ptr_->interrupt();
    if(thread_ptr_->joinable()) {
	  thread_ptr_->join();
	}
  }

  DisConnect();
  artificialauxiliary_open_ = false;
  YSOS_LOG_DEBUG("driver Close done");
}

int ArtificialAuxiliaryDriver::Connect() {
  YSOS_LOG_DEBUG("driver Connect execute");
  int n_return = YSOS_ERROR_FAILED;

  do {
#ifdef _WIN32    
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;
    wVersionRequested = MAKEWORD(2, 2);
    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
      break;
    }
    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
      WSACleanup();
      break;
    }
    sockclient_ = socket(AF_INET,SOCK_STREAM,0);

    SOCKADDR_IN addrSrv;
    addrSrv.sin_addr.S_un.S_addr=inet_addr("127.0.0.1");
    addrSrv.sin_family=AF_INET;
    addrSrv.sin_port=htons(12016);
    err = connect(sockclient_,(SOCKADDR*)&addrSrv,sizeof(SOCKADDR));
    if (err != 0) {
      WSACleanup();
      break;
    }
    ///< 注意要先connect再设置成非阻塞，否则connect会返回-1，虽然那样连接也是成功的
    u_long mode = 1;
    err = ioctlsocket(sockclient_,FIONBIO,&mode);
    if (err != 0) {
      WSACleanup();
      break;
    }
#else
    if( (sockclient_ = socket(AF_INET, SOCK_STREAM, 0)) < 0){  
      printf("create socket error: %s(errno: %d)\n", strerror(errno),errno);  
      break;  
    }  
    
    struct sockaddr_in addrSrv;  
    memset(&addrSrv, 0, sizeof(addrSrv));
    addrSrv.sin_family = AF_INET;  
    addrSrv.sin_port = htons(12016);  
    if( inet_pton(AF_INET, "127.0.0.1", &addrSrv.sin_addr) <= 0){  
      printf("inet_pton error for %s\n", "127.0.0.1\n");  
      break;
    }
  
    if( connect(sockclient_, (struct sockaddr*)&addrSrv, sizeof(addrSrv)) < 0){  
      printf("connect error: %s(errno: %d)\n",strerror(errno),errno);  
      break;  
    }
#endif

    n_return = YSOS_ERROR_SUCCESS;
  } while (0);

  YSOS_LOG_DEBUG("driver Connect done");
  return n_return;
}

void ArtificialAuxiliaryDriver::DisConnect() {
  YSOS_LOG_DEBUG("driver DisConnect execute");
#ifdef _WIN32
  closesocket(sockclient_);
  WSACleanup();
#else
  close(sockclient_);
#endif
  YSOS_LOG_DEBUG("driver DisConnect done");
}

int ArtificialAuxiliaryDriver::IoctlSend(BufferInterfacePtr pInputBuffer, BufferInterfacePtr pOutputBuffer) {
  YSOS_LOG_DEBUG("driver IoctlSend execute");
  int n_return = YSOS_ERROR_SUCCESS;
  int n_result = 0;

  if (!artificialauxiliary_open_) {
    YSOS_LOG_ERROR("has not connect to server");
    return YSOS_ERROR_FAILED;
  }

  do {
    if (pInputBuffer == NULL) {
      YSOS_LOG_DEBUG("driver pInputBuffer is null");
      n_return = YSOS_ERROR_FAILED;
      break;
    }

    UINT8* buffer;
    UINT32 buffer_size;
    n_return = pInputBuffer->GetBufferAndLength(&buffer, &buffer_size);
    if (YSOS_ERROR_SUCCESS != n_return) {
      break;
    }

    std::string send_str;
    send_str = (char *)buffer;

    char send_data_length[9];
    UINT64 length = strlen(send_str.c_str()) + 1;
#ifdef _WIN32
    sprintf_s(send_data_length, "%08ld", length);
#else
    sprintf(send_data_length, "%08ld", length);
#endif
    std::string send_real_str = send_data_length;
    send_real_str += send_str;

    YSOS_LOG_DEBUG("the driver send string is : " << send_real_str);
    n_result = send(sockclient_, send_real_str.c_str(), strlen(send_real_str.c_str())+1,0);
#ifdef _DEBUGVIEW
    ::OutputDebugString((std::string("ysos.ArtificialAuxiliary send string is:") + send_real_str + (0 >= n_result ? "  send failed." : "  send succ")).c_str());
#endif
    if (0 >= n_result) {
      YSOS_LOG_ERROR("send message failed: " << n_result);
      //if (WSAENOTCONN == n_result || WSAESHUTDOWN == n_result || WSAECONNABORTED == n_result || WSAECONNRESET == n_result || WSAENETRESET == n_result) {
        YSOS_LOG_ERROR("remote side is disconnect");
        DisConnect();
        artificialauxiliary_open_ = false;
      //}
      n_return = YSOS_ERROR_FAILED;
      break;
    }
  } while (0);

  YSOS_LOG_DEBUG("driver IoctlSend done");
  return n_return;
}

int ArtificialAuxiliaryDriver::IoctlRecv(BufferInterfacePtr pInputBuffer, BufferInterfacePtr pOutputBuffer) {
  YSOS_LOG_DEBUG("driver IoctlRecv execute");
  int n_return = YSOS_ERROR_SUCCESS;
  int n_result = 0;

  if (!artificialauxiliary_open_) {
    YSOS_LOG_ERROR("has not connect to server");
    return YSOS_ERROR_FAILED;
  }

  do {
    if (pOutputBuffer == NULL) {
      YSOS_LOG_DEBUG("driver pOutputBuffer is null");
      n_return = YSOS_ERROR_FAILED;
      break;
    }

    UINT8* buffer;
    UINT32 buffer_size;
    n_return = pOutputBuffer->GetBufferAndLength(&buffer, &buffer_size);
    if (YSOS_ERROR_SUCCESS != n_return) {
      YSOS_LOG_DEBUG("driver GetBufferAndLength fail");
      break;
    }
    memset(buffer, 0, buffer_size);

    char recv_head[9] = {0};
    char recv_buffer[10240] = {0};
    std::string str_temp;
    n_result = recv(sockclient_, (char *)recv_head, 8,0);
    YSOS_LOG_DEBUG("driver recv bytes:" << n_result << "; recv_head = " << recv_head);

    if (0 >= n_result) {
      n_return = YSOS_ERROR_FAILED;
      YSOS_LOG_ERROR("Receive message failed: " << n_result);
      break;
    } else if (8 > n_result) {
      str_temp += recv_head;
      int head_len = 8 - n_result;
      while (head_len > 0) {
        n_result = recv(sockclient_, (char *)recv_head, head_len,0);
        str_temp += recv_head;
        head_len -= n_result;
      }
    } else if (8 == n_result) {
      str_temp = recv_head;
    }

    int len = atoi(str_temp.c_str());
    YSOS_LOG_DEBUG("driver recv len: " << len);
    if (len > buffer_size) {
      DisConnect();
      artificialauxiliary_open_ = false;
      n_return = YSOS_ERROR_FAILED;
      YSOS_LOG_DEBUG("buffer size not big enough");
      break;
    }
    str_temp.clear();
    while (len > 0) {
      n_result = recv(sockclient_, (char *)recv_buffer, len, 0);
      str_temp += recv_buffer;
      len -= n_result;
    }

    YSOS_LOG_DEBUG("the driver receive string is:" << str_temp);
#ifdef _DEBUGVIEW
    ::OutputDebugString((std::string("ysos.ArtificialAuxiliary recv string is:") + str_temp ).c_str());
#endif
    memcpy(buffer, str_temp.c_str(), str_temp.length());
  } while (0);

  YSOS_LOG_DEBUG("driver IoctlRecv done");
  return n_return;
}

}