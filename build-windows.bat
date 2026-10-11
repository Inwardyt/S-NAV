@echo off
cd /d "%~dp0"
where gcc >nul 2>nul
if errorlevel 1 (
 echo Install MinGW-w64 GCC or open the MSYS2 UCRT64 terminal.
 pause
 exit /b 1
)
gcc -std=c99 -O2 -Wall -Wextra -Werror -static -mwindows src/win32.c src/snav.c src/render.c -lgdi32 -lcomdlg32 -lm -o S-NAV-Simulator.exe
if errorlevel 1 exit /b 1
gcc -std=c99 -O2 -Wall -Wextra -Werror -static -mwindows -DSNAV_SCREEN_ONLY src/win32.c src/snav.c src/render.c -lgdi32 -lcomdlg32 -lm -o S-NAV-Screen.exe
if errorlevel 1 exit /b 1
echo Built S-NAV-Simulator.exe and S-NAV-Screen.exe
pause
