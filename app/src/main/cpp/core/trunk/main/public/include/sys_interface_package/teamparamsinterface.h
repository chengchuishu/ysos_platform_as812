/**
 *@file teamparamsinterface.h
 *@brief Definition of TeamParamsInterface
 *@version 0.1
 *@author Steven.Shi
 *@date Created on: 2016-04-13 08:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef TEAMPARAMS_INTERFACE_H                                                  //NOLINT
#define TEAMPARAMS_INTERFACE_H                                                  //NOLINT

#include "../../../public/include/sys_interface_package/common.h"
#include "../../../public/include/sys_interface_package/bufferpool.h"
#include "../../../public/include/sys_interface_package/baseinterface.h"
#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/variant.hpp>

namespace ysos {

class BufferWrapInterface;

typedef boost::variant<
    bool, \
    int8_t, int16_t, int32_t, int64_t, \
    uint8_t, uint16_t, uint32_t, uint64_t, \
    float, double, \
    std::string> \
  default_variant_t;

/**
 *@brief 通用参数组接口.                                                           //NOLINT
 *存储参数的variant类型可以自定义, 默认为default_variant_t                            //NOLINT
 */
template <typename VariantType = default_variant_t>
class TeamParamsInterface : virtual public BufferWrapInterface {
  IMPLEMENT_PUBLIC_EMPTY_DESTRUCTOR(TeamParamsInterface)
 public:
  typedef VariantType variant_t;
  //  typedef shared_ptr<TeamParamsInterface<variant_t> > TeamParamsInterfacePtr;

  /**
   *@brief 根据指定的Key设置指定参数项, value_ptr若为空则删除可能存在的对应项, 否则,       //NOLINT
   对应项若此前不存在则设置为value_ptr所指, 否则更新为value_ptr所指                      //NOLINT
   若value_ptr为空且item_id == 0xffffffff且section_id == 0xffffffff则删除所有项     //NOLINT
   *@param [in]item_id: Key的高32位                                              //NOLINT
   *@param [in]section_id: Key的低32位                                           //NOLINT
   *@param [in]value_ptr: 若为空则试图删除,否则试图插入或更新                          //NOLINT
   *@return: 若成功则返回YSOS_ERROR_SUCCESS,否则返回非YSOS_ERROR_SUCCESS            // NOLINT
  */
  virtual int WriteItem(
    uint32_t item_id, uint32_t section_id, void *value_ptr) = 0;

  /**
   *@brief 根据指定的Key设置读取参数项                                               //NOLINT
   *@param [in]item_id: Key的高32位                                              //NOLINT
   *@param [in]section_id: Key的低32位                                           //NOLINT
   *@param [out]value_ptr: 若非空则试图将其所指置为读取到参数项的值                     //NOLINT
   *@return: 若成功则返回YSOS_ERROR_SUCCESS,否则返回非YSOS_ERROR_SUCCESS            // NOLINT
  */
  virtual int ReadItem(
    uint32_t item_id, uint32_t section_id, void *value_ptr) = 0;

  /**
   *@brief 根据指定的Key查找参数项                                                  //NOLINT
   *@param [in]item_id: Key的高32位                                              //NOLINT
   *@param [in]section_id: Key的低32位                                           //NOLINT
   *@return: 若指定项存在则返回YSOS_ERROR_SUCCESS,否则返回非YSOS_ERROR_SUCCESS        // NOLINT
  */
  virtual int FindItem(
    uint32_t item_id, uint32_t section_id) = 0;

  /**
   *@brief 复制                                                                  //NOLINT
   *@param [in]source_team_params_ptr: 源TeamParamsInterface*                   //NOLINT
   *@return: 若成功则返回YSOS_ERROR_SUCCESS,否则返回非YSOS_ERROR_SUCCESS            // NOLINT
   */
  virtual int CopyFrom(void* source_team_params_ptr) = 0;
};

}   // namespace ysos

#endif  //  TEAMPARAMS_INTERFACE_H                                              //NOLINT
