@echo off
setlocal enabledelayedexpansion

echo =========================================================
echo   QUADTREE INVISIBLE WATERMARKING - C++17 BACKEND LAUNCHER
echo =========================================================

REM Check if MinGW GCC is in PATH, if not add WinGet MinGW path
where g++ >nul 2>&1
if %ERRORLEVEL% neq 0 (
    set "WINGET_MINGW=C:\Users\DELL\AppData\Local\Microsoft\WinGet\Packages\BrechtSanders.WinLibs.POSIX.UCRT_Microsoft.Winget.Source_8wekyb3d8bbwe\mingw64\bin"
    if exist "!WINGET_MINGW!\g++.exe" (
        echo Adding WinLibs MinGW to session PATH...
        set "PATH=!WINGET_MINGW!;%PATH%"
    )
)

REM Build if executable does not exist
if not exist "build\QuadTreeServer.exe" (
    echo Building C++ executable with CMake...
    if not exist "build" mkdir build
    cd build
    cmake -G "MinGW Makefiles" ..
    cmake --build . --target QuadTreeServer
    cd ..
)

echo Starting HTTP server on port 8080...
echo Opening dashboard at http://localhost:8080...
start http://localhost:8080
cd build
QuadTreeServer.exe --port 8080 --frontend ../frontend --output ../output --input ../input
pause
