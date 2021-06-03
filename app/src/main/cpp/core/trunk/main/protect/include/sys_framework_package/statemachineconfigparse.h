/**
 *@file StateMachineConfigParse.h
 *@brief Definition of StateMachineConfigParse
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-21 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef SFP_STAT_MACHINE_CONFIG_PARSE_H_  // NOLINT
#define SFP_STAT_MACHINE_CONFIG_PARSE_H_  // NOLINT

/// 3rdparty headers //  NOLINT
//#include <tinyxml2/tinyxml2.h>
#include <tinyxml2/tinyxml2.h>
/// Private Ysos Headers
#include "basetransitioninfoimpl.h"
#include "../../../public/include/os_hal_package/lock.h"
#include "../../../protect/include/sys_framework_package/basecallbackimpl.h"

namespace ysos {

class StateMachineConfigParse;
typedef boost::shared_ptr<StateMachineConfigParse>  StateMachineConfigParsePtr;
/**
 *@brief  解析状态机Xml // NOLINT
*/
class StateMachineConfigParse: public BaseInterfaceImpl {
  DISALLOW_COPY_AND_ASSIGN(StateMachineConfigParse);
  DECLARE_PROTECT_CONSTRUCTOR(StateMachineConfigParse);
 public:
  ~StateMachineConfigParse();

  /**
  *@brief  解析transition_xml //  NOLINT
  *@param transition_xml 待解析的消息  //  NOLINT
  *@param transition_list 解析的结果
  *@return 成功返回YSOS_ERROR_SUCCESS，否则失败L  //  NOLINT
  */
  int ParseTransitionXml(const std::string &transition_xml, TransitionList &transition_list);
  /**
   *@brief  解析element //  NOLINT
   *@param element 待解析的消息  //  NOLINT
   *@param transition_list 解析的结果
   *@return 成功返回transition指针，否则失败返回NULL  //  NOLINT
   */
  TransitionPtr ParseTransitionXml(tinyxml2::XMLElement *element);
  /**
   *@brief  将service_event_name|callback_name解析出来，分别存放在service_event_list 和 service_event_map中 //  NOLINT
   *@param service_event_list 待解析的消息列表，解析完后，更新为service或event name  //  NOLINT
   *@param service_event_map 解析的结果：service_event_name  ---> callback_ptr   //  NOLINT
   *@return 成功返回YSOS_ERROR_SUCCESS，否则失败L  //  NOLINT
   */
  int ParseServiceEventList(StringList &service_event_list, ServiceMap &service_event_map);
  /**
   *@brief  将service_event_name|callback_name解析出来，分别存放在service_event 和 callback_name中 //  NOLINT
   *@param condition service_event_name|callback_name格式的字符串  //  NOLINT
   *@param cmd_or_event 存放service_event   //  NOLINT
   *@param callback_name 存放callback_name   //  NOLINT
   *@return 成功返回YSOS_ERROR_SUCCESS，否则失败L  //  NOLINT
   */
  int GetCmdAndCallback(const std::string &condition, std::string &service_event, std::string &callback_name);
  /**
   *@brief  解析transition_info //  NOLINT
   *@param element 待解析的消息的消息结点  //  NOLINT
   *@return 成功返回TransitionInfoInterfacePtr，否则失败  //  NOLINT
   */
  TransitionInfoInterfacePtr ParseTransitionInfo(tinyxml2::XMLElement *element);
  /**
     *@brief  解析mechanis_xml中的State //  NOLINT
     *@param element 待解析的消息结点  //  NOLINT
     *@param state_list 解析的结果
     *@return 无  //  NOLINT
     */
  void ParseMechanismState(const tinyxml2::XMLElement *element, Mechanism::StateList &state_list);
  /**
   *@brief  解析mechanis_xml中的State //  NOLINT
   *@param element 待解析的消息结点  //  NOLINT
   *@param state_list 解析的结果
   *@return 无  //  NOLINT
   */
  void ParseMechanismEvent(const tinyxml2::XMLElement *element, Mechanism::TStringList &event_list);
  /**
  *@brief  解析mechanis_xml中的State //  NOLINT
  *@param element 待解析的命令结点  //  NOLINT
  *@param state_list 解析的结果
  *@return 无  //  NOLINT
  */
  void ParseMechanismCmd(const tinyxml2::XMLElement *element, Mechanism::TStringList &cmd_list);
  /**
  *@brief  解析mechanis_xml中的Event //  NOLINT
  *@param element 待解析的消息结点  //  NOLINT
  *@param node_name 待解析的消息结点名称  //  NOLINT
  *@param string_list 解析的结果
  *@return 无  //  NOLINT
  */
  void ParseMechanismByNodeName(const tinyxml2::XMLElement *element, std::string &node_name, Mechanism::TStringList &string_list);
  /**
  *@brief  解析IoInfo //  NOLINT
  *@param  element  待解析的IoInfo  //  NOLINT
  *@return 成功返回IoInfoPtr，失败返回NULL  //  NOLINT
  */
  boost::shared_ptr<IoInfo>  ParseIoInfoPtr(tinyxml2::XMLElement *element);
  /**
     *@brief  解析mechanis_xml中的Transition //  NOLINT
     *@param element 待解析的消息结点  //  NOLINT
     *@param transition_list 解析的结果
     *@return 无  //  NOLINT
     */
  void ParseMechanismTransition(const tinyxml2::XMLElement *element, TransitionList &transition_list, const std::string &conf_dir);
  /**
   *@brief  处理transition的Event和Command命令 //  NOLINT
   *@param element 待解析的element  //  NOLINT
   *@param cmd_event_list 保存解析的结果  //  NOLINT
   *@param dest_name 待解析的目标结点名  //  NOLINT
   *@return 无  //  NOLINT
   */
  void ParseTransitionEventOrCommand(tinyxml2::XMLElement *element, std::list<std::string> &cmd_event_list, const std::string &dest_name);
  /**
   *@brief  根据service list获得serviceinfo list //  NOLINT
   *@param service_list 服务名链表  //  NOLINT
   *@param service_info_list 服务结构链表  //  NOLINT
   *@return 无  //  NOLINT
   */
  void GetServiceInfoFromServiceName(StringList &service_list, ServiceList &service_info_list);

  static const StateMachineConfigParsePtr Instance(void);

 private:
  static StateMachineConfigParsePtr   s_utility_;
  static boost::shared_ptr<LightLock> s_utility_lock_;
};
#define GetStateMachineConfigParse  StateMachineConfigParse::Instance
}
#endif
