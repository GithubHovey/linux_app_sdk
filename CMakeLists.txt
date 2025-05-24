cmake_minimum_required(VERSION 3.10)

set(SDK_PATH $ENV{LUCKFOX_SDK_PATH})
set(GLIBC_COMPILER $ENV{GLIBC_COMPILER})
# set(SYSROOT "${SDK_PATH}/sysdrv/source/buildroot/buildroot-2023.02.6/output/host/arm-buildroot-linux-uclibcgnueabihf/sysroot")
set(CMAKE_SYSROOT ${SDK_PATH}/sysdrv/source/buildroot/buildroot-2023.02.6/output/host/arm-buildroot-linux-uclibcgnueabihf/sysroot)
if(NOT SDK_PATH)
    message(WARNING "Please Set Luckfox-pico SDK Path. Such as:export LUCKFOX_SDK_PATH=/home/user/luckfox-pico")
    if (NOT GLIBC_COMPILER) 
        message(WARNING "Please Set arm32 glibc compiler Path. Such as:export GLIBC_COMPILER=/opt/arm-linux-gnueabihf/bin/arm-linux-gnueabihf-")
        return()
    else()
        set(CMAKE_C_COMPILER    "${GLIBC_COMPILER}gcc")
        set(CMAKE_CXX_COMPILER  "${GLIBC_COMPILER}g++")
        option(SYSTEM_UBUNTU "Compiled for Ubuntu systems" ON)
    endif()
else()
    set(CMAKE_C_COMPILER "${SDK_PATH}/tools/linux/toolchain/arm-rockchip830-linux-uclibcgnueabihf/bin/arm-rockchip830-linux-uclibcgnueabihf-gcc")
    set(CMAKE_CXX_COMPILER "${SDK_PATH}/tools/linux/toolchain/arm-rockchip830-linux-uclibcgnueabihf/bin/arm-rockchip830-linux-uclibcgnueabihf-g++")
endif()

project(robot.exe)

if(SYSTEM_UBUNTU)
    include_directories( 
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/include/glibc/drm
        ${CMAKE_SOURCE_DIR}/include/glibc/drm/libdrm
        ${CMAKE_SOURCE_DIR}/include/glibc/cjson
        ${CMAKE_SOURCE_DIR}/lib
        ${CMAKE_SOURCE_DIR}/lib/lvgl
        ${CMAKE_SOURCE_DIR}/lib/lv_drivers
        ${CMAKE_SOURCE_DIR}/custom 
        ${CMAKE_SOURCE_DIR}/generated 
        ${CMAKE_SOURCE_DIR}/generated/guider_customer_fonts 
        ${CMAKE_SOURCE_DIR}/generated/guider_fonts
        ${CMAKE_SOURCE_DIR}/generated/images 
        ${CMAKE_SOURCE_DIR}/lib/AIchat

        ${CMAKE_SOURCE_DIR}/src/nodes
        ${CMAKE_SOURCE_DIR}/src/modules
        ${CMAKE_SOURCE_DIR}/src/drivers
        ${CMAKE_SOURCE_DIR}/src/hal
        ${CMAKE_SOURCE_DIR}/src/utils
        ${CMAKE_SOURCE_DIR}/third_party
    )
    link_directories(${CMAKE_SOURCE_DIR}/lib/glibc/libdrm
                     ${CMAKE_SOURCE_DIR}/lib/glibc/libcjson)
    set(CMAKE_INSTALL_RPATH "$ORIGIN/lib")
else()
    include_directories( 
        ${CMAKE_SOURCE_DIR}/src/nodes
        ${CMAKE_SOURCE_DIR}/src/modules
        ${CMAKE_SOURCE_DIR}/src/drivers
        ${CMAKE_SOURCE_DIR}/src/hal
        ${CMAKE_SOURCE_DIR}/src/utils
        ${CMAKE_SOURCE_DIR}/third_party
        # ${CMAKE_SOURCE_DIR}/include
        # ${CMAKE_SOURCE_DIR}/include/uclibc/drm
        # ${CMAKE_SOURCE_DIR}/include/uclibc/cjson
        # ${CMAKE_SOURCE_DIR}/lib
        # ${CMAKE_SOURCE_DIR}/lib/lvgl
        # ${CMAKE_SOURCE_DIR}/lib/lv_drivers
        # ${CMAKE_SOURCE_DIR}/custom 
        # ${CMAKE_SOURCE_DIR}/generated 
        # ${CMAKE_SOURCE_DIR}/generated/guider_customer_fonts 
        # ${CMAKE_SOURCE_DIR}/generated/guider_fonts
        # ${CMAKE_SOURCE_DIR}/generated/images 
        # ${CMAKE_SOURCE_DIR}/lib/AIchat
    )
    # link_directories(${CMAKE_SOURCE_DIR}/lib/uclibc/libdrm
    #                  ${CMAKE_SOURCE_DIR}/lib/uclibc/libcjson)
    SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE) 
    SET(CMAKE_INSTALL_RPATH "/usr/lib")
endif()

# lvgl
# add_subdirectory(${CMAKE_SOURCE_DIR}/lib/lvgl EXCLUDE_FROM_ALL)
# add_subdirectory(${CMAKE_SOURCE_DIR}/lib/lv_drivers EXCLUDE_FROM_ALL)



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
#if GUI ENABEL

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
# message(STATUS "SRC = ${SRC}")  # 打印为 CMake 状态信息（带 -- 前缀）
# message(STATUS "GUI_SRC = ${GUI_SRC}")  # 打印为 CMake 状态信息（带 -- 前缀）


#AICHAT ENABEL
add_subdirectory(${CMAKE_SOURCE_DIR}/third_party/snowboy)

# set(SNOWBOY_LIBRARY "${CMAKE_CURRENT_SOURCE_DIR}/third_party/snowboy/lib/rpi/libsnowboy-detect.a")
include_directories( 
        ${CMAKE_SOURCE_DIR}/src/modules/aichat
        ${CMAKE_SOURCE_DIR}/src/drivers/audio/portaudio_wrapper
        ${CMAKE_SOURCE_DIR}/third_party/snowboy/include
    )
file(GLOB AICHAT_SRC
            ${CMAKE_SOURCE_DIR}/src/modules/aichat/*.cpp
            ${CMAKE_SOURCE_DIR}/src/drivers/audio/portaudio_wrapper/*.cpp
            
)



file(GLOB MCU_COM_SRC
            ${CMAKE_SOURCE_DIR}/src/modules/aichat/*.cpp
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

if(SYSTEM_UBUNTU)
    target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/glibc/drm
        ${CMAKE_CURRENT_SOURCE_DIR}/include/glibc/cjson
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/lib
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/lvgl/src
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/lvgl/font
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/communication
    )
else()
    target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/uclibc/drm
        ${CMAKE_CURRENT_SOURCE_DIR}/include/uclibc/cjson
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/lib
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/lvgl/src
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/lvgl/font
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/communication
    )
endif()

target_link_libraries(${PROJECT_NAME}
    PRIVATE lvgl
    # PRIVATE lvgl::drivers
    pthread
    drm
    cjson
    portaudio
    snowboy-detect-c-wrapper
)

if(SYSTEM_UBUNTU) 
    set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/install/${PROJECT_NAME}")
    install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX})
    file(GLOB LIBCJSON_FILES "${CMAKE_CURRENT_SOURCE_DIR}/lib/glibc/libcjson/*.so*")  
    file(GLOB LIBDRM_FILES "${CMAKE_CURRENT_SOURCE_DIR}/lib/glibc/libdrm/*.so*")  
    install(FILES ${LIBCJSON_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)                    
    install(FILES ${LIBDRM_FILES}   DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)                    
endif()
