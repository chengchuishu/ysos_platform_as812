/**
 *@file FilePathImpl.h
 *@brief Definition of FilePathImpl
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-21 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef OHP_FILEPATHIMPL_H  //NOLINT
#define OHP_FILEPATHIMPL_H  //NOLINT

/// ysos private headers //  NOLINT
#include "../../../public/include/sys_interface_package/common.h"
/// stl headers //  NOLINT
#include <string>
#include <vector>
/// boost headers //  NOLINT
#include <boost/filesystem.hpp>

namespace ysos {
//enum boost::filesystem::file_type;  //need delete for linux
//class boost::filesystem::path;      //need delete for linux
typedef boost::filesystem::space_info SpaceInfo;

/**
 *@brief 该类是FilePath的具体实现类
 *       异常是：filesystem_error
 */
class YSOS_EXPORT FilePathImpl {
 public:
  explicit FilePathImpl(const std::string filename);
  virtual ~FilePathImpl();

  /**
   *@brief 重置文件名 //NOLINT
   *@param filename 目标文件名
   *@return 无
   */
  void ResetFileName(const std::string filename);
   /**
   *@brief 文件是否存在 //NOLINT
   *@return 存在返回true，否则false
   */
  bool IsFileExist();
   /**
   *@brief 重置文件名 //NOLINT
   *@return 合法true,否则false
   */
  bool IsFileNameValid();
   /**
   *@brief 是否是普通文件 //NOLINT
   *@return 普通文件true,否则false
   */
  bool IsFile(void);
  /**
   *@brief 是否是普通文件 //NOLINT
   *@param filename 目标文件名
   *@return 普通文件true,否则false
   */
  static bool IsFile(const std::string &filename);
  /**
   *@brief 是否是目录 //NOLINT
   *@return 目录true,否则false
   */
  bool IsDirectory(void);
  /**
   *@brief 是否是目录 //NOLINT
   *@param filename 目标文件名
   *@return 目录true,否则false
   */
  static bool IsDirectory(const std::string &dirname);
  /**
   *@brief 获取文件的状态信息 //NOLINT
   *@return 状态信息结构
   */
  boost::filesystem::file_type GetFileStatus();
  /**
   *@brief 获取文件（夹）的上一层路径 //NOLINT
   *@return 上一层路径
   */
  std::string GetParentPath();
  /**
   *@brief 获取文件（夹）名 //NOLINT
   *@return 文件名
   */
  std::string GetStem();
  /**
   *@brief 获取完整文件（夹）名 //NOLINT
   *@return 完整文件名
   */
  std::string GetFilePath();
  /**
   *@brief 获取文件（夹）扩展名 //NOLINT
   *@return 文件扩展名
   */
  std::string GetFileExtention();
  /**
   *@brief 获取文件（夹）大小 //NOLINT
   *@return 文件大小值
   */
  int GetFileSize();
  /**
   *@brief 删除指定的文件 //NOLINT
   *@param dstfilename 待删除的文件名
   */
  void RenameFile(const std::string &dstfilename);
  /**
   *@brief 复制当前文件到指定的文件 //NOLINT
   *@param dstfilename 目标文件名
   */
  void CopyFile(const std::string &dstfilename);
  /**
   *@brief 创建文件 //NOLINT
   */
  bool CreateFile(void);
  /**
   *@brief 根据指定的文件名创建文件 //NOLINT
   *@param filename 目标文件名
   *@return 创建成功true,否则false
   */
  static bool CreateFile(const std::string &filename);
  /**
   *@brief 删除文件 //NOLINT
   *@return 删除成功true,否则false
   */
  bool RemoveFile(void);
  /**
   *@brief 创建目录 //NOLINT
   *@return 创建成功true,否则false
   */
  bool CreateDirectory(void);
  /**
   *@brief 删除目录 //NOLINT
   */
  void RemoveDirectory(void);
  /**
   *@brief 删除指定的目录 //NOLINT
   *@param path 待删除的目录路径
   */
  static void RemoveDirectory(const boost::filesystem::path &path);
  /**
   *@brief 获取当前文件所有的磁盘信息 //NOLINT
   *@return 磁盘信息结构
   */
  SpaceInfo GetCurDiskInfo(void);

 private:
  // 不支持赋值 //NOLINT
  FilePathImpl(const FilePathImpl &file_path);
  FilePathImpl& operator=(const FilePathImpl &file_path);

 protected:
  boost::filesystem::path  *file_path_;
};
}
#endif // OHP_FILEPATHIMPL_H //NOLINT