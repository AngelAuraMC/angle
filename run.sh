#!/bin/bash
set -euxo pipefail

OUT_DIR="out/android-arm64-testing"
PROJECT_DIR="/home/tom/Desktop/Projects/StudioProjects/PojavLauncher/testing/use-fcl-old-angle"
DEST_DIR="$PROJECT_DIR/app_pojavlauncher/src/main/jniLibs/arm64-v8a"

export PATH=/home/tom/Desktop/Projects/StudioProjects/depot_tools:$PATH
gclient sync -RDf
rm -r $OUT_DIR
mkdir $OUT_DIR
echo "# Build arguments go here.
# See \"gn args <out_dir> --list\" for available build arguments.
# Standard boilerplate for our usage
target_os = \"android\"
target_cpu = \"arm64\"

is_component_build = false # Dynamically links dependencies, rather than static
is_debug = false # Sets as \"release\" build
angle_expose_non_conformant_extensions_and_versions = true # Enables ES3.2
angle_build_tests = false # This is a big portion of the build time that is just never used because this goes into CI, so keep it off

# Stops libs.uncompressed from being built, which is only when symbol_level != 0
symbol_level = 0

# Stops ANGLE APKs from being built (we are never using those)
angle_standalone = false

# Disable all unneeded backends
angle_enable_gl = false
angle_enable_d3d9 = false
angle_enable_d3d11 = false
angle_enable_null = false
angle_enable_metal = false
angle_enable_wgpu = false
angle_enable_swiftshader = false

angle_enable_vulkan = true
angle_enable_essl = true
angle_enable_glsl = true

# Other stuff
angle_has_frame_capture = false # Turns off frame capture (we don't use it, why would you?! Also turns off rapidJSON)
build_angle_deqp_tests = false # This was somehow being set to true, am forcing it false 
angle_build_all = false # Why would we need all of it? (yes this is redundant)
# libcxx_abi_unstable = false # Apparently breaks std::string, doesn't seem to anymore. Comments mention this option causing slowdowns."  \
> out/android-arm64-testing/args.gn

gn gen $OUT_DIR
autoninja -C "$OUT_DIR"

cp "$OUT_DIR/libGLESv1_CM_angle.so" "$DEST_DIR"
cp "$OUT_DIR/libGLESv2_angle.so" "$DEST_DIR"
cp "$OUT_DIR/libEGL_angle.so" "$DEST_DIR"

cd "$PROJECT_DIR"

git add "$DEST_DIR/libGLESv1_CM_angle.so"
git add "$DEST_DIR/libGLESv2_angle.so"
git add "$DEST_DIR/libEGL_angle.so"

git commit -m "test(ANGLE): Bisecting, based on $(git rev-parse HEAD)"

./gradlew installDebug
