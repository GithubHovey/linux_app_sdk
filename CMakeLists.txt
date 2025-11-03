cmake_minimum_required(VERSION 3.10)
include(${CMAKE_BINARY_DIR}/kconfig.cmake)

if("${CROSS_COMPILE_ENABLED}" STREQUAL "y")# 是否交叉编译
    # Check and set sysroot
    if(NOT EXISTS ${CROSS_COMPILER_SYSROOT_PATH})
        message(FATAL_ERROR "Sysroot path does not exist: ${CROSS_COMPILER_SYSROOT_PATH}")
    endif()
    set(CMAKE_SYSROOT ${CROSS_COMPILER_SYSROOT_PATH})
    set(CMAKE_FIND_ROOT_PATH ${CMAKE_SYSROOT})  # 将 SYSROOT 设为搜索根路径
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)  # 只在 ROOT_PATH 下搜索头文件
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)  # 只在 ROOT_PATH 下搜索库
    message(STATUS "Sysroot Path: ${CMAKE_SYSROOT}")

    # Check and set C compiler
    if(NOT EXISTS ${CROSS_COMPILER_C_PATH})
        message(FATAL_ERROR "C compiler path does not exist: ${CROSS_COMPILER_C_PATH}")
    endif()
    set(CMAKE_C_COMPILER ${CROSS_COMPILER_C_PATH})
    message(STATUS "C Compiler Path: ${CMAKE_C_COMPILER} (verified)")

    # Check and set C++ compiler
    if(NOT EXISTS ${CROSS_COMPILER_CPP_PATH})
        message(FATAL_ERROR "C++ compiler path does not exist: ${CROSS_COMPILER_CPP_PATH}")
    endif()
    set(CMAKE_CXX_COMPILER ${CROSS_COMPILER_CPP_PATH})
    message(STATUS "C++ Compiler Path: ${CMAKE_CXX_COMPILER} (verified)")

    set(CMAKE_CXX_STANDARD 14)#c++14
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_C_STANDARD 99)#c99
    set(CMAKE_C_STANDARD_REQUIRED ON)

    add_compile_options(-fPIC -O3 -g0)

    project(${APP_NAME}.exe)
    
    add_subdirectory(apps)
  

else()
    message(FATAL_ERROR "Cross-compilation toolchain not properly configured. Please set up the toolchain before proceeding.")
endif()