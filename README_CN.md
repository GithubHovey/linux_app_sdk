# 简介
@hovey
# 工程目录介绍
项目根目录/
├── apps                        // 应用程序目录
│   ├── CMakeLists.txt          // apps层CMAKELISTS.txt
│   ├── demo_main               // 框架参考demo
│   ├── camera_calibration      // 相机标定程序
│   ├── guide_dart              // 比赛程序
│   └── nfc_upgrade             // NFC参数升级程序
├── build                       // 编译输出目录
│   └── demo_main
├── build.sh                   // 构建脚本
├── CMakeLists.txt             // 根目录CMAKELISTS.txt
├── Kconfig                    // menuconfig规则配置
├── pack                       // 打包目录
│   ├── app                    // 打包目标目录
│   ├── config                 // 配置文件
│   ├── out                    // 打包输出目录
│   └── script                 // 常用脚本，如kernel_upgrade.sh
├── README_CN.md
├── README.md
├── src                       // 自研框架源代码
│   ├── drivers               // 驱动程序目录
│   ├── hal                   // 硬件抽象层目录
│   ├── modules               // 模块目录
│   └── utils                 // 工具目录，如日志，ringbuf实现
└── third_party               // 第三方库目录，这部分代码不推荐改动，保持三方库的原始代码
    ├── lvgl_8.2
    ├── rockchip
    └── snowboy
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

``` $ ./build.sh -m``` 配置编译环境，请务必配置好自己的交叉编译路径，否则会编译失败
``` $ ./build.sh -app <app1_name>``` 构建app1应用程序
``` $ ./build.sh -app <app2_name>``` 构建app2应用程序
``` $ ./build.sh -pack ``` 打包固件到`pack/out`目录
# 运行程序
1. 连接主板到PC，abd或者ssh都可以
2. 将打包好的文件`pack/out/<app_name>.tar.gz`拷贝到主板上
3. 解压文件`tar -xzvf <app_name>.tar.gz`
4. 运行应用程序，如`./app.sh start`
5


