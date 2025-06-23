#!/bin/bash

# Configuration
SCRIPT_DIR=$(cd "$(dirname "$(readlink -f "$0")")" && pwd)
# LUCKFOX_SDK_PATH="/home/hovey/workspace/luckfox/sdk/luckfox-pico"

# Default flags
cross_compile_flag=ON
pack_flag=OFF

# Functions
usage() {
  echo "Usage: $0 [-s] [-p] [-m]"
  echo "Options:"
  echo "  -s    Compile for simulator (Ubuntu system)"
  echo "  -p    Package only (no compilation)"
  echo "  -m    Launch Kconfig menu configuration"
  echo ""
  echo "Default behavior (no flags):"
  echo "  - Creates build directory"
  echo "  - Generates kconfig.cmake"
  echo "  - Performs cross-compilation for rv1106"
  exit 1
}


local_compile() {
  # mkdir -p "$SIMULATOR_DIR"
  # cd "$SIMULATOR_DIR" || exit 1
  echo "Compiling for Ubuntu system..."
  # cmake -DUSE_GUI_GUIDER=ON ..
  # make -j4
}

cross_compile() {
  echo "Compiling for rv1106..."
#   export LUCKFOX_SDK_PATH="$LUCKFOX_SDK_PATH"
  cmake ..
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
  #   exit 1
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

create_build_dir()
{
  BUILD_DIR=$(grep '^CONFIG_BUILD_DIR_NAME=' .config 2>/dev/null | cut -d'"' -f2)
  mkdir -p "$BUILD_DIR"
  cd "$BUILD_DIR" || exit 1
}

kconfig2cmake()
{
  {
    echo "# Generated from ${SCRIPT_DIR}/.config at $(date +'%Y-%m-%d %H:%M:%S')"
    grep '^CONFIG_' ${SCRIPT_DIR}/.config | grep -v '^CONFIG_BUILD_DIR_NAME=' | \
        sed -E 's/^CONFIG_([^=]+)="?([^"]*)"?$/set(\1 "\2")/'
  } > "${SCRIPT_DIR}/$BUILD_DIR/kconfig.cmake"

  if [ -f "${SCRIPT_DIR}/$BUILD_DIR/kconfig.cmake" ]; then
    info "Generated: ${SCRIPT_DIR}/$BUILD_DIR/kconfig.cmake"
  else
    error "Error: ${SCRIPT_DIR}/$BUILD_DIR/kconfig.cmake was not generated"
    exit 1
  fi
}

# Main execution
main() {
    # Parse arguments
    
    while getopts "spm" opt; do
    case $opt in
        s) 
            cross_compile_flag=OFF
            # Execute only -s specific logic
            echo "compile for simulator."
            exit 0
            ;;
        p) 
            # Execute only packaging
            package
            exit 0
            ;;
        m)
            kconfig-mconf ${SCRIPT_DIR}/Kconfig
            exit 0
            ;;
        *) usage ;;
    esac
    done

    # Default execution (no flags)
    create_build_dir 
    kconfig2cmake
    # bash ${SCRIPT_DIR}/scripts/kconfig2cmake.sh ${SCRIPT_DIR}/.config
    cross_compile
}

main "$@"