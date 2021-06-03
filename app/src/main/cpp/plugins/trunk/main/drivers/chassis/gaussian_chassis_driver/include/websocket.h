/**
*@file websocket.h
*@brief Definition of gaussian chassis driver websocket
*@version 0.9.0.0
*@author Wang Xiaogui
*@date Created on: 2016-12-15 14:30:00
*@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
* 
*/

#ifndef YSOS_PLUGIN_WEBSOCKET_H_
#define YSOS_PLUGIN_WEBSOCKET_H_

/// OS Header
#include <map>

/// websocketpp Header
#include <websocketpp/config/asio_no_tls_client.hpp>  //NOLINT
#include <websocketpp/client.hpp>                     //NOLINT
#include <websocketpp/common/thread.hpp>              //NOLINT
#include <websocketpp/common/memory.hpp>              //NOLINT

/// Boost Header
#include <boost/thread/mutex.hpp>                     //NOLINT
#include <boost/thread/lock_guard.hpp>                //NOLINT
#include <boost/atomic/atomic.hpp>                    //NOLINT
#include <boost/date_time/posix_time/posix_time.hpp>  //NOLINT
#include <boost/atomic/detail/atomic_template.hpp>
#include <json/json.h>


#define TEST_PLAN_TIME 0

enum WEBSOCKET_ID{
  WEBSOCKET_DEVICE_ID     = 0x00,
  WEBSOCKET_NAVIGATION_ID = 0x01,
  WEBSOCKET_SYSHEALTH_ID  = 0x02,
  WEBSOCKET_GREET_ID      = 0x03,
};

typedef websocketpp::client<websocketpp::config::asio_client> client;

class connection_metadata {
public:
	connection_metadata(int id, websocketpp::connection_hdl hdl, std::string uri,void *param_ptr);
	~connection_metadata(void);

	void on_open(client *client_ptr, websocketpp::connection_hdl hdl);
	void on_fail(client *client_ptr, websocketpp::connection_hdl hdl);
	void on_close(client *client_ptr, websocketpp::connection_hdl hdl);
	void on_message_for_device(websocketpp::connection_hdl, client::message_ptr msg);
	void on_message_for_navigation(websocketpp::connection_hdl, client::message_ptr msg);
	void on_message_for_health(websocketpp::connection_hdl, client::message_ptr msg);
	void on_message_for_greet(websocketpp::connection_hdl, client::message_ptr msg);

	websocketpp::connection_hdl get_hdl() const;
	int get_id() const;
	std::string get_status() const;

  // navigation
  void set_move_latest_status(std::string move_status);
  std::string get_move_latest_status() ;
  void clear_latest_status();

  /// Device Status
  void set_battery_status(int battery_status);
  int  get_battery_status();

  void set_charge_status(bool charge_status);
  bool get_charge_status();

  void set_speed_status(float speed_status);
  float get_speed_status();

  void set_emergencystopstatus_status(bool emergencystopstatus);
  bool get_emergencystopstatus_status();

  /// Health Status
  void set_mcu_status(bool mcu_status);
  bool get_mcu_status();

  void set_laser_status(bool laser_status);
  bool get_laser_status();

  void set_router_status(bool router_status);
  bool get_router_status();

  /// greet
  void set_greet_status_code(int status_code);
  int  get_greet_status_code();

  /// greet
  void set_error_status_code(int status_code);
  int  get_error_status_code();

public:
  typedef websocketpp::lib::shared_ptr<connection_metadata> ptr;

private:
  int m_id;
  websocketpp::connection_hdl m_hdl;
  std::string m_status;
  std::string m_uri;
  std::string m_server;
  std::string m_error_reason;

  // navigation
  boost::mutex m_mutex;
  std::string m_move_latest_status;

  //Device Status
  boost::atomic<int>    m_battery_status;        ///< 电池状态
  boost::atomic<int>    m_charge_status;         ///< 是否在充电
  boost::atomic<float>  m_speed_status_;         ///< 速度状态
  boost::atomic<bool>   m_emergencystopstatus_;  ///< 急停状态

  //Health Status
  boost::atomic<bool> m_bconnect_mcu_;    ///< 微处理单元连接是否正常
  boost::atomic<bool> m_bconnect_laser_;  ///< 激光设备连接是否正常
  boost::atomic<bool> m_bconnect_router_; ///< 路由器连接是否正常

  // status code
  boost::atomic<int> m_greet_status_code_;  ///< greet status code
  boost::atomic<int> m_error_status_code_;  ///< error status code

#if TEST_PLAN_TIME
  boost::posix_time::ptime time_last_;
#endif
  // void *param_ptr_;
};



class websocket_endpoint {
public:
  websocket_endpoint(void);
  ~websocket_endpoint(void); 

  int  connect(int new_id, std::string const &uri,void *param_ptr);
  void close(int id, websocketpp::close::status::value code, std::string reason);
  void send(int id, std::string message);

  connection_metadata::ptr get_metadata(int id) const;

private:
  typedef std::map<int,connection_metadata::ptr> con_list;
  con_list m_connection_list;
  client m_endpoint;
  websocketpp::lib::shared_ptr<websocketpp::lib::thread> m_thread;
};

#endif ///< YSOS_PLUGIN_WEBSOCKET_H_
