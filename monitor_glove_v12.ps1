# Monitor GLOVE v1.2 Test Progress

$file = "glove_test_v12_result.txt"
$startTime = Get-Date

Write-Host "`n╔════════════════════════════════════════════════════════╗" -ForegroundColor Cyan
Write-Host "║       GLOVE v1.2 测试进度监控                     ║" -ForegroundColor Cyan
Write-Host "╚════════════════════════════════════════════════════════╝`n" -ForegroundColor Cyan

while ($true) {
    Clear-Host
    $now = Get-Date
    $elapsed = $now - $startTime
    
    Write-Host "`n╔════════════════════════════════════════════════════════╗" -ForegroundColor Cyan
    Write-Host "║       GLOVE v1.2 测试进度 - $(Get-Date -Format 'HH:mm:ss')       ║" -ForegroundColor Cyan
    Write-Host "╚════════════════════════════════════════════════════════╝`n" -ForegroundColor Cyan
    
    Write-Host "⏱️  已运行时间: " -NoNewline -ForegroundColor Yellow
    Write-Host "$([math]::Floor($elapsed.TotalMinutes)) 分 $($elapsed.Seconds) 秒`n" -ForegroundColor White
    
    if (Test-Path $file) {
        $size = (Get-Item $file).Length
        $lines = Get-Content $file -ErrorAction SilentlyContinue
        
        Write-Host "📊 文件状态:" -ForegroundColor Yellow
        Write-Host "  大小: $([math]::Round($size/1KB, 2)) KB" -ForegroundColor White
        Write-Host "  行数: $($lines.Count)`n" -ForegroundColor White
        
        Write-Host "📝 最新输出 (最后 15 行):" -ForegroundColor Yellow
        Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Gray
        
        $lines | Select-Object -Last 15 | ForEach-Object {
            if ($_ -match "Build time|Recall|Query|Distance") {
                Write-Host $_ -ForegroundColor Green
            } elseif ($_ -match "Error|Failed") {
                Write-Host $_ -ForegroundColor Red
            } else {
                Write-Host $_ -ForegroundColor White
            }
        }
        
        Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Gray
        
        # Check if completed
        if ($lines -match "Recall@10") {
            Write-Host "`n✅ 测试完成!" -ForegroundColor Green
            
            # Extract key metrics
            $buildLine = $lines | Select-String "Build time:" | Select-Object -Last 1
            $recallLine = $lines | Select-String "Recall@10:" | Select-Object -Last 1
            $queryLine = $lines | Select-String "Average search time:" | Select-Object -Last 1
            $distLine = $lines | Select-String "Average distance computations:" | Select-Object -Last 1
            
            Write-Host "`n📊 关键指标:" -ForegroundColor Yellow
            Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Gray
            if ($buildLine) { Write-Host "  $buildLine" -ForegroundColor Cyan }
            if ($recallLine) { Write-Host "  $recallLine" -ForegroundColor Cyan }
            if ($queryLine) { Write-Host "  $queryLine" -ForegroundColor Cyan }
            if ($distLine) { Write-Host "  $distLine" -ForegroundColor Cyan }
            
            break
        }
    } else {
        Write-Host "⏳ 等待测试文件生成..." -ForegroundColor Yellow
    }
    
    Write-Host "`n💡 提示: 按 Ctrl+C 退出监控 (不影响测试)" -ForegroundColor Gray
    Start-Sleep -Seconds 5
}

Write-Host "`n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Gray
Write-Host "监控结束" -ForegroundColor Cyan
