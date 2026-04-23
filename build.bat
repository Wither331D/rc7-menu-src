@echo off
echo Building rc7_menu_auth EXE...

if not exist build mkdir build
cd build

echo Running CMake...
cmake .. -G "Visual Studio 17 2022" -A x64
if errorlevel 1 goto :fail

echo Building Release...
cmake --build . --config Release
if errorlevel 1 goto :fail

cd ..

echo.
echo Build successful!
echo EXE location: build\Release\rc7_menu_auth.exe
echo.
pause
exit /b 0

:fail
cd ..
echo.
echo Build failed.
echo.
pause
exit /b 1
