/*
# config.h
# Created on: 2016-04-26 13:59:20
# Original author: XuLanyue
# Copyright (c) 2016 YunShen Technology. All rights reserved.
# 
# Can be included by OsHalPackage only.
*/

#ifndef OHP_CONFIG_H        //NOLINT
#define OHP_CONFIG_H        //NOLINT
#define PLATFORM_WINDOWS                1
#define PLATFORM_LINUX                  2
#define PLATFORM_ANDROID                3

#define PLATFORM_PHASE_DEBUG            1
#define PLATFORM_PHASE_DEBUG_RELEASE    2
#define PLATFORM_PHASE_RELEASE          3

#define CURRENT_PLATFORM_PHASE          PLATFORM_PHASE_DEBUG

#ifdef WIN32
#define CURRENT_PLATFORM    PLATFORM_WINDOWS
else
#define CURRENT_PLATFORM    PLATFORM_LINUX    // new add fro linux
#endif


#if (CURRENT_PLATFORM == PLATFORM_WINDOWS)
#pragma warning(disable: 4250)          //  dominance
#pragma warning(disable: 4996)          //  std::_Fill_n
//  void* YSOSMAlloc_(int nSize) {
//    return GlobalAlloc(GPTR, nSize);
//  }
//  virtual void YSOSFree_(void* p) {
//    GlobalFree((HGLOBAL)p);
//  }
#endif

//#ifndef CURRENT_PLATFORM
//#define CURRENT_PLATFORM
//#endif

//#if (CURRENT_PLATFORM == PLATFORM_LINUX)
//
//#endif
//#if (CURRENT_PLATFORM == PLATFORM_ANDROID)
//
//#endif

#endif  // OHP_CONFIG_H     //NOLINT
