# GLOVE v2.0 实时监控脚本 (HNSW + K-Means)
# 5秒刷新

$ErrorActionPreference = 'SilentlyContinue'
$startTime = Get-Date

function Show-Header {
    Clear-Host
    Write-Host "╔════════════════════════════════════════════════════════╗" -ForegroundColor Cyan
    Write-Host "║     GLOVE v2.0 测试监控 (HNSW + K-Means)          ║" -ForegroundColor Cyan
    Write-Host "╚════════════════════════════════════════════════════════╝" -ForegroundColor Cyan
    Write-Host ""
}

function Show-Stats {
    param($content)
    
    Write-Host "📊 架构: K-Means(k=150) + Per-Partition HNSW(M=12, ef_c=80)" -ForegroundColor Yellow
    Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Gray
    Write-Host ""
    
    # 提取关键指标
    if ($content -match "Build time: (\d+) ms") {
        $buildMs = [int]$matches[1]
        $buildMin = [math]::Round($buildMs / 60000, 1)
        $status = if ($buildMin -lt 30) { "✓" } else { "✗" }
        Write-Host "  构建时间: $buildMin 分钟 $status" -ForegroundColor $(if ($buildMin -lt 30) { "Green" } else { "Red" })
    }
    
    if ($content -match "Recall@10:\s+([\d\.]+)") {
        $recall = [float]$matches[1]
        $recallPct = [math]::Round($recall * 100, 2)
        $status = if ($recall -ge 0.98) { "✓" } else { "✗" }
        Write-Host "  召回率@10: $recallPct% $status" -ForegroundColor $(if ($recall -ge 0.98) { "Green" } else { "Red" })
    }
    
    if ($content -match "Average search time:\s+([\d\.]+) ms") {
        $queryTime = [float]$matches[1]
        Write-Host "  查询时间: $queryTime ms" -ForegroundColor Cyan
    }
    
    if ($content -match "Average distance computations per query:\s+([\d\.]+)") {
        $distComp = [float]$matches[1]
        Write-Host "  距离计算: $([math]::Round($distComp, 0)) per query" -ForegroundColor Cyan
    }
    
    Write-Host ""
}

while ($true) {
    Show-Header
    
    $elapsed = (Get-Date) - $startTime
    Write-Host "⏱️  运行时间: $($elapsed.ToString('hh\:mm\:ss'))" -ForegroundColor White
    Write-Host ""
    
    if (Test-Path "glove_test_v2_result.txt") {
        $content = Get-Content "glove_test_v2_result.txt" -Raw
        Show-Stats -content $content
        
        Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Gray
        Write-Host "📝 最新输出 (最后15行):" -ForegroundColor Yellow
        Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Gray
        Get-Content "glove_test_v2_result.txt" -Tail 15 | ForEach-Object {
            Write-Host "  $_" -ForegroundColor White
        }
        
        # 检测完成
        if ($content -match "测试完成" -or $content -match "✅") {
            Write-Host ""
            Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Green
            Write-Host "🎉 测试已完成!" -ForegroundColor Green
            Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Green
            break
        }
    } else {
        Write-Host "⏳ 等待测试启动..." -ForegroundColor Yellow
        Write-Host "   输出文件: glove_test_v2_result.txt" -ForegroundColor Gray
    }
    
    Write-Host ""
    Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Gray
    Write-Host "刷新间隔: 5秒 | Ctrl+C 退出监控" -ForegroundColor DarkGray
    
    Start-Sleep -Seconds 5
}

Write-Host ""
Write-Host "按任意键退出..." -ForegroundColor Gray
$null = $host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
