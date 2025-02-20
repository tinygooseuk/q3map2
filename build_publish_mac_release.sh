#!/bin/zsh
PROJ_PATH=~/Unity/ChumpParty_UVCS
TREMBLE_PLUGINS_PATH=$PROJ_PATH/Assets/ThirdParty/Tremble/Plugins

echo Kill Unity
killall Unity
killall HotReload

echo Building ARM64...
sleep 1
mkdir -p build_arm
mkdir -p $TREMBLE_PLUGINS_PATH
cd build_arm
cmake -DCMAKE_BUILD_TYPE=Release .. && make -j8 q3map2 && cp tools/quake3/libq3map2.dylib $TREMBLE_PLUGINS_PATH/ARM64/q3map2.dylib
cd ..

echo Building X64...
sleep 1
mkdir -p build_x64
mkdir -p $TREMBLE_PLUGINS_PATH
cd build_x64
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES="x86_64" .. && make -j8 q3map2 && cp tools/quake3/libq3map2.dylib $TREMBLE_PLUGINS_PATH/x86_64/q3map2.dylib
cd ..

echo done!
