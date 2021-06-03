/*
# externinterface.h
# Definition of extern interface
# Created on: 2017-03-20 15:53:55
# Original author: jinchengzhe
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
# version 0.1, 20170320, created by JinChengZhe
*/

#ifndef SPS_EXTERN_INTERFACE_H_
#define SPS_EXTERN_INTERFACE_H_

/// C++ Standard Headers
#include <vector>
#include <algorithm>
#include <deque>
/// ThirdParty Headers
#include <boost/shared_ptr.hpp>  // NOLINT
#include <boost/thread.hpp>
#include <boost/assert.hpp>
#include <boost/concept_check.hpp>
#include "../../../public/include/ysoslog/ysoslog.h"


namespace ysos {

namespace extern_interface {
/**
*@brief 全局日志对象  // NOLINT
*/
extern log4cplus::Logger logger_;  ///< 日志对象

/**
*@brief 数值参数  // NOLINT
*/
enum {
  IS_INITIAL = 0,  ///< 初始状态  // NOLINT
  IS_RUNNING = 1,  ///< 运行状态  // NOLINT
  IS_STOPPED = 2,  ///< 停止状态  // NOLINT
  IS_UNINITIALIZED = 10,  ///< 未初始化  // NOLINT
  IS_INITIALIZED = 11,  ///< 初始化  // NOLINT
  MSG_CHECK_NEW_VERSION = 101,  ///< 检测是否需要更新  // NOLINT
  MSG_GET_CURRENT_VERSION,  ///< 获取当前版本信息命令  // NOLINT
  MSG_GET_ALL_AVAILABLE_VERSION,  ///< 获取最新版本信息命令  // NOLINT
  MSG_DOWNLOAD_FILES,  /// 下载文件命令  // NOLINT
  MSG_PREPARE_UPDATE_COMPLETE,  // 更新准备完毕命令  // NOLINT
  MSG_GET_UPDATE,  ///< 更新版本命令  // NOLINT
  MSG_DOWNLOAD_FILE_PROGRESS,  /// 下载文件进度命令  // NOLINT
  MSG_START = 1000,  ///< 开始命令  // NOLINT
  MSG_STOP = 1001,  ///< 关闭命令  // NOLINT
  MSG_INTERNAL_CHECK_NEW_VERSION,  ///< 检测是否需要更新内部用消息  // NOLINT
  MSG_KILL_PROCESS,  ///< 杀死指定进程  // NOLINT
  MSG_EXECUTE_PROCESS,  ///< 运行指定进程  // NOLINT
  MSG_DISCONNECT,  ///< 停止通讯连接  // NOLINT
  MSG_EXECUTE_INSTALL_EXE_FAILED,  ///< 运行安装脚本失败  // NOLINT
};

/**
*@brief JobQueue的具体实现  // NOLINT
*/
template<typename Job>
class JobQueue : boost::noncopyable {
 public:
  typedef Job JobType;
  typedef std::deque<JobType> QueueType;

  typedef boost::mutex MutexType;
  typedef boost::unique_lock<MutexType> LockType;
  typedef boost::condition_variable_any ConditionType;

  BOOST_CONCEPT_ASSERT((boost::SGIAssignable<JobType>));
  BOOST_CONCEPT_ASSERT((boost::DefaultConstructible<JobType>));

  JobQueue(void) : stop_flag_(false) {}
  ~JobQueue(void) {}

  void Push(const JobType& job) {
    LockType lock(mutex_);
    job_queue_.push_back(job);
    condition_.notify_one();
    return;
  }

  JobType Pop(void) {
    LockType lock(mutex_);
    while (true == job_queue_.empty() && false == stop_flag_) {
      condition_.wait(mutex_);
    }

    if (true == stop_flag_) {
      return JobType();
    }

    BOOST_ASSERT(true != job_queue_.empty());

    JobType temp = job_queue_.front();
    job_queue_.pop_front();

    return temp;
  }

  void Stop(void) {
    stop_flag_ = true;
    condition_.notify_all();
    return;
  }

  int GetSize(void) {
    LockType lock(mutex_);
    return job_queue_.size();
  }

  void Clear(void) {
    LockType lock(mutex_);
    job_queue_.clear();
  }

 private:
  QueueType job_queue_;
  MutexType mutex_;
  ConditionType condition_;
  bool stop_flag_;
};

/**
*@brief Worker的具体实现  // NOLINT
*/
template<typename Queue>
class Worker {
 public:
  typedef Queue QueueType;
  typedef typename Queue::JobType JobType;
  typedef boost::function<bool(JobType&)> FunctionType;

  template<typename Func>
  Worker(QueueType& queue, Func func, int thread_count = 1)
    : queue_(queue),
      function_(func),
      thread_count_(thread_count) {
    BOOST_ASSERT(0 < thread_count_);
  }

  Worker(QueueType& queue, int thread_count = 1)
    : queue_(queue),
      thread_count_(thread_count) {
    BOOST_ASSERT(0 < thread_count_);
  }

  void Start() {
    BOOST_ASSERT(function_);
    if (0 < thread_goup_.size()) {
      return;
    }

    for (int ii = 0; ii < thread_count_; ++ii) {
      thread_goup_.create_thread(boost::bind(&Worker::DoWork, this));
    }

    return;
  }

  template<typename Func>
  void Start(Func func) {
    function_ = func;
    Start();
    return;
  }

  void Run() {
    Start();
    thread_goup_.join_all();
    return;
  }

  void Stop() {
    function_ = 0;
    queue_.Stop();
    return;
  }

 private:
  void DoWork() {
    for (;;) {
      JobType job = queue_.Pop();
      if (!function_ || !function_(job)) {
        break;
      }
    }
    return;
  }

  QueueType& queue_;
  FunctionType function_;
  int thread_count_;
  boost::thread_group thread_goup_;
};

/**
*@brief 消息结构体声明  // NOLINT
*/
typedef struct ExternMessage {
  enum {
    MSG_TYPE_NONE = 100,
    MSG_TYPE_SERVICE,
    MSG_TYPE_EVENT,
  };

  unsigned type_;
  unsigned id_;
  std::vector<std::string> contents_;

  ExternMessage(void)
    : type_(MSG_TYPE_NONE),
      id_(0) {}

  ExternMessage(const unsigned id, const std::string& content_0 = "", const std::string& content_1 = "", const std::string& content_2 = "")
    : type_(MSG_TYPE_NONE),
      id_(id) {
    contents_.push_back(content_0);
    contents_.push_back(content_1);
    contents_.push_back(content_2);
  }

  ExternMessage(const unsigned type, const unsigned id, const std::string& content_0 = "", const std::string& content_1 = "", const std::string& content_2 = "")
    : type_(type),
      id_(id) {
    contents_.push_back(content_0);
    contents_.push_back(content_1);
    contents_.push_back(content_2);
  }

//   ExternMessage(const ExternMessage& extern_message) {
//     id_ = extern_message.id_;
//     std::copy(extern_message.contents_.begin(), extern_message.contents_.end(), contents_.begin());
//   }

  ~ExternMessage(void) {
    id_ = 0;
    contents_.clear();
  }

  ExternMessage& operator=(const ExternMessage& extern_message) {
    if (this != &extern_message) {
      this->id_ = extern_message.id_;
      this->contents_.clear();
      std::copy(extern_message.contents_.begin(), extern_message.contents_.end(), this->contents_.begin());
    }

    return *this;
  }

  void SetContent(const std::string& content) {
    contents_.push_back(content);
  }

  std::string GetContent(const unsigned index) const {
    if (index < contents_.size()) {
      return contents_[index];
    }
    return std::string("");
  }

  unsigned GetContentCount(void) const {
    return contents_.size();
  }

  void ClearContent(void) {
    contents_.clear();
  }
} *ExternMessagePtr;

//////////////////////////////////////////////////////////////////////////
/*
观察者模式接口声明
*/
//////////////////////////////////////////////////////////////////////////
class Observer;
typedef boost::shared_ptr<Observer> ObserverPtr;
class Subject {
 public:
  typedef std::vector<ObserverPtr>::iterator ObserversItr;
  virtual ~Subject(void) {}

  /**
  *@brief 增加观察者  // NOLINT
  *@param observer_ptr[Input]： 观察者指针  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  virtual int Attach(const ObserverPtr& observer_ptr) = 0;
  /**
  *@brief 删除观察者  // NOLINT
  *@param observer_ptr[Input]： 观察者指针  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  virtual int Detach(const ObserverPtr& observer_ptr) = 0;
  /**
  *@brief 通知观察者  // NOLINT
  *@param message[Input]： 消息  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  virtual int Notify(const ExternMessage& message) = 0;
 protected:
  std::vector<ObserverPtr> observers_;  ///< 观察者队列
};
typedef boost::shared_ptr<Subject> SubjectPtr;

class Observer {
 public:
  Observer(const SubjectPtr& subject_ptr) {
    subject_ptr_ = subject_ptr;
  }
  virtual ~Observer(void) {}
  /**
  *@brief 更新  // NOLINT
  *@param message[Input]： 消息  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  virtual int Update(const ExternMessage& message) = 0;
 protected:
  SubjectPtr subject_ptr_;
};

//////////////////////////////////////////////////////////////////////////
/*
命令模式接口声明
*/
//////////////////////////////////////////////////////////////////////////
class Invoker;
typedef boost::shared_ptr<Invoker> InvokerPtr;
class Receiver;
typedef boost::shared_ptr<Receiver> ReceiverPtr;
class Command {
 public:
  Command(const InvokerPtr invoker_ptr, const ReceiverPtr& receiver_ptr) {
    invoker_ptr_ = invoker_ptr;
    receiver_ptr_ = receiver_ptr;
  }
  virtual ~Command(void) {}
  virtual int Execute(const ExternMessage& message) = 0;

 protected:
  InvokerPtr invoker_ptr_;
  ReceiverPtr receiver_ptr_;
};
typedef boost::shared_ptr<Command> CommandPtr;

class Receiver {
 public:
  typedef boost::function<void(const ExternMessage& message)> ResponseMessageHandler;
  virtual ~Receiver(void) {}
  virtual int Action(const ExternMessage& input_message, ExternMessage& output_message) = 0;
  virtual int Action(const ExternMessage& message, const ResponseMessageHandler& response_message_handler) = 0;
};

class Invoker {
 public:
  virtual ~Invoker(void) {}
  virtual int SetCommand(const CommandPtr& command_ptr) = 0;
  virtual int ExecuteCommand(const ExternMessage& message) = 0;
  virtual int Notify(const ExternMessage& message) = 0;
};


//////////////////////////////////////////////////////////////////////////
/*
中介者模式接口声明
*/
//////////////////////////////////////////////////////////////////////////
/**
 *@brief 中介者类声明  // NOLINT
 */
class Colleague;
typedef boost::shared_ptr<Colleague> ColleaguePtr;
class Mediator {
 public:
  virtual ~Mediator(void) {}

  /**
  *@brief 消息中转，协同类之间消息交换  // NOLINT
  *@param message[Input]： 消息  // NOLINT
  *@param colleague_ptr[Input]： 协同类指针  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  virtual int Send(const ExternMessage& message, const ColleaguePtr& colleague_ptr) = 0;
  /**
  *@brief 主动向协同类发送消息 // NOLINT
  *@param message[Input]： 消息  // NOLINT
  *@param colleague_ptr[Input]： 协同类指针  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  virtual int Notify(const ExternMessage& message, const ColleaguePtr& colleague_ptr) = 0;
  /**
  *@brief 广播消息给所有协同类  // NOLINT
  *@param message[Input]： 消息  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  virtual int NotifyAll(const ExternMessage& message) = 0;
};
typedef boost::shared_ptr<Mediator> MediatorPtr;

/**
 *@brief 协同类声明  // NOLINT
 */
class Colleague {
 public:
  virtual ~Colleague(void) {}
  Colleague(const MediatorPtr& mediator_ptr) {
    mediator_ptr_ = mediator_ptr;
  }

  /**
  *@brief 消息中转，协同类之间消息交换  // NOLINT
  *@param message[Input]： 消息  // NOLINT
  *@param colleague_ptr[Input]： 协同类指针  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  int Send(const ExternMessage& message);
  /**
  *@brief 处理接收到中介者中转的消息  // NOLINT
  *@param message[Input]： 消息  // NOLINT
  *@return： 成功返回YSOS_ERROR_SUCCESS，失败返回YSOS_ERROR_FAILED  // NOLINT
  */
  virtual int Notify(const ExternMessage& message) = 0;

 protected:
  MediatorPtr mediator_ptr_;  ///< 中介者指针
};

}  /// namespace extern_interface

}  /// namespace ysos

#endif
