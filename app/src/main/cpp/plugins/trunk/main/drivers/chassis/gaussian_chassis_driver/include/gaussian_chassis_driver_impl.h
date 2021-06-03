/**
*@file gaussian_chassis_driverinterface.h
*@brief Definition of gaussian chassis driver
*@version 0.9.0.0
*@author Lu Min, Wang Xiaogui
*@date Created on: 2016-12-01 12:33:00
*@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
* 
*/

#ifndef YSOS_PLUGIN_DRIVER_GAUSSIAN_CHASSIS_IMPL_H_
#define YSOS_PLUGIN_DRIVER_GAUSSIAN_CHASSIS_IMPL_H_

/// Ysos Headers
#include "../../../../../../core/trunk/main/public/include/sys_interface_package/common.h"			    //NOLINT
#include "../../../../../../core/trunk/main/public/include/sys_interface_package/bufferpool.h"		  //NOLINT
#include "../../../../../../core/trunk/main/public/include/core_help_package/baseinterfaceimpl.h"	//NOLINT
#include "../../../../../../core/trunk/main/public/include/os_hal_package/driverinterface.h"       //NOLINT
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basedriverimpl.h"       //NOLINT

/// Boost Headers
#include <boost/shared_ptr.hpp>                 //NOLINT
#include <boost/shared_array.hpp>               //NOLINT
#include <boost/property_tree/ptree.hpp>        //NOLINT
#include <boost/property_tree/json_parser.hpp>  //NOLINT
#include <boost/typeof/typeof.hpp>              //NOLINT
#include <boost/thread.hpp>                     //NOLINT

/// Web Socket Header
#include "../include/websocket.h"

namespace ysos {

/**
*@brief 驱动接口，用于管理底层驱动封装实例
*/
class YSOS_EXPORT GaussianChassisDriver :public BaseDriverImpl {
  DISALLOW_COPY_AND_ASSIGN(GaussianChassisDriver)
  DECLARE_PROTECT_CONSTRUCTOR(GaussianChassisDriver)
  DECLARE_CREATEINSTANCE(GaussianChassisDriver)

 public:
   virtual ~GaussianChassisDriver();

  /**
  *@brief 获取属性参数                                       // NOLINT
  *@param itypeid[Input] 属性ID                              // NOLINT
  *@param itype_ptr[Output]属性值                            // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int GetProperty(int itypeid, void *itype_ptr);

  /**
  *@brief 设置属性参数                                       // NOLINT
  *@param itypeid[Input] 属性ID                              // NOLINT
  *@param itype_ptr[Output]属性值                            // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int SetProperty(int itypeid, void *itype_ptr);

  /**
  *@brief 打开底层驱动，并传入配置参数                       // NOLINT
  *@param pParams[Input]： 驱动所需的打开参数                // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int Open(void *params_ptr = NULL);

  /**
  *@brief 打开底层驱动，并传入配置参数                       // NOLINT
  *@param ： 无                                              // NOLINT
  *@return： 无                                              // NOLINT
  */
  virtual void Close(void *params_ptr = NULL);

  /**
  *@brief 从底层驱动中读取数据                                // NOLINT
  *@param pBuffer[Output]： 读取的数据缓冲                    // NOLINT
  *@param context_ptr[In|Out]： Driver上下文                  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值   // NOLINT
  */
  virtual int Read(BufferInterfacePtr outbuffer_ptr, DriverContexPtr context_ptr = NULL);

  /**
  *@brief 写数据到底层驱动中                                  // NOLINT
  *@param input_buffer_ptr[Input]： 写入的数据缓冲            // NOLINT
  *@param output_buffer_ptr[Output]： 读取的数据缓冲          // NOLINT
  *@param context_ptr[In|Out]： Driver上下文                  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值   // NOLINT
  */
  virtual int Write(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr = NULL, DriverContexPtr context_ptr = NULL);

  /**
  *@brief 控制驱动状态/配置参数到驱动/读取状态等              // NOLINT
  *@param ictrl_id[Input]： ID                                // NOLINT
  *@param input_buffer_ptr[Input]： 写入的数据缓冲            // NOLINT
  *@param output_buffer_ptr[Output]： 读出的数据缓冲          // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值   // NOLINT
  */
  virtual int Ioctl(int ictrl_id, BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr);

private:
   int IoctlConnect(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr);
   int IoctlDisconnect(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr);
   int IoctlReconnect(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr);
   int IoctlRestart(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr);
   int IoctlGetDeviceInfo(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr);
   int IoctlSetDeviceInfo(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr);
   int IoctlEnableHeartBeat(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr);
   int IoctlDisableHeartBeat(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr);
   int IoctlMapInitialize(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr);
   int IoctlGetMapInfo(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr);
   int IoctlSetMapInfo(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr);
   int IoctlGetVirtualObstacle(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr);
   int IoctlSetVirtualObstacle(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr);
   int IoctlSetPredefinedPosition(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr);
   int IoctlGetPredefinedPosition(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr);

   int IoctlSetObstacleDetectionRange(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr);
   int IoctlGetObstacleDetectionRange(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr);

   int IoctlGetMoveSpeed(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr);
   int IoctlSetMoveSpeed(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr);
   int IoctlSendHeartBeat(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr);
   int IoctlGetDeviceStatus(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr);
   int IoctlGetHealthStatus(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr);
   int IoctlGetLocalizationStatus(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr);
   int IoctlGetNavigationStatus(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr);
   int IoctlGetSensorData(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr);
   int IoctlGetObstacleData(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr);
   int IoctlGoHome(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr);
   int IoctlStartMoveByDirection(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr);
   int IoctlStartMoveTo(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr);
   int IoctlStartMoveByPath(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr);

   /// 转到绝对角度
   int IoctlStartRotateTo(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr); 
   
   ///转动到一个相对角度
   int IoctlStartRotate(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr); 

  /// 暂停导航
  int IoctlPauseMove(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr);

  /// 恢复导航
  int IoctlResumeMove(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr);

  /// 停止导航 
  int IoctlStopMove(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr);

  /// 开始主动迎宾
  int IoctlStartGreet(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr);

  /// 暂停主动迎宾
  int IoctlPauseGreet(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr);

  /// 恢复主动迎宾
  int IoctlResumeGreet(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr);

  /// 停止主动迎宾
  int IoctlStopGreet(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr);

  /// 设置迎宾参数
  int IoctlSetGreetParam(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr);

  /// 开始 
  int IoctlStartRecordPath(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr);

  /// 停止
  int IoctlStopRecordPath(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr);


private:
  /// 检查初始化是否完成
  int IoctlInitFinish(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr);

  /// 获取版本信息
  int IoctlGetVersion(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr);

  /// 关机
  int IoctlPowerOff(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr);

  /// 获取转动状态
  int IoctlGetRotateStatus(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr);

  /// 获取激光传感器数据
  int IoctlGetLaserInfo(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr);

  /// 获取超声数据
  int IoctlGetUltrasonicInfo(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr);

  /// 获取防碰撞数据
  int IoctlGetProtectorInfo(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr);

  /// 开始充电
  int IoctlStartCharge(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr);

  /// 停止充电
  int IoctlStopCharge(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr);

  /// 获取地图图片,输出为png格式的图片流
  int IoctlGetMapPicture(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr);

  /// 删除地图
  int IoctlDelMap(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr);

  /// 修改地图名称
  int IoctlRenameMap(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr);

  /// 开始扫描地图
  int IoctlStartScanMap(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr);

  /// 取消扫描地图
  int IoctlCancelScanMap(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr);

  /// 停止扫描地图
  int IoctlStopScanMap(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr);

  /// 移动数据
  int IoctlMobileData(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr);

  /// 激光检查障碍物距离,获取最近距离
  int IoctlGetNearestLaserInfoInAngleRange(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr);

  /// get greet status code
  int IoctlGetGreetStatusCode(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr);

  /// check error status
  int IoctlCheckErrorStatus(BufferInterfacePtr input_buffer_ptr, BufferInterfacePtr output_buffer_ptr);

private:
  int GetChassisVersion();

  int GetDirectionPoint(const unsigned int idirection, float &fposx, float &fposy, float &fangle);

  int ClearLatestNavigationStatus();

private:
  /// 检查HTTP返回结果
  int Check_Http_Result(BufferInterfacePtr input_buffer_ptr,bool bhave_data);

private:
  int websocket_connect_device_status(UINT8 *server_addr_ptr,UINT32 iport,UINT8 *object_name_ptr);

  int websocket_close_device_status(void);

  int websocket_connect_navigation_status(UINT8 *server_addr_ptr,UINT32 iport,UINT8 *object_name_ptr);

  int websocket_close_navigation_status(void);

  int websocket_connect_health_status(UINT8 *server_addr_ptr,UINT32 iport,UINT8 *object_name_ptr);

  int websocket_close_health_status(void);

  int websocket_connect_greet_status(UINT8 *server_addr_ptr,UINT32 iport,UINT8 *object_name_ptr);

  int websocket_close_greet_status(void);

private:
  /**
  *@brief HTTP GET方法                                                 // NOLINT
  *@param server_addr_ptr[Input]： 地址                                // NOLINT
  *@param iport[Input]： 端口                                          // NOLINT
  *@param object_name_ptr[Input]： GET命令                             // NOLINT
  *@param output_data_ptr[Input]： 输出                                // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值            // NOLINT
  */
  int http_method_get(UINT8 *server_addr_ptr,UINT32 iport,UINT8 *object_name_ptr,BufferInterfacePtr output_data_ptr);

  /**
  *@brief HTTP POST方法                                                // NOLINT
  *@param server_addr_ptr[Input]： 地址                                // NOLINT
  *@param iport[Input]： 端口                                          // NOLINT
  *@param object_name_ptr[Input]： POST命令                            // NOLINT
  *@param input_data_ptr[Input]： POST数据                             // NOLINT
  *@param output_data_ptr[Output]： 输出                               // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值            // NOLINT
  */
  int http_method_post(UINT8 *server_addr_ptr,UINT32 iport,UINT8 *object_name_ptr,UINT8 *input_data_ptr,BufferInterfacePtr output_data_ptr);

  /**
  *@brief HTTP 接收数据回调方法                                        // NOLINT
  *@param ptr[Output]： 返回数据                                       // NOLINT
  *@param size[Output]： 输出数据块数                                  // NOLINT
  *@param nmemb[Output]： 输出数据长度                                 // NOLINT
  *@param response[Output]： 返回数据                                  // NOLINT
  *@return： 成功返回块大小，失败返回YSOS_ERROR_FAIL                   // NOLINT
  */
  static size_t http_receive_callback(void *ptr, size_t size, size_t nmemb, void *response);

private:
  /**
  *@brief Web Socket Connect方法                                       // NOLINT
  *@param itype_id[Input]： 见WEBSOCKET_ID                             // NOLINT
  *@param server_addr_ptr[Input]： 地址                                // NOLINT
  *@param iport[Input]： 端口                                          // NOLINT
  *@param object_name_ptr[Input]： GET命令                             // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值            // NOLINT
  */
  int websocket_connect_status(int itype_id,UINT8 *server_addr_ptr,UINT32 iport,UINT8 *object_name_ptr);

  /**
  *@brief Web Socket Close方法                                         // NOLINT
  *@param itype_id[Input]： 见WEBSOCKET_ID                             // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值            // NOLINT
  */
  int websocket_close_status(int itype_id);

private:
  /**
  *@brief definition of direction which may be command chassis to move to or turn to
  */
  enum ChassisMoveDirection {
    TDDirection_FORWARD   = 0x00,
    TDDirection_BACKWARD  = 0x01,
    TDDirection_TURNRIGHT = 0x02,
    TDDirection_TURNLEFT  = 0x03,
  };

  /**
  *@brief definition of type Sensor
  */
  enum ChassisSensor {
    TDSensor_Laser      = 0x00,
    TDSensor_Ultrasonic = 0x01,
    TDSensor_Protector  = 0x02,
  };

private:
  //boost::mutex mutex_lock_;         ///< 锁

  UINT32 server_port_;
  UINT32 http_time_out_;
  //boost::shared_array<char> server_addr_ptr_;
  //boost::shared_array<char> out_buffer_ptr_;
  websocket_endpoint socket_endpoint_;

  std::string version_sdp_;         ///< 底盘版本
  std::string version_sdk_;         ///< 高仙SDK版本
  std::string chassis_module_type_; ///< 高仙模块类型
  std::string chassis_productId_;   ///< 高仙产品ID

  std::string connect_string_;
  std::string connect_map_; 
  bool is_sdp_connected_;
  bool is_websocket_connect_device_;
  bool is_websocket_connect_navigation_;
  bool is_websocket_connect_health_;
  bool is_websocket_connect_greet_;

  bool    is_enable_heart_;
  bool    is_open_;
  UINT32  connect_sleep_time_;

  float speed_line_;
  float speed_angular_;
  float obstacle_detection_range_;
  float obstacle_detection_angle_;
};

typedef boost::shared_ptr<GaussianChassisDriver> GaussianChassisDriverImplPtr;

} ///< namespace ysos

#endif  ///< YSOS_PLUGIN_DRIVER_GAUSSIAN_CHASSIS_IMPL_H_  //NOLINT
