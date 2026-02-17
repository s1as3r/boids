#!/usr/bin/env bash
CC="gcc"
BUILD_PATH="build"
SRC="../src"
EXE_NAME="boids"

CIMGUI_PATH=$(realpath  ./external/cimgui/)
RLIMGUI_PATH=$(realpath ./external/rlImGui)
RAYLIB_PATH=$(realpath ./external/raylib/src)

build_rlimgui() {
    cd "$RLIMGUI_PATH" || return 1
    g++ -O2 -c rlImGui.cpp -o rlImGui.o \
        -I. \
        -I"$CIMGUI_PATH" \
        -I"$CIMGUI_PATH/imgui" \
        -I"$RAYLIB_PATH" \
        -DPLATFORM_DESKTOP \
        -fno-exceptions -fno-rtti

    ar -rcs librlImGui.a rlImGui.o
}

build_raylib() {
    cd "$RAYLIB_PATH" &&
        make PLATFORM=PLATFORM_DESKTOP RAYLIB_BUILD_MODE=RELEASE RAYLIB_LIBTYPE=STATIC -j4 RAYLIB_PROJECT_RELEASE_PATH=. -B
}

build_cimgui() {
    cd "$CIMGUI_PATH" && make -B static
}

build_external() {
    git submodule update --init --recursive
    (build_raylib) && (build_cimgui) && (build_rlimgui)
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
    "$RAYLIB_PATH/libraylib.a"
    "$RLIMGUI_PATH/librlImGui.a"
    "$CIMGUI_PATH/libcimgui.a"
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
    "-I$RAYLIB_PATH"
    "-I$CIMGUI_PATH"
    "-I$RLIMGUI_PATH"
    "-Wall"
    "-Wextra"
    "-Wpedantic"
    "-Werror"
    "-Og"
    "-std=c11"
)

# Build commands
EXE_CMD=("$CC" "${DEFINES[@]}" "${DEBUG[@]}" "${COMP_FLAGS[@]}" \
         "-o" "$EXE_NAME" "$SRC/main.c" "${LIBS[@]}")

EXE_CMD_STR=$(IFS=' '; echo "${EXE_CMD[*]}")

# Create build directory if it doesn't exist
if [[ ! -d "$BUILD_PATH" ]]; then
    echo "Created Build Directory"
    mkdir -p "$BUILD_PATH"
fi

no_build_deps=false
while [[ $# -gt 0 ]]; do
    case $1 in
        -n|--no-build-deps)
            no_build_deps=true
            shift;;
        *)
            shift;;
    esac
done

if [[ $no_build_deps == "false" ]]; then
    echo "=== Building dependencies ==="
    if ! build_external; then
        echo "building dependencies failed"
        exit 1
    fi
fi

cd "$BUILD_PATH" || exit
echo "===== $EXE_NAME ====="
echo "$EXE_CMD_STR"
eval "$EXE_CMD_STR"
cd ..
