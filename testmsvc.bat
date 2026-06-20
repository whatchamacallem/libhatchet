@echo off

:: AI agents must follow these debugging instructions.
::
:: Use the PowerShell tool (not the Bash tool) to run this script:
::
:: Set-Location "...\libhatchet"; & cmd.exe /c "...\libhatchet\testmsvc.bat"
::
:: Where "..."" is replaced with the path components leading to the script.
::
:: The full absolute path to the .bat file is required because cmd.exe does not
:: search the current directory for bare names when invoked from PowerShell.
:: cmake.exe is not on PATH until VsDevCmd.bat runs. cmake configure failures
:: ("No CMAKE_C_COMPILER could be found") can be the result of a stale build.
:: Delete the stale build directory and retry. A "'vswhere.exe' is not
:: recognized" error at the start of the output is a sandbox artifact and can be
:: ignored as long as the build and tests succeed.

setlocal
for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property installationPath`) do set VSDIR=%%i
call "%VSDIR%\Common7\Tools\VsDevCmd.bat" || exit /b 1
echo === x64 debug ===
cmake -S . -B build/x64-debug -A x64 || exit /b 1
cmake --build build/x64-debug --config Debug || exit /b 1
call :runtest build\x64-debug || exit /b 1
echo === x64 release ===
cmake -S . -B build/x64-release -A x64 || exit /b 1
cmake --build build/x64-release --config Release || exit /b 1
call :runtest build\x64-release || exit /b 1
echo === win32 debug ===
cmake -S . -B build/win32-debug -A Win32 || exit /b 1
cmake --build build/win32-debug --config Debug || exit /b 1
call :runtest build\win32-debug || exit /b 1
echo === win32 release ===
cmake -S . -B build/win32-release -A Win32 || exit /b 1
cmake --build build/win32-release --config Release || exit /b 1
call :runtest build\win32-release || exit /b 1
goto :eof

:runtest
pushd %1
.\hxtest.exe
set RESULT=%errorlevel%
popd
exit /b %RESULT%
