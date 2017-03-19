//
// Copyright (c) Mario Garcia, MIT License.
//
#ifndef __PLATFORM_HPP
#define __PLATFORM_HPP

#if defined(PBR_MINIMUM_VERSION) && defined(PBR_CURRENT_VERSION)
 #if defined(_WIN32)
  #define WIN32_LEAN_AND_MEAN
  #include <Windows.h>
 #elif defined(__linux__) || defined(__POSIX__)
  #error "Temporary linux ban... Windows is only supported for now..."
 #elif defined(__APPLE__) && defined(__MACH__)
  #error "Mac OSX is not a supported operating system yet! I just dont own a mac to test..."
 #else
  #error "Unknown operating system detected."
 #endif 
#else
 #error "pbr version does not exist"
#endif 

#endif // __PLATFORM_HPP