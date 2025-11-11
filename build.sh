#!/bin/bash

# Configuration
SCRIPT_DIR=$(cd "$(dirname "$(readlink -f "$0")")" && pwd)

# Default flags
cross_compile_flag=ON
pack_flag=OFF
app_name=""
debug_flag=OFF
all_flag=OFF

# Functions
usage() {
  echo "Usage: $0 [-s | -p | -m | -clean | -app app_name [-d] | -all [-d]]"
  echo "Options:"
  echo "  -s           Compile for simulator (Ubuntu system,独立模式)"
  echo "  -p           Package only (no compilation,独立模式)"
  echo "  -m           Launch Kconfig menu configuration (独立模式)"
  echo "  -clean      删除build目录下的所有内容 (独立模式)"
  echo "  -app name    指定要编译的app名称 (必须)"
  echo "  -d           添加debug符号 (仅与-app或-all一起使用)"
  echo "  -all         编译.config中CONFIG_APP_NAMES指定的所有应用"
  echo ""
  echo "示例: $0 -app demo_main     # 编译demo_main应用"
  echo "      $0 -app demo_main -d # 编译demo_main应用并带debug符号"
  echo "      $0 -all              # 编译所有配置的应用"
  echo "      $0 -all -d           # 编译所有配置的应用并带debug符号"
  echo "      $0 -clean             # 清理build目录"
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

# 从.config文件读取APP_NAMES配置
read_app_names_from_config() {
  if [ -f "${SCRIPT_DIR}/.config" ]; then
    local app_names_line=$(grep '^CONFIG_APP_NAMES=' "${SCRIPT_DIR}/.config")
    if [ -n "$app_names_line" ]; then
      # 提取引号内的内容，并去除引号
      local app_names=$(echo "$app_names_line" | sed -E 's/^CONFIG_APP_NAMES="([^"]*)"$/\1/')
      echo "$app_names"
    else
      echo ""
    fi
  else
    echo ""
  fi
}

# 批量编译所有应用
compile_all_apps() {
  local app_names=$(read_app_names_from_config)
  
  if [ -z "$app_names" ]; then
    echo "错误: 未在.config文件中找到CONFIG_APP_NAMES配置"
    exit 1
  fi
  
  echo "从.config文件中读取的应用列表: $app_names"
  
  # 用逗号分隔应用名称
  IFS=',' read -ra app_list <<< "$app_names"
    # 定义颜色代码
  local BLUE='\033[0;34m'
  local NC='\033[0m' # No Color
  echo -e "${BLUE}开始编译以下应用:${NC}"
  printf "${BLUE}%s${NC}\n" "${app_list[@]}"
  echo ""
  
  for app in "${app_list[@]}"; do
    app=$(echo "$app" | xargs)  # 去除前后空格
    if [ -n "$app" ]; then
      echo "=== 编译应用: $app ==="
      app_name="$app"
      create_build_dir
      kconfig2cmake
      cross_compile
      echo "=== 应用 $app 编译完成 ==="
      echo ""
      cd "$SCRIPT_DIR"  # 回到脚本目录，为下一个应用准备
    fi
  done
  
  echo "所有应用编译完成!"
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
  
  # 查找build目录下的所有exe文件...
  local exe_files=$(find build -name "*.exe" -type f)
  if [ -n "$exe_files" ]; then
    echo "找到以下exe文件:"
    echo "$exe_files"
    
    # 复制所有exe文件到app目录
    while IFS= read -r exe_file; do
      if [ -f "$exe_file" ]; then
        echo "拷贝: $exe_file -> $app_dir/"
        cp "$exe_file" "$app_dir/"
      fi
    done <<< "$exe_files"
  else
    echo "错误: 在build目录下未找到任何exe文件"
    exit 1
  fi
    # 拷贝pack/config/目录下的配置文件
  local config_dir="$pack_dir/config"
  if [ -d "$config_dir" ]; then
    echo "拷贝配置文件..."
    
    # 拷贝config.yaml
    if [ -f "$config_dir/config.yaml" ]; then
      echo "拷贝: $config_dir/config.yaml -> $app_dir/"
      cp "$config_dir/config.yaml" "$app_dir/"
    else
      echo "警告: 未找到 $config_dir/config.yaml"
    fi
    
    # logconf.yaml
    if [ -f "$config_dir/logconf.yaml" ]; then
      echo "拷贝: $config_dir/logconf.yaml -> $app_dir/"
      cp "$config_dir/logconf.yaml" "$app_dir/"
    else
      echo "警告: 未找到 $config_dir/logconf.yaml"
    fi
  else
    echo "警告: 配置目录 $config_dir 不存在"
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
    rm -rf build/*
    echo "清理完成"
  else
    echo "build目录不存在，无需清理"
  fi
}
check_and_create_app_dirs() {
  local app_names=$(read_app_names_from_config)
  
  if [ -z "$app_names" ]; then
    echo "警告: 未在.config文件中找到CONFIG_APP_NAMES配置，跳过应用目录检查"
    return 0
  fi
  
  echo "检查应用目录..."
  
  # 用逗号分隔应用名称
  IFS=',' read -ra app_list <<< "$app_names"
  
  # 定义颜色代码
  local BLUE='\033[0;34m'
  local GREEN='\033[0;32m'
  local YELLOW='\033[1;33m'
  local NC='\033[0m' # No Color
  
  echo -e "${BLUE}配置的应用列表: $app_names${NC}"
  
  for app in "${app_list[@]}"; do
    app=$(echo "$app" | xargs)  # 去除前后空格
    if [ -n "$app" ]; then
      local app_dir="apps/${app}"
      local cmake_file="${app_dir}/CMakeLists.txt"
      
      if [ -d "$app_dir" ]; then
        echo -e "${GREEN}✓ 应用目录已存在: $app_dir${NC}"
        
        # 检查CMakeLists.txt是否存在
        if [ -f "$cmake_file" ]; then
          echo -e "${GREEN}  ✓ CMakeLists.txt已存在${NC}"
        else
          echo -e "${YELLOW}  ⚠ 创建空的CMakeLists.txt${NC}"
          echo "# CMakeLists.txt for ${app}" > "$cmake_file"
          echo "# 自动生成于 $(date +'%Y-%m-%d %H:%M:%S')" >> "$cmake_file"
          echo "" >> "$cmake_file"
        fi
      else
        echo -e "${YELLOW}⚠ 创建应用目录: $app_dir${NC}"
        mkdir -p "$app_dir"
        
        # 创建main子目录
        local main_dir="${app_dir}/main"
        mkdir -p "$main_dir"
        echo -e "${GREEN}  ✓ 创建main目录: $main_dir${NC}"
        
        # 创建空的CMakeLists.txt
        echo -e "${YELLOW}  ⚠ 创建空的CMakeLists.txt${NC}"
        echo "# CMakeLists.txt for ${app}" > "$cmake_file"
        echo "# 自动生成于 $(date +'%Y-%m-%d %H:%M:%S')" >> "$cmake_file"
        echo "# file(GLOB SRC \"main/*.cpp\" \"main/*.c\")" >> "$cmake_file"
        echo "" >> "$cmake_file"
        echo "# add_executable(\${PROJECT_NAME} \${SRC})" >> "$cmake_file"
        echo "" >> "$cmake_file"
        
        echo -e "${GREEN}  ✓ 应用目录结构创建完成${NC}"
      fi
    fi
  done
  
  echo -e "${BLUE}应用目录检查完成${NC}"
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
      check_and_create_app_dirs
      exit 0
    fi
    if [[ "$1" == "-clean" ]]; then
      clean_build
      exit 0
    fi

    # 编译模式
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
        -all)
          all_flag=ON
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

    # 检查编译模式
    if [ "$all_flag" = "ON" ]; then
      if [ -n "$app_name" ]; then
        echo "错误: -all 和 -app 不能同时使用"
        usage
      fi
      compile_all_apps
    elif [ -n "$app_name" ]; then
      # 单个应用编译模式
      create_build_dir
      kconfig2cmake
      cross_compile
    else
      echo "Error: 必须指定-app app_name 或 -all"
      usage
    fi
}

main "$@"