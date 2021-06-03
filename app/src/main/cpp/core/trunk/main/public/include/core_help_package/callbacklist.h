/**
 *@file CallbackList.h
 *@brief Definition of CallbackList
 *@version 0.1
 *@author XuLanyue
 *@date Created on: 2016-04-26 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef CHP_CALLBACK_LIST_H_          //NOLINT
#define CHP_CALLBACK_LIST_H_          //NOLINT

#include "utility.h"
/// STL headers //  NOLINT
#include <list>
/// Private Ysos Headers
#include "../../../public/include/sys_interface_package/common.h"
#include "../../../public/include/sys_interface_package/callbackqueue.h"
#include "../../../public/include/sys_interface_package/agentinterface.h"

namespace ysos {
/**
  *@brief  实现一个Observer列表模板类 //  NOLINT
  */
template<typename T>
class YSOS_EXPORT ObjectList {
  DISALLOW_COPY_AND_ASSIGN(ObjectList);
  //DECLARE_PROTECT_CONSTRUCTOR(XmlUtil);
public:
  ObjectList(){

  }

  virtual ~ObjectList(){
    Clear();
  }

  int AddObject(T object_ptr){
    if(object_list_.end() != std::find(object_list_.begin(), object_list_.end(), object_ptr)) {
      return YSOS_ERROR_HAS_EXISTED;
    }

    object_list_.push_back(object_ptr);

    return YSOS_ERROR_SUCCESS;
  }
  int RemoveObject(T object_ptr){
    std::list<CallbackInterfacePtr>::iterator it = std::find(object_list_.begin(), object_list_.end(), object_ptr);
    if(it != object_list_.end()) {
      object_list_.erase(it);
    }

    return YSOS_ERROR_SUCCESS;
  }
  void Clear(void) {
    object_list_.clear();
  }

  int Notify(BufferInterfacePtr input_buffer, BufferInterfacePtr output_buffer, void *context = NULL){
    typename std::list<T>::iterator it = object_list_.begin();  //need add 'typename' for linux
    for(; it !=object_list_.end(); ++it) {
      /*(*it)->Callback(input_buffer, output_buffer, context);*/
      int ret = RealNotify(*it, input_buffer, output_buffer, context);
      if(YSOS_ERROR_SUCCESS != ret) {
        YSOS_LOG_ERROR_DEFAULT("Notify failed: " << ret);
      }
    }

    return YSOS_ERROR_SUCCESS;
  }

  ///<  int OnDispatchMessage(const std::string &service_name, const std::string &event_key, const std::string &callback_name, const std::string &event_param) //  NOLINT
  int Notify(const std::string &service_name, const std::string &event_key, const std::string &callback_name, const std::string &event_param){
    typename std::list<T>::iterator it = object_list_.begin();  //need add 'typename' for linux
    for(; it !=object_list_.end(); ++it) {
      /*(*it)->Callback(input_buffer, output_buffer, context);*/
      int ret = RealNotify(*it, service_name, event_key, callback_name, event_param);
      if(YSOS_ERROR_SUCCESS != ret) {
        YSOS_LOG_ERROR_DEFAULT("Notify failed: " << ret);
      }
    }

    return YSOS_ERROR_SUCCESS;
  }

protected:
  virtual int RealNotify(T &element, BufferInterfacePtr input_buffer, BufferInterfacePtr output_buffer, void *context = NULL) {
    return YSOS_ERROR_SUCCESS;
  }

  virtual int RealNotify(T &element, const std::string &service_name, const std::string &event_key, const std::string &callback_name, const std::string &event_param) {
    return YSOS_ERROR_SUCCESS;
  }

protected:
  std::list<T>   object_list_;
};

class YSOS_EXPORT CallbackList: public ObjectList<CallbackInterfacePtr> {
public:
  CallbackList(){

  }

  ~CallbackList(){
  }

  virtual int RealNotify(CallbackInterfacePtr &element, BufferInterfacePtr input_buffer, BufferInterfacePtr output_buffer, void *context = NULL) {
    return element->Callback(input_buffer, output_buffer, context);
  }
};
typedef boost::shared_ptr<CallbackList>   CallbackListPtr;

//class YSOS_EXPORT AgentList: public ObjectList<AgentInterfacePtr> {
//public:
//  AgentList(){
//
//  }
//
//  ~AgentList(){
//  }
//
//  virtual int RealNotify(AgentInterfacePtr &element, const std::string &service_name, const std::string &event_key, const std::string &callback_name, const std::string &event_param) {
//    return element->OnDispatchMessage(service_name, event_key, callback_name, event_param);
//  }
//};
//typedef boost::shared_ptr<AgentList>   AgentListPtr;
}
#endif  // CHP_CALLBACK_LIST_H_       //NOLINT
