@echo off

set VS2008_DEVENV="C:\Program Files\Microsoft Visual Studio 9.0\Common7\IDE\devenv.exe"
set BUILD_LOG_FILE=".\buildlog.txt"
set BUILD_TYPE=build
set CONFIG_NAME=Debug

if {%1%} == {} (
    echo 未指定参数，使用默认值。
) else (
    if {%2%} == {} goto usage_error

    if /i "%1%" == "build" (
        set BUILD_TYPE=build
    ) else (
        if /i "%1%" == "rebuild" (set BUILD_TYPE=rebuild) else (goto usage_error)
    )

    if /i "%2%" == "Debug" (
        set CONFIG_NAME=Debug
    ) else (
        if /i "%2%" == "Release" (set CONFIG_NAME=Release) else (goto usage_error)
    )
)

echo 编译参数：%BUILD_TYPE%, %CONFIG_NAME%

set SLN_01=wxWidgets-2.9.1\build\msw\wx_vc9.sln


if exist %BUILD_LOG_FILE% del %BUILD_LOG_FILE%

echo.

echo 正在编译：%SLN_01% ...
%VS2008_DEVENV% %SLN_01% /%BUILD_TYPE% %CONFIG_NAME% /out %BUILD_LOG_FILE%



echo.
echo 编译完成，请查看文件%BUILD_LOG_FILE%，检查编译过程是否正确。
echo.

xcopy /E /Y "wxWidgets-2.9.1\include\*" "..\window\include\"
xcopy /E /Y "wxWidgets-2.9.1\lib\*" "..\window\lib\"

set BUILD_LOG_FILE=".\buildlog-c.txt"
rem #if exist %BUILD_LOG_FILE% del %BUILD_LOG_FILE%

rem #echo 正在清理：%SLN_01% ...

rem #%VS2008_DEVENV% %SLN_01% /clean %CONFIG_NAME% /out %BUILD_LOG_FILE%

goto end

rem ========================================================================

:usage_error

echo.
echo 使用方法：
echo     %0% [build^|rebuild] [release^|debug]
echo.
echo 若不指定参数，则默认为：build, release
echo.

:end

PAUSE

