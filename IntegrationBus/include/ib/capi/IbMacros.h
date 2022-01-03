// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#ifdef __cplusplus
#define IB_BEGIN_DECLS extern "C" {
#define IB_END_DECLS }
#else
#define IB_BEGIN_DECLS 
#define IB_END_DECLS 
#endif

// define compiler specific export / import attributes
#ifdef EXPORT_IntegrationBusAPI
// define IntegrationBusAPI as EXPORT
#    if defined(_MSC_VER)
#        define IntegrationBusAPI __declspec(dllexport)
#    elif defined(__GNUC__)
#        define IntegrationBusAPI __attribute__((visibility("default")))
#    else
#        define IntegrationBusAPI
#        pragma warning Unknown dynamic link export semantics.
#    endif
#else
// declare IntegrationBusAPI as IMPORT
#    if defined(_MSC_VER)
#        define IntegrationBusAPI __declspec(dllimport)
#    elif defined(__GNUC__)
#        define IntegrationBusAPI
#    else
#        define IntegrationBusAPI
#        pragma warning Unknown dynamic link import semantics.
#    endif
#endif  