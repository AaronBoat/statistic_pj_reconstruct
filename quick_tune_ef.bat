@echo off
REM Quick parameter tuning using graph cache
REM Tests multiple ef_search values to find optimal balance

set DATASET=..\data_o\data_o\glove
set CACHE_FILE=..\data_o\data_o\glove_graph_cache.bin

echo ========================================
echo Quick ef_search Parameter Tuning
echo ========================================
echo.

REM Check if cache exists
if exist %CACHE_FILE% (
    echo [OK] Found graph cache: %CACHE_FILE%
    echo.
) else (
    echo [INFO] No cache found, will build graph first...
    echo Building and saving graph...
    test_solution_cache.exe %DATASET% --save-cache %CACHE_FILE%
    echo.
)

echo Testing different ef_search values...
echo.

REM Test ef_search values: 1800, 2000, 2200, 2400, 2600
for %%e in (1800 2000 2200 2400 2600) do (
    echo ----------------------------------------
    echo Testing ef_search = %%e
    echo ----------------------------------------
    test_solution_cache.exe %DATASET% --use-cache %CACHE_FILE% --ef-search %%e | findstr /C:"search time" /C:"distance computations" /C:"Recall"
    echo.
)

echo ========================================
echo Parameter tuning complete
echo ========================================
