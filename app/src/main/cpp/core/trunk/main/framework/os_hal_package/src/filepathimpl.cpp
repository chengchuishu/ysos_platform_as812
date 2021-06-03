/**
 *@file FilePathImpl.cpp
 *@brief Definition of FilePathImpl
 *@version 0.1
 *@author dhongqian
 *@date Created on: 2016-04-21 13:59:20
 *@copyright Copyright (c) 2016 YunShen Technology. All rights reserved.
 * 
 */

#include <stdexcept>  //need add for linux
#include <fstream>    //need add for linux
/// ysos private headers //  NOLINT
#include "../../../protect/include/os_hal_package/filepathimpl.h"
/// stl headers //  NOLINT
#include <ostream>
/// boost headers //  NOLINT
#include <boost/filesystem.hpp>   //NOLINT

namespace ysos {
FilePathImpl::FilePathImpl(const std::string filename) {
  file_path_ = new boost::filesystem::path(filename);
}

FilePathImpl::~FilePathImpl() {
  if (NULL != file_path_) {
    delete file_path_;
  }
}

void FilePathImpl::ResetFileName(const std::string filename) {
  delete file_path_;
  file_path_ = new boost::filesystem::path(filename);
}

bool FilePathImpl::IsFileExist() {
  return boost::filesystem::exists(*file_path_);
}

bool FilePathImpl::IsFile(void) {
  return boost::filesystem::is_regular_file(*file_path_);
}

bool FilePathImpl::IsFile(const std::string &filename) {
  return boost::filesystem::is_regular_file(filename);
}

bool FilePathImpl::IsDirectory(void) {
  return boost::filesystem::is_directory(*file_path_);
}

bool FilePathImpl::IsDirectory(const std::string &dirname) {
  return boost::filesystem::is_directory(dirname);
}

bool FilePathImpl::IsFileNameValid() {
#ifdef _MSC_VER
  return boost::filesystem::windows_name((file_path_->filename().string()));
#else
  return boost::filesystem::portable_posix_name((file_path_->filename().c_str())); //need update for linux
#endif
}

boost::filesystem::file_type FilePathImpl::GetFileStatus() {
  return boost::filesystem::status(*file_path_).type();
}

std::string FilePathImpl::GetParentPath() {
  return file_path_->parent_path().string();
}

std::string FilePathImpl::GetStem() {
  return file_path_->stem().string();
}

std::string FilePathImpl::GetFilePath() {
  return file_path_->string();
}

std::string FilePathImpl::GetFileExtention() {
  return file_path_->extension().string();
}

int FilePathImpl::GetFileSize() {
  if (!boost::filesystem::is_regular_file(*file_path_)) {
    std::logic_error ex("unexpected error"); // add for linux
    throw std::exception(ex);   // need update for linux
  }

  return static_cast<int>(boost::filesystem::file_size(*file_path_));
}

void FilePathImpl::RenameFile(const std::string &dstfilename) {
  boost::filesystem::rename(*file_path_, dstfilename);
}

void FilePathImpl::CopyFile(const std::string &dstfilename) { //NOLINT
  boost::filesystem::copy_file(*file_path_, dstfilename);
}

bool FilePathImpl::CreateFile(void) {
  return FilePathImpl::CreateFile(file_path_->string());
}

bool FilePathImpl::CreateFile(const std::string &filename) {
  std::ofstream off;
  off.open(filename, std::ios_base::out);
  off.close();

  return true;
}

bool FilePathImpl::RemoveFile(void) {
  if (!boost::filesystem::is_regular_file(*file_path_)) {
    std::logic_error ex("unexpected error"); // add for linux
    throw std::exception(ex);   // need update for linux
  }

  return boost::filesystem::remove(*file_path_);
}

bool FilePathImpl::CreateDirectory(void) {
  return boost::filesystem::create_directory(*file_path_);
}

void FilePathImpl::RemoveDirectory(void) {
  RemoveDirectory(*file_path_);
}

void FilePathImpl::RemoveDirectory(const boost::filesystem::path &path) {
  if (!boost::filesystem::exists(path)) {
    return;
  }

  // 非文件夹，报错 //NOLINT
  if (!boost::filesystem::is_directory(path)) {
    std::logic_error ex("invalid directory name"); // add for linux
    throw std::exception(ex);  // need update for linux
  }

  if (boost::filesystem::is_empty(path)) {
    boost::filesystem::remove(path);
  } else {
    boost::filesystem::remove_all(path);
  }
}

SpaceInfo FilePathImpl::GetCurDiskInfo() {
  return boost::filesystem::space(*file_path_);
}
}