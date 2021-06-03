/**
  *@file ModuleLinkImpl.h
  *@brief Definition of ModuleLinkImpl
  *@version 0.1
  *@author XuLanyue
  *@date Created on: 2016-05-11 13:59:20
  *@copyright Copyright © 2016 YunShen Technology. All rights reserved.
    
  *@todo
  ///  BufferPool, AllocatorProperty
  ///  Thread
  ///  协商完之后在SetProperty
  ///  获取BufferAmount/PrefixLength/SelfAllocator的时机为读完配置时.
  ///  为方便测试,目前设置PrefixLength/SelfAllocator的时机暂定为
  ///  Module创建之后, Open之前, Link之时.
  ///  不考虑BufferAmount.
  ///  Repeater路由项的动态添加/删除/清空
  ///    以Destination结尾
  ///    以Repeater结尾
  ///    ModuleLink与Strategy//Module, IoCtl: GetReapeater
  ///  lock.
  ///  对于没有链上的Module如何处理?(只要有头有尾即可).//只要有头即可
  ///  在创建Repeater时, 备份原有普通边及CallbackQueue
  */
#ifndef SIP_MODULE_LINK_IMPL_H_//NOLINT
#define SIP_MODULE_LINK_IMPL_H_//NOLINT

#include <string>
#include <list>
#include <set>

#include "../../../public/include/sys_interface_package/moduleinterface.h"
#include "../../../public/include/os_hal_package/lock.h"
#include "../../../public/include/core_help_package/baseinterfaceimpl.h"

#define MODULE_LOWEST_PRIORITY                      0

#define ADD_MODULE_CURR_ADDED                       1
#define ADD_MODULE_PREV_ADDED                       2
#define ADD_MODULE_NEXT_ADDED                       4
#define ADD_MODULE_PREV_CURR_LINKED                 8
#define ADD_MODULE_CURR_NEXT_LINKED                 16

#define LINK_CASE_ILLEGAL                           0
#define LINK_CASE_CURR_ONLY                         1
#define LINK_CASE_PREV_CURR                         3
#define LINK_CASE_CURR_NEXT                         5
#define LINK_CASE_PREV_CURR_NEXT                    7

#define MODULE_INVALID_IOCTL_COMMAND                -1

#define ENABLE_SOURCE_AS_WHOLE_LINK                 1

/// 1, 可行, 临时决定; 0, 以后讨论
#define ALWAYS_STOP_LINK_IF_ADD_OR_DEL_EDGE         1

/*
  对于ModuleLink:
  CMD_OPEN, CMD_STOP, CMD_PAUSE, CMD_RUN, CMD_FLUSH, CMD_CLOSE,
  CMD_MODULE_LINK_ADD_INHERENT_REPEATER
  都是ModuleLink内部调用的,外部不应直接显式地调用.
*/

#define BASE_REPEATER_MODULE_IMPL_CLASS_NAME        "BaseRepeaterModuleImpl"
#define BASE_REPEATER_CALLBACK_IMPL_CLASS_NAME      "BaseRepeaterCallbackImpl"
namespace ysos {

class ModuleLinkInfo;
typedef boost::shared_ptr<ModuleLinkInfo> ModuleLinkInfoPtr;
typedef std::list<ModuleLinkInfoPtr> ModuleLinkInfoList;
typedef std::list<ModuleLinkInfoPtr>::iterator ModuleLinkInfoIterator;

typedef std::pair<std::string, std::string>           EdgeOfModuleName;
typedef std::list<EdgeOfModuleName>                   EdgeOfModuleNameList;
typedef EdgeOfModuleNameList::iterator                EdgeOfModuleNameListIterator;

/// 返回int的访问ModuleLink函数类型定义
typedef int (*VISIT_FUNC_INT)(ModuleLinkInfoIterator, void*, void*, bool&, int&);

/// 在Strategy向ModuleLink请求根据包含指定名称的Module获取对应的Repeater时,
/// 设置module_name_成员, 传入RepeaterIncludeModule*
/// ModuleLink将置repeater_ptr_成员.
struct RepeaterIncludeModule {
  RepeaterIncludeModule() {
    module_name_ = "";
    repeater_ptr_list_.clear();
  }
  ~RepeaterIncludeModule() {
    module_name_ = "";
    repeater_ptr_list_.clear();
  }
  std::string module_name_;
  ModuleInterfacePtrList repeater_ptr_list_;
};

class YSOS_EXPORT SubLinkInfoInModuleLink {
 public:
  SubLinkInfoInModuleLink() {
    module_ptr_ = NULL;
    logic_name_ = "";
    active_ = false;
  }
  ModuleInterfacePtr module_ptr_;
  ModuleLinkInfoIterator link_info_position_;
  std::string logic_name_;          // logic name of the module
  bool active_;                     // is active

  ///  约定: 若一层已有Repeater, 则添加该层的边到Repeater时不再创建新的Repeater.
  ///  NextToDo
  ///  link_flag: Linked, Unlinked: for Each Dst
  ///  ModuleLink维持逻辑意义. 逻辑名称:源
  ///  LinkOpen:Commit
  ///  LinkClose:Decommit.
  ///  ModuleInLink:
  ///  prev回报状态.
  ///    每一条子链的状态.
  ///    return_value.
  ///    sublink         module
  ///    ----------------================
  ///    0bnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn
  ///    通报错误状态.
  ///  Ioctl:切换CurrentIoctl.
};

/// 为恢复Repeater移除前的两个Module之间的链接信息
/// 为保存两个Module之间的链接信息
class YSOS_EXPORT LinkInfoBetweenModules {
 public:
  bool inherent_edge;                             /// 固有的Repeater存在
  ModulePtrEdge edge;
  std::list<CallbackInterfacePtr> callback_list;  /// for prev module
  ModuleLinkInfoList next_module_list_ptr;        /// for prev module
  ModuleLinkInfoList prev_module_list_ptr;        /// for next module
  LinkInfoBetweenModules() {
    inherent_edge = false;
    edge.first = NULL;
    edge.second = NULL;
    callback_list.clear();
    next_module_list_ptr.clear();
    prev_module_list_ptr.clear();
  }
  ~LinkInfoBetweenModules() {
    inherent_edge = false;
    edge.first = NULL;
    edge.second = NULL;
    callback_list.clear();
    next_module_list_ptr.clear();
    prev_module_list_ptr.clear();
  }
};

typedef std::list<LinkInfoBetweenModules>
  LinkInfoBetweenModulesList;

typedef std::list<LinkInfoBetweenModules>::iterator
  LinkInfoBetweenModulesListIterator;

/// 这里假定一个Level后只能有至多一个Repeater
class YSOS_EXPORT RepeaterModuleInfo {
 public:
  std::string name;                           ///< 名称: SourceModuleName_level_after_I_Repeater
  std::string famous_name;                    ///< 名称: SourceModuleName_level_after_I_Repeater
  bool inherent;                              ///< 是否固有的Repeater
  uint32_t level_after;                       ///< Repeater位于该层之后
  ModulePtrEdgeList inherent_edge_set;        ///< 固有边的集合
  ModuleInterfacePtrList inherent_prev_set;   ///< 固有PrevModule的集合
  ModuleInterfacePtrList inherent_next_set;   ///< 固有NextModule的集合
  ModulePtrEdgeList current_edge_set;         ///< 当前边的集合
  ModuleInterfacePtrList current_prev_set;    ///< 当前PrevModule的集合
  ModuleInterfacePtrList current_next_set;    ///< 当前NextModule的集合

  /// 备份边及其PrevModule的NextCallbackQueue中的Callback
  LinkInfoBetweenModulesList backup_link_info;
};

typedef boost::shared_ptr<RepeaterModuleInfo> RepeaterModuleInfoPtr;
typedef std::list<RepeaterModuleInfoPtr> RepeaterModuleInfoPtrList;
typedef std::list<RepeaterModuleInfoPtr>::iterator
  RepeaterModuleInfoPtrListIterator;

typedef std::list<SubLinkInfoInModuleLink> SubLinkInfoList;
typedef std::list<SubLinkInfoInModuleLink>::iterator SubLinkInfoListIterator;

class YSOS_EXPORT ModuleLinkInfo {
 public:
  explicit ModuleLinkInfo(const ModuleInterfacePtr &module_ptr);
  virtual ~ModuleLinkInfo();

  ModuleInterfacePtr module_ptr_;                   ///< 当前模块
  std::string module_path_;
  uint16_t module_priority_;
  bool is_strategy_;
  bool new_thread_;

  /// Level是基于1的
  /// LevelX层和LevelY层之间的Repeater的level_ = (X << 16) + Y;
  /// 若LevelX层后的Repeater无后继则该Repeater的level_ = (X << 16) + 0;
  /// LevelX层的Module的level_ = X * 0x10001;
  uint16_t level_;
  bool inherent_;                                   ///< 此Module是该Link的固有Module
  ModuleInterface::ModuleDataInfo module_properties;
  std::list<ModuleLinkInfoPtr> prev_module_list_;   ///< 逆链搜索
  std::list<ModuleLinkInfoPtr> next_module_list_;   ///< 顺链搜索
};

class YSOS_EXPORT ModuleLinkImpl : public ModuleLinkInterface,
  public BaseInterfaceImpl {
  DISALLOW_COPY_AND_ASSIGN(ModuleLinkImpl)
  DECLARE_PROTECT_CONSTRUCTOR(ModuleLinkImpl)
  DECLARE_CREATEINSTANCE(ModuleLinkImpl)
 public:
  virtual ~ModuleLinkImpl();

  /**
   *@brief 类似于模块的初始化功能, 同步, 从前往后, 调用Open
   *@return 成功返回0，否则失败
  */
  virtual int Initialize(void *param = NULL);

  /**
   *@brief 清除资源,Initialize的逆操作
   *@param param： 初始化的参数
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值
   */
  virtual int UnInitialize(void *param = NULL);

  /**
   *@brief 获取接口的属性值
   *@param type_id[Input]： 属性的ID
   *@param type[Input/Output]：属性值的值
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值
   */
  virtual int GetProperty(int type_id, void *type);

  /**
   *@brief 设置接口的属性值
   *@param type_id[Input]： 属性的ID
   *@param type[Input/Output]： 属性值的值
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值
   */
  virtual int SetProperty(int type_id, void *type);

  /*
  ControlRangeString:
  "module"              /// only for one module;ioctl supported, other(Open,Close,Stop,Pause,Run,Flush) UNSUPPORTED
  ""or"|"               /// whole
  "|tail"               /// from head to tail
  "|module"             /// UNSUPPORTED: from head to non-tail module; 非open/close, 先控制其后的, 再自此向头控制
  "module|tail"         /// from non-head module to tail
  "module|"             /// from non-head module to each tail
  "moduleA|moduleB"     /// UNSUPPORTED: moduleA not head, moduleB not tail, for sub link segment
  */

  /**
   *@brief 类似于模块的初始化功能, 同步, 从前往后, 打开相关的Module
   *@param param: NULL, 整个Link; else, ControlRangeString, std::string*, 指定tail, 子链(或子链的列表)
   *@param context: 上下文
   *@return 成功返回0，否则失败
   */
  virtual int Open(LPVOID param = NULL, LPVOID context = NULL);

  /**
   *@brief 减少计数, 同步, 从前往后, 关闭相关的Module
   *@param param: NULL, 整个Link; else, ControlRangeString, std::string*, 指定tail, 子链(或子链的列表)
   *@return 成功返回0，否则失败
   */
  virtual int Close(LPVOID param = NULL);

  /**
    *@brief 控制函数，可对该接口进行详细控制.
     异步, 从后往前.
     若param中逻辑子链的名字为空则委托给各Module
     逻辑的,从某个DestinationModule往Source.
     传入的是一个Module的名字,发给Repeater该Module发出的所有的边.
   *@param ctrl_id[Input]： 详细控制ID号
   *@param param[Input/Output]：详细配置的参数
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值
   */
  virtual int Ioctl(INT32 control_id, LPVOID param = NULL);

  /**
   *@brief 停止, 异步, 从后往前
   *@param ： NULL, 整个Link; else, ControlRangeString, std::string*
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值
   */
  virtual int Stop(LPVOID param = NULL);

  /**
   *@brief 暂停, 异步, 从后往前
   *@param ： NULL, 整个Link; else, ControlRangeString, std::string*
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值
   */
  virtual int Pause(LPVOID param = NULL);

  /**
   *@brief 运行, 异步, 从后往前
   *@param ： NULL, 整个Link; else, ControlRangeString, std::string*
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值
   */
  virtual int Run(LPVOID param = NULL);

  /**
   *@brief 冲洗, 同步, 从后往前, 快速将内部缓冲中剩余数据处理完
   *@param ： NULL, 整个Link; else, ControlRangeString, std::string*
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值
   */
  virtual int Flush(LPVOID param = NULL);

  /**
   *@brief 获得Link的状态
   *@param timeout[Input]： 超时值
   *@param state[Output]：状态
   *@param param[Input]：当前为空
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值
   */
  virtual int GetState(UINT32 time_out, INT32 *state, LPVOID param = NULL);

  /**
   *@brief 添加模块到模块链
   *@param curr_module_ptr[Input]： 要添加的模块指针
   *@param prev_module_ptr[Input]： 要添加的模块的前置模块指针
   *@param next_module_ptr[Input]： 要添加的模块的后置模块指针
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值
   */
  virtual int AddModule(
    ModuleInterfacePtr curr_module_ptr,
    ModuleInterfacePtr prev_module_ptr = NULL,
    ModuleInterfacePtr next_module_ptr = NULL);

   /**
   *@brief 添加中继器模块到模块链, 并设置其路由集
   *@param repeater_ptr[Input]： 要添加的中继器模块指针
   *@param edges_list[Input]： 该中继器的路由集
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值
   */
  virtual int AddModule(ModuleInterfacePtr repeater_ptr, const ModulePtrEdgeList &edges_list);

   /**
   *@brief 从模块链移除模块，按照模块名称
   *@param module_name[Input]： 模块名称
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值
   */
  virtual int RemoveModule(const std::string &module_name);

   /**
   *@brief 从模块链查找模块，按照模块名称
   *@param module_ptr[Output]： 查找到的模块
   *@param module_name[Input]： 模块名称
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值
   */
  virtual int FindModuleByName(
    ModuleInterfacePtr *module_ptr,
    const std::string &module_name);

   /**
   *@brief 从模块链查找模块，按照模块类型
   *@param module_ptr[Output]： 查找到的模块
   *@param module_type[Input]： 模块类型，也即ModuleInterface::ModuleType
   *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回相应错误值
   */
  virtual int FindModuleByType(ModuleInterfacePtr *module_ptr, int module_type);

  /// -1, 出错
  /// 0, current不在from到to对应的链片段上
  /// 1, current在from到to对应的链片段上
  int IsModuleOnSubLinkPiece(
    const std::string &range_from, const std::string &range_to, const std::string &module_name);
 protected:
  /// 获取Module所在LinkInfo链的头
  /// 若已在链中则置link_info_head_ptr,并返回true;否则返回false
  bool GetTheHeadOfLinkInfoListByModule(
    ModuleInterfacePtr module_ptr,
    ModuleLinkInfoPtr& link_info_head_ptr);

  bool GetTheHeadPositionOfLinkInfoListByModule(
    ModuleInterfacePtr module_ptr,
    ModuleLinkInfoIterator& link_info_head_position);

  bool IsModuleAlreadyInLink(const ModuleInterfacePtr &module_ptr, ModuleLinkInfoPtr& link_info_ptr);
  bool IsModuleAlreadyInLink(ModuleInterface *module_ptr, ModuleLinkInfoPtr& link_info_ptr);

  int GetModuleLinkInfoPtrByName(ModuleLinkInfoPtr& link_info_ptr, const std::string &module_name);

  int UnlinkModule(ModuleInterfacePtr curr);

  int UnlinkModules(
    ModuleInterfacePtr prev,
    ModuleInterfacePtr next,
    RepeaterModuleInfoPtr* repeater_info_ptr_ptr = NULL);

  int RemoveNonRepeaterModule(const ModuleInterfacePtr &module_ptr);

  /// 移除Repeater
  int RemoveRepeaterModule(const ModuleInterfacePtr &module_ptr, bool fixed_repeater);

  /// NextToDo: 搜索子链应在全部Module链接完成之后
  bool InsertModuleIfPossible(
    ModuleInterfacePtr module_ptr,
    ModuleLinkInfoPtr &module_link_info_ptr,
    bool &already_in_list);

  void InsertModuleIfPossible(
    ModuleInterfacePtr curr_module_ptr,
    ModuleInterfacePtr prev_module_ptr,
    ModuleInterfacePtr next_module_ptr,
    bool &curr_already_in_list,
    bool &prev_already_in_list,
    bool &next_already_in_list,
    ModuleLinkInfoPtr &curr_module_link_info_ptr,
    ModuleLinkInfoPtr &prev_module_link_info_ptr,
    ModuleLinkInfoPtr &next_module_link_info_ptr);

  void UpdatePathForSelfAndDescendent(
    ModuleLinkInfoPtr root,
    std::string& prefix,
    bool add_prefix);

  int GetEdgesFromString(
    const std::string& vertex_or_edge_name,
    ModulePtrEdgeList& edge_list,
    ModuleInterfacePtr& curr_module,
    ModuleInterfacePtr& next_module);

  /// PrevModule与NextModule的类型是否匹配
  /// NextToDo: Buffer的尺寸和数量是否匹配
  bool IsPrevOutAndNextInMatched(
    ModuleInterfacePtr prev,
    ModuleInterfacePtr next);

  void IsModuleInModuleLinkInfoList(
    ModuleInterfacePtr module_ptr,
    ModuleLinkInfoPtr head_link_info_ptr,
    bool& ret_value);
  bool CanAddToPrevField(ModuleLinkInfoPtr module_link_info_ptr);
  bool CanAddToPrevField(
    ModuleLinkInfoPtr prev,
    ModuleLinkInfoPtr next);

  bool ExistSourceModule(ModuleLinkInfoPtr module_link_head_ptr);
  bool ExistSourceModule();

  bool ExistCycleIfModulesLinked(
    ModuleLinkInfoPtr prev_link_head_ptr,
    ModuleLinkInfoPtr next_link_head_ptr);

  bool ExistCycleIfModulesLinked(
    ModuleInterfacePtr prev_module_ptr,
    ModuleInterfacePtr next_module_ptr);

  bool AddModuleLinkInfo(
    ModuleLinkInfoPtr prev,
    ModuleLinkInfoPtr next);

  /// 每次AddModule之后调用
  bool IsEachModuleLinked();

  bool IsLinkCompleted();

  /// 设置链接完成状态
  void SetLinkCompleted(bool link_completed);

  ///   重建ModuleLink信息树, 在AddModule完成且IsEachModuleLinked时调用.
  bool RebuildModuleLinkInfoTree();
  bool RebuildSubLinkInfoInModuleLink();
  void RebuildModuleLinkInfoTree_(ModuleLinkInfoIterator root);
  bool DumpCallbackQueue();
  void DumpAll();

  int VisitLinkInfoTree(ModuleLinkInfoIterator root, void* param_ptr, void* context_ptr, bool &failed,
    VISIT_FUNC_INT visit_func, SubLinkInfoInModuleLink *sub_link_ptr);

  /**
    *@brief 从链的某个模块起, 向链首方向, 到链的某个模块止, 控制单条逻辑子链片段; 若遇Repeater则控制权被转给Repeater并返回.
    *@param from_ptr[Input]： 从该指针对应的模块起, 若为空则表示自每一个尾模块起.
    *@param to_ptr[Input]： 到该指针对应的模块止, 若为空则表示到首模块.
    *@param param_ptr[Input]：控制参数
    *@param command[Input]： 控制命令
    *@param control_type[Input]：控制类别
    *@return： 返回0表示控制权转交给了Repeater,返回1表示需要继续处理
    */
  int ControlSubLink(
    ModuleLinkInfoPtr from_ptr,
    ModuleLinkInfoPtr to_ptr,
    void* param_ptr,
    int command,
    int control_type);

  int IterateControlSubLink(void* param_ptr, int command, int control_type);

  int ExecuteCommand(
    int command,
    int control_type,
    ModuleInterfacePtr module_ptr,
    void* param_ptr);

  int GetControlRangePart(void* param, std::string &range_from, std::string &range_to);
  int GetCaseOfControlRange(void* param, ModuleInterface** from_ptr_ptr, ModuleInterface** to_ptr_ptr);

  /// Control single module by ModuleLink.
  int ControlModule(INT32 control_id, ModuleInterface *module_ptr, void* param);

  /// 添加固有Repeater.
  /// 调用时机: Link在Open前,而其他固有Module已链入.
  int OnAddInherentRepeater(void *param);

  /// 若必要则添加Repeater, 止能其中的原有普通边, 添加边到Repeater中(以监听).
  int OnAddEdgeToRepeater(std::string *param);

  /// 从Repeater中移除边, 使能其中的原有普通边.
  int OnRemoveEdgeFromRepeater(std::string *param);

  /// NextToDo: 考虑Repeater扮演Destination角色的情形.
  /// 边的链接信息是否在Link的Repeater中已有记录.
  /// 若有则置repeater_info_ptr_list_iterator并返回true;否则返回false.
  bool ExistLinkInfoBetweenModulesOfEdgeInLink(
    ModuleInterfacePtr prev,
    ModuleInterfacePtr next,
    bool& in_inherent_repeater_list,
    RepeaterModuleInfoPtrListIterator &repeater_info_ptr_list_iterator);

  bool ExistRepeaterWithBeginLevel(
    uint32_t level,
    bool &inherent,
    bool act_as_destination,
    RepeaterModuleInfoPtrListIterator& repeater_info_ptr_list_iterator);

  bool ExistRepeaterWithBeginModule(
    ModuleInterfacePtr prev,
    bool &inherent,
    RepeaterModuleInfoPtrListIterator& repeater_info_ptr_list_iterator);

  bool IsEdgeAsNormalAlreadyInModuleLink(const ModulePtrEdge &edge);

  bool ExistEdgeAsNormalAlreadyInModuleLink(const ModulePtrEdgeList &edge_list);

  bool OnCaseExistRepeaterWithBeginLevel(
    ModulePtrEdgeList& edge_list,
    bool in_inherent_repeater_list,
    RepeaterModuleInfoPtrListIterator& repeater_info_ptr_list_iterator,
    RepeaterModuleInfoPtr& repeater_info_ptr,
    bool& restart_required);

  bool OnCaseNotExistRepeaterWithBeginLevel(
    ModulePtrEdgeList& edge_list,
    bool in_inherent_repeater_list,
    RepeaterModuleInfoPtrListIterator& repeater_info_ptr_list_iterator,
    RepeaterModuleInfoPtr& repeater_info_ptr,
    ModuleInterfacePtr& curr_module,
    ModuleInterfacePtr& next_module,
    uint32_t& level,
    uint32_t level_repeater,
    bool& act_as_destination,
    bool& restart_required,
    bool inherent_repeater = false);

  int GetLinkDepth(uint32_t* depth_ptr);

  bool IsModuleOnSubLinkByLinkInfo(ModuleInterface *module_ptr,
    ModuleInterfacePtr module_sptr, ModuleLinkInfoPtr link_info_ptr);

  bool IsModuleOnSubLink(ModuleInterface *module_ptr, SubLinkInfoInModuleLink *sub_link_ptr);

  bool GetSubLinkByModule(ModuleInterface *module_ptr, std::list<SubLinkInfoInModuleLink> &sub_link_info_list,
    SubLinkInfoInModuleLink &sub_link);

  bool IsModuleOnSubLink(ModuleInterface *module_ptr, ModuleInterface *range_to_module_ptr,
    std::list<SubLinkInfoInModuleLink> &sub_link_info_list);

  int IsModuleOnSubLinkPiece(
  const ModuleLinkInfoPtr &mlip_range_from,
  const ModuleLinkInfoPtr &mlip_range_to,
  const ModuleLinkInfoPtr &mlip_module_name,
  const std::string &next_non_repeater);

  int GetPrevModuleSetByRepeaterName(
    const std::string &repeater_name, const std::string &next_non_repeater, std::set<std::string> &prev_module_name_set);

  int GetSetOfModuleWithoutNextInRepeater(
    const std::string &repeater_name,
    std::list<std::string> &name_set_of_module_without_next);

  bool MakeEdgeList(
    ModulePtrEdgeList& edge_list,
    EdgeOfModuleNameList& edge_of_name_list);

  void DoCompleteForNode(ModuleLinkInfoIterator it);
 protected:
   /// Link在初始化中Open后立即Run, 对应于Link配置文件中的autorun, 默认为false
   bool autorun_;

  /// module_link操作锁.
  LightLock module_link_lock_;

  /// 该ModuleLink是固有的.
  bool inherent_;

  /// 若该链已打开则为true, 否则为false.
  /// 在Open和Close时将改变其状态.
  /// 在Link open前链入固有Repeater时将使用该状态.
  bool link_opened_;

  /// Link的当前状态
  // int link_stat_;

  /// Link的长度.
  /// 置为其中level最大的Module的level的高16位.
  uint32_t link_depth_;

  /// 若已经将所有的Module链成了一棵树, 则为true, 否则为false.
  bool tree_built_;

  /// 存放ModuleLink里的子链的部分信息.
  std::list<SubLinkInfoInModuleLink> sub_link_info_list_;

  /// 存放ModuleLink里的Source的位置.
  ModuleLinkInfoIterator source_position_of_the_link;

  /// 存放ModuleLink里的所有Module. Link时使用, AddModule和RemoveModule时更新.
  std::list<ModuleLinkInfoPtr> module_link_info_list_;

  /// 存放固有的Repeater的相关信息.
  RepeaterModuleInfoPtrList inherent_repeater_info_list_;

  /// 存放临时的Repeater的相关信息.
  RepeaterModuleInfoPtrList temporary_repeater_info_list_;
};
}
#endif  //  SIP_BASE_MODULE_IMPL_H_//NOLINT

