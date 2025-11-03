#!/bin/bash

# Configuration
SCRIPT_DIR=$(cd "$(dirname "$(readlink -f "$0")")" && pwd)

# Default flags
cross_compile_flag=ON
pack_flag=OFF
app_name=""
debug_flag=OFF

# Functions
usage() {
  echo "Usage: $0 [-s | -p | -m | --clean | -app app_name [-d]]"
  echo "Options:"
  echo "  -s           Compile for simulator (Ubuntu system,独立模式)"
  echo "  -p           Package only (no compilation,独立模式)"
  echo "  -m           Launch Kconfig menu configuration (独立模式)"
  echo "  --clean      删除build目录下的所有内容 (独立模式)"
  echo "  -app name    指定要编译的app名称 (必须)"
  echo "  -d           添加debug符号 (仅与-app一起使用)"
  echo ""
  echo "示例: $0 -app demo_main     # 编译demo_main应用"
  echo "      $0 -app demo_main -d # 编译demo_main应用并带debug符号"
  echo "      $0 -clean       # 清理build目录"
  exit 1
}

local_compile() {
  echo "Compiling for Ubuntu system..."
  # ...existing code...
}

cross_compile() {
  echo "Compiling for rv1106..."
  cmake_args="-DAPP_NAME=${app_name}"
  if [ "$debug_flag" = "ON" ]; then
    cmake_args="$cmake_args -DCMAKE_BUILD_TYPE=Debug"
  fi
  echo "cmake ../.. $cmake_args"
  cmake ../.. $cmake_args
  make -j4
}

package() {
  echo "Packaging..."
  local pack_dir="pack"
  local out_dir="$pack_dir/out"  # Changed output directory
  local app_dir="$pack_dir/app"
  local commit_hash=$(git rev-parse --short HEAD 2>/dev/null || echo "nogit")
  if [ -n "$(git status --porcelain 2>/dev/null)" ]; then
    commit_hash="${commit_hash}-dirty"
  fi
  # Create directories
  mkdir -p "$app_dir" "$out_dir"  # Added out_dir creation
  
  # Copy binary
  if [ -f "build/robot.exe" ]; then
    cp "build/robot.exe" "$app_dir/"
  else
    echo "Error: Binary not found at build/robot.exe"
    exit 1
  fi
  
  # # Copy GUI config
  # local gui_config="src/modules/gui/gui_guider_480x480/custom/lvgl_config"
  # if [ -d "$gui_config" ]; then
  #   cp -r "$gui_config" "$app_dir/"
  # else
  #   echo "Error: GUI config not found at $gui_config"
  # fi

  # Copy script directory contents
  if [ -d "$pack_dir/script" ]; then
    cp -r "$pack_dir/script/"* "$app_dir/"  # Added script copy
  else
    echo "Warning: Script directory not found at $pack_dir/script"
  fi
  
  # Create tar archive with commit hash in out directory
  cd "$pack_dir" || exit 1
  tar -cvf "out/app_${commit_hash}.tar" app/  # Changed output path
  echo "Package created at $out_dir/app_${commit_hash}.tar"
}

# 新的 create_build_dir 实现
create_build_dir() {
  BUILD_DIR="build/${app_name}"
  mkdir -p "$BUILD_DIR"
  cd "$BUILD_DIR" || exit 1
}

kconfig2cmake()
{
  {
    echo "# Generated from ${SCRIPT_DIR}/.config at $(date +'%Y-%m-%d %H:%M:%S')"
    grep '^CONFIG_' ${SCRIPT_DIR}/.config | grep -v '^CONFIG_BUILD_DIR_NAME=' | \
        sed -E 's/^CONFIG_([^=]+)="?([^\"]*)"?$/set(\1 "\2")/'
  } > "${SCRIPT_DIR}/build/${app_name}/kconfig.cmake"

  if [ -f "${SCRIPT_DIR}/build/${app_name}/kconfig.cmake" ]; then
    echo "Generated: ${SCRIPT_DIR}/build/${app_name}/kconfig.cmake"
  else
    echo "Error: ${SCRIPT_DIR}/build/${app_name}/kconfig.cmake was not generated"
    exit 1
  fi
}

# 清理build目录函数
clean_build() {
  echo "清理build目录..."
  if [ -d "build" ]; then
    echo "删除build目录及其所有内容..."
    rm -rf build
    echo "清理完成"
  else
    echo "build目录不存在，无需清理"
  fi
}

# Main execution
main() {
    # Parse arguments
    if [ $# -eq 0 ]; then
      usage
    fi

    # 独立模式参数优先级
    if [[ "$1" == "-s" ]]; then
      local_compile
      exit 0
    fi
    if [[ "$1" == "-p" ]]; then
      package
      exit 0
    fi
    if [[ "$1" == "-m" ]]; then
      kconfig-mconf ${SCRIPT_DIR}/Kconfig
      exit 0
    fi
    if [[ "$1" == "-clean" ]]; then
      clean_build
      exit 0
    fi

    # 编译模式，必须有-app
    while [[ $# -gt 0 ]]; do
      case $1 in
        -app)
          shift
          if [ -z "$1" ]; then
            echo "Error: -app 后必须跟app名称"
            usage
          fi
          app_name="$1"
          ;;
        -d)
          debug_flag=ON
          ;;
        *)
          echo "未知参数: $1"
          usage
          ;;
      esac
      shift
    done

    if [ -z "$app_name" ]; then
      echo "Error: 必须指定-app app_name"
      usage
    fi

    create_build_dir
    kconfig2cmake
    cross_compile
}

main "$@"