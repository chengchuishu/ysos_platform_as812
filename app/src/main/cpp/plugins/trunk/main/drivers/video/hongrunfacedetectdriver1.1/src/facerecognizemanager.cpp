/**   //NOLINT
  *@file FaceRecognizeManager.cpp
  *@brief Definition of
  *@version 0.1
  *@author wanfenfen
  *@date Created on: 2018:4:12   18:53
  *@copyright Copyright(c) 2016 YunShen Technology. All rights reserved.
  * 
  *@howto Usage:
  *@todo
  */


/// self headers
#include "../include/facerecognizemanager.h"

/// ysos headers
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"

/// private headers
#include "../include/common/mycommondefine.h"
#include "../include/face_module_config.h"



namespace ysos {
    FaceRecognizeManager::FaceRecognizeManager() {
        face_recognize_ptr_ = NULL;
        config_use_strategy_1n_ =2; //1:为有模糊人脸上送模糊人脸 2：只上送详细人脸信息
        config_face_recog_return_sensitive_message_ =0;
        config_min_spacing_exclusive_similar_ = (float)0.1;
        config_call1n_max_times_ = 1;
        data_ptr_ = NULL;
    }

    FaceRecognizeManager::~FaceRecognizeManager() {
        boost::lock_guard<boost::recursive_mutex> guard(mutex_public_);
    }

    void FaceRecognizeManager::Init(FaceComparisonInterfacePtr face_recognize_ptr) {
        boost::lock_guard<boost::recursive_mutex> guard(mutex_public_);
        assert(face_recognize_ptr);
        face_recognize_ptr_ = face_recognize_ptr;
        data_ptr_ = GetDataInterfaceManager()->GetData(GLOBAL_DATA_KEY);
        if (!data_ptr_) {
            //YSOS_LOG_ERROR("get data_ptr failed");
            assert(data_ptr_ && "get data_ptr failed");
        }
        return;
    }
    void FaceRecognizeManager::UnInit() {
        boost::lock_guard<boost::recursive_mutex> guard(mutex_public_);
        face_recognize_ptr_ = NULL;
        data_ptr_ = NULL;
    }

    int  FaceRecognizeManager::FaceRecognition(const std::vector<TDFaceDetectInfoPtr> &arr_faces, std::vector<TDFaceDetectUnit> &arr_faces_unit) {
        boost::lock_guard<boost::recursive_mutex> guard(mutex_public_);
        assert(face_recognize_ptr_);
        if(NULL == face_recognize_ptr_) {
            YSOS_LOG_ERROR_CUSTOM(MY_LOGGER, "error, face recognize objec is null");
            return YSOS_ERROR_FAILED;
        }
        {
            //清理一下线程
            thread_group_face_server_.remove_done_thread();
        }
        int fun_ret = YSOS_ERROR_FAILED;
        assert(data_ptr_ && "data ptr cant be null");
        if(NULL == data_ptr_) {
            YSOS_LOG_ERROR_CUSTOM(MY_LOGGER, "face recognition, global data ptr is null");
            return YSOS_ERROR_FAILED;
        }
        //const std::string terminal_id = FaceModuleConfig::GetInstance()->GetTermID();
        //const std::string org_id = FaceModuleConfig::GetInstance()->GetOrgID();
        std::string terminal_id = "00000001";
        std::string org_id = "000000006" ;
        //data_ptr_->GetData("orgId", org_id);
        //data_ptr_->GetData("termId", terminal_id);
        //assert(terminal_id.length() >0 && org_id.length() >0 && "FaceRecognition terminalid, orgid cant be null");
        if(terminal_id.length() <=0 || org_id.length() <=0) {
            YSOS_LOG_ERROR_CUSTOM(MY_LOGGER, "FaceRecognition terminalid, orgid is null");
            return YSOS_ERROR_FAILED;
        }
        const std::vector<TDFaceDetectInfoPtr> &arr_faces_after_filter = arr_faces;
        bool have_change = false;
        static std::vector<TDFaceDetectUnit> t_vec_lasttime_face_unit;
        // 检索本地是否有缓存的人脸详细信息,如果没有，则创建线程去处理
        // int count_have_cache_person_info =0;
        std::vector<TDFaceDetectUnit> arr_face_detect_unit;
        std::vector<int> vec_need_to_get1n_info_index;
        for (unsigned int i= 0; i<arr_faces_after_filter.size(); ++i) {
            TDFaceDetectUnit face_unit;
            face_unit.face_detect_info_ptr_ = arr_faces_after_filter[i];
            face_unit.have_perfect_face_ =0;
            int get_cache_ret =GetCachePeopleInfoFromLocal(arr_faces_after_filter[i]->trackid, &face_unit, NULL, NULL, NULL);
            bool retry_face_detail_info = false;
            if (YSOS_ERROR_SUCCESS == get_cache_ret) {
                //更新位置信息，人脸质量信息（如果有更高人脸质量）
                TDFaceDetectInfoPtr old_detect_ptr = face_unit.face_detect_info_ptr_;
                if ((arr_faces_after_filter[i]->face_quality_score - old_detect_ptr->face_quality_score) >0.05 ///< 相差比较大的分数时才更新，防止频繁更新
                        && (arr_faces_after_filter[i]->vec_face_feature.size() >0 || arr_faces_after_filter[i]->img_base64_data.length() >0)) {
                    //old_detect_ptr->face_feature = arr_faces_after_filter[i]->face_feature;
                    old_detect_ptr->vec_face_feature = arr_faces_after_filter[i]->vec_face_feature;
                    old_detect_ptr->vec_face_prob_feature = arr_faces_after_filter[i]->vec_face_prob_feature;
                    old_detect_ptr->img_base64_data = arr_faces_after_filter[i]->img_base64_data;
                    old_detect_ptr->age = arr_faces_after_filter[i]->age;
                    old_detect_ptr->gender = arr_faces_after_filter[i]->gender;
                    old_detect_ptr->face_quality_score = arr_faces_after_filter[i]->face_quality_score;
                    // todo 如果原先人脸质量小，则比较出来的人脸分数低，后检测到更高人脸质量时应该重新再去比对一次
                    {
                        if (-1 == face_unit.have_perfect_face_) {
                            //face_unit.have_perfect_face_ =0;
                            retry_face_detail_info = true;
                            YSOS_LOG_INFO_CUSTOM(MY_LOGGER, "检测到更高人脸质量，重新再去比对一次， trackid:" <<  old_detect_ptr->trackid);
                        }
                    }
                }
                if(-1 == face_unit.have_perfect_face_ && false == retry_face_detail_info
                        && (arr_faces_after_filter[i]->vec_face_feature.size() >0 || arr_faces_after_filter[i]->img_base64_data.length() >0)) {
                    int have_retry_time = 0;
                    std::map<int, int>::iterator it_find = map_trackid_call1n_retry_times_.find(arr_faces_after_filter[i]->trackid);
                    if(it_find != map_trackid_call1n_retry_times_.end()) {
                        have_retry_time = it_find->second;
                    }
                    if(have_retry_time <= config_call1n_max_times_) {
                        ++have_retry_time;
                        map_trackid_call1n_retry_times_[arr_faces_after_filter[i]->trackid] = have_retry_time;
                        YSOS_LOG_INFO_CUSTOM(MY_LOGGER, "上一次没检测到人脸，再次去检测人脸详细信息，当前重发次数:" << have_retry_time);
                        retry_face_detail_info = true;
                    }
                    else {
                        //不再获取，已超过最高重发次数
                    }
                }
                /// 再次创建线程去获取去获取
                if(retry_face_detail_info) {
                    face_unit.have_perfect_face_ = 5;  ///< 再次去获取
                    YSOS_LOG_INFO_CUSTOM(MY_LOGGER, "重新去比对，trackid:" << arr_faces_after_filter[i]->trackid);
                    TDFaceDetectInfoPtr new_detect_ptr(new TDFaceDetectInfo);
                    if (new_detect_ptr) {
                        *new_detect_ptr = *arr_faces_after_filter[i];
                    }
                    boost::thread *temp_thread =thread_group_face_server_.create_thread(boost::bind(&FaceRecognizeManager::ThreadGetFace1nAndCache, this, terminal_id, org_id, new_detect_ptr));
                    thread_group_face_server_.add_this_thread_to_unfinish_remove_list(temp_thread);
                }
                old_detect_ptr->horizontal_angle = arr_faces_after_filter[i]->horizontal_angle;
                old_detect_ptr->vertical_angle = arr_faces_after_filter[i]->vertical_angle;
                old_detect_ptr->horizontal_ratio = arr_faces_after_filter[i]->horizontal_ratio;
                old_detect_ptr->vertical_ratio = arr_faces_after_filter[i]->vertical_ratio;
                old_detect_ptr->estimate_distance = arr_faces_after_filter[i]->estimate_distance;
                TDFaceDetectInfo *pdinfo = old_detect_ptr.get();
                AddOrUpdateToLocalPeople(old_detect_ptr->trackid, NULL, &pdinfo, /*NULL*/&face_unit.have_perfect_face_, NULL);
            }
            else {
                // not have cache
                AddOrUpdateToLocalPeople(face_unit.face_detect_info_ptr_->trackid, &face_unit, NULL, NULL, NULL);
            }
            if (0 == face_unit.have_perfect_face_) {
                if (arr_faces_after_filter[i]->face_quality_score < /*config_face_recog_min_face_quality_*/0.8 || (arr_faces_after_filter[i]->img_base64_data.length() <=0)) {
                    //face_unit.have_perfect_face_ =0;  ///< not create thread to check; check next time;
                    //continue;
                }
                else {
                    vec_need_to_get1n_info_index.push_back(i);
                }
            }
            arr_face_detect_unit.push_back(face_unit);
        }
        boost::this_thread::interruption_point();
        if(vec_need_to_get1n_info_index.size() >0 && /*config_param.callback_1n_person*/true && face_recognize_ptr_ && false == face_recognize_ptr_->IsSupportMulPersionOneTime1N()) { ///< 一次一个人查询1比n
            std::vector<FaceComparisonResult1NPtr> vec_comparisonresult;
            if (1 == config_use_strategy_1n_) { ///< 先抛出模糊人脸，同时查询详细人脸信息
                // 本地缓存没有，创建线程去处理
                std::vector<int> vec_trackid;
                for (unsigned int i =0; i< vec_need_to_get1n_info_index.size(); ++i) {
                    TDFaceDetectInfoPtr new_detect_ptr(new TDFaceDetectInfo);
                    if (new_detect_ptr) {
                        *new_detect_ptr = *(arr_face_detect_unit[vec_need_to_get1n_info_index[i]].face_detect_info_ptr_);
                    }
                    // 本地缓存没有，创建线程去处理
                    boost::thread *temp_thread =thread_group_face_server_.create_thread(boost::bind(&FaceRecognizeManager::ThreadGetFace1nAndCache, this, terminal_id, org_id, new_detect_ptr));
                    thread_group_face_server_.add_this_thread_to_unfinish_remove_list(temp_thread);
                    int have_perfect_flag =5;
                    AddOrUpdateToLocalPeople(arr_face_detect_unit[vec_need_to_get1n_info_index[i]].face_detect_info_ptr_->trackid, NULL, NULL, &have_perfect_flag, NULL);
                }
            }
            else if (2 == config_use_strategy_1n_) {   ///< 获取详细人脸信息再向上抛出
                std::string error_message;
                std::vector<boost::thread *> vec_thread_1n;
                for (unsigned int i =0; i< vec_need_to_get1n_info_index.size(); ++i) {
                    TDFaceDetectInfoPtr new_detect_ptr(new TDFaceDetectInfo);
                    if (new_detect_ptr) {
                        *new_detect_ptr = *(arr_face_detect_unit[vec_need_to_get1n_info_index[i]].face_detect_info_ptr_);
                    }
                    // 本地缓存没有，创建线程去处理
                    boost::thread *temp_thread =thread_group_face_server_.create_thread(boost::bind(&FaceRecognizeManager::ThreadGetFace1nAndCache, this, terminal_id, org_id, new_detect_ptr));
                    vec_thread_1n.push_back(temp_thread);
                    //thread_group_face_server_.add_this_thread_to_unfinish_remove_list(temp_thread);
                }
                for (unsigned int i=0; i< vec_need_to_get1n_info_index.size(); ++i) {
                    if (NULL == vec_thread_1n[i]) {
                        continue;
                    }
                    if (vec_thread_1n[i]->joinable()) {
                        vec_thread_1n[i]->join();
                    }
                    thread_group_face_server_.remove_thread(vec_thread_1n[i]);
                }
                for (unsigned int i=0; i<vec_need_to_get1n_info_index.size(); ++i) {
                    int index = vec_need_to_get1n_info_index[i];
                    int check_ret = GetCachePeopleInfoFromLocal(arr_face_detect_unit[index].face_detect_info_ptr_->trackid, NULL, NULL,
                                    &arr_face_detect_unit[index].have_perfect_face_, &arr_face_detect_unit[index].perfect_face_1n_ptr_);
                    assert(YSOS_ERROR_SUCCESS == check_ret);
                }
            }
            else {
                assert(false);
                YSOS_LOG_ERROR_CUSTOM(MY_LOGGER, "error, not support use stragegy, config_use_strategy_1n_:" << config_use_strategy_1n_);
                return YSOS_ERROR_FAILED;
            }
        }
        else if(vec_need_to_get1n_info_index.size() >0 &&/* config_param.callback_1n_person*/true && face_recognize_ptr_ && face_recognize_ptr_->IsSupportMulPersionOneTime1N()) {  ///< 1次多个人脸查询
            std::vector<std::string> vec_face_imagebase64_;
            //std::vector<int> real_getimage_index_;
            std::vector<int> vec_face_trackid_;
            for (unsigned int i=0; i< vec_need_to_get1n_info_index.size(); ++i) {
                int index = vec_need_to_get1n_info_index[i];
                assert(arr_faces_after_filter[index]->img_base64_data.length() >0);
                vec_face_imagebase64_.push_back(arr_faces_after_filter[index]->img_base64_data);
                //real_getimage_index_.push_back(index);
                vec_face_trackid_.push_back(arr_faces_after_filter[index]->trackid);
            }
            if (1 == config_use_strategy_1n_) { ///< 先上送模糊人脸信息，同时获取详细人脸信息
                for (unsigned int i=0; i< vec_face_trackid_.size(); ++i) {
                    int have_perfect_flag =5;
                    AddOrUpdateToLocalPeople(vec_face_trackid_[i], NULL, NULL, &have_perfect_flag, NULL);
                }
                // 本地缓存没有，创建线程去处理
                boost::thread *temp_thread =thread_group_face_server_.create_thread(boost::bind(&FaceRecognizeManager::ThreadGetFace1nAndCacheMul, this, terminal_id, org_id,
                                            vec_face_imagebase64_, vec_face_trackid_));
                thread_group_face_server_.add_this_thread_to_unfinish_remove_list(temp_thread);
            }
            else if(2 == config_use_strategy_1n_) {   ///< 直接上送详细人脸信息
                boost::thread *temp_thread =thread_group_face_server_.create_thread(boost::bind(&FaceRecognizeManager::ThreadGetFace1nAndCacheMul, this, terminal_id, org_id,
                                            vec_face_imagebase64_, vec_face_trackid_));
                if(temp_thread && temp_thread->joinable()) {
                    temp_thread->join();
                    thread_group_face_server_.remove_thread(temp_thread);
                }
                for (unsigned int i=0; i<vec_need_to_get1n_info_index.size(); ++i) {
                    int index = vec_need_to_get1n_info_index[i];
                    int check_ret = GetCachePeopleInfoFromLocal(arr_face_detect_unit[index].face_detect_info_ptr_->trackid, NULL, NULL,
                                    &arr_face_detect_unit[index].have_perfect_face_, &arr_face_detect_unit[index].perfect_face_1n_ptr_);
                    assert(YSOS_ERROR_SUCCESS == check_ret);
                }
            }
            else {
                assert(false);
                YSOS_LOG_ERROR_CUSTOM(MY_LOGGER, "error, not support use stragegy, config_use_strategy_1n_:" << config_use_strategy_1n_);
                return YSOS_ERROR_FAILED;
            }
        }
        boost::this_thread::interruption_point();
        //与一次比较是否有不同
        //have_change = true;  ///< 用于上层判断位置信息变了的情况
        //t_vec_lasttime_face_unit = arr_face_detect_unit;
        //if (false == have_change) {
        //  return YSOS_ERROR_SUCCESS;
        //}
        //boost::this_thread::interruption_point();
        //// 去掉一下base64
        //if (false == config_param.callback_face_imgbase64) {
        //  for (int i =0; i<arr_face_detect_unit.size(); ++i) {
        //    if (NULL == arr_face_detect_unit[i].face_detect_info_ptr_) {
        //      continue;
        //    }
        //    arr_face_detect_unit[i].face_detect_info_ptr_->img_base64_data = "";
        //  }
        //}
        ////去掉facefeature
        //if (false == config_param.callback_face_feature && config_param.callback_1n_person) {
        //  for (int i =0 ; i< arr_face_detect_unit.size(); ++i) {
        //    arr_face_detect_unit[i].face_detect_info_ptr_->face_feature = "";
        //  }
        //}
        /////< 所有人都有人的详细信息
        ////先抛出一次所有人的信息
        //std::string json_data;
        //int iret = EncodeJson(arr_face_detect_unit, json_data);
        //YSOS_LOG_DEBUG("faceinfo:" << json_data.c_str());
        //if (YSOS_ERROR_SUCCESS != iret) {
        //  return iret;
        //}
        //AlertTopMessage(AMP_TIP_FACE_CALLBACK_FACE_DETECT_INFO,json_data);
        /// 清空本地保存的多余的人脸数据，比当前人脸trackid少50的都删除(default save 50 person)
        if (arr_faces_after_filter.size() >0) {
            boost::lock_guard<boost::mutex> guard(mutex_map_facecallback_face_units_);
            if(map_facecallback_face_units_.size() >50) {
                int min_trackid = arr_faces_after_filter[0]->trackid -50;
                if(min_trackid <0) {
                    min_trackid =0;
                }
                for (unsigned int i=1; i< arr_faces_after_filter.size(); ++i) {
                    if (min_trackid > arr_faces_after_filter[i]->trackid) {
                        min_trackid = arr_faces_after_filter[i]->trackid;
                    }
                }
                std::map<int, TDFaceDetectUnit>::iterator itremove = map_facecallback_face_units_.begin();
                for (itremove; itremove != map_facecallback_face_units_.end();) {
                    if (itremove->second.face_detect_info_ptr_->trackid < min_trackid) {
                        itremove = map_facecallback_face_units_.erase(itremove);
                        continue;
                    }
                    ++ itremove;
                }
            }
        }
        arr_faces_unit = arr_face_detect_unit; ///< 输出结果
        return YSOS_ERROR_SUCCESS;
    }


    int FaceRecognizeManager::FaceCompareAccordDetailInfo(const int &track_id_a, const int &track_id_b, int &is_match) {
        is_match =-1;
        boost::lock_guard<boost::mutex> lock_guard(mutex_map_facecallback_face_units_);
        std::map<int, TDFaceDetectUnit>::iterator itfind_a = map_facecallback_face_units_.find(track_id_a);
        std::map<int, TDFaceDetectUnit>::iterator itfind_b = map_facecallback_face_units_.find(track_id_b);
        if(itfind_a == map_facecallback_face_units_.end() || itfind_b == map_facecallback_face_units_.end()) {
            return YSOS_ERROR_FAILED;
        }
        if(1 !=itfind_a->second.have_perfect_face_ || 1 != itfind_b->second.have_perfect_face_) {
            is_match =-1;
            return YSOS_ERROR_SUCCESS;
        }
        TDFaceDetectUnit &unit_a_ref = itfind_a->second;
        TDFaceDetectUnit &unit_b_ref = itfind_b->second;
        if(1 == unit_a_ref.have_perfect_face_ && 1 == unit_b_ref.have_perfect_face_
                && unit_a_ref.perfect_face_1n_ptr_ && unit_b_ref.perfect_face_1n_ptr_
                && unit_a_ref.perfect_face_1n_ptr_->vec_faces.size() >0 && unit_b_ref.perfect_face_1n_ptr_->vec_faces.size() >0
          ) {
            FaceCompareResultPtr &face_detail_a_ref = unit_a_ref.perfect_face_1n_ptr_->vec_faces[0];
            FaceCompareResultPtr &face_detail_b_ref =unit_b_ref.perfect_face_1n_ptr_->vec_faces[0];
            if(face_detail_a_ref->person_id == face_detail_b_ref->person_id) {
                is_match = 1;
                return YSOS_ERROR_SUCCESS;
            }
        }
        is_match = 0;
        return YSOS_ERROR_SUCCESS;
    }

    int FaceRecognizeManager::GetCachePeopleInfoFromLocal(const int iTrackid,  TDFaceDetectUnit *pfaceunit
            ,TDFaceDetectInfoPtr *pfacedetect_ptr,
            int *pface_perface_face,FaceComparisonResult1NPtr *pfacecomparison_ptr) {
        if (iTrackid <0) {
            //YSOS_LOG_ERROR_CUSTOM(MY_LOGGER, "get cache people info from local error, trarck <0, track:", iTrackid);
            return YSOS_ERROR_FAILED;
        }
        boost::lock_guard<boost::mutex> lock_guard(mutex_map_facecallback_face_units_);
        std::map<int, TDFaceDetectUnit>::iterator itfind =  map_facecallback_face_units_.find(iTrackid);
        if (itfind != map_facecallback_face_units_.end()) {
            if (pfaceunit) {
                //*FindUnit = itfind->second;
                itfind->second.Clone(*pfaceunit);
            }
            if (pfacedetect_ptr) {
                if (NULL == *pfacedetect_ptr) {
                    pfacedetect_ptr->reset(new TDFaceDetectInfo);
                    if (NULL == *pfacedetect_ptr) {
                        return YSOS_ERROR_FAILED;
                    }
                }
                if (itfind->second.face_detect_info_ptr_) {
                    **pfacedetect_ptr = *(itfind->second.face_detect_info_ptr_);
                }
                else  {
                    *pfacedetect_ptr = NULL;
                }
            }
            if (pface_perface_face) {
                *pface_perface_face = itfind->second.have_perfect_face_;
            }
            if (pfacecomparison_ptr) {
                if (NULL == *pfacecomparison_ptr) {
                    pfacecomparison_ptr->reset(new FaceComparisonResult1N);
                    if (NULL == *pfacecomparison_ptr) {
                        return YSOS_ERROR_FAILED;
                    }
                }
                if (itfind->second.perfect_face_1n_ptr_) {
                    itfind->second.perfect_face_1n_ptr_->Clone(*pfacecomparison_ptr->get());
                }
                else {
                    *pfacecomparison_ptr = NULL;
                }
            }
            return YSOS_ERROR_SUCCESS;
        }
        return YSOS_ERROR_FAILED;
    }

    int FaceRecognizeManager::GetCachePeopleInfoFromLocal(const std::string &personid, TDFaceDetectUnit *pfaceunit
            ,TDFaceDetectInfoPtr *pfacedetect_ptr,
            int *pface_perface_face,FaceComparisonResult1NPtr *pfacecomparison_ptr) {
        if(personid.length() <=0) {
            return YSOS_ERROR_FAILED;
        }
        boost::lock_guard<boost::mutex> lock_guard(mutex_map_facecallback_face_units_);
        std::map<int, TDFaceDetectUnit>::reverse_iterator itfind =  map_facecallback_face_units_.rbegin();
        for (itfind; itfind != map_facecallback_face_units_.rend(); ++itfind) {
            if(1 !=itfind->second.have_perfect_face_) {
                continue;
            }
            if(NULL == itfind->second.perfect_face_1n_ptr_ || 0 == itfind->second.perfect_face_1n_ptr_->vec_faces.size() <=0) {
                assert(false);
                continue;
            }
            if(itfind->second.perfect_face_1n_ptr_->vec_faces[0]->person_id != personid) {
                continue;
            }
            // find
            if (pfaceunit) {
                //*FindUnit = itfind->second;
                itfind->second.Clone(*pfaceunit);
            }
            if (pfacedetect_ptr) {
                if (NULL == *pfacedetect_ptr) {
                    pfacedetect_ptr->reset(new TDFaceDetectInfo);
                    if (NULL == *pfacedetect_ptr) {
                        return YSOS_ERROR_FAILED;
                    }
                }
                if (itfind->second.face_detect_info_ptr_) {
                    **pfacedetect_ptr = *(itfind->second.face_detect_info_ptr_);
                }
                else  {
                    *pfacedetect_ptr = NULL;
                }
            }
            if (pface_perface_face) {
                *pface_perface_face = itfind->second.have_perfect_face_;
            }
            if (pfacecomparison_ptr) {
                if (NULL == *pfacecomparison_ptr) {
                    pfacecomparison_ptr->reset(new FaceComparisonResult1N);
                    if (NULL == *pfacecomparison_ptr) {
                        return YSOS_ERROR_FAILED;
                    }
                }
                if (itfind->second.perfect_face_1n_ptr_) {
                    itfind->second.perfect_face_1n_ptr_->Clone(*pfacecomparison_ptr->get());
                }
                else {
                    *pfacecomparison_ptr = NULL;
                }
            }
            return YSOS_ERROR_SUCCESS;
        }
        return YSOS_ERROR_FAILED;
    }


    int FaceRecognizeManager::AddOrUpdateToLocalPeople(const int trackid, TDFaceDetectUnit *pfaceunit,
            TDFaceDetectInfo **pfacedetect_ptr,
            int *pface_perface_face,FaceComparisonResult1N **pfacecomparison_ptr) {
        boost::lock_guard<boost::mutex> lock_guard(mutex_map_facecallback_face_units_);
        std::map<int, TDFaceDetectUnit>::iterator itfind =  map_facecallback_face_units_.find(trackid);
        if (itfind != map_facecallback_face_units_.end()) {
            // find
            if (pfaceunit) {
                pfaceunit->Clone(itfind->second);
            }
            if (pfacedetect_ptr) {
                *itfind->second.face_detect_info_ptr_ =**pfacedetect_ptr;
            }
            if (pfacecomparison_ptr) {
                if (*pfacecomparison_ptr) {
                    if (NULL ==itfind->second.perfect_face_1n_ptr_) {
                        itfind->second.perfect_face_1n_ptr_.reset(new FaceComparisonResult1N);
                        if (NULL ==itfind->second.perfect_face_1n_ptr_) {
                            return YSOS_ERROR_FAILED;
                        }
                    }
                    *itfind->second.perfect_face_1n_ptr_ = **pfacecomparison_ptr;
                }
                else {
                    itfind->second.perfect_face_1n_ptr_ = NULL;
                }
            }
            if (pface_perface_face) {
                itfind->second.have_perfect_face_ = *pface_perface_face;
            }
            return YSOS_ERROR_SUCCESS;
        }
        // not find
        if (pfaceunit) {
            TDFaceDetectUnit tmpunit;
            pfaceunit->Clone(tmpunit);
            map_facecallback_face_units_[trackid] = tmpunit;
        }
        return YSOS_ERROR_SUCCESS;
    }

    void FaceRecognizeManager::ThreadGetFace1nAndCache(const std::string terminal_id, const std::string org_id,const TDFaceDetectInfoPtr face_detect_ptr) {
        FaceComparisonResult1NPtr result_1n_ptr(new FaceComparisonResult1N);
        assert(face_detect_ptr && result_1n_ptr);
        if (NULL == face_detect_ptr || NULL == result_1n_ptr) {
            return;
        }
        if (face_detect_ptr->/*face_feature*/img_base64_data.length() <=0) {
            return;
        }
        assert(face_recognize_ptr_);
        if(!face_recognize_ptr_) {
            return;
        }
        std::vector<std::string> vec_face_base64;
        vec_face_base64.clear();
        vec_face_base64.push_back(face_detect_ptr->img_base64_data);
        std::string error_message;
        std::vector<FaceComparisonResult1NPtr> vec_comparisonresult;
        int iret = face_recognize_ptr_->Comparison1NAccordBase64(terminal_id.c_str(), org_id.c_str(),
                   vec_face_base64, 3, 0, NULL, /*result_1n_ptr.get()*/vec_comparisonresult, &error_message);
        if(vec_comparisonresult.size() >0) {
            result_1n_ptr = vec_comparisonresult[0];
        }
        if (YSOS_ERROR_SUCCESS != iret) {  ///< 连网出错了
            assert(false);
            //result_1n_ptr = NULL;
            result_1n_ptr->group_id ="-1"; ///< netword is not valid
            YSOS_LOG_ERROR_CUSTOM(MY_LOGGER, "error, get face1n:" <<  error_message.c_str());
            int have_serarch_people_flag =0; ///< cant search agagin
            AddOrUpdateToLocalPeople(face_detect_ptr->trackid, NULL, NULL, &have_serarch_people_flag, NULL);
            return;  ///< 直接返回，不缓存到本地,这样下载可以再去查一下。
        }
        //std::string error_message;
        bool bhave_perfect_face = CheckHavePerfectFace(result_1n_ptr.get(), &error_message);
        if(bhave_perfect_face &&result_1n_ptr) {
            /// 这里只保存一份人脸数据（防止其他地方再次判断是不是有最佳人脸）
        }
        else if(false == bhave_perfect_face && result_1n_ptr) {
            result_1n_ptr->vec_faces.clear();
        }
        int iperface_face = bhave_perfect_face ? 1 :-1;
        // 缓存到本地
        FaceComparisonResult1N *p1n = result_1n_ptr.get();
        AddOrUpdateToLocalPeople(face_detect_ptr->trackid, NULL, NULL, &iperface_face, &p1n);
        return;
    }


    void FaceRecognizeManager::ThreadGetFace1nAndCacheMul(const std::string terminal_id, const std::string org_id,const std::vector<std::string> vec_image_base64s, const std::vector<int> vec_image_trackid) {
        assert(vec_image_base64s.size() == vec_image_trackid.size());
        if (vec_image_base64s.size() != vec_image_trackid.size()) {
            YSOS_LOG_ERROR_CUSTOM(MY_LOGGER, "error, size not compare, image base64 vec, trackid size");
            return;
        }
        assert(face_recognize_ptr_);
        if(!face_recognize_ptr_) {
            return;
        }
        std::vector<FaceComparisonResult1NPtr> vec_comparisonresult;
        std::string error_message;
        //int compret_ret =face_recognize_ptr_->Comparison1N(vec_image_base64s,0, vec_comparisonresult, &error_message);
        int compret_ret =face_recognize_ptr_->Comparison1NAccordBase64(terminal_id.c_str(), org_id.c_str(), vec_image_base64s,3, 0, NULL, vec_comparisonresult, &error_message);
        if (YSOS_ERROR_SUCCESS != compret_ret) {
            assert(false);
            for (unsigned int i=0; i< vec_image_trackid.size(); ++i) {
                FaceComparisonResult1NPtr result_1n_ptr(new FaceComparisonResult1N);
                //result_1n_ptr = NULL;
                result_1n_ptr->group_id ="-1"; ///< netword is not valid
                YSOS_LOG_ERROR_CUSTOM(MY_LOGGER, "error, get face1n:" <<  error_message.c_str());
                int have_serarch_people_flag =0; ///< can search agagin
                AddOrUpdateToLocalPeople(vec_image_trackid[i], NULL, NULL, &have_serarch_people_flag, NULL);
            }
            return;  ///< 直接返回，不缓存到本地,这样下载可以再去查一下。
        }
        assert(vec_image_trackid.size() == vec_comparisonresult.size());
        if (vec_image_trackid.size() != vec_comparisonresult.size()) {
            YSOS_LOG_ERROR_CUSTOM(MY_LOGGER, "error, size not compare");
            return /*YSOS_ERROR_FAILED*/;
        }
        for (unsigned int i=0; i<vec_image_trackid.size(); ++i) {
            FaceComparisonResult1NPtr &result_1n_ptr = vec_comparisonresult[i];
            //std::string error_message;
            bool bhave_perfect_face = CheckHavePerfectFace(result_1n_ptr.get(), &error_message);
            if(false == bhave_perfect_face && result_1n_ptr) {
                result_1n_ptr->vec_faces.clear();
            }
            int iperface_face = bhave_perfect_face ? 1 :-1;
            // 缓存到本地
            FaceComparisonResult1N *p1n = result_1n_ptr.get();
            AddOrUpdateToLocalPeople(vec_image_trackid[i], NULL, NULL, &iperface_face, &p1n);
        }
        return;
    }

    int FaceRecognizeManager::SetConfigKeyValue(const std::string &key, const std::string &value) {
        if(value.length() <=0) {
            return YSOS_ERROR_FAILED;
        }
        if("call1n_max_times" == key) {
            config_call1n_max_times_ = atoi(value.c_str());
        }
        return YSOS_ERROR_FAILED;
    }

    bool FaceRecognizeManager::CheckHavePerfectFace(FaceComparisonResult1N *recog_info, std::string *message) {
        assert(recog_info);
        if (NULL == recog_info) {
            if (message) {
                *message = "没有人脸1比n数据";
            }
            return false;
        }
        if (recog_info->vec_faces.size() <=0) {
            if (message) {
                *message = "没有一个可用的最佳人脸数据";
            }
            return false;
        }
#ifdef _WIN32        
        // 满足要求：最低人脸质量>=0.8
        if (atof(recog_info->vec_faces[0]->score.c_str()) < /*config_face_recog_min_face_quality_*/FaceModuleConfig::GetInstance()->GetMinRecognizeScore()) {
            if (message) {
                char tempbuf[1024]= {'\0'};
                sprintf_s(tempbuf, sizeof(tempbuf), "最佳人脸的匹配度:%s 小于分数:%lf", recog_info->vec_faces[0]->score.c_str(), /*config_face_recog_min_face_quality_*/FaceModuleConfig::GetInstance()->GetMinRecognizeScore());
                *message = tempbuf;
                if (config_face_recog_return_sensitive_message_) {
                    sprintf_s(tempbuf, sizeof(tempbuf), "|personname:%s, person_id:%s",recog_info->vec_faces[0]->person_name.c_str(), recog_info->vec_faces[0]->person_id.c_str());
                    *message += tempbuf;
                }
            }
            return false;
        }
        // 满足要求：最相似的两个人至少相差0.1
        if (recog_info->vec_faces.size() >=2) {
            if (atof(recog_info->vec_faces[0]->score.c_str()) - atof(recog_info->vec_faces[1]->score.c_str()) <= config_min_spacing_exclusive_similar_) {
                if (message) {
                    char tempbuf[1024] = {'\0'};
                    sprintf_s(tempbuf, sizeof(tempbuf), "最佳人脸，有匹配到两个最相似的人脸,两个人脸的分数分别为:%s,%s",
                              recog_info->vec_faces[0]->score.c_str(), recog_info->vec_faces[1]->score.c_str());
                    *message = tempbuf;
                    if (config_face_recog_return_sensitive_message_) {
                        sprintf_s(tempbuf, sizeof(tempbuf), "|person name1:%s, person_id:%s|person name2:%s, person_id:%s",
                                  recog_info->vec_faces[0]->person_name.c_str(), recog_info->vec_faces[0]->person_id.c_str(),
                                  recog_info->vec_faces[1]->person_name.c_str(), recog_info->vec_faces[1]->person_id.c_str());
                        *message += tempbuf;
                    }
                }
                return false;
            }
        }
#else
        // 满足要求：最低人脸质量>=0.8
        if (atof(recog_info->vec_faces[0]->score.c_str()) < /*config_face_recog_min_face_quality_*/FaceModuleConfig::GetInstance()->GetMinRecognizeScore()) {
            if (message) {
                char tempbuf[1024]= {'\0'};
                sprintf(tempbuf, "最佳人脸的匹配度:%s 小于分数:%lf", recog_info->vec_faces[0]->score.c_str(), /*config_face_recog_min_face_quality_*/FaceModuleConfig::GetInstance()->GetMinRecognizeScore());
                *message = tempbuf;
                if (config_face_recog_return_sensitive_message_) {
                    sprintf(tempbuf, "|personname:%s, person_id:%s",recog_info->vec_faces[0]->person_name.c_str(), recog_info->vec_faces[0]->person_id.c_str());
                    *message += tempbuf;
                }
            }
            return false;
        }
        // 满足要求：最相似的两个人至少相差0.1
        if (recog_info->vec_faces.size() >=2) {
            if (atof(recog_info->vec_faces[0]->score.c_str()) - atof(recog_info->vec_faces[1]->score.c_str()) <= config_min_spacing_exclusive_similar_) {
                if (message) {
                    char tempbuf[1024] = {'\0'};
                    sprintf(tempbuf, "最佳人脸，有匹配到两个最相似的人脸,两个人脸的分数分别为:%s,%s",
                              recog_info->vec_faces[0]->score.c_str(), recog_info->vec_faces[1]->score.c_str());
                    *message = tempbuf;
                    if (config_face_recog_return_sensitive_message_) {
                        sprintf(tempbuf, "|person name1:%s, person_id:%s|person name2:%s, person_id:%s",
                                  recog_info->vec_faces[0]->person_name.c_str(), recog_info->vec_faces[0]->person_id.c_str(),
                                  recog_info->vec_faces[1]->person_name.c_str(), recog_info->vec_faces[1]->person_id.c_str());
                        *message += tempbuf;
                    }
                }
                return false;
            }
        }
#endif
        return true;
    }
}