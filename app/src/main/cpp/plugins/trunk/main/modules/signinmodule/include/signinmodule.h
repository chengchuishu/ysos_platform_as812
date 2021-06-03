/**
 *@file signinmodule.h
 *@brief sign in module interface
 *@version 1.0
 *@author l.j..
 *@date Created on: 2016-10-20 14:50:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

#ifndef SIGN_IN_MODULE_H_
#define SIGN_IN_MODULE_H_

/// Private Headers //  NOLINT
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basethreadmoduleimpl.h"
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/basemodulecallbackimpl.h"
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/manager_declare.h"
#include "../../../../../../core/trunk/main/protect/include/sys_framework_package/packageconfigimpl.h"
#include "../../../../../../core/trunk/main/protect/include/os_hal_package/datainterfacemanager.h"
#include "../../../../../../core/trunk/main/public/include/os_hal_package/driverinterface.h"
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"
#include "../../../public/include/httpclient/httpclient.h"

#include "../../../public/include/common/commonenumdefine.h"
#include "../../../public/include/common/commonstructdefine.h"

namespace ysos {

/**
 *@brief  签到模块逻辑控制层 //NOLINT
*/
class YSOS_EXPORT SignInModule : public BaseThreadModuleImpl {
  DECLARE_CREATEINSTANCE(SignInModule);
  DISALLOW_COPY_AND_ASSIGN(SignInModule);
  DECLARE_PROTECT_CONSTRUCTOR(SignInModule);

 public:
  virtual ~SignInModule(void);
  /**
  *@brief 配置函数，可对该接口进行详细配置  // NOLINT
  *@param ctrl_id[Input]： 详细配置ID号  // NOLINT
  *@param param[Input]：详细配置的参数  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  int Ioctl(INT32 control_id, LPVOID param = nullptr);

  /**
   *@brief 冲洗，快速将内部缓冲中剩余数据处理完  // NOLINT
   *@param ： 无  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Flush(LPVOID param = nullptr);

  /**
   *@brief 基本初始化  // NOLINT
   *@param param： 初始化的参数  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int Initialize(LPVOID param = nullptr);

  /**
   *@brief 清除资源,Initialize的逆操作  // NOLINT
   *@param param： 初始化的参数 // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int UnInitialize(LPVOID param = nullptr);
  
  virtual int GetProperty(int iTypeId, void *piType);

 protected:
  /**
  *@brief 打开并初始化Module计算单元  // NOLINT
  *@param param[Input]： 输入的配置参数  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int RealOpen(LPVOID param = nullptr);
  /**
   *@brief 运行  // NOLINT
   *@param ： 无  // NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
   */
  virtual int RealRun(void);
  /**
  *@brief 暂停  // NOLINT
  *@param ： 无  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int RealPause(void);
  /**
  *@brief 停止  // NOLINT
  *@param ： 无  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int RealStop(void);
  /**
  *@brief 关闭该Module  // NOLINT
  *@param ： 无  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值  // NOLINT
  */
  virtual int RealClose(void);
  /**
  *@brief  初始化ModuleDataInfo //  NOLINT
  *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
  */
  virtual int InitalDataInfo(void);

  /**
  *@brief  配置文件中设置的Key、Value对，会在Initialized函数中调用 //  NOLINT
  *@param key 配置参数中的Key  // NOLINT
  *@param value 与Key对应的Value  // NOLINT
  *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
  */
  virtual int Initialized(const std::string &key, const std::string &value);

 protected:
  DriverInterfacePtr    driver_prt_;    //< driver的指针 //  NOLINT

 private:
  // 尝试多次签到
  int SignIn();
  // 尝试一次签到
  int SignInOnce(const std::vector<std::string>& mac_addresses);
  // 解析签到结果
  bool ParseSigninResponse(const std::string& json_string);
  
  // 获取网络状态
  std::string GetNetworkStatus();

  std::string robot_url_;               //< 签到地址
  std::string finance_url_;             //< 金融交易签到地址
  std::string store_path_;              //< 签到文件路径
  std::string org_id_;                  //< 机构号
  std::string term_id_;                 //< 终端号
  std::string proxy_ip_;                //< 代理IP
  std::string proxy_port_;              //< 代理端口
  std::string proxy_user_;              //< 代理用户名
  std::string proxy_pwd_;               //< 代理密码
  HttpClientInterface *http_client_;    //< 通讯类指针
  DataInterfacePtr data_ptr_;           //< 内存指针

  // 签到尝试次数
  int sign_in_time_;
  // 签到事件数据
  std::string data_;
  // 是否签到成功
  bool is_sign_in_;

  /**
  *@brief  解析返回数据并写入文件 //  NOLINT
  *@param srcData 签到返回数据  // NOLINT
  *@param path 文件路径  // NOLINT
  *@return 成功返回YSOS_ERROR_SUCCESS，否则失败  //  NOLINT
  */
  int AlyAndWrite(std::string robot_data, std::string finance_data, std::string path);

};

}
#endif    ///SIGN_IN_MODULE_H_    //NOLINT