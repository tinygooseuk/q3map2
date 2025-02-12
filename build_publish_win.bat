set PROJ_PATH=S:\Code\Unity\ChumpParty_UVCS
set TREMBLE_PLUGINS_PATH=%PROJ_PATH%\Assets\ThirdParty\Tremble\Plugins

set VCPKG_ROOT=C:\Users\joe\Desktop\vcpkg
set VS_ROOT=C:\Program Files\Microsoft Visual Studio\2022\Community

echo Building x64 Debug...
timeout /t 1

mkdir build_win64

mkdir %TREMBLE_PLUGINS_PATH%

cd build_win64

cmake .. -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake ^
 && "%VS_ROOT%\MSBuild\Current\Bin\MSBuild.exe" -maxcpucount:32 Q3Map2.sln ^
 && robocopy /S Debug %TREMBLE_PLUGINS_PATH% *.dll

echo %TREMBLE_PLUGINS_PATH%

cd ..