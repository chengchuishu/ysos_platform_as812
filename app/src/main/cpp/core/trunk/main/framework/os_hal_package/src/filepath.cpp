/**
 *@file FilePath.cpp
 *@brief Definition of FilePath
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-21 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

#include <stdexcept>  // add for linux

/// ysos private headers //  NOLINT
#include "../../../public/include/os_hal_package/filepath.h" //NOLINT
#include "../../../protect/include/os_hal_package/filepathimpl.h" //NOLINT
/// boost headers //  NOLINT
#include <boost/algorithm/string.hpp>

namespace ysos {
#define CHECK_FILE_NAME(filename) do {\
  if (filename.empty()) {\
  throw boost::filesystem::filesystem_error("invalid filename", boost::system::error_code());\
  }\
}while(0);

FilePath::FilePath(const std::string filename) {
  CHECK_FILE_NAME(filename)
  file_path_impl_ = new FilePathImpl(filename);
}

FilePath::~FilePath() {
  if (NULL != file_path_impl_) {
    delete file_path_impl_;
  }
}

void FilePath::ResetFileName(const std::string filename) {
  CHECK_FILE_NAME(filename)
  file_path_impl_->ResetFileName(filename);
}

bool FilePath::IsFileExist() {
  return file_path_impl_->IsFileExist();
}

bool FilePath::IsFileNameValid() {
  return file_path_impl_->IsFileNameValid();
}

bool FilePath::IsFile(void) {
  return file_path_impl_->IsFile();
}

bool FilePath::IsFile(const std::string &filename) {
  return FilePathImpl::IsFile(filename);
}

bool FilePath::IsDirectory(void) {
  return file_path_impl_->IsDirectory();
}

bool FilePath::IsDirectory(const std::string &dirname) {
  return FilePathImpl::IsDirectory(dirname);
}

FileType FilePath::GetFileStatus() {
  return file_path_impl_->GetFileStatus();
}

std::string FilePath::GetParentPath() {
  return file_path_impl_->GetParentPath();
}

std::string FilePath::GetStem() {
  return file_path_impl_->GetStem();
}

std::string FilePath::GetFilePath() {
  return file_path_impl_->GetFilePath();
}

std::string FilePath::GetFileExtention() {
  return file_path_impl_->GetFileExtention();
}

int FilePath::GetFileSize() {
  return file_path_impl_->GetFileSize();
}

void FilePath::RenameFile(const std::string &dstfilename) {
  CHECK_FILE_NAME(dstfilename)
  file_path_impl_->RenameFile(dstfilename);
}

void FilePath::RenameFile(const std::string &srcfilename, const std::string &dstfilename) { //NOLINT
  CHECK_FILE_NAME(srcfilename)
  CHECK_FILE_NAME(dstfilename)
  boost::filesystem::rename(srcfilename, dstfilename);
}

void FilePath::CopyFile(const std::string &dstfilename) {
  CHECK_FILE_NAME(dstfilename)
  file_path_impl_->CopyFile(dstfilename);
}

void FilePath::CopyFile(const std::string &srcfilename, const std::string &dstfilename) { //NOLINT
  CHECK_FILE_NAME(srcfilename)
  CHECK_FILE_NAME(dstfilename)
  boost::filesystem::copy_file(srcfilename, dstfilename);
}

bool FilePath::CreateFile(void) {
  return file_path_impl_->CreateFile();
}

bool FilePath::CreateFile(const std::string &filename) {
  CHECK_FILE_NAME(filename)
  return FilePathImpl::CreateFile(filename);
}

bool FilePath::RemoveFile(void) {
  return file_path_impl_->RemoveFile();
}

bool FilePath::RemoveFile(const std::string &filename) {
  CHECK_FILE_NAME(filename)
  if (!boost::filesystem::is_regular_file(filename)) {
    std::logic_error ex("unexpected error"); // add for linux
    throw std::exception(ex);   // need update for linux
  }

  return boost::filesystem::remove(filename);
}

bool FilePath::CreateDirectory(void) {
  return file_path_impl_->CreateDirectory();
}

bool FilePath::CreateDirectory(const std::string &dirname) {
  CHECK_FILE_NAME(dirname)
  return boost::filesystem::create_directory(dirname);
}

void FilePath::RemoveDirectory(void) {
  file_path_impl_->RemoveDirectory();
}

void FilePath::RemoveDirectory(const std::string &dirname) {
  CHECK_FILE_NAME(dirname)
  boost::filesystem::path dir_path = dirname;

  FilePathImpl::RemoveDirectory(dir_path);
}

std::string FilePath::GetCurPath() {
  return boost::filesystem::initial_path<boost::filesystem::path>().string();
}

SpaceInfo FilePath::GetCurDiskInfo() {
  return file_path_impl_->GetCurDiskInfo();
}

std::vector<std::string> FilePath::GetAllFiles(const std::string &dirname) {
  CHECK_FILE_NAME(dirname)
  std::vector<std::string> file_array;  //NOLINT
  if(!boost::filesystem::exists(dirname)) {
    return file_array;
  }

  boost::filesystem::directory_iterator end;
  for (boost::filesystem::directory_iterator pos(dirname); pos != end; ++pos) {
    if (boost::filesystem::is_regular_file(*pos)) {
      std::string file_path = pos->path().string();
      boost::replace_all(file_path, "\\", "/");
      file_array.push_back(file_path);  //NOLINT
    }
  }

  return file_array;
}
}
