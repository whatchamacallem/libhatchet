@echo off
:: Debugging instructions for AI:
::
:: cmake.exe is not on PATH until VsDevCmd.bat runs. VsDevCmd.bat does not
:: change the working directory when called with no args, so relative paths used
:: below remain valid. Use .\hxtest.exe not hxtest.exe. cmd.exe does not search
:: the current directory for bare executable names. cmake configure failures
:: ("No CMAKE_C_COMPILER could be found") can be the result of a stale build.
:: Delete the stale build directory and retry. To capture output from
:: PowerShell: cmd /c """testmsvc.bat""" 2>&1 | ...

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
