/**   //NOLINT
  *@file FaceRecognizeManager.h
  *@brief Definition of
  *@version 0.1
  *@author wanfenfen
  *@date Created on: 2018:4:12   18:53
  *@copyright Copyright(c) 2016 YunShen Technology. All rights reserved.
  * 
  *@howto Usage:
  *@todo
  */

#ifndef  YSOS_PLUGIN_HONGRUN_FACE_RECOGNIZER_H_
#define YSOS_PLUGIN_HONGRUN_FACE_RECOGNIZER_H_

/// stl headers
#include <string>
#include <vector>
#include <map>

/// boost headers
#include <boost/shared_ptr.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/threadpool.hpp>

/// ysos headers
#include "../../../../../../core/trunk/main/protect/include/os_hal_package/datainterfacemanager.h"

// private headers
#include "../../../public/include/common/commonenumdefine.h"
#include "../../../public/include/common/commonstructdefine.h"

#include "../../../public/include/common/facedetectstruct.h"
#include "../../../public/include/common/threadgroupex.hpp"
#include "../include/facecomparisoninterface.h"

namespace ysos {
    class FaceRecognizeManager {
        public:
            FaceRecognizeManager();

            ~FaceRecognizeManager();

            /**
             *@brief 初始化操作
             *@param face_recognize_ptr[Input] 待传入的人脸1比n对象
             *@return
             */
            void Init ( FaceComparisonInterfacePtr face_recognize_ptr );

            /**
             *@brief 释放资源操作
             *@param
             *@return
             */
            void UnInit();

            /**
             *@brief 设置配置参数
             *@param key[Input] 传入的待配置参数名
             *@param value[Input] 传入的待配置参数值
             *@return 0 sucess, -1 fail
             */
            int SetConfigKeyValue ( const std::string &key, const std::string &value );

            /**
             *@brief 获取人脸识别结果
             *@param arr_faces[Input] 人脸模糊信息（人脸检测的结果）
             *@param arr_faces_unit[Output] 人脸信息（包含传进来的模糊信息 和附加的详细人脸信息）
             *@return 0 success, -1 fail
             */
            int  FaceRecognition ( const std::vector<TDFaceDetectInfoPtr>& arr_faces, std::vector<TDFaceDetectUnit>& arr_faces_unit );

            /**
             *@brief 根据人脸详细信息进行人脸比对
             *       主要是根据详细人脸的personid字段来比对判断是不是同一个人
             *       是同一个人的条件是：两个人都有详细人脸同时personid相同
             *@param track_id_a[Input] 待比对的人脸a的跟踪号
             *@param track_id_b[Input] 待比对的人脸b的跟踪号
             *@param is_match[Output]  是否是同一个人，如果为1则表示是同一个人，如果为0则表示不是同一个人，－1表示不确定（其中至少有一个人没有详细人脸信息）
             *@return 0 比对成功, -1 比对失败, 其中至少有1个人没有记录，比对不了
             */
            int  FaceCompareAccordDetailInfo ( const int& track_id_a, const int& track_id_b, int& is_match );

        protected:
            int AddOrUpdateToLocalPeople ( const int trackid, TDFaceDetectUnit* pfaceunit, TDFaceDetectInfo** pfacedetect_ptr,
                                           int *pface_perface_face, FaceComparisonResult1N** pfacecomparison_ptr );

            /**
            *@brief
            *@param
            *@return  0 find, -1 not find
            */
            int GetCachePeopleInfoFromLocal ( const int iTrackid,  TDFaceDetectUnit* pfaceunit
                                              , TDFaceDetectInfoPtr* pfacedetect_ptr,
                                              int *pface_perface_face, FaceComparisonResult1NPtr *pfacecomparison_ptr );
            int GetCachePeopleInfoFromLocal ( const std::string& personid, TDFaceDetectUnit* pfaceunit
                                              , TDFaceDetectInfoPtr* pfacedetect_ptr,
                                              int *pface_perface_face, FaceComparisonResult1NPtr *pfacecomparison_ptr );

            void ThreadGetFace1nAndCache ( const std::string terminal_id, const std::string org_id, const TDFaceDetectInfoPtr face_detect_ptr );
            void ThreadGetFace1nAndCacheMul ( const std::string terminal_id, const std::string org_id, const std::vector<std::string> vec_image_base64s, const std::vector<int> vec_image_trackid );

            /**
             *@brief 判断是否有最佳人脸，根据人脸识别阈值判断是否有最佳人脸
             *       如果有至少2个人同时满足人脸识别阈值，则判断最高分数的人与其他人是否相差很大（有另一个阈值），如果相差不大则也当作是识别不了了（防止认错人）
             *       如果有最佳人脸，则最佳人脸的的索引为0（即第1个人数，人脸分数 从高到低）
             *@param recog_info[Input] 人脸识别后所有的结果
             *@pram message[Output] 出错信息
             *@return 0 判断成功, -1 判断时出错，出错信息会输出在参数message中。
             */
            bool CheckHavePerfectFace ( FaceComparisonResult1N* recog_info, std::string* message );

        private:

            boost::recursive_mutex mutex_public_;
            DataInterfacePtr data_ptr_;                             //< 内存指针


            int config_use_strategy_1n_;   ///< 默认是1：1.先抛出模糊人脸，同时查询详细人脸信息,2 获取详细人脸信息再向上抛出
            int config_face_recog_return_sensitive_message_;   ///< 默认0，是否返回敏感信息，用于调试。
            float config_min_spacing_exclusive_similar_;       ///< 默认是0.1， 人脸1比n，最相似人脸必须满足比第2相似人脸分数多于此值
            /// 连接后以1比n最多多少次查询，默认是1
            /// 如果人脸检测有人脸质量，则可设为1（检测到更高人脸质量时会再次查询人脸详细信息）
            /// 如果人脸检测没有人脸质量值，则可设为3或更大值）
            ///（由于虹软没有人脸质量，如果一开始发给后台的人脸质量不高，则会比对不到人，则通过多次比对来获取）
            int config_call1n_max_times_;
            boost::mutex mutex_map_facecallback_face_units_;
            std::map<int, TDFaceDetectUnit> map_facecallback_face_units_;      ///< 缓存的人脸详细信息，first:trackid, second:faceunits
            std::map<int, int> map_trackid_call1n_retry_times_;                      ///< 由于没有人脸质量（不知道人脸模糊度），这个优化一下，多传几次后台查一下, first:trackid, second:try times

            FaceComparisonInterfacePtr face_recognize_ptr_;    ///< 人脸1比n接口（连接服务器后台）
            boostex::thread_group thread_group_face_server_;   ///< 所有连接服务器后台的线程组

    };

}
#endif