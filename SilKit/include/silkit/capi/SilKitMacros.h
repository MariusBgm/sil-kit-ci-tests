// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#ifdef __cplusplus
#define SILKIT_BEGIN_DECLS extern "C" {
#define SILKIT_END_DECLS }
#else
#define SILKIT_BEGIN_DECLS 
#define SILKIT_END_DECLS 
#endif

#ifdef SILKIT_BUILD_STATIC
#    define SilKitAPI
# elif defined(EXPORT_SilKitAPI)
// define compiler specific export / import attributes
// define SilKitAPI as EXPORT
#    if defined(_WIN32)
#        define SilKitAPI __declspec(dllexport)
#    elif defined(__GNUC__)
#        define SilKitAPI __attribute__((visibility("default")))
#    else
#        define SilKitAPI
#        pragma warning Unknown dynamic link export semantics.
#    endif
#else
// declare SilKitAPI as IMPORT
#    if defined(_WIN32)
#        define SilKitAPI __declspec(dllimport)
#    elif defined(__GNUC__)
#        define SilKitAPI
#    else
#        define SilKitAPI
#        pragma warning Unknown dynamic link import semantics.
#    endif
#endif  

// Utilities for more readable definitions
#ifndef BIT 
#    define BIT(X) (1 << (X))
#endif
