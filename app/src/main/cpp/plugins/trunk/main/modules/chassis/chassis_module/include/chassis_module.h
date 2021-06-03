/**
*@file gaussian_chassis_module.h
*@brief Definition of gaussian chassis module
*@version 0.9.0.0
*@author Lu Min, Wang Xiaogui
*@date Created on: 2016-12-06 17:56:00
*@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
* 
*/

#ifndef YSOS_PLUGIN_MODULE_CHASSIS_H_
#define YSOS_PLUGIN_MODULE_CHASSIS_H_

/// Private Headers  //NOLINT
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basemoduleimpl.h"          //NOLINT
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basemodulecallbackimpl.h"  //NOLINT
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basethreadmoduleimpl.h"    //NOLINT
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basedriverimpl.h"                 //NOLINT
#include "../../../../../../core/trunk/main/public/include/os_hal_package/driverinterface.h"
#include "../../../../../../core/trunk/main/protect/include/os_hal_package/datainterfacemanager.h"

/// Boost Headers
#include <boost/shared_ptr.hpp>                 //NOLINT
#include <boost/shared_array.hpp>               //NOLINT
#include <boost/typeof/typeof.hpp>              //NOLINT
#include <boost/thread.hpp>                     //NOLINT

namespace ysos {

/**
*@brief  ChassisModule
*/
class YSOS_EXPORT ChassisModule : public BaseThreadModuleImpl {
  DECLARE_PROTECT_CONSTRUCTOR(ChassisModule);
  DISALLOW_COPY_AND_ASSIGN(ChassisModule);
  DECLARE_CREATEINSTANCE(ChassisModule);

private:
  /**
  *@brief 底盘类型
  */
  enum  ChassisType{
   CHASSIS_TYPE_GAUSSISS = 0x00,
   CHASSIS_TYPE_SILAN    = 0x01,
  };

   /**
  *@brief 底盘连接状态
  */
  enum  ChassisStatus{
   CHASSIS_STATUS_NONE = 0x00,
   CHASSIS_STATUS_DISCONNECT = 0x01,
   CHASSIS_STATUS_CONNECTED  = 0x02,
   CHASSIS_STATUS_EXCEPTION  = 0x03,
  };

  /**
  *@brief 底盘行走模式
  */
  enum  ChassisMoveMode{
   CHASSIS_MODE_MOVE_STOP   = 0x00,
   CHASSIS_MODE_MOVE_STILL  = 0x01,
   CHASSIS_MODE_MOVE_POINT  = 0x02,
   CHASSIS_MODE_MOVE_CRUISE = 0x03,
   CHASSIS_MODE_MOVE_RANDOM = 0x04,
   CHASSIS_MODE_MOVE_GOHOME = 0x05,
   CHASSIS_MODE_MOVE_DIRECTION    = 0x06,
   CHASSIS_MODE_MOVE_CAMERA_POINT = 0x07,
   CHASSIS_MODE_MOVE_RELATIVELY   = 0x08,
   CHASSIS_MODE_MOVE_MAPINIT = 0x09,
  };

  /**
  *@brief 底盘位置坐标
  */
  struct ChassisLocation {
    float fposx;
    float fposy;
    float fposz;
    float fposangle;
  };

  /**
  *@brief 底盘位置坐标
  */
  struct ChassisRelative {
    float distance;
    float angle;
  };

 public:
  /**
  *@brief 该模块的能力参数
  */
  typedef struct {
    UINT32 dw_formats;  ///< supported chassis formats
    UINT16 w_channels;  ///< specifying whether the module supports mono (1) or stereo (2) input
    UINT16 w_reserved1; ///< 保留值
  } GaussianChassisCaps;

  /**
  *@brief 配置函数，可对该接口进行详细配置
  *@param ctrl_id[Input]： 详细配置ID号
  *@param param[Input]：详细配置的参数
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值
  */
  int Ioctl(INT32 ctrl_id, LPVOID param = NULL);

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
  *@param piType[Output] property value
  *@return success return YSOS_ERROR_SUCCESS,
  *        fail return one of (YSOS_ERROR_INVALID_ARGUMENTS)
  */
  virtual int SetProperty(int iTypeId, void *piType);

  /**
  *@brief 基本初始化
  *@param： 无
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值
  */
  int Initialize(void *param=NULL);

protected:
  /**
  *@brief 类似于模块的初始化功能，子类实现，只关注业务
  *@param open需要的参数
  *@return 成功返回0，否则失败
  */
  int RealOpen(LPVOID param = NULL);

  /**
  *@brief 运行，子类实现，只关注业务
  *@param ： 无
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值
  */
  int RealRun();

  /**
  *@brief 暂停，子类实现，只关注业务
  *@param ： 无
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值
  */
  int RealPause();

  /**
  *@brief 停止运行，关闭模块，子类实现，只关注业务
  *       只有当ref_cout为0时，才能正常关闭
  *@return 成功返回0，否则失败
  */
  int RealStop();

  /**
  *@brief 关闭，子类实现，只关注业务
  *@return 成功返回0，否则失败
  */
  int RealClose();

  /**
  *@brief  初始化ModuleDataInfo
  *@return 成功返回YSOS_ERROR_SUCCESS，否则失败
  */
  int InitalDataInfo();

  /**
  *@brief 设置音频采集的缓存
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值
  */
  int SetupChassisBuffer();

  /**
  *@brief  配置文件中设置的Key、Value对，会在Initialized函数中调用    //NOLINT
  *@param key 配置参数中的Key                                         //NOLINT
  *@param value 与Key对应的Value                                      //NOLINT
  *@return 成功返回YSOS_ERROR_SUCCESS，否则失败                       //NOLINT
  */
  virtual int Initialized(const std::string &key, const std::string &value);

private:
  /**
  *@brief 获取当前导航状态
  *@return： 返回导航状态
  */
  std::string Get_Current_Navigation_Status(const UINT8 *input_buffer_ptr);

  /**
  *@brief 获取移动目标点的Json数据
  *@return： 返回获取移动目标点的Json数据
  */
  std::string Make_MoveTo_Point(float fposx,float fposy,float fangle);

  /**
  *@brief 获取转动到目标位置的Json数据
  */
  std::string Make_Rotate_Position(float fangle);

  /**
  *@brief 获取移动命令的Json数据
  */
  std::string Make_Moveing_Cmd(float linearSpeed, float angularSpeed);

  /**
  *@brief 获取当前位置数据
  */
  bool Get_Current_Point_Pos(const UINT8 *input_buffer_ptr,float &fposx,float &fposy,float &fangle);

  /**
  *@brief 计算两个坐标间距离
  */
  float Calculate_Distance_To_ObjPoint(float fcurposx,float fcurposy,float fobjposx,float fobjposy);

  /**
  *@brief 计算当前位置到目标位置的距离
  */
  float Get_Distance_From(const std::string &aim);

  /**
  *@brief 获取底盘类型
  */
  int Get_Chassis_Type();

  /**
  *@brief 获取底盘状态
  */
  int Get_Chassis_Connect_Status();

   /**
  *@brief 设置底盘状态
  */
  int Set_Chassis_Connect_Status(int chassis_status);

  /**
  *@brief 构造连接底盘输入Json数据
  */
  std::string Make_Connect_Json_String();

  /**
  *@brief 构造底盘充电输入Json数据
  */
  std::string Make_Gohome_Json_String();

  /**
  *@brief 构造底盘充电输入Json数据
  */
  std::string Make_Greet_Json_String();

  /**
  *@brief 本地巡航下获取本次应转角度
  */
  int Get_Need_Rotate(int& direction, int& angle);

  /**
  *@brief 本地巡航参数更新
  */
  int Get_Move_Still(const std::string &strsource);

  /**
  *@brief 移动指令参数解析
  */
  int Get_Move_Direction(const std::string &strsource,float &value);

  /**
  *@brief 定点名称参数解析
  */
  bool Get_Move_Point(const std::string &strsource,struct ChassisLocation &point);

  /**
  *@brief 视频像素参数解析
  */
  bool Get_Move_Camera_Point(const std::string &strsource,struct ChassisLocation &point);

  /**
  *@brief 相对位置参数解析
  */
  bool Get_Move_Relative_Point(const std::string &strsource,struct ChassisLocation &point);

  /**
  *@brief 相对位置转目标点
  */
  bool Make_Point_From_Relative(ChassisRelative &relative,struct ChassisLocation &point);

private:
  /**
  *@brief 心跳线程
  *@return
  */
  static int Connect_Heart_Thread(LPVOID lpParam);

  /**
  *@brief 移动功能线程
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值
  */
  static int Move_Thread(LPVOID lpParam);

private:
  int Move_Still(const std::string &strsource);
  int Move_To_Point(const ChassisLocation& locaptr, bool persist = true);
  int Move_Cruise();
  int Go_Home();
  int Move_Direction(const std::string &strsource);
  int Stop_Move();
  int Start_Greet();
  int Stop_Greet();

  int Wait_For_Rotating();
  int Wait_For_Moving(const int maxPlanCnt, bool persist = true);

private:
  bool Set_Move_Command(const int movemode, const std::string& param);
  bool Get_Move_Command(int &movemode, std::string& param);
  bool Is_Move_Command_Updated();
  bool Is_Move_Mode(int move_mode);
  bool Set_Move_Status(const char* state);
  bool Get_Move_Status(BufferInterfacePtr &out_buffer_ptr);

private:
  DriverInterfacePtr    chassis_driver_ptr_;      ///< Module模块的Driver指针     //NOLINT
  CallbackInterfacePtr  module_callback_ptr_;     ///< Module模块的Callback指针   //NOLINT

  BufferInterfacePtr    move_input_buffer_ptr_;   ///< 分配的buffer,Move操作      //NOLINT
  BufferInterfacePtr    move_output_buffer_ptr_;  ///< 分配的buffer，Move操作     //NOLINT
  BufferInterfacePtr    heart_input_buffer_ptr_;  ///< 分配的buffer，心跳操作     //NOLINT
  BufferInterfacePtr    heart_output_buffer_ptr_; ///< 分配的buffer，心跳操作     //NOLINT

  int time_interval_;                             ///< 回调线程时间间隔           //NOLINT

  int chassis_type_;                              ///< 底盘类型,见ChassisType     //NOLINT
  std::string releate_driver_name_;               ///< 底盘Driver对象名称         //NOLINT
  std::string releate_callback_name_;             ///< 底盘Callback对象名称       //NOLINT

private:
  /// 连接模块
  std::string strconnect_addr_;                  ///< 连接ip地址                  //NOLINT
  std::string strmap_name_;                      ///< 连接地图名称                //NOLINT
  std::string strmap_position_;                  ///< 连接地图位置名称            //NOLINT
  std::string strmap_charging_point_;            ///< 连接地图充点电名称          //NOLINT
  std::string strmap_greet_area_;                ///< 连接地图主动迎宾名称        //NOLINT
  bool        binit_direct_;                     ///< 连接地图是否转圈            //NOLINT

  /// 心跳模块
  bool chassis_heart_enable_;                    ///< 是否使用心跳机制            //NOLINT
  int chassis_connect_status_;                   ///< 底盘状态，见ChassisStatus   //NOLINT
  boost::thread connect_heart_thread_;           ///< 心跳线程句柄                //NOLINT

  /// 移动模块
  boost::mutex move_mutex_;                              ///< 读写锁
  int move_mode_;                                        ///< 移动模式
  bool move_cmd_chg_;
  bool move_status_chg_;
  std::string move_cmd_param_;
  std::string move_status_;                              ///< 移动状态
  boost::thread move_operation_thread_;                  ///< 移动线程句柄

  /// 定点巡航模块
  unsigned int cruise_index_;                            ///< 定点巡航索引
  std::vector<struct ChassisLocation> vecpoints_cruise_; ///< 定点巡航点集合

  /// 移动定点
  std::map<std::string,struct ChassisLocation> move_points_;

  /// 本地巡航模块
  std::string still_rotate_name_;
  float still_rotate_x_;
  float still_rotate_y_;
  float still_rotate_angle_;
  float still_rotate_range_;
  int still_rotate_step_;
  int still_rotate_step_interval_;
  std::string still_rotate_mode_;

  float cmd_move_step_;
  float cmd_move_speed_;
  float cmd_rotate_step_;
  float cmd_rotate_speed_;

	DataInterfacePtr      data_ptr_;      //< 内存指针
};

} ///< namespace ysos

#endif  ///< YSOS_PLUGIN_MODULE_GAUSSIAN_CHASSIS_H_   //NOLINT
