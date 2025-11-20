@echo off
echo Creating MySolution.tar...
tar -cvf MySolution.tar MySolution.h MySolution.cpp
if %ERRORLEVEL% EQU 0 (
    echo MySolution.tar created successfully!
) else (
    echo Failed to create tar file!
)
