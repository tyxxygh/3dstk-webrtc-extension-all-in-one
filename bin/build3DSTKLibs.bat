:: Name:      buildWebRTC.bat
:: Purpose:   Builds WebRTC lib
:: Author:    Sergej Jovanovic
:: Email:     sergej@gnedo.com
:: Twitter:   @JovanovicSergej
:: Revision:  December 2017 - initial version

@ECHO off
SETLOCAL EnableDelayedExpansion

SET msVS_Path=""
set powershell_path=%SYSTEMROOT%\System32\WindowsPowerShell\v1.0\powershell.exe
SET failure=0

SET startTime=0
SET endingTime=0

::log levels
SET logLevel=4											
SET error=0														
SET info=1														
SET warning=2													
SET debug=3														
SET trace=4	

SET baseBuildPath=webrtc\xplatform\webrtc\out

CALL:print %info% "Webrtc build is started. It will take couple of minutes."
CALL:print %info% "Working ..."

SET currentPlatform=%PLATFORM%
SET linkPlatform=%currentPlatform%

CALL:print %info%  "%PLATFORM%"
CALL:print %info%  "%Platform !currentPlatform!"

SET startTime=%time%

CALL:determineVisualStudioPath

CALL:buildNativeLibs

GOTO:done

:determineVisualStudioPath

SET progfiles=%ProgramFiles%
IF NOT "%ProgramFiles(x86)%" == "" SET progfiles=%ProgramFiles(x86)%

REM Check if Visual Studio 2017 is installed
SET msVS_Path="%progfiles%\Microsoft Visual Studio\2017"
SET msVS_Version=14

IF EXIST !msVS_Path! (
	SET msVS_Path=!msVS_Path:"=!
	IF EXIST "!msVS_Path!\Community" SET msVS_Path="!msVS_Path!\Community"
	IF EXIST "!msVS_Path!\Professional" SET msVS_Path="!msVS_Path!\Professional"
	IF EXIST "!msVS_Path!\Enterprise" SET msVS_Path="!msVS_Path!\Enterprise"
	IF EXIST "!msVS_Path!\VC\Tools\MSVC" SET tools_MSVC_Path=!msVS_Path!\VC\Tools\MSVC
)

IF NOT EXIST !msVS_Path! CALL:error 1 "Visual Studio 2017 is not installed"

for /f %%i in ('dir /b %tools_MSVC_Path%') do set tools_MSVC_Version=%%i

CALL:print %debug% "Visual Studio path is !msVS_Path!"
CALL:print %debug% "Visual Studio 2017 Tools MSVC Version is !tools_MSVC_Version!"

GOTO:EOF


:buildNativeLibs
    CALL:print %warning% "Building 3d streaming toolkit native and uwp libs"
	::!msVS_Path!\MSBuild\15.0\Bin\amd64\msbuild.exe %~dp0\..\webrtc\windows\solutions\WebRtc.sln /property:Platform=x86;Configuration=Release
	::!msVS_Path!\MSBuild\15.0\Bin\amd64\msbuild.exe %~dp0\..\webrtc\windows\solutions\WebRtc.sln /property:Platform=x64;Configuration=Release
	!msVS_Path!\MSBuild\15.0\Bin\amd64\msbuild.exe %~dp0\..\webrtc\windows\solutions\WebRtc.sln /property:Platform=win32_x64;Configuration=Release
	::!msVS_Path!\MSBuild\15.0\Bin\amd64\msbuild.exe %~dp0\..\webrtc\windows\solutions\WebRtc.sln /property:Platform=x86;Configuration=Debug
	::!msVS_Path!\MSBuild\15.0\Bin\amd64\msbuild.exe %~dp0\..\webrtc\windows\solutions\WebRtc.sln /property:Platform=x64;Configuration=Debug
	!msVS_Path!\MSBuild\15.0\Bin\amd64\msbuild.exe %~dp0\..\webrtc\windows\solutions\WebRtc.sln /property:Platform=win32_x64;Configuration=Debug

    IF ERRORLEVEL 1 CALL:error 1 "Building 3d streaming toolkit native and uwp libs has failed"s

	CALL:print %warning% "Copying all libraries and headers to dist folder"
	%powershell_path% -ExecutionPolicy ByPass -File bin\3dtoolkitSetup.ps1
GOTO:EOF

REM Print logger message. First argument is log level, and second one is the message
:print

SET logType=%1
SET logMessage=%~2

IF %logLevel% GEQ  %logType% (
	IF %logType%==0 ECHO [91m%logMessage%[0m
	IF %logType%==1 ECHO [92m%logMessage%[0m
	IF %logType%==2 ECHO [93m%logMessage%[0m
	IF %logType%==3 ECHO %logMessage%
	IF %logType%==4 ECHO %logMessage%
)

GOTO:EOF

REM Print the error message and terminate further execution if error is critical.Firt argument is critical error flag (1 for critical). Second is error message
:error
SET criticalError=%~1
SET errorMessage=%~2

IF %criticalError%==0 (
	ECHO.
	CALL:print %warning% "WARNING: %errorMessage%"
	ECHO.
) ELSE (
	ECHO.
	CALL:print %error% "CRITICAL ERROR: %errorMessage%"
	ECHO.
	ECHO.
	CALL:print %error% "FAILURE: Building WebRtc library has failed!"
	ECHO.
	SET endTime=%time%
	CALL:showTime
	POPD
	::terminate batch execution
	CALL %~dp0\batchTerminator.bat
)
GOTO:EOF

:showTime

SET options="tokens=1-4 delims=:.,"
FOR /f %options% %%a in ("%startTime%") do SET start_h=%%a&SET /a start_m=100%%b %% 100&SET /a start_s=100%%c %% 100&SET /a start_ms=100%%d %% 100
FOR /f %options% %%a in ("%endTime%") do SET end_h=%%a&SET /a end_m=100%%b %% 100&SET /a end_s=100%%c %% 100&SET /a end_ms=100%%d %% 100

SET /a hours=%end_h%-%start_h%
SET /a mins=%end_m%-%start_m%
SET /a secs=%end_s%-%start_s%
SET /a ms=%end_ms%-%start_ms%
IF %ms% lss 0 SET /a secs = %secs% - 1 & SET /a ms = 100%ms%
IF %secs% lss 0 SET /a mins = %mins% - 1 & SET /a secs = 60%secs%
IF %mins% lss 0 SET /a hours = %hours% - 1 & SET /a mins = 60%mins%
IF %hours% lss 0 SET /a hours = 24%hours%

SET /a totalsecs = %hours%*3600 + %mins%*60 + %secs% 

IF 1%ms% lss 100 SET ms=0%ms%
IF %secs% lss 10 SET secs=0%secs%
IF %mins% lss 10 SET mins=0%mins%
IF %hours% lss 10 SET hours=0%hours%

:: mission accomplished
ECHO [93mTotal execution time: %hours%:%mins%:%secs% (%totalsecs%s total)[0m

GOTO:EOF

:done
ECHO.
CALL:print %info% "Success: 3D Streaming Toolkit WebRtc native and UWP libraries were built successfully. Find them in dist/ folder."
ECHO.
SET endTime=%time%
CALL:showTime
:end
