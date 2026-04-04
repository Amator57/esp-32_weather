@echo off
setlocal EnableDelayedExpansion

REM === [1] Визначення каталогу проєкту ===
REM 🔧 ЗМІНИ це, якщо працюєш з іншим шляхом:
set PROJECT_DIR=%CD%

REM === [2] Пошук ELF-файлу ===
for /f "delims=" %%f in ('dir /s /b "%PROJECT_DIR%\.pio\build\*\firmware.elf"') do (
  set "ELF_PATH=%%f"
)

if not exist "!ELF_PATH!" (
  echo ❌ ELF-файл не знайдено. Переконайтесь, що прошивка зібрана.
  pause
  exit /b 1
)

echo 🧠 Використовується ELF-файл:
echo    !ELF_PATH!
echo.

REM === [3] Шлях до утиліти addr2line ===
set TOOLCHAIN=%USERPROFILE%\.platformio\packages\toolchain-xtensa-esp32\bin\xtensa-esp32-elf-addr2line.exe

if not exist "%TOOLCHAIN%" (
  echo ❌ Не знайдено утиліту addr2line:
  echo    %TOOLCHAIN%
  pause
  exit /b 1
)

REM === [4] Зчитування backtrace адрес ===
set BT_FILE=backtrace.txt
if not exist "%BT_FILE%" (
  echo ❌ Файл %BT_FILE% не знайдено. Створи файл з адресами backtrace (по 1 рядку або через пробіл).
  pause
  exit /b 1
)

REM === [5] Збір усіх адрес в один рядок ===
set ADDRESSES=
for /f "tokens=*" %%a in (%BT_FILE%) do (
  set ADDRESSES=!ADDRESSES! %%a
)

echo 📍 Адреси для декодування: !ADDRESSES!
echo ---------------------------------------
echo 🔍 Результати:
echo.

"%TOOLCHAIN%" -pfiaC -e "!ELF_PATH!" !ADDRESSES!

echo.
echo ✅ Декодування завершено.
pause
