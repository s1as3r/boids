#!/usr/bin/env bash
BUILD_PATH="build"
SRC="../src"
EXE_NAME="boids.html"

CIMGUI_PATH=$(realpath  ./external/cimgui/)
RLIMGUI_PATH=$(realpath ./external/rlImGui/)
RAYLIB_PATH=$(realpath ./external/raylib/)
EMSDK_PATH=$(realpath ~/.local/emsdk)
SHELL_HTML="$SRC/minshell.html"

build_raylib_web() {
    cd "$RAYLIB_PATH/src" &&
        make PLATFORM=PLATFORM_WEB RAYLIB_BUILD_MODE=RELEASE RAYLIB_LIBTYPE=STATIC -j4 EMSDK_PATH="$EMSDK_PATH" RAYLIB_PROJECT_RELEASE_PATH=. -B
}

build_cimgui_web() {
    cd "$CIMGUI_PATH" || return 1

    emcc -O2 -c cimgui.cpp -o cimgui.o \
        -I./imgui/ \
        -fno-exceptions -fno-rtti

    emcc -O2 -c ./imgui/imgui.cpp -o ./imgui/imgui.o \
        -I./imgui/ \
        -fno-exceptions -fno-rtti

    emcc -O2 -c ./imgui/imgui_draw.cpp -o ./imgui/imgui_draw.o \
        -I./imgui/ \
        -fno-exceptions -fno-rtti

    emcc -O2 -c ./imgui/imgui_demo.cpp -o ./imgui/imgui_demo.o \
        -I./imgui/ \
        -fno-exceptions -fno-rtti

    emcc -O2 -c ./imgui/imgui_tables.cpp -o ./imgui/imgui_tables.o \
        -I./imgui/ \
        -fno-exceptions -fno-rtti

    emcc -O2 -c ./imgui/imgui_widgets.cpp -o ./imgui/imgui_widgets.o \
        -I./imgui/ \
        -fno-exceptions -fno-rtti

    emar rcs libcimgui.web.a cimgui.o ./imgui/imgui.o ./imgui/imgui_draw.o \
        ./imgui/imgui_demo.o ./imgui/imgui_tables.o ./imgui/imgui_widgets.o
}

build_rlimgui_web() {
    cd "$RLIMGUI_PATH" || return 1

    em++ -O2 -c rlImGui.cpp -o rlImGui.o \
        -I. \
        -I"$CIMGUI_PATH" \
        -I"$CIMGUI_PATH/imgui" \
        -I"$RAYLIB_PATH/src" \
        -DPLATFORM_WEB \
        -DRLIMGUI_ALWAYS_TRACK_MOUSE \
        -fno-exceptions -fno-rtti

    emar rcs librlImGui.web.a rlImGui.o
}

build_external_web() {
    git submodule update --init --recursive
    (build_raylib_web) && (build_cimgui_web) && (build_rlimgui_web)
}

# compile time defines
DEFINES=(
    # -DBOIDS_DEBUG_DRAW
)

WEB_FLAGS=(
    "-I$CIMGUI_PATH"
    "-I$RLIMGUI_PATH"
    "-I$RAYLIB_PATH/src"
    "-std=c11"
    "-Wall"
    "-DPLATFORM_WEB"
    "-DGRAPHICS_API_OPENGL_ES2"
)

WEB_LIBS=(
    "$RAYLIB_PATH/src/libraylib.web.a"
    "$RLIMGUI_PATH/librlImGui.web.a"
    "$CIMGUI_PATH/libcimgui.web.a"
)


EM_FLAGS=(
    "-sUSE_GLFW=3"
    "-sASYNCIFY"
    "-sALLOW_MEMORY_GROWTH=1"
    "-sFULL_ES2=1"
    "--shell-file" "$SHELL_HTML"
    "-Os"
    "-Wall"
)

EXE_CMD=("emcc" "${WEB_FLAGS[@]}" "${DEFINES[@]}" \
         "$SRC/main.c" \
         "${WEB_LIBS[@]}" \
         "${EM_FLAGS[@]}" \
         "-o" "$EXE_NAME")

EXE_CMD_STR=$(IFS=' '; echo "${EXE_CMD[*]}")

if [[ ! -d "$BUILD_PATH" ]]; then
    echo "Created Build Directory"
    mkdir -p "$BUILD_PATH"
fi

echo "=== Building dependencies ==="
if ! build_external_web; then
    echo "building web dependencies failed"
    exit 1
fi

cd "$BUILD_PATH" || exit 1
if [ ! -f "$SHELL_HTML" ]; then
    echo "Error: shell.html not found at $SHELL_HTML"
    exit 1
fi

echo "===== $EXE_NAME ====="
echo "$EXE_CMD_STR"
eval "$EXE_CMD_STR"
cd ..
