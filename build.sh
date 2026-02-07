#!/usr/bin/env bash
BUILD_PATH="build"
SRC="../src"
EXE_NAME="boids"
CC="gcc"

CIMGUI_PATH=$(realpath  ./external/cimgui/)
CIMGUI_LIB="$CIMGUI_PATH"
RLIMGUI_PATH=$(realpath ./external/rlImGui)
RLIMGUI_LIB="$RLIMGUI_PATH/bin/Release"

build_rlimgui() {
    cd "$RLIMGUI_PATH" && 
        premake5 gmake && 
        make config=release_x64 rlImGui
}

build_cimgui() {
    cd "$CIMGUI_PATH" &&
        make static
}

build_external() {
    git submodule update --init --recursive
    (build_cimgui) && (build_rlimgui)
}

# debug flags
DEBUG=(
    "-g"
)

# compile time defines
DEFINES=(
    # -DBOIDS_DEBUG_DRAW
)

# linux platform libraries
LIBS=(
    "-L$CIMGUI_LIB"
    "-L$RLIMGUI_LIB"
    "-lraylib"
    "-lrlImGui"
    "-lcimgui"
    "-lm"
    "-lpthread"
    "-ldl"
    "-lX11"
    "-lGL"
    "-lrt"
    "-lstdc++"
)

# compiler flags
COMP_FLAGS=(
    "-I$(realpath ./external/cimgui/)"
    "-I$(realpath ./external/rlImGui/)"
    "-Wall"
    "-Wextra"
    "-Wpedantic"
    "-Werror"
    "-Og"
    "-std=c11"
)

# Build commands
EXE_CMD=("$CC" "${@}" "${DEFINES[@]}" "${DEBUG[@]}" "${COMP_FLAGS[@]}" \
         "-o" "$EXE_NAME" "$SRC/main.c" "${LIBS[@]}")

EXE_CMD_STR=$(IFS=' '; echo "${EXE_CMD[*]}")

# Create build directory if it doesn't exist
if [[ ! -d "$BUILD_PATH" ]]; then
    echo "Created Build Directory"
    mkdir -p "$BUILD_PATH"
fi

echo "=== Building dependencies ==="
if ! build_external; then
    echo "building dependencies failed"
    exit 1
fi

cd "$BUILD_PATH" || exit
echo "===== $EXE_NAME ====="
echo "$EXE_CMD_STR"
eval "$EXE_CMD_STR"
cd ..
