/**
 *@file SingletonTemplate.h
 *@brief 单例基类
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-13 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef CHP_SINGLETON_H_     //NOLINT
#define CHP_SINGLETON_H_     //NOLINT

#include <memory>
/// boost headers //  NOLINT
#include <boost/shared_ptr.hpp>
#include <boost/thread/recursive_mutex.hpp>


namespace ysos {
// 
// /**
//  *@brief 通过Lock实现的单例基类
//  */
// template<typename T>
// class SingletonTemplate {
//  public:
//   /**
//    *@brief 静态函数，对外使用，如：BufferUtility::Instance()->GetMessageHead(message_ptr);  // NOLINT
//    *@return： 成功返回BufferUtility指针，失败返回NULL  // NOLINT
//    */
//   static const boost::shared_ptr<T> Instance(void);
// 
//  protected:
//   static boost::shared_ptr<T>   s_instance_;
//   static boost::shared_ptr<LightLock> s_lock_;
// };
// 
// template<typename T>
// boost::shared_ptr<T>  SingletonTemplate<T>::s_instance_ = NULL;
// 
// template<typename T>
// boost::shared_ptr<LightLock> SingletonTemplate<T>::s_lock_ = boost::shared_ptr<LightLock>(new LightLock());
// 
// template<typename T>
// const boost::shared_ptr<T> SingletonTemplate<T>::Instance(void) {
//   if (NULL == s_instance_) {
//     AutoLockOper lock(s_lock_.get());
//     if (NULL == s_instance_) {
//       s_instance_ = boost::shared_ptr<T>(new T());
//     }
//   }
// 
//   return s_instance_;
// }
// 
// 
// 
// 
// 
// /**
//  *@brief 通过Lock实现的单例基类
//  */
// template <typename T>
// class ISingleton {
//  public:
//   static T& GetInstance() {
//     static boost::mutex s_mutex;
//     if (s_instance.get() == NULL) {
//       boost::mutex::scoped_lock lock(s_mutex);
//       if (s_instance.get() == NULL) {
//         s_instance.reset(new T());
//       }
//       // 'lock' will be destructed now. 's_mutex' will be unlocked.
//     }
//     return *s_instance;
//   }
// 
//  protected:
//   ISingleton() { }
//   ~ISingleton() { }
// 
//   // Use auto_ptr to make sure that the allocated memory for instance
//   // will be released when program exits (after main() ends).
//   static std::auto_ptr<T> s_instance;
// 
//  private:
//   ISingleton(const ISingleton&);
//   ISingleton& operator =(const ISingleton&);
// };
// 
// template <typename T>
// std::auto_ptr<T> ISingleton<T>::s_instance;

/*********************************************************************
************  singleton macro                     ********************
**********************************************************************/
/**
 *@brief shared_ptr默认析构类函数
 */
class DeleteSingtonFunction {
 public:
  void operator()(void* object) {
    if(NULL != object) {
      /*delete object;*/ 
      free(object); // need update for linux
      }
  }
};

#define  DECLARE_SINGLETON_VARIABLE(CLASS_NAME) \
public:\
  static const boost::shared_ptr<CLASS_NAME> Instance(void);\
class Delete##CLASS_NAME##SingtonFunction {\
public:\
  void operator()(CLASS_NAME* object) {\
  if(NULL != object) {\
  delete object;\
  object = NULL; \
    }\
  }\
  };\
  friend class Delete##CLASS_NAME##SingtonFunction; \
private:\
  static boost::shared_ptr<CLASS_NAME>   s_instance_;\
  static boost::shared_ptr<boost::recursive_mutex> s_lock_;

#define  DEFINE_SINGLETON(CLASS_NAME) \
  boost::shared_ptr<CLASS_NAME> CLASS_NAME::s_instance_=NULL;\
  boost::shared_ptr<boost::recursive_mutex> CLASS_NAME::s_lock_ = boost::shared_ptr<boost::recursive_mutex>(new boost::recursive_mutex());\
  const boost::shared_ptr<CLASS_NAME> CLASS_NAME::Instance(void) {\
  if (NULL == s_instance_) {\
    s_lock_->lock();\
    if (NULL == s_instance_) {\
    Delete##CLASS_NAME##SingtonFunction delete_function; \
    s_instance_ = boost::shared_ptr<CLASS_NAME>(new CLASS_NAME(#CLASS_NAME), delete_function);\
    }\
    s_lock_->unlock();\
  }\
  return s_instance_;\
  }
}  // namespace ysos

#endif //  OHP_SINGLETON_H_  //NOLINT
