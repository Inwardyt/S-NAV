@echo off
cd /d "%~dp0"
where gcc >nul 2>nul
if errorlevel 1 (
 echo Install MinGW-w64 GCC or open the MSYS2 UCRT64 terminal.
 echo Then run: gcc -std=c99 -O2 -static -mwindows src/win32.c src/snav.c src/render.c -lgdi32 -lcomdlg32 -lm -o S-NAV.exe
 pause
 exit /b 1
)
gcc -std=c99 -O2 -static -mwindows src/win32.c src/snav.c src/render.c -lgdi32 -lcomdlg32 -lm -o S-NAV.exe
if errorlevel 1 exit /b 1
echo Built S-NAV.exe
pause
