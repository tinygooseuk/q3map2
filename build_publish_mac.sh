#!/bin/zsh
PROJ_PATH=~/Unity/ChumpParty_UVCS
TREMBLE_PLUGINS_PATH=$PROJ_PATH/Assets/ThirdParty/Tremble/Editor/Plugins

echo Kill Unity
killall Unity
killall HotReload

echo Building ARM64...
mkdir -p build_arm
mkdir -p $TREMBLE_PLUGINS_PATH
cd build_arm
cmake .. && make -j8 && cp tools/quake3/libq3map2.dylib $TREMBLE_PLUGINS_PATH/libq3map2_osx_arm64.dylib
cd ..

echo Building X64...
mkdir -p build_x64
mkdir -p $TREMBLE_PLUGINS_PATH
cd build_x64
cmake -DCMAKE_OSX_ARCHITECTURES="x86_64" .. && make -j8 && cp tools/quake3/libq3map2.dylib $TREMBLE_PLUGINS_PATH/libq3map2_osx_x64.dylib
cd ..

echo done!