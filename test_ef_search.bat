@echo off
setlocal enabledelayedexpansion

echo ========================================
echo 测试不同ef_search参数
echo ========================================
echo.

REM 测试ef_search: 2200, 2400, 2600
for %%e in (2200 2400 2600) do (
    echo.
    echo [测试] ef_search = %%e
    echo ----------------------------------------
    
    REM 备份并修改参数
    copy /Y MySolution_backup.cpp MySolution.cpp >nul
    
    REM 使用PowerShell进行替换
    powershell -Command "(Get-Content MySolution.cpp) -replace 'ef_search = 2000', 'ef_search = %%e' | Set-Content MySolution.cpp"
    
    REM 编译
    g++ -std=c++11 -O3 -mavx2 -mfma -march=native -fopenmp test_solution.cpp MySolution.cpp -o test_solution.exe 2>nul
    if errorlevel 1 (
        echo 编译失败
        goto :end
    )
    
    REM 运行测试
    .\test_solution.exe ..\data_o\data_o\glove | findstr /C:"Build time" /C:"Recall@10" /C:"Total search" /C:"Average distance"
    echo.
)

:end
REM 恢复原始文件
copy /Y MySolution_backup.cpp MySolution.cpp >nul
echo.
echo ========================================
echo 测试完成
echo ========================================
