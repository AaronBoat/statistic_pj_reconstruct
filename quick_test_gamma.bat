@echo off
chcp 65001 >nul
echo ==========================================
echo 快速NGT优化测试 (使用图缓存)
echo ==========================================
echo.

set CACHE_FILE=..\data_o\data_o\glove_graph_cache.bin

if not exist "%CACHE_FILE%" (
    echo 错误: 未找到图缓存文件
    echo 请先运行: test_solution_cache.exe ..\data_o\data_o\glove --save-cache
    pause
    exit /b 1
)

echo 找到图缓存文件: %CACHE_FILE%
echo.

echo [测试1] gamma=0 (标准HNSW)
echo ----------------------------------------
powershell -Command "(Get-Content MySolution.cpp) -replace 'gamma = [0-9.]+;', 'gamma = 0.0;' | Set-Content MySolution.cpp"
g++ -std=c++11 -O3 -mavx2 -mfma -march=native -fopenmp test_solution.cpp MySolution.cpp -o test_quick.exe 2>nul
if errorlevel 1 (
    echo 编译失败！
    pause
    exit /b 1
)
echo 运行 (加载缓存)...
test_quick.exe ..\data_o\data_o\glove --use-cache 2>&1 | findstr /C:"loaded" /C:"Recall" /C:"search time" /C:"distance"
echo.

echo [测试2] gamma=0.15 (较激进)
echo ----------------------------------------
powershell -Command "(Get-Content MySolution.cpp) -replace 'gamma = [0-9.]+;', 'gamma = 0.15;' | Set-Content MySolution.cpp"
g++ -std=c++11 -O3 -mavx2 -mfma -march=native -fopenmp test_solution.cpp MySolution.cpp -o test_quick.exe 2>nul
echo 运行...
test_quick.exe ..\data_o\data_o\glove --use-cache 2>&1 | findstr /C:"Recall" /C:"search time" /C:"distance"
echo.

echo [测试3] gamma=0.19 (推荐值)
echo ----------------------------------------
powershell -Command "(Get-Content MySolution.cpp) -replace 'gamma = [0-9.]+;', 'gamma = 0.19;' | Set-Content MySolution.cpp"
g++ -std=c++11 -O3 -mavx2 -mfma -march=native -fopenmp test_solution.cpp MySolution.cpp -o test_quick.exe 2>nul
echo 运行...
test_quick.exe ..\data_o\data_o\glove --use-cache 2>&1 | findstr /C:"Recall" /C:"search time" /C:"distance"
echo.

echo [测试4] gamma=0.25 (保守)
echo ----------------------------------------
powershell -Command "(Get-Content MySolution.cpp) -replace 'gamma = [0-9.]+;', 'gamma = 0.25;' | Set-Content MySolution.cpp"
g++ -std=c++11 -O3 -mavx2 -mfma -march=native -fopenmp test_solution.cpp MySolution.cpp -o test_quick.exe 2>nul
echo 运行...
test_quick.exe ..\data_o\data_o\glove --use-cache 2>&1 | findstr /C:"Recall" /C:"search time" /C:"distance"
echo.

echo ==========================================
echo 测试完成！恢复gamma=0.19
echo ==========================================
powershell -Command "(Get-Content MySolution.cpp) -replace 'gamma = [0-9.]+;', 'gamma = 0.19;' | Set-Content MySolution.cpp"
del test_quick.exe >nul 2>&1
pause
