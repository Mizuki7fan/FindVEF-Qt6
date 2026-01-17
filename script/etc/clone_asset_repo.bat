@echo off
chcp 65001 > nul
setlocal enabledelayedexpansion

echo ========================================
echo Asset Repository 克隆脚本
echo ========================================
echo.

REM 获取当前脚本所在目录的上两级目录作为项目根目录
set SCRIPT_DIR=%~dp0
for %%i in ("%SCRIPT_DIR%..\..\") do set PROJECT_ROOT=%%~fi
set AST_DIR=%PROJECT_ROOT%ast

echo 项目根目录: %PROJECT_ROOT%
echo 目标目录: %AST_DIR%
echo.

REM 检查OneDrive环境变量
if "%OneDrive%"=="" (
    echo 错误: OneDrive 环境变量未设置
    echo 请确保OneDrive已正确安装并配置
    pause
    exit /b 1
)

set REPO_PATH=%OneDrive%\GitRepo\Asset.git
echo Git仓库路径: %REPO_PATH%

REM 检查仓库路径是否存在
if not exist "%REPO_PATH%" (
    echo 错误: Git仓库路径不存在: %REPO_PATH%
    echo 请检查仓库路径是否正确
    pause
    exit /b 1
)

echo.

REM 1. 删除本地的ast目录
if exist "%AST_DIR%" (
    echo 正在删除现有的 ast 目录...
    rmdir /s /q "%AST_DIR%"
    if !ERRORLEVEL! neq 0 (
        echo 错误: 无法删除目录 %AST_DIR%
        echo 请检查目录是否被占用
        pause
        exit /b 1
    )
    echo ✓ 已删除现有目录
) else (
    echo ℹ ast 目录不存在，跳过删除步骤
)

echo.

REM 2. 临时设置环境变量GIT_LFS_SKIP_SMUDGE=1
echo 设置环境变量 GIT_LFS_SKIP_SMUDGE=1...
set GIT_LFS_SKIP_SMUDGE=1
echo ✓ 环境变量已设置

echo.

REM 3. 克隆仓库到本地ast目录
echo 开始克隆仓库 (跳过LFS文件自动下载)...
echo 源仓库: %REPO_PATH%
echo 目标目录: %AST_DIR%
echo.

git clone "%REPO_PATH%" "%AST_DIR%"

if !ERRORLEVEL! neq 0 (
    echo.
    echo ❌ 克隆失败！
    echo 可能的原因:
    echo   - Git未安装或不在PATH中
    echo   - 仓库路径错误
    echo   - 权限问题
    echo   - 网络连接问题
    pause
    exit /b 1
)

echo.
echo  克隆完成！
echo.
