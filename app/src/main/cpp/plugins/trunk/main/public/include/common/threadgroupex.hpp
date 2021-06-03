/**   //NOLINT
  *@file threadgroupex.h
  *@brief Definition of boost 线程组扩展，把boost1.59版本中的threadgroup拷过来了。重写这个类
  *       由于原先的线程组
  *@version 0.1
  *@author wanfenfen
  *@date Created on: 2017:5:13   14:36
  *@copyright Copyright(c) 2016 YunShen Technology. All rights reserved.
  * 
  *@howto Usage:
  *@todo
  */

#ifndef BOOST_THREAD_DETAIL_THREAD_GROUP_EX_HPP
#define BOOST_THREAD_DETAIL_THREAD_GROUP_EX_HPP
#pragma once

#include <list>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/lock_guard.hpp>

#include <boost/config/abi_prefix.hpp>

#ifdef BOOST_MSVC
#pragma warning(push)
#pragma warning(disable:4251)
#endif

namespace boostex
{
  using namespace boost;
  class thread_group /*: public boost::thread::thread_group*/
  {
  private:
    thread_group(thread_group const&);
    thread_group& operator=(thread_group const&);
  public:
    thread_group() {}
    ~thread_group()
    {
      for(std::list<thread*>::iterator it=threads.begin(),end=threads.end();
        it!=end;
        ++it)
      {
        delete *it;
      }
    }

    bool is_this_thread_in()
    {
      thread::id id = this_thread::get_id();
      boost::shared_lock<shared_mutex> guard(m);
      for(std::list<thread*>::iterator it=threads.begin(),end=threads.end();
        it!=end;
        ++it)
      {
        if ((*it)->get_id() == id)
          return true;
      }
      return false;
    }

    bool is_thread_in(thread* thrd)
    {
      if(thrd)
      {
        thread::id id = thrd->get_id();
        boost::shared_lock<shared_mutex> guard(m);
        for(std::list<thread*>::iterator it=threads.begin(),end=threads.end();
          it!=end;
          ++it)
        {
          if ((*it)->get_id() == id)
            return true;
        }
        return false;
      }
      else
      {
        return false;
      }
    }

    template<typename F>
    thread* create_thread(F threadfunc)
    {
      boost::lock_guard<shared_mutex> guard(m);
      //std::auto_ptr<thread> new_thread(new thread(threadfunc));
      std::unique_ptr<thread> new_thread(new thread(threadfunc)); //add for linux    /*std::auto_ptr已弃用*/
      threads.push_back(new_thread.get());
      return new_thread.release();
    }

    //void add_thread(thread* thrd)
    //{
    //  if(thrd)
    //  {
    //    BOOST_THREAD_ASSERT_PRECONDITION( ! is_thread_in(thrd) ,
    //      thread_resource_error(static_cast<int>(system::errc::resource_deadlock_would_occur), "boost::thread_group: trying to add a duplicated thread")
    //      );

    //    boost::lock_guard<shared_mutex> guard(m);
    //    threads.push_back(thrd);
    //  }
    //}

    void remove_thread(thread* thrd)  ///< 原先的remove_thread 与 add_thread 成对（而不是create_thread)，所以不会删除线程
    {
      boost::lock_guard<shared_mutex> guard(m);
      std::list<thread*>::iterator const it=std::find(threads.begin(),threads.end(),thrd);
      if(it!=threads.end())
      {
        delete *it;  ///< 这里得删除一下，Wff add 20170619 原先这里没有这个
        threads.erase(it);
      }
    }

    void join_all()
    {
      BOOST_THREAD_ASSERT_PRECONDITION( ! is_this_thread_in() ,
        thread_resource_error(static_cast<int>(system::errc::resource_deadlock_would_occur), "boost::thread_group: trying joining itself")
        );
      boost::shared_lock<shared_mutex> guard(m);

      for(std::list<thread*>::iterator it=threads.begin(),end=threads.end();
        it!=end;
        ++it)
      {
        if ((*it)->joinable())
          (*it)->join();
      }
    }

#if defined BOOST_THREAD_PROVIDES_INTERRUPTIONS
    void interrupt_all()
    {
      boost::shared_lock<shared_mutex> guard(m);

      for(std::list<thread*>::iterator it=threads.begin(),end=threads.end();
        it!=end;
        ++it)
      {
        (*it)->interrupt();
      }
    }
#endif

    size_t size() const
    {
      boost::shared_lock<shared_mutex> guard(m);
      return threads.size();
    }
  private:
    std::list<thread*> threads;
    mutable shared_mutex m;

    
    //////////////////////////////////////////////////////////////////////////
  public:
    // 以下是添加的函数
    /**
     *@brief 加入到线程已完成列表中，这函数只能是在线程的最后一句代码。
     *       注意，如果调用了线程的join函数（同时jsin成功），则此函数产生的效果无。
     *@param 
     *@return   
     */
    //void add_this_thread_to_done_list() {
    //  thread::id id = this_thread::get_id();
    //  boost::lock_guard<shared_mutex> guard(done_m);
    //  thread_have_done_.push_back(id);
    //}

    /**
     *@brief 删除已经完工的线程，并释放内存
     *       如果原先的线程被join过，则此方法会无效，被join过的纯种 thread::id 为0
     *@param 
     *@return   
     */
    //void remove_done_thread() {
    //  boost::lock_guard<shared_mutex> guard(done_m);
    //  boost::lock_guard<shared_mutex> guard2(m);

    //  std::list<thread::id>::iterator it = thread_have_done_.begin();
    //  for (it; it!= thread_have_done_.end(); ++it)
    //  {
    //    std::list<thread*>::iterator it2 = threads.begin();
    //    for (it2; it2 != threads.end(); ++it2)
    //    {
    //      thread* pthread = *it2;
    //      if(*it == pthread->get_id()) {
    //        threads.erase(it2);
    //        delete pthread;
    //        break;
    //      }
    //    }
    //  }
    //  thread_have_done_.clear();
    //}

    //void add_this_thread_to_done_remove_list(boost::thread* have_done_thread) {
    //  if(NULL == have_done_thread)
    //    return;

    //  boost::lock_guard<shared_mutex> guard(remove_m);
    //  threads_have_done_.push_back(have_done_thread);
    //}

    /// 注意 此函数中传入not_finish_thread 必须保证在调用代码中不再使用。
    /// 否则此线程会被去检测执行完同时被删除，如果调用代码中使用了这个线程，则会出错
    void add_this_thread_to_unfinish_remove_list(boost::thread* not_finish_thread) {
      if(NULL == not_finish_thread)
        return;

      boost::lock_guard<shared_mutex> guard(remove_m);
      threads_have_not_finish_.push_back(not_finish_thread);
    }

    /**
     *@brief 分两步执行，一是删除那些的确是执行完的。
     *       注意事件：此函数不能在原先的线程组中某个线程中执行，防止死锁
     *@param 
     *@return   
     */
    void remove_done_thread() {

      boost::lock_guard<shared_mutex> guard(remove_m);
      // 未执行完的线程
      {
        std::list<thread*>::iterator it = threads_have_not_finish_.begin();
        for (it; it != threads_have_not_finish_.end();)
        {
          boost::thread* ptemp_thread = *it;
          assert(ptemp_thread);

          
          if((false == ptemp_thread->joinable()) 
            || (ptemp_thread->joinable() && ptemp_thread->try_join_for(boost::chrono::milliseconds(0)))) {
            threads_have_done_.push_back(ptemp_thread); /// have finish
            it = threads_have_not_finish_.erase(it);
          }else {
            // not finish
            ++it;
          }
        }
      }

      // 执行完的线程
      {
        std::list<thread*>::iterator it = threads_have_done_.begin();
        for (it; it!= threads_have_done_.end(); ++it)
        {
          boost::thread* ptemp_thread = *it;
          assert(ptemp_thread);
          remove_thread(ptemp_thread);
        }
        threads_have_done_.clear();
      }

    }
   private:
     //mutable shared_mutex done_m;
     //std::list<thread::id> thread_have_done_;
     std::list<thread*> threads_have_done_;
     std::list<thread*> threads_have_not_finish_;
     mutable shared_mutex remove_m;
    //////////////////////////////////////////////////////////////////////////
  };
}

#ifdef BOOST_MSVC
#pragma warning(pop)
#endif

#include <boost/config/abi_suffix.hpp>

#endif
