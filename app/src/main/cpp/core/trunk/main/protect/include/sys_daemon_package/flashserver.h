#ifndef FLASH_SERVER_H_
#define FLASH_SERVER_H_

#include <malloc.h>
#include <iostream>
#include <vector>
#include <boost/asio.hpp>
#include <boost/thread.hpp>


namespace ysos {

namespace sys_daemon_package {

typedef boost::shared_ptr<boost::asio::ip::tcp::socket> socket_ptr;

typedef void(*Communication_CallBack)(int iYSOSState);


class Server {
public:
  explicit Server(const char* ip_address);
  void Run();
  void Accept();
  void Close();

  int Register_CallBack_Function(void *pCallBackFun);

private:
  void AcceptHandler(const boost::system::error_code& ec, socket_ptr sock);
  void ReadHandler(const boost::system::error_code&ec, socket_ptr sock);

  static int Run_Thread(void* lpParam);

  boost::asio::io_service service_;
  boost::asio::ip::tcp::acceptor acceptor_;
  std::vector<char> buffer_;

  std::vector<void*> m_vectCallback_List; //
};

}  /// namespace sys_daemon_package

}  /// namespace ysos

#endif  // FLASH_SERVER_H_