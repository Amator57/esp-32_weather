@echo off
:: Переходимо в директорію, де знаходиться скрипт (і firmware.elf)
cd /d "%~dp0"

:: Перевіряємо наявність ELF
if not exist firmware.elf (
  echo ❌ ELF-файл не знайдено. Переконайтесь, що прошивка зібрана.
  pause
  exit /b
)

:: Перевіряємо наявність backtrace.txt
if not exist backtrace.txt (
  echo ❌ Файл backtrace.txt не знайдено. Вставте стек у цей файл.
  pause
  exit /b
)

echo 🔍 Розшифровка backtrace...

:: Вказуємо повний шлях до xtensa-esp32-elf-addr2line
"C:\Users\eatam\.platformio\packages\toolchain-xtensa-esp32\bin\xtensa-esp32-elf-addr2line.exe" -pfiaC -e firmware.elf < backtrace.txt

pause

