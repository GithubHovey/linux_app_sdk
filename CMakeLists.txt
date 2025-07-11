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

    add_executable(${PROJECT_NAME} 
            ${SRC}
            ${AICHAT_SRC}
    )
    #rkmedia test
    # include_directories( 
    #     ${CMAKE_SYSROOT}/usr/include/easymedia
    #     ${CMAKE_SYSROOT}/usr/include/rkaiq/uAPI
    #     ${CMAKE_SYSROOT}/usr/include/rkaiq/xcore
    #     ${CMAKE_SYSROOT}/usr/include/rkaiq/common
    #     ${CMAKE_SYSROOT}/usr/include/rkaiq/algos
    #     ${CMAKE_SYSROOT}/usr/include/rkaiq/iq_parser
    #     ${CMAKE_SYSROOT}/usr/include/rkaiq/ipc_server
    #     ${CMAKE_SOURCE_DIR}/src/nodes/test/
    # )

    # file(GLOB RKMEDIA_TEST_SRC
    #     ${CMAKE_SOURCE_DIR}/src/nodes/test/rkmedia_vi_rga_test.c
    #     ${CMAKE_SOURCE_DIR}/src/nodes/test/sample_common_isp.c
    # )
    # add_executable(rkmedia_test ${RKMEDIA_TEST_SRC})
    # target_link_libraries(rkmedia_test PRIVATE
    #     easymedia
    #     rkaiq
    #     pthread
    # )
    # target_compile_definitions(rkmedia_test PRIVATE
    # RKAIQ
    # )

    target_link_libraries(${PROJECT_NAME} PRIVATE
        pthread
        PRIVATE spdlog::spdlog
        cjson
        yaml-cpp
    )
#GUI
    if("${USE_MODULE_GUI}" STREQUAL "y")
        pkg_check_modules(LIBDRM REQUIRED libdrm)
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
                    ${CMAKE_SOURCE_DIR}/src/hal/drm.c
                    ${CMAKE_SOURCE_DIR}/src/hal/fbdev.c
                    ${CMAKE_SOURCE_DIR}/src/hal/evdev.c
        )
        target_link_libraries(${PROJECT_NAME} PRIVATE
            lvgl
            drm
        )
        target_sources(${PROJECT_NAME} PRIVATE
            ${GUI_SRC}
        )
    endif()

#AICHAT ENABEL
    if("${USE_MODULE_AICHAT}" STREQUAL "y")
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
        target_link_libraries(${PROJECT_NAME} PRIVATE
            portaudio
            snowboy-detect-c-wrapper
        )
        target_sources(${PROJECT_NAME} PRIVATE
            ${AICHAT_SRC}
        )
    endif()

    if("${USE_MODULE_VISION}" STREQUAL "y")
        file(GLOB VISION_SRC
            ${CMAKE_SOURCE_DIR}/src/modules/vision/*.cpp
            ${CMAKE_SOURCE_DIR}/src/drivers/camera/*.cpp
            

        )
        include_directories( 
                ${CMAKE_SOURCE_DIR}/src/modules/vision
                ${CMAKE_SOURCE_DIR}/src/drivers/camera
            )
        if("${USE_V4L2}" STREQUAL "y")
            list(APPEND VISION_SRC ${CMAKE_SOURCE_DIR}/src/hal/v4l2_capture.cpp
                                     ${CMAKE_SOURCE_DIR}/src/hal/rkcapture.cpp
                                     ${CMAKE_SOURCE_DIR}/third_party/rockchip/rkaiq/sample_common_isp.c                                 
                                     )
            
            include_directories( 
                ${CMAKE_SOURCE_DIR}/third_party/rockchip/rkaiq
                ${CMAKE_SYSROOT}/usr/include/easymedia
                ${CMAKE_SYSROOT}/usr/include/rkaiq/uAPI
                ${CMAKE_SYSROOT}/usr/include/rkaiq/xcore
                ${CMAKE_SYSROOT}/usr/include/rkaiq/common
                ${CMAKE_SYSROOT}/usr/include/rkaiq/algos
                ${CMAKE_SYSROOT}/usr/include/rkaiq/iq_parser
                ${CMAKE_SYSROOT}/usr/include/rkaiq/ipc_server
            )

        endif()
        target_sources(${PROJECT_NAME} PRIVATE
            ${VISION_SRC}
        )
        target_compile_definitions(${PROJECT_NAME} PRIVATE
            USE_RKMEDIA
            RKAIQ
        )
        target_link_libraries(${PROJECT_NAME} PRIVATE
            easymedia
            rkaiq
        )
    endif()

else()
    message(FATAL_ERROR "Cross-compilation toolchain not properly configured. Please set up the toolchain before proceeding.")
endif()

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