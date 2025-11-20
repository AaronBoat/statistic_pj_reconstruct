@echo off
echo Initializing git repository...
cd /d c:\codes\data_pj\reconstruct

if not exist .git (
    git init
    echo Git repository initialized.
) else (
    echo Git repository already exists.
)

echo.
echo Adding files...
git add .gitignore
git add README.md
git add DEVLOG.md
git add AGENT.md
git add MySolution.h
git add MySolution.cpp
git add test_solution.cpp
git add test_simple.cpp
git add Makefile
git add build.bat
git add test_simple.bat
git add package.bat

echo.
echo Committing...
git commit -m "feat: initial HNSW implementation for vector retrieval"

echo.
echo Git status:
git status

echo.
echo Done! Repository initialized with first commit.
pause
