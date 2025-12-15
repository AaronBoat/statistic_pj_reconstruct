@echo off
REM Monitor fine-tuning progress
setlocal enabledelayedexpansion

:loop
cls
echo ========================================
echo Fine-Tuning Progress Monitor
echo ========================================
echo Time: %TIME%
echo.

if exist tuning_results.json (
    echo Current Results:
    echo ----------------------------------------
    python -c "import json; data=json.load(open('tuning_results.json')); results=data['results']; print(f'Completed: {len(results)}/18 configs'); passing=[r for r in results if r['pass']]; print(f'Passing (>=98%% recall): {len(passing)}'); print(); [print(f\"  {i+1}. M={r['M']}, ef_c={r['ef_c']}, ef_s={r['ef_s']}: {r['metrics']['recall10']*100:.1f}%% recall, {r['metrics']['build_min']:.1f}min build, {r['metrics']['search_ms']}ms search\") for i,r in enumerate(results[-5:])]" 2>nul
    echo.
    echo Latest update: 
    python -c "import json; data=json.load(open('tuning_results.json')); print(data['timestamp'])" 2>nul
) else (
    echo No results file yet. Waiting for first test to complete...
)

echo.
echo ----------------------------------------
echo Press Ctrl+C to stop monitoring
echo Refreshing in 30 seconds...
timeout /t 30 >nul
goto loop
