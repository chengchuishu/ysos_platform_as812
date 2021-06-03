/**   //NOLINT
  *@file hongrunfacedetect.h
  *@brief Definition of
  *@version 0.1
  *@author wanfenfen
  *@date Created on: 2018:4:12   15:10
  *@copyright Copyright(c) 2016 YunShen Technology. All rights reserved.
  * 
  *@howto Usage:
  *@todo
  */


#ifndef YSOS_PLUGIN_HONGRUN_FACE_DETECTOR_H_
#define YSOS_PLUGIN_HONGRUN_FACE_DETECTOR_H_

// boost headers
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/thread.hpp>
#include <boost/shared_array.hpp>
#include <boost/thread/mutex.hpp>

/// opencv headers
#include "../../../../../thirdparty/opencv/include/opencv2/opencv.hpp"
#include "../../../../../thirdparty/opencv/include/opencv2/highgui/highgui.hpp"

/// hongruan sdk headers
#include "../../../../../thirdparty/hongruan/include/merror.h"
#include "../../../../../thirdparty/hongruan/include/amcomdef.h"

// private headers
#include "../../../../../../core/trunk/main/public/include/core_help_package/utility.h"

#include "../../../public/include/common/commonenumdefine.h"
#include "../../../public/include/common/commonstructdefine.h"

#include "../../../public/include/common/facedetectstruct.h"
#include "facedetector.h"

namespace ysos {


    class FaceDetectComparisonHR : public FaceDetectorInterface {
        public:

            FaceDetectComparisonHR();
            ~FaceDetectComparisonHR();

            /// inherit from FaceDetectorInterface
            virtual int Init ( const char* mouule_file_path );
            virtual int UnInit();
            virtual int SetConfigKeyValue ( const std::string &key, const std::string &value );
            virtual int FaceDetect ( const char* pdata, const int& widht, const int& heignt, const int& channels,
                                     const bool& need_track, const bool& need_face_qulity, const bool& need_key_point,
                                     const bool& need_face_attribute, const bool& need_face_nationality,
                                     const bool& get_face_image_base64,
                                     const bool& get_field_feature,
                                     const bool& get_prob_feature,
                                     std::vector<TDFaceDetectInfoPtr>& vec_face_detect_info );
            virtual int Comparison11Local ( const char* face_fetatue1, const int face_fetatue1_length,
                                            const char* face_feature2, const int face_feature2_length, FaceComparisonResult11* compare_result );

        private:

            int InitFDEngine();
            int UnInitFDEngine();
            int InitGenderEngine();
            int UnInitGenderEngine();
            int InitAgeEngine();
            int UnInitAgeEngine();
            int InitFTEngine();
            int UnInitFTEngine();
            int InitFREngine();
            int UnInitFREngine();

            /**
             *@brief 清理一下最近跟踪的人脸信息，丢失人脸1分钟的都清掉
             *@param
             *@return
             */
            void ClearTrackFaceInfoList();

            /**
             *@brief 更新人脸跟踪号, 由于虹软人脸跟踪号 不是唯一的，所以这里重新 ，根据人脸1比1来分配人脸跟踪id号
             *@param
             *@return
             */
            int UpdateFaceTrackId ( std::vector<TDFaceDetectInfoPtr>& vec_face_info, std::vector<cv::Mat>& vec_face_mat );

            /**
            *@brief 通过人脸的大小, 估计人的距离（不是很精确，只是精估）
            *@param face_ratio[Input] 人脸在摄像头中的比例（如果摄像头水平方向比垂直方向范围大，则取垂直方向的比例，反之取水平方向的比例）
            *@return  估计后的人的距离，如果小于或等于0，则表示估计不出来
            */
            float EstimatePeopleDistanceAccordFaceSize ( const float face_ratio );

        private:

            boost::recursive_mutex mutex_public_fun_;

            std::string hr_appid_;           ///< 用户申请 SDK 时获取的 App Id
            std::string hr_fd_sdkkey_;       ///< 用户申请 SDK 时获取的 SDK Key  人脸检测(FD)
            std::string hr_ft_sdkkey_;       ///< 人脸追踪(FT)
            std::string hr_gender_sdkkey_;   ///< 性别识别(Gender)
            std::string hr_age_sdkkey_;      ///< 年龄识别(Age)
            std::string hr_fr_sdkkey_;       ///< 人脸识别(FR)

            int hr_max_face_count_;         ///< 用户期望引擎最多能检测出的人脸数 有效值范围[1,100]，默认是10
            int hr_min_scale_;        ///< 用于数值表示的最小人脸尺寸 有效值范围[2,50] 推荐值 16

            MHandle hr_facedetect_engine_handle_;
            boost::shared_array<MByte> hr_fd_work_buf_ptr_;  ///< 分配给引擎使用的内存地址
            boost::shared_array<MRECT> hr_face_input_rect_ptr_;
            boost::shared_array<MInt32> hr_face_input_orient_ptr_;

            MHandle hr_gender_engine_handle_;
            boost::shared_array<MByte> hr_gender_work_buf_ptr_;

            MHandle hr_age_engine_handle_;
            boost::shared_array<MByte> hr_age_work_buf_ptr_;

            MHandle hr_facetrack_engine_handle_;
            boost::shared_array<MByte> hr_facetrace_work_buf_ptr_;

            MHandle hr_facerecognition_engine_handle_;
            boost::shared_array<MByte> hr_facerecognition_work_buf_ptr_;

            struct TDTrackFaceInfo
            {
                    int track_id;
                    int32_t track_begin_clock;  ///< 最开始检测到的时间
                    int32_t track_latest_clock; ///< 最近一次跟踪到的时间
                    TDFaceDetectInfoPtr face_detect_ptr;

                    cv::Mat face_mat;
                public:
                    TDTrackFaceInfo() {
                        track_id = -1;
                        track_begin_clock = track_latest_clock = 0;
                        face_detect_ptr = NULL;
                    }
            };
            typedef boost::shared_ptr<TDTrackFaceInfo> TDTrackFaceInfoPtr;

            float config_score_min_face_match_;                    ///<  本地人脸 （虹软1比2），最小平均相似度，默认是0.6
            float config_face_track_min_blur_score_;               ///< 人脸跟踪，最小的人脸清晰度，越大表示越清晰，推荐范围0.65-1.0，默认是0.7
            float config_face_track_min_yaw_;                      ///< 人脸跟踪，人脸偏航角(左右摇头，向左为正，向右为负） 大于此的则过滤，默认是35
            float config_face_track_min_pitch_;                    ///< 人脸跟踪，人脸俯仰角（上下点头,仰头为正，低头为负）大于此的则过滤，默认是35
            float face_trace_min_cache_face_info_milliseonds_;     ///< ;人脸跟踪，跟踪人脸缓存时间（单位：毫秒），默认是30000

            /*
             人脸跟踪流程：
             1.人脸检测（获取人脸性别，人脸特征），过滤人脸：低质量，人脸不清晰，不脸角度不好，人脸性别检测不了的，人脸特征没有的
             2.与历史人脸比对（当前人脸使用人脸特征，历史人脸使用probe特征），查找相似人
             3.找到相似人，使用相似人的跟踪号，如果当前人脸质量更高，则获取人脸probe特征，更新到历史人脸数据中
             4.如果没找到相似人，则分配新的跟踪号，获取人脸probe特征，加入到历史人脸数据中
             */
            boost::mutex mutex_face_traing_oper_;
            std::list<TDTrackFaceInfoPtr> list_track_face_info_;
            int max_track_id_;


            std::string tmp_face_image_track_hr_folder_;            ///< 默认是：../log/tmp_face_image_track_hr
            std::string tmp_face_image_track_useself_folder_;       ///< 默认是：../log/tmp_face_image_track_useself
            std::string tmp_face_image_track_recheck_foler_;        ///< 默认是：../log/tmp_face_image_track_recheck

            int config_estimate_people_distance_use_cameraid_;  ///< 评估人脸距离使用摄像头ID号. 0 表示未知， 1 logic HD720P, 3 realsense, 3 usb 广角摄像头,默认是0
            float config_max_horizon_angle_half_;               ///< 评估人脸角度，摄像头最大的水平和垂直角度,这个模块中不方便获取摄像头的这个参数，所以在配置文件中配置，默认是30度
            float config_max_vertical_angle_half_;
    };

}

#endif