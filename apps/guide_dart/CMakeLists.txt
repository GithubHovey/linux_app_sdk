

#pkgconfig
set(ENV{PKG_CONFIG_PATH} "${CMAKE_SYSROOT}/usr/lib/pkgconfig")
find_package(PkgConfig REQUIRED)

#日志系统
find_package(spdlog REQUIRED)
message(STATUS "spdlog include dirs: ${spdlog_INCLUDE_DIRS}")
message(STATUS "spdlog libraries: ${spdlog_LIBRARIES}")

add_executable(${PROJECT_NAME} 
        ${SRC}
        # ${AICHAT_SRC}
)
# rkmedia test
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
#     ${CMAKE_SOURCE_DIR}/third_party/rockchip/rkaiq/sample_common_isp.c
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
    # cjson
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
#VISION ENABLE
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
                                #  ${CMAKE_SOURCE_DIR}/src/hal/rkcapture.cpp
                                    ${CMAKE_SOURCE_DIR}/third_party/rockchip/rkaiq/sample_common_isp.c                                 
                                    )
        
        # include_directories( 
        #     ${CMAKE_SOURCE_DIR}/third_party/rockchip/rkaiq
        #     ${CMAKE_SYSROOT}/usr/include/easymedia
        #     ${CMAKE_SYSROOT}/usr/include/rkaiq/uAPI
        #     ${CMAKE_SYSROOT}/usr/include/rkaiq/xcore
        #     ${CMAKE_SYSROOT}/usr/include/rkaiq/common
        #     ${CMAKE_SYSROOT}/usr/include/rkaiq/algos
        #     ${CMAKE_SYSROOT}/usr/include/rkaiq/iq_parser
        #     ${CMAKE_SYSROOT}/usr/include/rkaiq/ipc_server
        # )

    endif()
    target_sources(${PROJECT_NAME} PRIVATE
        ${VISION_SRC}
    )
    # target_compile_definitions(${PROJECT_NAME} PRIVATE
    #     USE_RKMEDIA
    #     RKAIQ
    # )
    # target_link_libraries(${PROJECT_NAME} PRIVATE
    #     easymedia
    #     rkaiq
    # )
endif()
