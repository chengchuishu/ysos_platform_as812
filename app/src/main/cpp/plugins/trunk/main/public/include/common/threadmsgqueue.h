/**   //NOLINT
  *@file threadmsgqueue.h
  *@brief Definition of 消息队列，优先级+FIFO, 支持多并发调用
  *       处理消息的线程这里不负责创建，由外部创建
  *       在此文档的后面，可以看到在线程中处理消息的例子代码。
  *@version 0.1
  *@author wanfenfen
  *@date Created on: 2016:6:11   21:38
  *@copyright Copyright(c) 2016 YunShen Technology. All rights reserved.
  * 
  */
#ifndef TD_THREADMSGQUEUE_H  //NOLINT
#define TD_THREADMSGQUEUE_H  //NOLINT

#ifdef _WIN32
/// windowss headers
  #include <Windows.h>  //NOLINT
  #include <assert.h>  //NOLINT
  #include <limits.h>  //NOLINT
#else
  //TODO:add for linux
  #include "event.h"
#endif

/// stl headers
#include <list>  //NOLINT
#include <map>   //NOLINT

/// boost headers
#include <boost/thread/mutex.hpp>  //NOLINT
#include <boost/thread/lock_guard.hpp>  //NOLINT
#include <boost/shared_ptr.hpp>  //NOLINT
#include <boost/shared_array.hpp>  //NOLINT
#include <boost/thread/thread.hpp>

/**
  *@brief 优先级的定义，值最大，优先级越低
  */
enum ThreadMsgPriority {
  ThreadMsgPriorityMax= 0,          ///< 最高优先级
  ThreadMsgPriorityCommon =5,       ///< 普通优先级
  ThreadMsgPriorityMin =10          ///< 最低优先级
};

/**
  *@brief 消息队列，优先级+FIFO, 支持多并发调用
  *       如果有消息进入队列，则事件event_thread()会被触发，外部可以通过此事件得到有消息了
  *       处理消息的线程这里不负责创建，由外部创建
  *       在此文档的后面，可以看到在线程中处理消息的例子代码。
  */
 //TODO:add for linux
template<typename UTMsgValue>
class ThreadMsgPriorityQueue {
protected:
  template<typename TMsgValue>
  class ThreadMsg {
  public:
    ThreadMsg() {
      umsg_ = 0;
      upriority_ = 0;
      //wparam_ = 0;
      //lparam_ = 0;
      ////char_buf_ptr_ = NULL;
      //itimeout_ = 10000;
      //pret_value_ = NULL;
      //iundefin1_ = 0;

      umsg_id_ =0;
      hsynchronize_handle_ = NULL;
    }

    ~ThreadMsg() {
    }
    inline unsigned int  msg_id() const {
      return umsg_id_;
    }

    bool operator ==(const ThreadMsg<TMsgValue>& proOther) const {
      return umsg_id_ == proOther.umsg_id_?true:false;
    }
    inline void* synchronize_handle() const {
      return hsynchronize_handle_;
    }
    // public:  /// 为了方便，以下成员变量设为公有
    unsigned int umsg_;
    unsigned int upriority_;  // priority[0 -100], 0 biggst priority,100 min prority
    //int itimeout_;  // time out milliseconds; //default 10 second;
    //void* pret_value_;

    TMsgValue msg_value_;
  private:
    //template<typename  TMsgValue>
    //friend class ThreadMsgPriorityQueue;

    unsigned int umsg_id_;
    void* hsynchronize_handle_;
  };


 public:
#ifdef _WIN32
  explicit ThreadMsgPriorityQueue() {
    umax_msg_id_ =0;
    hevent_thread_ = CreateEvent(NULL, false, false, NULL);
    pnext_thread_msgqueue_ = NULL;
  }
  inline void* event_thread() const {
    return hevent_thread_;
  }
#else
    //TODO:add for linux
    explicit ThreadMsgPriorityQueue() {
    umax_msg_id_ =0;
    hevent_thread_ = event_create(false, false);//replace old process
    pnext_thread_msgqueue_ = NULL;
  }
  inline event_handle event_thread() const {
    return hevent_thread_;
  }
#endif
  // inline void set_event_thread(void* proEventHandle){hevent_thread_ =proEventHandle;}

  /**
   *@brief 放入消除队列
   *@param msg_type[Input] 消息类型
   *@param msg_priority[Input] 消息的优先级，值范围是[0,10] 值越大，优先级越低，最高优先级是0，可以参考枚举ThreadMsgPriority
   *                           默认可以使用枚举：ThreadMsgPriorityMax，ThreadMsgPriorityCommon，ThreadMsgPriorityMin
   *                           
   *@param pmsg_value[Input] 需要放入消息队列的消息值
   *@return   
   */
  unsigned int PushMsg(const unsigned int msg_type, const int msg_priority, UTMsgValue* pmsg_value, const bool is_synchronize = false/*,const int timeout_millseconds =0, void* pret_value = NULL*/) {

    assert(pmsg_value);

    if(NULL == pmsg_value)
      return 0;

    ThreadMsg<UTMsgValue> tmsg;
    tmsg.umsg_ = msg_type;
    tmsg.upriority_ = msg_priority;
    tmsg.msg_value_ = *pmsg_value;
    //tmsg.itimeout_ = timeout_millseconds;
    //tmsg.pret_value_ = pret_value;
    return PushMsg(tmsg);
  }

  bool PopMsg(unsigned int& msg_type, UTMsgValue& pThreadMsg) {
    ThreadMsg<UTMsgValue> tmsg;
    if(PopMsg(&tmsg)) {
      msg_type = tmsg.umsg_;
      pThreadMsg = tmsg.msg_value_;
      return true;
    }else {
      return false;
    }
  }
  
  bool RemoveMsgAccordMessageType(const unsigned int iMsgType, const unsigned int iMaxMessageID =0) {
    boost::lock_guard<boost::mutex> lock_guard_member(mutex_member_);

    //std::list<ThreadMsg<UTMsgValue>>::iterator itsearch = list_msg_queue_.begin();//add for linux
    auto itsearch = list_msg_queue_.begin();//add for linux
    for (itsearch; itsearch != list_msg_queue_.end();) {      
      if (iMsgType == (*itsearch).umsg_&& (0 == iMaxMessageID || (*itsearch).msg_id() < iMaxMessageID ) ){
        itsearch = list_msg_queue_.erase(itsearch);
        break;
        //continue;  ///< continue for
      }
      ++itsearch;
    }

    return 0;
  }
  bool RemoveMsgAccordMessageID(const unsigned int iMsgID) {
    boost::lock_guard<boost::mutex> lock_guard_member(mutex_member_);
    printf("RemoveMsgAccordMessageID:%d\n", iMsgID);

    //std::list<ThreadMsg<UTMsgValue>>::iterator itsearch = list_msg_queue_.begin(); //add for linux
    auto itsearch = list_msg_queue_.begin(); //add for linux
    for (itsearch; itsearch != list_msg_queue_.end();) {
      if (iMsgID == (*itsearch).msg_id()) {
        itsearch = list_msg_queue_.erase(itsearch);  ///< only delete once
        break;
        // continue;  ///< continue for
      }
       ++itsearch;
    }

    return 0;
  }
  bool RemoveMsgAll() {
    boost::lock_guard<boost::mutex> lock_guard_member(mutex_member_);
    printf("RemoveMsgAll... \n");
    list_msg_queue_.clear();
    return 0;
  }
  /**
   *@brief 定义消息之间的一种killer,killed的映射关系，如果在执行PushMsg的时候，消息是一个Killer,
   *       则消息队列里的 killed类型的消息会被删除。
   *       如果killer与killed是同一种消息类型，则相当于消息队列中同一时刻此种消息类型只允许存在一个或没有
   *@param proKillerMsgType 主消息类型
   *@param proKilledMsgType 被杀的消息类型
   *@return
   */
  bool AddMsgKillMap(unsigned int iKillerMsgType, unsigned int iKilledMsgType) {
    boost::lock_guard<boost::mutex> lock_guard_memeber(mutex_member_);
    //  python hint:Omit template arguments from make_pair OR use pair directly OR if appropriate, construct a pair directly  [build/explicit_make_pair] [4]  //NOLINT
    //mulmap_msg_kill_.insert(std::make_pair<unsigned int, unsigned int>(iKillerMsgType, iKilledMsgType));
    mulmap_msg_kill_.insert(std::make_pair(iKillerMsgType, iKilledMsgType));//add for linux
    return true;
  }
  bool KillMsgSlave(unsigned int iKillerMsgType, const unsigned int iMaxMessageID =0) {
    boost::lock_guard<boost::mutex> lock_guard_memeber(mutex_member_);

    /// kill msg
    MsgKillIt itbegin = mulmap_msg_kill_.lower_bound(iKillerMsgType);
    MsgKillIt itend = mulmap_msg_kill_.upper_bound(iKillerMsgType);
    for (itbegin; itbegin != itend; ++itbegin) {
      unsigned int killed_msg = itbegin->second;

      //std::list<ThreadMsg<UTMsgValue>>::iterator itsearch = list_msg_queue_.begin(); //add for linux
      auto itsearch = list_msg_queue_.begin(); //add for linux
      for (itsearch; itsearch != list_msg_queue_.end();) {
        if (killed_msg == (*itsearch).umsg_ && (0 == iMaxMessageID || (*itsearch).msg_id() < iMaxMessageID )) {
          itsearch = list_msg_queue_.erase(itsearch);
          continue;  ///< continue for
        }
        ++itsearch;
      }
    }
    return true;
  }

  inline unsigned int GetMsgCount() {
    boost::lock_guard<boost::mutex> lock_guard_member(mutex_member_);
    unsigned int ucount =list_msg_queue_.size();
    return ucount;
  }
  inline void Setnext_thread_msgqueue(ThreadMsgPriorityQueue* proNextMsgQueue) {
    boost::lock_guard<boost::mutex> lock_guard_member(mutex_member_);

    assert(proNextMsgQueue);
    pnext_thread_msgqueue_ = proNextMsgQueue;
  }
  inline ThreadMsgPriorityQueue* next_thread_queue() {
    return pnext_thread_msgqueue_;
  }

protected:
  unsigned int PushMsg(ThreadMsg<UTMsgValue>& ThreadMessage) {
  //  KillMsgSlave(ThreadMessage.umsg_);
   // g_Log.Trace_Print(TraceLogLevel_DEBUG, __FILE__, __LINE__, "in PushMsgex");
    /*  if (NULL != ThreadMessage.hsynchronize_handle_ && bNeedReCreateEvent)
    ThreadMessage.hsynchronize_handle_ = CreateEvent(NULL, NULL, FALSE, NULL);*/

    {  ///< alert this blace can not deleted;
      boost::lock_guard<boost::mutex> lock_guard_member(mutex_member_);

      list_msg_queue_.push_back(ThreadMessage);
#ifdef _WIN32
      SetEvent(hevent_thread_);
#else
      //TODO:add for linux
      //set event handleid
      event_set(hevent_thread_);
#endif
    }
    //if (ThreadMessage.hsynchronize_handle_) {
    //  std::cerr<<"is hsynchronize_handle_" <<std::endl;
    //  //g_Log.Trace_Print(TraceLogLevel_DEBUG, __FILE__, __LINE__, "is hsynchronize_handle_");
    //  /*
    //  WaitForSingleObject @param dwMilliseconds
    //  The time-out interval, in milliseconds.
    //  If a nonzero value is specified, the function waits until the object is signaled or the interval elapses.
    //  If dwMilliseconds is zero, the function does not enter a wait state if the object is not signaled;
    //  it always returns immediately. If dwMilliseconds is INFINITE, the function will return only when the object is signaled.
    //  注意，如果主线程中调用WaitForSingleObject，子线程中弹出了模式窗口（弹出模式窗口后才会setEvent)，
    //  则子线程中的模式窗口会被阻塞（响应不了）直到当前主线程中的WaitForSingleObject超时返回后，子线程中的模式窗口
    //  才会有响应。
    //  */
    //  DWORD dwwait_ret = WaitForSingleObject(ThreadMessage.hsynchronize_handle_, ThreadMessage.itimeout_);  ///< maxtime
    //  if (WAIT_OBJECT_0 != dwwait_ret) {
    //    //g_Log.Trace_Print(TraceLogLevel_DEBUG, __FILE__, __LINE__, "WAIT_OBJECT_0 != dwwait_ret");
    //    // assert(FALSE);
    //    // printf("WaitForSingleObject TIMEOUT\n");
    //    RemoveMsgAccordMessageID(ThreadMessage.msg_id());  ///< delete from msg queue
    //    //g_Log.Trace_Print(TraceLogLevel_DEBUG, __FILE__, __LINE__, "WAIT_OBJECT_0 != dwwait_ret end");
    //    return 0;
    //  }
    //}
    //g_Log.Trace_Print(TraceLogLevel_DEBUG, __FILE__, __LINE__, "PushMsg end");
    return ThreadMessage.msg_id();
  }

  /**
   *@brief pop a meesage from queue // NOLINT
   *@param pThreadMsg[Output] 用于接收pop的消息
   *@return success return true
            fail return false
   */
  bool PopMsg(ThreadMsg<UTMsgValue>* pThreadMsg) {  ///< fifo + priority
    if (list_msg_queue_.size() == 0)
      return false;
 
    assert(NULL != pThreadMsg);

    boost::lock_guard<boost::mutex> lock_gurad_memeber(mutex_member_);

    if (list_msg_queue_.size() > 1)  
    {
      ///< only msg queue size bigger than 1, SetEvent can math the WaitForSingleObjectd
#ifdef _WIN32
      SetEvent(hevent_thread_);  ///< set event if have msg;
#else
      //TODO: add for linux   --///< set event if have msg;
      int ret = event_set(hevent_thread_);
      printf("event_set ----> ret = [%d] !!! \n", ret);
#endif
    }
    /// get the max proirty msg
    unsigned int umax_prority = ThreadMsgPriorityMin+1;
    //std::list<ThreadMsg<UTMsgValue>>::iterator itfind = list_msg_queue_.begin(); //add for linux
    //std::list<ThreadMsg<UTMsgValue>>::iterator it1 = list_msg_queue_.begin(); //add for linux
    auto itfind = list_msg_queue_.begin(); //add for linux
    auto it1 = list_msg_queue_.begin(); //add for linux

    for (it1; it1 != list_msg_queue_.end(); ++it1) {
      if ((*it1).upriority_ < umax_prority) {
        umax_prority = (*it1).upriority_;
        itfind = it1;
      }
    }
   
    assert(itfind != list_msg_queue_.end());

    // recvMsg = (*itfind).umsg;
    // recvPriority = (*itfind).upriority;
    if (NULL != pThreadMsg)
      *pThreadMsg = *itfind;
    list_msg_queue_.remove(*itfind);
    return true;
  }

private:
#ifdef _WIN32
  void* hevent_thread_;
#else
  event_handle hevent_thread_;
#endif  
  std::list<ThreadMsg<UTMsgValue>> list_msg_queue_;  ///< accord priority
  unsigned int umax_msg_id_;
  boost::mutex mutex_member_;
  ThreadMsgPriorityQueue* pnext_thread_msgqueue_;  ///< push the msg to the next queue when current is finish
  std::multimap<unsigned int, unsigned int> mulmap_msg_kill_;  ///< 一个消息的处理会kill掉其他的消息
  typedef std::multimap<unsigned int, unsigned int>::iterator MsgKillIt;
};

//typedef void (*ProcessMsgFun)(ThreadMsgPriorityQueue* pThreadMsgQueue, ThreadMsg ThreadMessage);
//HANDLE CreateMsgThread(ThreadMsgPriorityQueue* pThreadMsgQueue, ProcessMsgFun pProcessMsgFun);

#endif //TD_THREADMSGQUEUE_H  //NOLINT

/*
消息队列的使用 例子


class MyMsgValue {
public:
  int v1;
  int v2;
};

int _tmain(int argc, _TCHAR* argv[])
{
  MyMsgValue msg;
  msg.v1 =1;
  msg.v2 =2;

  ThreadMsgPriorityQueue<MyMsgValue> queue;
  queue.PushMsg(MSG_MYTEST, 0, &msg, false);

  // create thread to process

	return 0;
}

int ThreadFun() {
  while(0 == asynchronize_stop_flag_) {
    DWORD dwWaitRet = WaitForSingleObject(pthread_msg_queue->event_thread(), 1000);
    if (WAIT_OBJECT_0 != dwWaitRet)
      continue;

    /// peek msg
    int msg_type =-1;
    MyMsgValue msg_value;
    bool bhave_msg = pthread_msg_queue->PopMsg(msg_type, msg_value);
    if (false == bhave_msg)
      continue;  ///< continue while;

    switch(msg_type) {
    case MSG_MYTEST:
      {
        // TO SOMETHIG
      }
      break;
    }
  }
}


*/