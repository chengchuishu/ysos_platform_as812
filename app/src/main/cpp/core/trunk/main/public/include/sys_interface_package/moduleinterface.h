/**
 *@file moduleinterface.h
 *@brief Definition of message
 *@version 0.1
 *@author Steven.Shi
 *@date Created on: 2016-04-13 08:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef MODULE_LINK_INTERFACE_H//NOLINT
#define MODULE_LINK_INTERFACE_H//NOLINT

#include <set>
#include "../../../public/include/sys_interface_package/common.h"
#include "../../../public/include/sys_interface_package/baseinterface.h"
#include "../../../public/include/sys_interface_package/callbackqueue.h"
#include <boost/shared_ptr.hpp>

namespace ysos {
class BaseInterface;

  /**
   *@brief 动态控制接口，主要用于控制ModuleInterface，也可扩展使用
   */
class YSOS_EXPORT ControlInterface {
  IMPLEMENT_PUBLIC_EMPTY_DESTRUCTOR(ControlInterface)
 public:

  /**
   *@brief 配置函数，可对该接口进行详细配置//NOLINT
   *@param ctrl_id[Input]： 详细配置ID号//NOLINT
   *@param param[Input/Output]：详细配置的参数//NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值//NOLINT
   */
  virtual int Ioctl(INT32 ctrl_id, LPVOID param = NULL) = 0;

  /**
   *@brief 停止//NOLINT
   *@param ： 无//NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值//NOLINT
   */
  virtual int Stop(LPVOID param = NULL) = 0;

  /**
   *@brief 暂停//NOLINT
   *@param ： 无//NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值//NOLINT
   */
  virtual int Pause(LPVOID param = NULL) = 0;

  /**
   *@brief 运行//NOLINT
   *@param ： 无//NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值//NOLINT
   */
  virtual int Run(LPVOID param = NULL) = 0;

  /**
   *@brief 冲洗，快速将内部缓冲中剩余数据处理完//NOLINT
   *@param ： 无//NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值//NOLINT
   */
  virtual int Flush(LPVOID param = NULL) = 0;

  /**
   *@brief 获得被控制模块的状态//NOLINT
   *@param timeout[Input]： 超时值//NOLINT
   *@param state[Output]：状态//NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值//NOLINT
   */
  virtual int GetState(UINT32 timeout, INT32 *state, LPVOID param = NULL) = 0;
};

typedef boost::shared_ptr<ControlInterface> ControlInterfacePtr;

/// 表示Module自身
#define MODULE_SELF_OWNER_ID  0
//enum ModuleStat {
//  CLOSE = 0,
//  STOP,
//  PAUSE,
//  RUN,
//  ModuleStat_End
//};

  /**
   *@brief Module接口,系统内部的基本计算单元
     能力模块封装接口,所有内部的实际处理逻辑都由该接口封装,
     真正的处理逻辑将由Callback_Interface来完成,每个模块Module将拥有2个Callback_Queue,
     前Queue用于自己的处理,并设置给上级Module,后Queue用于调用下级Module,
     可实现多种输入数据和多种输出数据的,每个Module根据自己的处理逻辑实现自己前Callback
   */
class YSOS_EXPORT ModuleInterface :
  virtual public BaseInterface,
  virtual public ControlInterface {
  IMPLEMENT_PUBLIC_EMPTY_DESTRUCTOR(ModuleInterface)
 public:
  /*enum ModuleType {
    Source = 0,           ///< 0个prev, >=1个next//NOLINT
    Transform,            ///< 1个prev, >=1个next//NOLINT
    Destination,          ///< 1个prev, 0个next//NOLINT
    Repeater,             ///< >=1个prev, >=0个next;优先级最高, 禁止修改数据//NOLINT
  };
  enum Properties {
    In_DataTypes = 0,     ///< 能力输入数据类型，可确定协议格式
    Out_DataTypes,        ///< 能力输出数据类型，可确定协议格式
    Self_Allocator,       ///< 能力是否需要自己分配器
    Buffer_Prefix_Length, ///< 能力分配缓冲需要的前缀长度
    Buffer_Length,        ///< 能力分配缓冲需要的最大长度
    Buffer_Data_Revise,   ///< 如能力共享分配器，下级能力是否修改数据
    Buffer_Number,        ///< 能力需要最大缓冲个数
    Max_Capbility,        ///< 能力可同步处理的最大计算个数
    Properties_End
  };*/
  struct ModuleDataInfo {
    int                module_type;     ///< Module的类型
    CallbackDataType   in_datatypes;    ///< 输入的数据类型，多个类型间用'|'分割
    CallbackDataType   out_datatypes;   ///< 输出的数据类型，多个类型间用'|'分割
    bool          is_self_allocator;    ///< 是否需要自己的分配器
    uint32_t      prefix_length;   ///< 前缀的长度
    uint32_t      buffer_length;   ///< 缓冲数据的最大长度
    bool          is_data_revise;  ///< 是否需要修改数据
    uint32_t      buffer_number;   ///< 缓冲区缓冲的最大个数
    uint32_t      max_capacity;    ///< Module支持的最大能力
  };

   /**
   *@brief 打开并初始化Module计算单元//NOLINT
   *@param param[Input]： 输入的配置参数//NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值//NOLINT
   */
  virtual int Open(LPVOID param = NULL, LPVOID context = NULL) = 0;

  /**
   *@brief 关闭该Module//NOLINT
   *@param ： 无//NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值//NOLINT
   */
  virtual int Close(LPVOID param = NULL) = 0;

   /**
   *@brief 添加实际的计算逻辑回调函数//NOLINT
   *@param callback_ptr[Input]： 输入的回调函数指针//NOLINT
   *@param owner_id[Input]： 隶属模块ID//NOLINT
   *@param type[Input]： 暂定为优先级，视实际情况扩展//NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值//NOLINT
   */
  virtual int AddCallback(CallbackInterfacePtr callback_ptr, INT64 &owner_id, INT32 type) = 0;

  /**
   *@brief 删除计算逻辑回调函数，按照隶属模块ID//NOLINT
   *@param owner_id[Input]： 隶属模块ID//NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值//NOLINT
   */
  virtual int RemoveCallback(INT64 &owner_id) = 0;
};

typedef boost::shared_ptr<ModuleInterface> ModuleInterfacePtr;
typedef std::list<ModuleInterfacePtr> ModuleInterfacePtrList;
typedef std::list<ModuleInterfacePtr>::iterator ModuleInterfacePtrListIterator;
typedef std::pair<ModuleInterfacePtr, ModuleInterfacePtr> ModulePtrEdge;
typedef std::pair<ModuleInterface*, ModuleInterface*> ModuleEdge;
typedef std::list<ModulePtrEdge> ModulePtrEdgeList;
typedef std::list<ModulePtrEdge>::iterator ModulePtrEdgeListIterator;

/// 用于保存Link中某一个节点的信息//NOLINT
/// 也用于Link边信息、层信息中//NOLINT
struct InfoOfNodeInModuleLink {
  std::string node_logic_name;
  std::string node_uuid_name;
  uint32_t node_id;
  uint32_t node_layer;
  void* node_ptr;
  InfoOfNodeInModuleLink() {
    node_logic_name = "";
    node_uuid_name = "";
    node_id = 0;
    node_layer = 0;
    node_ptr = NULL;
  }
};
typedef boost::shared_ptr<InfoOfNodeInModuleLink>   \
  InfoOfNodeInModuleLinkPtr;
typedef std::list<InfoOfNodeInModuleLinkPtr>        \
  InfoOfNodeInModuleLinkPtrList;
typedef InfoOfNodeInModuleLinkPtrList::iterator     \
  InfoOfNodeInModuleLinkPtrListIterator;

struct InfoOfEdgeInModuleLink {
  std::string edge_logic_name;
  uint32_t edge_id;
  InfoOfNodeInModuleLinkPtr prev_node_info_ptr;
  InfoOfNodeInModuleLinkPtr next_node_info_ptr;
  InfoOfEdgeInModuleLink() {
    edge_logic_name = "";
    edge_id = 0;
    prev_node_info_ptr = NULL;
    next_node_info_ptr = NULL;
  }
};
typedef boost::shared_ptr<InfoOfEdgeInModuleLink>   \
  InfoOfEdgeInModuleLinkPtr;
typedef std::list<InfoOfEdgeInModuleLinkPtr>        \
  InfoOfEdgeInModuleLinkPtrList;
typedef InfoOfEdgeInModuleLinkPtrList::iterator     \
  InfoOfEdgeInModuleLinkPtrListIterator;

  /**
   *@brief ModuleLink接口，系统内部的基本逻辑单元
     能力链接口，管理相互之间有数据传输处理逻辑关系的能力模块，
     将多个能力模块按照链表的形式串接起来，
     并把最后接口的处理结果发送给Strategy_Interface中，
     由Strategy_Interface进行综合分析处理
   */
class YSOS_EXPORT ModuleLinkInterface :
  virtual public BaseInterface,
  virtual public ControlInterface {
  IMPLEMENT_PUBLIC_EMPTY_DESTRUCTOR(ModuleLinkInterface)
public:

  /**
   *@brief 添加模块到模块链//NOLINT
     prev,curr若所在链长度==1则其Module可来自外部或内部.//NOLINT
     prev,curr若所在链长度>1则其Module必来自本ModuleLink已有Module集合.//NOLINT
     prev,curr若非空则可能指向其所在链的任何一个节点//NOLINT
     prev与curr均非空时连接的条件:curr为其所在链的首节点,且prev!=curr,且无环//NOLINT
     curr与next均非空时连接的条件:next为其所在链的首节点,且curr!=next,且无环//NOLINT
   *@param curr_module_ptr[Input]： 要添加的模块指针//NOLINT
   *@param prev_module_ptr[Input]： 要添加的模块的前置模块指针//NOLINT
   *@param next_module_ptr[Input]： 要添加的模块的后置模块指针//NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值//NOLINT
   */
  virtual int AddModule(
    ModuleInterfacePtr curr_module_ptr,
    ModuleInterfacePtr prev_module_ptr = NULL,
    ModuleInterfacePtr next_module_ptr = NULL) = 0;

  /**
   *@brief 添加中继器模块到模块链, 并设置其路由集//NOLINT
   *@param repeater_ptr[Input]： 要添加的中继器模块指针//NOLINT
   *@param edges_list[Input]： 该中继器的路由集//NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值//NOLINT
   */
  virtual int AddModule(
    ModuleInterfacePtr repeater_ptr,
    const ModulePtrEdgeList &edges_list) = 0;

  /**
   *@brief 从模块链移除模块，按照模块名称//NOLINT
     查找相关模块,若找到则://NOLINT
       停止之并断开该Module的连接,//NOLINT
   *@param module_name[Input]： 模块名称//NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值//NOLINT
   */
  virtual int RemoveModule(const std::string &module_name) = 0;

  /**
   *@brief 从模块链查找模块，按照模块名称//NOLINT
   *@param module_ptr[Output]： 查找到的模块//NOLINT
   *@param module_name[Input]： 模块名称//NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值//NOLINT
   */
  virtual int FindModuleByName(
    ModuleInterfacePtr *module_ptr,
    const std::string &module_name) = 0;

  /**
   *@brief 从模块链查找模块，按照模块类型//NOLINT
   *@param module_ptr[Output]： 查找到的模块//NOLINT
   *@param module_type[Input]： 模块类型，也即ModuleInterface::ModuleType//NOLINT
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值//NOLINT
   */
  virtual int FindModuleByType(
    ModuleInterfacePtr *module_ptr, int module_type) = 0;

  /**
  int Ioctl(INT32 ctrol_id, LPVOID param)中对参数param的说明:
  param应可转换为非空的PropertyInfo* control_info
  control_info->str_id是控制范围字符串ControlRangeString:
  "module"              /// only for one module;ioctl supported, other(Open,Close,Stop,Pause,Run,Flush) unsupported
  ""or"|"               /// whole
  "|tail"               /// from head to tail
  "|module"             /// UNSUPPORTED: from head to non-tail module; 非open/close, 先控制其后的, 再自此向头控制
  "module|tail"         /// from non-head module to tail
  "module|"             /// from non-head module to each tail
  "moduleA|moduleB"     /// UNSUPPORTED: moduleA not head, moduleB not tail, for sub link segment
  注:tail = (module is Destination or Repeater after Destination;
  */
};

typedef boost::shared_ptr<ModuleLinkInterface> ModuleLinkInterfacePtr;

} // namespace ysos

#endif  //MODULE_LINK_INTERFACE_H//NOLINT
