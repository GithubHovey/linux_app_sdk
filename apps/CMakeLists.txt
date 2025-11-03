# apps/CMakeLists.txt
message(STATUS "=== Configuring Applications ===")

if(NOT DEFINED APP_NAME)
    message(FATAL_ERROR "APP_NAME 未定义，请通过 -DAPP_NAME=xxx 指定要编译的应用！")
    return()
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${APP_NAME}/CMakeLists.txt")
    message(STATUS "正在编译应用: ${APP_NAME}")
    # 既包含 src 下的源码，也包含当前 app 的 main 目录下的 .cpp 文件
    file(GLOB SRC ${CMAKE_CURRENT_SOURCE_DIR}/${APP_NAME}/main/*.cpp
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
    add_subdirectory(${APP_NAME})
else()
    message(FATAL_ERROR "未找到 ${APP_NAME} 目录或 CMakeLists.txt，无法添加子项目！")
    return()
endif()