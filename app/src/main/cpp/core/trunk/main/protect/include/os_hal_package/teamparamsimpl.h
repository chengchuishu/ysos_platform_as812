/**
*@file TeamParamsImpl.h
*@brief Definition of TeamParamsImpl
*@date Created on: 2016-04-25 13:59:20
*@author Original author: XuLanyue
*@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
*@ 
*/

#ifndef TEAM_PARAMS_IMPL_H                                                  //NOLINT
#define TEAM_PARAMS_IMPL_H                                                  //NOLINT

#include "../../../protect/include/core_help_package/bufferwrapimpl.h"
#include "../../../public/include/os_hal_package/lock.h"
#include "../../../public/include/sys_interface_package/common.h"
#include "../../../public/include/sys_interface_package/teamparamsinterface.h"

#include <map>
#include <list>
#include <string>
#include <boost/variant.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include "../../../public/include/os_hal_package/config.h"

#if (CURRENT_PLATFORM == PLATFORM_WINDOWS)
#pragma warning(disable: 4250)      //  dominance
#endif

namespace ysos {

//need add for linux 
typedef boost::variant<
    bool, \
    int8_t, int16_t, int32_t, int64_t, \
    uint8_t, uint16_t, uint32_t, uint64_t, \
    float, double, \
    std::string> \
  default_variant_t;
//need add for linux 
template<typename VariantType = default_variant_t>    ////need update for linux 
class YSOS_EXPORT TeamParamsImpl :
  public TeamParamsInterface<VariantType>,
  public BufferWrapImpl {
  DISALLOW_COPY_AND_ASSIGN(TeamParamsImpl)
  DECLARE_CREATEINSTANCE(TeamParamsImpl)
 protected:
  TeamParamsImpl(const std::string &strClassName = "TeamParamsImpl") {
    // YSOS_LOG_DEBUG("TeamParamsImpl()");
  }
public:
  typedef VariantType variant_t;   //need add for linux 
  typedef std::list<uint64_t> TeamParamsKey;
  typedef std::map<uint64_t, variant_t> TeamParamsMap;
  virtual ~TeamParamsImpl() {
    team_params_keys_.clear();
    param_map_.clear();
    // YSOS_LOG_DEBUG("~TeamParamsImpl()");
  }

  virtual int WriteItem(uint32_t iItemID, uint32_t iSectionID, void *pValue) {
    int return_value = YSOS_ERROR_FAILED;
    uint64_t uKey = ((uint64_t)iSectionID << 32) + iItemID;
    {
      team_params_lock_.Lock();
      if (NULL == pValue &&
        iItemID == (uint32_t)0xffffffff &&
        iSectionID == (uint32_t)0xffffffff) {                               //  Delete All
          team_params_keys_.clear();
          param_map_.clear();
      }
      if (YSOS_ERROR_SUCCESS == FindItem_(iItemID, iSectionID, false)) {
        if (NULL != pValue) {                                               //  Update
          param_map_[uKey] = *(variant_t*)pValue;                           //NOLINT
        } else {
          typename TeamParamsMap::iterator itInner;                                  //  Delete     //need update for linux
          itInner = param_map_.find(uKey);
          param_map_.erase(itInner);

          std::list<uint64_t>::iterator it;
          for (it = team_params_keys_.begin();
            it != team_params_keys_.end();
            ++it) {
              if ((*it) == uKey) {
                team_params_keys_.erase(it);
                break;
              }
          }
        }
        return_value = YSOS_ERROR_SUCCESS;
      } else {
        if (NULL != pValue) {                                               //  Insert
#if 1
          param_map_[uKey] = *(variant_t*)pValue;                           //NOLINT
#else
          param_map_.insert(std::pair<uint64_t, variant_t>(uKey, *(variant_t*)pValue));
#endif
          team_params_keys_.push_back(uKey);
        }
        return_value = YSOS_ERROR_SUCCESS;
      }
      team_params_lock_.Unlock();
    }
    return return_value;
  }

  virtual int ReadItem(uint32_t iItemID, uint32_t iSectionID, void *pValue) {
    int return_value = YSOS_ERROR_FAILED;
    uint64_t uKey = ((uint64_t)iSectionID << 32) + iItemID;
    typename TeamParamsMap::iterator it;   //need update for linux
    if (NULL != pValue) {
      {
        team_params_lock_.Lock();
        it = param_map_.find(uKey);
        if (YSOS_ERROR_SUCCESS == FindItem_(iItemID, iSectionID, false)) {
          *(variant_t*)pValue = param_map_[uKey];                           //NOLINT
          return_value = YSOS_ERROR_SUCCESS;
        }
        team_params_lock_.Unlock();
      }
    }
    return return_value;
  }

  virtual int FindItem(uint32_t iItemID, uint32_t iSectionID) {
    return FindItem_(iItemID, iSectionID, true);
  }

  virtual int CopyFrom(
    void* source_team_params_ptr) {
    int return_value = YSOS_ERROR_FAILED;
    TeamParamsImpl<VariantType>* ptr
      //  = source_team_params_ptr; //  .get();
      = (TeamParamsImpl<VariantType>*)source_team_params_ptr;
    if (ptr) {
      this->WriteItem(0xffffffff, 0xffffffff, NULL);
      std::list<uint64_t>::iterator it;
      for (it = ptr->team_params_keys_.begin();
        it != ptr->team_params_keys_.end();
        ++it) {
        variant_t var;
        ptr->ReadItem(
          (uint32_t)(((*it) & (uint64_t)0xffffffff)),
          (uint32_t)((*it) >> 32),
          (void*)&var);
        this->WriteItem(
          (uint32_t)(((*it) & (uint64_t)0xffffffff)),
          (uint32_t)((*it) >> 32),
          (void*)&var);
      }
      return_value = YSOS_ERROR_SUCCESS;
    }
    return return_value;
  }

  virtual void Dump() {
    // YSOS_LOG_DEBUG("Dump the TeamParams:");
    std::list<uint64_t>::iterator it;
    for (it = team_params_keys_.begin();
      it != team_params_keys_.end();
      ++it) {
      variant_t var;
      ReadItem(
        (uint32_t)(((*it) & (uint64_t)0xffffffff)),
        (uint32_t)((*it) >> 32),
        (void*)&var);
      // YSOS_LOG_DEBUG("Item: " << (uint32_t)(((*it) & (uint64_t)0xffffffff)) << " Section: " << (uint32_t)((*it) >> 32) << " Ptr: "<< (void*)&var);
    }
  }

  TeamParamsKey team_params_keys_;
 private:
  //  这里的Key由SectionId和ItemId分别作为uint64_t的高32位和低32位拼接而成          //NOLINT
  TeamParamsMap param_map_;
  LightLock team_params_lock_;

  int FindItem_(uint32_t iItemID, uint32_t iSectionID, bool bLock) {
    int return_value = YSOS_ERROR_FAILED;
    uint64_t uKey = ((uint64_t)iSectionID << 32) + iItemID;
    typename TeamParamsMap::iterator it;   //need update for linux
    {
      if (bLock) {
        team_params_lock_.Lock();
      }
      it = param_map_.find(uKey);
      if (it != param_map_.end()) {
        return_value = YSOS_ERROR_SUCCESS;
      }
      if (bLock) {
        team_params_lock_.Unlock();
      }
    }
    return return_value;
  }
};
typedef TeamParamsImpl<default_variant_t> DefaultTeamParamsImpl;
typedef boost::shared_ptr<DefaultTeamParamsImpl> DefaultTeamParamsImplPtr;
}   // namespace ysos

#endif  // TEAM_PARAMS_IMPL_H                                               //NOLINT
