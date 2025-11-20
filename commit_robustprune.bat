@echo off
echo Adding RobustPrune optimization...
cd /d c:\codes\data_pj\reconstruct

git add MySolution.cpp
git commit -m "feat: add RobustPrune heuristic for neighbor selection

- Implement diversity-aware neighbor selection
- Avoid clustering by checking distance between selected neighbors
- Maintains graph quality with 1.2x tolerance factor"

echo.
git log --oneline -n 5
echo.
echo Commit completed!
pause
