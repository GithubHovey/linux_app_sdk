cmake_minimum_required(VERSION 3.10)
include(${CMAKE_BINARY_DIR}/kconfig.cmake)

if("${CROSS_COMPILE_ENABLED}" STREQUAL "y")
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

    project(robot.exe)
    #pkgconfig
    set(ENV{PKG_CONFIG_PATH} "${CMAKE_SYSROOT}/usr/lib/pkgconfig")
    find_package(PkgConfig REQUIRED)
    #cjson
    # pkg_check_modules(CJSON REQUIRED libcjson)
    include_directories(
        ${CMAKE_SYSROOT}/usr/include/cjson
    )

    #日志系统
    find_package(spdlog REQUIRED)
    message(STATUS "spdlog include dirs: ${spdlog_INCLUDE_DIRS}")
    message(STATUS "spdlog libraries: ${spdlog_LIBRARIES}")
    #架构源码路径
    file(GLOB SRC ${CMAKE_SOURCE_DIR}/src/*.cpp
            ${CMAKE_SOURCE_DIR}/src/nodes/*.cpp
            ${CMAKE_SOURCE_DIR}/src/modules/*.cpp
            ${CMAKE_SOURCE_DIR}/src/drivers/*.cpp
            ${CMAKE_SOURCE_DIR}/src/drivers/**/*.cpp
            ${CMAKE_SOURCE_DIR}/src/hal/*.cpp
            ${CMAKE_SOURCE_DIR}/src/hal/*.c
            ${CMAKE_SOURCE_DIR}/src/utils/*.cpp
            ${CMAKE_SOURCE_DIR}/src/utils/*.c
    )
    #架构基本目录
    include_directories( 
        ${CMAKE_SOURCE_DIR}/src/nodes
        ${CMAKE_SOURCE_DIR}/src/modules
        ${CMAKE_SOURCE_DIR}/src/drivers
        ${CMAKE_SOURCE_DIR}/src/hal
        ${CMAKE_SOURCE_DIR}/src/utils
        ${CMAKE_SOURCE_DIR}/third_party
    )
    #GUI
    # pkg_check_modules(LIBDRM REQUIRED libdrm)
    include_directories(
        ${CMAKE_SYSROOT}/usr/include/libdrm
    )
    include_directories(      
            ${CMAKE_SOURCE_DIR}/src/modules/gui
            ${CMAKE_SOURCE_DIR}/third_party/lvgl_8.2
            ${CMAKE_SOURCE_DIR}/third_party/lvgl_8.2/lvgl
            ${CMAKE_SOURCE_DIR}/third_party/lvgl_8.2/lvgl/src
            ${CMAKE_SOURCE_DIR}/third_party/lvgl_8.2/lvgl/font
            ${CMAKE_SOURCE_DIR}/src/modules/gui/nxp_guiguider/custom 
            ${CMAKE_SOURCE_DIR}/src/modules/gui/nxp_guiguider/generated 
            ${CMAKE_SOURCE_DIR}/src/modules/gui/nxp_guiguider/generated/guider_customer_fonts 
            ${CMAKE_SOURCE_DIR}/src/modules/gui/nxp_guiguider/generated/guider_fonts
            ${CMAKE_SOURCE_DIR}/src/modules/gui/nxp_guiguider/generated/images 
    )
    add_subdirectory(${CMAKE_SOURCE_DIR}/third_party/lvgl_8.2/lvgl EXCLUDE_FROM_ALL)
    file(GLOB GUI_SRC ${CMAKE_SOURCE_DIR}/src/*.cpp
                ${CMAKE_SOURCE_DIR}/src/modules/gui/*.cpp
                ${CMAKE_SOURCE_DIR}/src/modules/gui/nxp_guiguider/custom/*.c
                ${CMAKE_SOURCE_DIR}/src/modules/gui/nxp_guiguider/generated/*.c
                ${CMAKE_SOURCE_DIR}/src/modules/gui/nxp_guiguider/generated/images/*.c
                ${CMAKE_SOURCE_DIR}/src/modules/gui/nxp_guiguider/generated/guider_fonts/*.c
    )
    #AICHAT ENABEL
    add_subdirectory(${CMAKE_SOURCE_DIR}/third_party/snowboy)
    include_directories( 
            ${CMAKE_SOURCE_DIR}/src/modules/aichat
            ${CMAKE_SOURCE_DIR}/src/drivers/audio/portaudio_wrapper
            ${CMAKE_SOURCE_DIR}/third_party/snowboy/include
        )
    file(GLOB AICHAT_SRC
                ${CMAKE_SOURCE_DIR}/src/modules/aichat/*.cpp
                ${CMAKE_SOURCE_DIR}/src/drivers/audio/portaudio_wrapper/*.cpp
                
    )

    add_executable(${PROJECT_NAME} 
            ${SRC}
            ${GUI_SRC}
            ${AICHAT_SRC}
    )
    # debug
    add_compile_options(-fPIC -Wall -O3 -g0)
    # warnning
    add_compile_options(
        -Wall -Wshadow -Wundef -Wmissing-prototypes -Wno-discarded-qualifiers -Wall -Wextra -Wno-unused-function -Wno-error=strict-prototypes -Wpointer-arith 
        -fno-strict-aliasing -Wno-error=cpp -Wuninitialized -Wmaybe-uninitialized -Wno-unused-parameter -Wno-missing-field-initializers -Wtype-limits -Wsizeof-pointer-memaccess 
        -Wno-format-nonliteral -Wno-cast-qual -Wunreachable-code -Wno-switch-default -Wreturn-type -Wmultichar -Wformat-security -Wno-ignored-qualifiers -Wno-error=pedantic 
        -Wno-sign-compare -Wno-error=missing-prototypes -Wdouble-promotion -Wclobbered -Wdeprecated -Wempty-body -Wtype-limits -Wshift-negative-value -Wstack-usage=2048 
        -Wno-unused-value -Wno-unused-parameter -Wno-missing-field-initializers -Wuninitialized -Wmaybe-uninitialized -Wall -Wextra -Wno-unused-parameter 
        -Wno-missing-field-initializers -Wtype-limits -Wsizeof-pointer-memaccess -Wno-format-nonliteral -Wpointer-arith -Wno-cast-qual -Wmissing-prototypes 
        -Wunreachable-code -Wno-switch-default -Wreturn-type -Wmultichar -Wno-discarded-qualifiers -Wformat-security -Wno-ignored-qualifiers -Wno-sign-compare -std=c99
    )
    #link
    target_link_libraries(${PROJECT_NAME}
        PRIVATE lvgl
        PRIVATE spdlog::spdlog
        pthread
        # ${LIBDRM_LIBRARIES}
        drm
        cjson
        portaudio
        snowboy-detect-c-wrapper
    )
else()
    message(FATAL_ERROR "Cross-compilation toolchain not properly configured. Please set up the toolchain before proceeding.")
endif()