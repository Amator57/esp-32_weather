@echo off
setlocal

REM === Налаштування ===
REM 🔧 Шлях до ELF-файлу (онови, якщо потрібно)
set ELF_PATH=C:\Projects_PlatformIO\ESP32__Web_13_07_2025\.pio\build\esp32doit-devkit-v1\firmware.elf

REM 🔧 Шлях до addr2line (зазвичай не змінюється, якщо PlatformIO в системі)
set TOOLCHAIN="C:\Users\eatam\.platformio\packages\toolchain-xtensa-esp32\bin\xtensa-esp32-elf-addr2line.exe"

REM === Перевірка наявності параметрів ===
if "%~1"=="" (
  echo ❌ Помилка: Не вказано адреси backtrace.
  echo 📌 Використання:
  echo     decode_backtrace.bat 0x40083c15 0x4008ed4d ...
  echo.
  pause
  exit /b 1
)

REM === Декодування ===
echo 🔍 Декодування backtrace...
%TOOLCHAIN% -pfiaC -e %ELF_PATH% %*

pause
