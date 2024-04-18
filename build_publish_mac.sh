#!/bin/zsh
PROJ_PATH=~/Unity/TrembleFPS
TREMBLE_PLUGINS_PATH=$PROJ_PATH/Assets/Tremble/Editor/Plugins

echo Building ARM64...
mkdir -p build_arm
mkdir -p $TREMBLE_PLUGINS_PATH/osx_arm64
cd build_arm
cmake .. && make -j8 && cp tools/quake3/libq3map2.dylib $TREMBLE_PLUGINS_PATH/osx_arm64/libq3map2.dylib
cp /opt/homebrew/Cellar/libpng/1.6.43/lib/libpng.dylib $TREMBLE_PLUGINS_PATH/osx_arm64/libpng.dylib
cp /opt/homebrew/Cellar/glib/2.80.0_2/lib/libglib-2.0.dylib $TREMBLE_PLUGINS_PATH/osx_arm64/libglib-2.0.dylib
cd ..

echo Building X64...
mkdir -p build_x64
mkdir -p $TREMBLE_PLUGINS_PATH/osx_x64
cd build_x64
cmake -DCMAKE_OSX_ARCHITECTURES="x86_64" .. && make -j8 && cp tools/quake3/libq3map2.dylib $TREMBLE_PLUGINS_PATH/osx_x64/libq3map2.dylib
cp /usr/local/Cellar/libpng/1.6.43/lib/libpng.dylib $TREMBLE_PLUGINS_PATH/osx_x64/libpng.dylib
cp /usr/local/Cellar/glib/2.80.0_2/lib/libglib-2.0.dylib $TREMBLE_PLUGINS_PATH/osx_x64/libglib-2.0.dylib
cd ..

echo done!