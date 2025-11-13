

#pkgconfig
set(ENV{PKG_CONFIG_PATH} "${CMAKE_SYSROOT}/usr/lib/pkgconfig")
find_package(PkgConfig REQUIRED)

#日志系统
find_package(spdlog REQUIRED)
message(STATUS "spdlog include dirs: ${spdlog_INCLUDE_DIRS}")
message(STATUS "spdlog libraries: ${spdlog_LIBRARIES}")

file(GLOB SRC ${CMAKE_SOURCE_DIR}/apps/${APP_NAME}/main/*.cpp
    ${CMAKE_SOURCE_DIR}/src/modules/*.cpp
    ${CMAKE_SOURCE_DIR}/src/drivers/*.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/*.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/log/*.cpp 
    )
include_directories( 
    ${CMAKE_SOURCE_DIR}/src/modules
    ${CMAKE_SOURCE_DIR}/src/drivers
    ${CMAKE_SOURCE_DIR}/src/hal
    ${CMAKE_SOURCE_DIR}/src/utils
)

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
    rga
)


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
                                )
    

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

