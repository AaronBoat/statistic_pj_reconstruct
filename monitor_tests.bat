@echo off
:loop
cls
echo ============================================================
echo  测试进度监控 - %TIME%
echo ============================================================
echo.

echo [GLOVE测试 - IVF算法]
echo ------------------------------------------------------------
if exist glove_test_ivf.txt (
    for %%A in (glove_test_ivf.txt) do (
        echo 文件大小: %%~zA 字节
        echo 最后更新: %%~tA
    )
    echo.
    echo 最新输出:
    powershell -Command "Get-Content glove_test_ivf.txt -Tail 8"
) else (
    echo 未开始
)

echo.
echo.
echo [SIFT测试 - HNSW算法]
echo ------------------------------------------------------------
if exist sift_test_new.txt (
    for %%A in (sift_test_new.txt) do (
        echo 文件大小: %%~zA 字节
        echo 最后更新: %%~tA
    )
    echo.
    echo 最新输出:
    powershell -Command "Get-Content sift_test_new.txt -Tail 8"
) else (
    echo 未开始
)

echo.
echo ============================================================
echo 按 Ctrl+C 停止监控，30秒后自动刷新...
echo ============================================================
timeout /t 30 /nobreak > nul
goto loop
