/**
 *@file FilePath.h
 *@brief Definition of FilePath
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-21 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */
#ifndef OHP_FILEPATH_H  //NOLINT
#define OHP_FILEPATH_H  //NOLINT

/// ysos private headers //  NOLINT
#include "../../../public/include/sys_interface_package/common.h"
/// stl headers //  NOLINT
#include <string>
#include <vector>
/// boost headers //  NOLINT
#include <boost/filesystem.hpp> //NOLINT

namespace ysos {
class FilePathImpl;

typedef boost::filesystem::file_type FileType;
typedef boost::filesystem::space_info SpaceInfo;

/**
 * @brief 对文件的合法性检查、获取文件信息、文件的变更及当前路径、磁盘信息获取等。
 *        为使用方便，提供了相关的静态函数该类的异常是：filesystem_error
 */
class YSOS_EXPORT FilePath {
 public:
  explicit FilePath(const std::string filename);
  virtual ~FilePath();

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
  FileType GetFileStatus();
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
   *@brief 删除指定的文件 //NOLINT
   *@param srcfilename 源文件名
   *@param dstfilename 待删除的文件名
   */
  static void RenameFile(const std::string &srcfilename, const std::string &dstfilename); //NOLINT
  /**
   *@brief 复制当前文件到指定的文件 //NOLINT
   *@param dstfilename 目标文件名
   */
  void CopyFile(const std::string &filename);
  /**
   *@brief 复制当前文件到指定的文件 //NOLINT
   *@param srcfilename 源文件名
   *@param dstfilename 目标文件名
   */
  static void CopyFile(const std::string &srcfilename, const std::string &dstfilename); //NOLINT
  /**
   *@brief 创建文件 //NOLINT
   */
  bool CreateFile(void);
  /**
   *@brief 创建文件 //NOLINT
   *@filename 创建指定的文件
   *@return 成功true,失败false
   */
  static bool CreateFile(const std::string &filename);
  /**
   *@brief 删除文件 //NOLINT
   *@return 成功true,失败false
   */
  bool RemoveFile(void);
  /**
   *@brief 删除指定的文件 //NOLINT
   *@param dstfilename 待删除的文件名
   *@return 成功true,失败false
   */
  static bool RemoveFile(const std::string &dstfilename);
  /**
   *@brief 创建目录 //NOLINT
   *@return 创建成功true,否则false
   */
  bool CreateDirectory(void);
  /**
   *@brief 创建目录 //NOLINT
   *@param dirname 待创建的目录名
   *@return 创建成功true,否则false
   */
  static bool CreateDirectory(const std::string &dirname);
  /**
   *@brief 删除目录 //NOLINT
   */
  void RemoveDirectory(void);
  /**
   *@brief 删除指定的目录 //NOLINT
   *@param dirname 待删除的目录路径
   */
  static void RemoveDirectory(const std::string &dirname);

  /**
   *@brief 获取当前文件所在的路径 //NOLINT
   *@return 当前文件路径
   */
  static std::string GetCurPath(void);
  /**
   *@brief 获取当前文件所有的磁盘信息 //NOLINT
   *@return 磁盘信息结构
   */
  SpaceInfo GetCurDiskInfo(void);
  /**
   *@brief 获取指定目录下的所有文件 // NOLINT
   *@param dirname 指定的目录
   *@return 获取到的文件列表
   */
  static std::vector<std::string> GetAllFiles(const std::string &dirname);

 protected:
  FilePathImpl *file_path_impl_;
};
}
#endif // OHP_FILEPATH_H  //NOLINT