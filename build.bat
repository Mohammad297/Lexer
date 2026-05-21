@echo off
setlocal enabledelayedexpansion

:: 1. Force directory isolation variables
set WORKSPACE=%~dp0
set SRC_DIR=!WORKSPACE!src
set BUILD_DEBUG=!WORKSPACE!build\debug
set BUILD_RELEASE=!WORKSPACE!build\release
set BIN_DEBUG=!WORKSPACE!bin\debug
set BIN_RELEASE=!WORKSPACE!bin\release
set LIB_DEBUG=!WORKSPACE!lib\debug
set LIB_RELEASE=!WORKSPACE!lib\release

set DIST_DIR=E:\Development\C++\Libraries

:: 2. Handle the "clean" argument (Delete AND Recreate immediately)
if "%~1"=="clean" (
    echo Resetting and recreating all workspace paths...
    if exist "!WORKSPACE!build" rmdir /s /q "!WORKSPACE!build"
    if exist "!WORKSPACE!bin"   rmdir /s /q "!WORKSPACE!bin"
    if exist "!WORKSPACE!lib"   rmdir /s /q "!WORKSPACE!lib"
    if exist "!WORKSPACE!dist"  rmdir /s /q "!WORKSPACE!dist"
    
    mkdir "!BUILD_DEBUG!" "!BUILD_RELEASE!" "!BIN_DEBUG!" "!BIN_RELEASE!" "!LIB_DEBUG!" "!LIB_RELEASE!"
    echo All directories cleared and reset.
    exit /b 0
)

:: 3. Safety fallback to ensure all target directory structures exist firmly
if not exist "!BUILD_DEBUG!"    mkdir "!BUILD_DEBUG!"
if not exist "!BUILD_RELEASE!"  mkdir "!BUILD_RELEASE!"
if not exist "!BIN_DEBUG!"      mkdir "!BIN_DEBUG!"
if not exist "!BIN_RELEASE!"    mkdir "!BIN_RELEASE!"
if not exist "!LIB_DEBUG!"      mkdir "!LIB_DEBUG!"
if not exist "!LIB_RELEASE!"    mkdir "!LIB_RELEASE!"
if not exist "!DIST_DIR!"       mkdir "!DIST_DIR!"

:: 4. Detect and initialize MSVC environment if cl.exe is missing from PATH
where cl.exe >nul 2>nul
if %errorlevel% neq 0 (
    echo cl.exe not found in PATH. Attempting to locate vcvarsall.bat...
    set "VS_PATH="
    for /f "usebackq tokens=*" %%i in (`"C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property installationPath`) do set "VS_PATH=%%i"
    
    if defined VS_PATH (
        if exist "!VS_PATH!\VC\Auxiliary\Build\vcvarsall.bat" (
            call "!VS_PATH!\VC\Auxiliary\Build\vcvarsall.bat" x64 >nul
        ) else (
            echo CRITICAL ERROR: Found VS installation but vcvarsall.bat is missing.
            exit /b 1
        )
    ) else (
        echo CRITICAL ERROR: Visual Studio installation not detected by vswhere.
        exit /b 1
    )
)

:: 5. Handle the "debug" pipeline
if "%~1"=="debug" (
    echo Starting BUILD ALL [Debug Mode]...
    
    cl.exe /nologo /std:c++20 /EHsc /Zi /Od /MDd /D_DEBUG /c "!SRC_DIR!\main.cpp" "!SRC_DIR!\lex\*.cpp" /Fo:"!BUILD_DEBUG!\\" /I "!SRC_DIR!" /I "E:\Development\C++\Libraries\include"
    if !errorlevel! neq 0 exit /b !errorlevel!
    
    :: FIXED: Added "!BUILD_DEBUG!\checkers.obj" to the library archive
    lib.exe /nologo /OUT:"!LIB_DEBUG!\lexer.lib" "!BUILD_DEBUG!\engine.obj" "!BUILD_DEBUG!\lexer.obj" "!BUILD_DEBUG!\checkers.obj" "!BUILD_DEBUG!\token_stream.obj"
    if !errorlevel! neq 0 exit /b !errorlevel!
    
    cl.exe /nologo /std:c++20 /EHsc /Zi /Od /MDd /D_DEBUG "!BUILD_DEBUG!\main.obj" "!LIB_DEBUG!\lexer.lib" /Fe:"!BIN_DEBUG!\lexer_test_debug.exe" /Fo:"!BUILD_DEBUG!\\" /link /ILK:"!BUILD_DEBUG!\launch_test_debug.ilk" /PDB:"!BUILD_DEBUG!\launch_test_debug.pdb"

    echo Packaging Debug SDK...
    
    if not exist "!DIST_DIR!\include\lex" mkdir "!DIST_DIR!\include\lex"
    if not exist "!DIST_DIR!\lib\debug" mkdir "!DIST_DIR!\lib\debug"
    
    :: Copy only the strict public headers into the namespaced layout
    copy /Y "!SRC_DIR!\lex\engine.hpp" "!DIST_DIR!\include\lex\" >nul
    copy /Y "!SRC_DIR!\lex\token.hpp" "!DIST_DIR!\include\lex\" >nul
    copy /Y "!SRC_DIR!\lex\token_stream.hpp" "!DIST_DIR!\include\lex\" >nul
    
    :: Copy the compiled binary artifact
    copy /Y "!LIB_DEBUG!\lexer.lib" "!DIST_DIR!\lib\debug\" >nul
    copy /Y "!BIN_DEBUG!\lexer_test_debug.exe" "!DIST_DIR!\bin\" >nul
    
    echo Debug Build and SDK Deployment Completed Successfully.
    exit /b 0
)

:: 6. Handle the "release" pipeline
if "%~1"=="release" (
    echo Starting BUILD ALL [Release Mode]...
    
    cl.exe /nologo /std:c++20 /EHsc /O2 /DNDEBUG /MD /c "!SRC_DIR!\main.cpp" "!SRC_DIR!\lex\*.cpp" /Fo:"!BUILD_RELEASE!\\" /I "!SRC_DIR!" /I "E:\Development\C++\Libraries\include"
    if !errorlevel! neq 0 exit /b !errorlevel!
    
    :: FIXED: Added "!BUILD_RELEASE!\checkers.obj" to the library archive
    lib.exe /nologo /OUT:"!LIB_RELEASE!\lexer.lib" "!BUILD_RELEASE!\engine.obj" "!BUILD_RELEASE!\lexer.obj" "!BUILD_RELEASE!\checkers.obj" "!BUILD_RELEASE!\token_stream.obj"
    if !errorlevel! neq 0 exit /b !errorlevel!
    
    cl.exe /nologo /std:c++20 /EHsc /O2 /DNDEBUG /MD "!BUILD_RELEASE!\main.obj" "!LIB_RELEASE!\lexer.lib" /Fe:"!BIN_RELEASE!\lexer_test.exe" /Fo:"!BUILD_RELEASE!\\" /link

    echo Packaging Release SDK...

    if not exist "!DIST_DIR!\include\lex" mkdir "!DIST_DIR!\include\lex"
    if not exist "!DIST_DIR!\lib\release" mkdir "!DIST_DIR!\lib\release"
    if not exist "!DIST_DIR!\bin" mkdir "!DIST_DIR!\bin"
    
    :: Copy public headers
    copy /Y "!SRC_DIR!\lex\engine.hpp" "!DIST_DIR!\include\lex\" >nul
    copy /Y "!SRC_DIR!\lex\token.hpp" "!DIST_DIR!\include\lex\" >nul
    copy /Y "!SRC_DIR!\lex\token_stream.hpp" "!DIST_DIR!\include\lex\" >nul
    
    :: Copy compiled library and testing binaries
    copy /Y "!LIB_RELEASE!\lexer.lib" "!DIST_DIR!\lib\release\" >nul
    copy /Y "!BIN_RELEASE!\lexer_test.exe" "!DIST_DIR!\bin\" >nul
    
    echo Release Build and SDK Deployment Completed Successfully.
    exit /b 0
)

echo Usage: build.bat [debug ^| release ^| clean]
exit /b 1
