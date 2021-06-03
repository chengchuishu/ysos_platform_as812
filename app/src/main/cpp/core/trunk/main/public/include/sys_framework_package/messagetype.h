/**
 *@file MessageType.h
 *@brief messagetype id map
 *@version 1.0
 *@author dhongqian
 *@date Created on: 2016-05-11 10:06:00
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef SFP_MESSAGE_TYPE_H_
#define SFP_MESSAGE_TYPE_H_

/// STL Headers
#include <map>
/// Boost Headers
#include <boost/shared_ptr.hpp>
/// Private Ysos Headers
#include "../../../public/include/sys_interface_package/common.h"
#include "../../../public/include/core_help_package/baseinterfaceimpl.h"
#include "../../../protect/include/core_help_package/propertytree.h"

namespace ysos {
class LightLock;
class MessageType;
class PropertyTree;
typedef boost::shared_ptr<MessageType> MessageTypePtr;

class YSOS_EXPORT MessageType: public BaseInterfaceImpl {
  DISALLOW_COPY_AND_ASSIGN(MessageType);
  MessageType(const std::string& message_type="MessageType");

 public:
  ~MessageType();

  /**
    *@brief  解析Xml文件 //  NOLINT
    *@param  file_name  待解析的Xml的文件名 //  NOLINT
    *@return 成功返回0，失败返回其余值  //  NOLINT
    */
  int ParseMessageType(const std::string &file_name);
  /**
    *@brief  通过Message的Name直接获取message id 值 //  NOLINT
    *@param  message_name  要获取事件ID的事件名称 //  NOLINT
    *@return 成功返回非0值，失败返回0  //  NOLINT
    */
  uint32_t  GetMessageTypeId(const std::string &message_name);
  /**
    *@brief  通过Message的ID直接获取message Name 值 //  NOLINT
    *@param  message_id  要获取事件名称的ID值 //  NOLINT
    *@return 成功返回事件名，失败返回空  //  NOLINT
    */
  std::string  GetMessageTypeName(const uint32_t &message_id);
  /**
    *@brief  通过Event的Name直接获取Service Name //  NOLINT
    *@param  event_name  要获取事件名称 //  NOLINT
    *@return 成功返回事件名，失败返回空  //  NOLINT
    */
  std::string  GetServiceByEvent(const std::string &event_name);
  /**
    *@brief  通过Service的Name直接获取Event Name //  NOLINT
    *@param  service_name  要获取Service名称 //  NOLINT
    *@return 成功返回服务名，失败返回空  //  NOLINT
    */
  std::string  GetEventByService(const std::string &service_name);

  static MessageTypePtr Instance();

protected:
  /**
    *@brief  更新Name和ID间的关系 //  NOLINT
    *@param  name  Event name信息 //  NOLINT
    *@param  id  Event id信息 //  NOLINT
    *@return 无  //  NOLINT
    */
  void UpdateNameIdMap(TreeNodeIterator name, TreeNodeIterator id);
  /**
    *@brief  更新Event和Service间的关系 //  NOLINT
    *@param  event  Event name信息 //  NOLINT
    *@param  service  service name信息 //  NOLINT
    *@return 无  //  NOLINT
    */
  void UpdateEventServiceMap(TreeNodeIterator event, TreeNodeIterator service);

 private:
  std::map<uint32_t, std::string>     id_name_map_;
  std::map<std::string, uint32_t>     name_id_map_;
  std::map<std::string, std::string>  event_service_map_;
  std::map<std::string, std::string>  service_event_map_;

  static MessageTypePtr message_type_ptr_;
  static boost::shared_ptr<LightLock> light_lock_ptr_;
};
}
#define  GetMessageType     ysos::MessageType::Instance
#endif
