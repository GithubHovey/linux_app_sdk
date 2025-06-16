# 简介
@hovey
# 工程目录介绍
项目根目录/
├── CMakeLists.txt          # 主构建文件
├── Kconfig                 # 配置选项定义
├── config/                 # 配置相关
│   ├── auto.conf           # 自动生成的配置
│   └── config.h.in         # 配置头文件模板
├── scripts/
│   ├── menuconfig.py       # 配置界面脚本
│   └── conf2cmake.py       # 配置转CMake变量
└── src/                    # 源代码目录
# 环境搭建
`kconfig-frontends` 是一个将 Linux 内核配置系统(Kconfig)独立出来的工具集，它允许在非内核项目中使用 Linux 内核风格的配置界面。本项目也使用Kconfig来配置编译环境。
```$ sudo apt-get install kconfig-frontends```
```mermaid
graph LR
    A[Kconfig界面] -->|用户配置| B(.config)
    B -->|转换脚本| C(toolchain.cmake)
    C -->|cmake -DCMAKE_TOOLCHAIN_FILE| D[构建系统]
```
# 编译步骤
``` $ ./build.sh menuconfig ``` 配置编译环境
``` $ ./build.sh ``` 编译工程
``` $ ./build.sh -pack ``` 打包固件到`pack/out`目录



