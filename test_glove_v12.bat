@echo off
REM GLOVE v1.2 Test - Balanced Parameters
echo ========================================
echo GLOVE Dataset Test v1.2 (Balanced)
echo ========================================
echo.
echo Parameters:
echo   M = 20 (balanced connectivity)
echo   ef_construction = 200 (high quality)
echo   ef_search = 450 (very extensive search)
echo.
echo Expected Performance:
echo   - Recall@10: 95-98%%
echo   - Build time: 25-28 minutes
echo   - Query time: ~2.0 ms
echo   - Distance computations: ~27,000-30,000
echo ========================================
echo.

.\test_solution_v12.exe ..\data_o\data_o\glove

echo.
echo ========================================
echo Test completed!
echo ========================================
pause
