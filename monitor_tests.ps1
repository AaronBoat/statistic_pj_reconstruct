# Monitor Test Progress

Write-Host "`n=== Test Progress Monitor ===" -ForegroundColor Cyan

while ($true) {
    Clear-Host
    Write-Host "`n╔════════════════════════════════════════════════════════╗" -ForegroundColor Cyan
    Write-Host "║          测试进度监控 - $(Get-Date -Format 'HH:mm:ss')          ║" -ForegroundColor Cyan
    Write-Host "╚════════════════════════════════════════════════════════╝`n" -ForegroundColor Cyan
    
    # Check SIFT v6 test
    if (Test-Path "sift_test_v6_result.txt") {
        $siftSize = (Get-Item "sift_test_v6_result.txt").Length
        Write-Host "📊 SIFT v6.0 测试:" -ForegroundColor Yellow
        Write-Host "   文件大小: $siftSize bytes" -ForegroundColor White
        Write-Host "   最后10行:" -ForegroundColor Gray
        Get-Content "sift_test_v6_result.txt" -Tail 10 -ErrorAction SilentlyContinue | ForEach-Object { Write-Host "   $_" -ForegroundColor White }
        Write-Host ""
    }
    
    # Check GLOVE v1 test
    if (Test-Path "glove_test_v1_result.txt") {
        $gloveSize = (Get-Item "glove_test_v1_result.txt").Length
        Write-Host "📊 GLOVE v1.0 测试:" -ForegroundColor Yellow
        Write-Host "   文件大小: $gloveSize bytes" -ForegroundColor White
        Write-Host "   最后10行:" -ForegroundColor Gray
        Get-Content "glove_test_v1_result.txt" -Tail 10 -ErrorAction SilentlyContinue | ForEach-Object { Write-Host "   $_" -ForegroundColor White }
        Write-Host ""
    }
    
    # Check running processes
    $processes = Get-Process | Where-Object {$_.ProcessName -like "*test_solution*"}
    if ($processes) {
        Write-Host "🔄 运行中的进程:" -ForegroundColor Green
        $processes | ForEach-Object {
            $cpu = [math]::Round($_.CPU, 2)
            $mem = [math]::Round($_.WorkingSet64 / 1MB, 2)
            Write-Host "   PID: $($_.Id), CPU: ${cpu}s, 内存: ${mem}MB" -ForegroundColor White
        }
    } else {
        Write-Host "⚠ 没有运行中的测试进程" -ForegroundColor Yellow
    }
    
    Write-Host "`n按 Ctrl+C 退出监控" -ForegroundColor Gray
    Start-Sleep -Seconds 5
}
