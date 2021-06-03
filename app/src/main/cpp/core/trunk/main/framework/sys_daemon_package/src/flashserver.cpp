#include "../../../protect/include/sys_daemon_package/flashserver.h"
#include "../../../protect/include/sys_daemon_package/externinterface.h"

using namespace boost::asio;
using namespace ysos::extern_interface;

namespace ysos {

namespace sys_daemon_package {

const int kPort = 843;
//const int kPort = 6002;
const char* kIpAddress = "127.0.0.1";
const char kCrossDomain[] = "<?xml version=\"1.0\"?>\
							<cross-domain-policy>\
							<allow-access-from domain=\"*\" to-ports=\"6002\"/>\
							</cross-domain-policy>";
const std::string kRequest = "<policy-file-request/>";

const std::string kYsosState = "finish";
const char kYsosStateResult[]= "ok";

typedef boost::shared_ptr<ip::tcp::socket> socket_ptr;
typedef boost::shared_ptr<io_service> service_ptr;


Server::Server(const char* ip_address): buffer_(512, 0),acceptor_(service_, ip::tcp::endpoint(ip::address::from_string(ip_address), kPort)) {
  m_vectCallback_List.clear();
  Accept();
}

void Server::Run() {
  boost::thread(boost::bind(&Server::Run_Thread, this));
  if(logger_.valid()) YSOS_LOG_DEBUG("Server::Run Start"); 
}

void Server::Accept() {
  socket_ptr sock(new ip::tcp::socket(service_));
  acceptor_.async_accept(*sock, boost::bind(&Server::AcceptHandler, this,boost::asio::placeholders::error, sock));
}

void Server::AcceptHandler(const boost::system::error_code& ec, socket_ptr sock) {
  if (ec) {
    if(logger_.valid()) YSOS_LOG_DEBUG("Server::ReadHandler Error:"<< ec); 
    return;
  }

  if(sock)sock->async_read_some(buffer(buffer_),boost::bind(&Server::ReadHandler, this,boost::asio::placeholders::error, sock));

  // Keep service running.
  Accept();
}

void Server::ReadHandler(const boost::system::error_code&ec, socket_ptr sock) {
  if (ec) {
	if(logger_.valid()) YSOS_LOG_DEBUG("Server::ReadHandler Error:"<< ec); 
    return;
  }
  std::size_t write_size = 0;
  if(logger_.valid()) YSOS_LOG_DEBUG("Server::ReadHandler Receive:"<< &buffer_[0]); 

  if (kRequest == &buffer_[0]){
    if(sock) {
	  std::size_t write_size = sock->write_some(buffer(kCrossDomain));
	  if(logger_.valid()) YSOS_LOG_DEBUG("Server::ReadHandler Send Flash OK!Send Result:"<<write_size);
	  boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
    } else {
      if(logger_.valid()) YSOS_LOG_DEBUG("Server::ReadHandler Socket Exception!");
	}
  } else if(kYsosState == &buffer_[0] ){
	  if(sock){
        write_size = sock->write_some(buffer(kYsosStateResult));
	    if(logger_.valid()) YSOS_LOG_DEBUG("Server::ReadHandler Send YSOS OK!Send Result:"<< write_size);
		boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
	  }else {
        if(logger_.valid()) YSOS_LOG_DEBUG("Server::ReadHandler Socket Exception!");
	  }
	  for (unsigned int i = 0; i < m_vectCallback_List.size(); i++) {
		  Communication_CallBack fCallbackFunc = reinterpret_cast<Communication_CallBack>(m_vectCallback_List[i]);
		  fCallbackFunc(1);
	  }
  }

  if(sock)sock->close();

  // Keep service running.
  Accept();
}

void Server::Close()
{
  service_.stop();
  if(logger_.valid()) YSOS_LOG_DEBUG("Server::Close"); 
}

int Server::Register_CallBack_Function( void *pCallBackFun )
{
  if (pCallBackFun != NULL)
  {
    m_vectCallback_List.push_back(pCallBackFun);
  }
  return 0;
}

int Server::Run_Thread( void *lpParam )
{
	Server *pServer = (Server*)lpParam;
	if(pServer != NULL )pServer->service_.run();
	return 0;
}


}  /// namespace sys_daemon_package

}  /// namespace ysos