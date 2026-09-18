@echo off
chcp 65001 >nul

echo.
echo   ╔══════════════════════════════════════════╗
echo   ║   SSN IDE Başlatılıyor...                ║
echo   ╚══════════════════════════════════════════╝
echo.

cd /d "%~dp0"

REM Node.js yolunu bul
set "NODE_DIR=%~dp0node"
if exist "%NODE_DIR%\node.exe" (
    set "PATH=%NODE_DIR%;%PATH%"
) else (
    where node >nul 2>nul
    if errorlevel 1 (
        echo   HATA: Node.js bulunamadi!
        echo   Lutfen Node.js kurun: https://nodejs.org
        pause
        exit /b 1
    )
)

REM Compiler kontrolü
if not exist "compiler.exe" (
    echo   UYARI: compiler.exe bulunamadi!
    echo   Derlemek icin Visual Studio Developer Command Prompt'ta:
    echo     cl /EHsc /utf-8 /O2 compiler.cpp /Fe:compiler.exe
    echo.
)

REM npm modülleri yüklü mü kontrol et
if not exist "node_modules\express" (
    echo   Express yukleniyor...
    call npm install --silent
)

REM Port 3000 meşgulse eski sunucuyu kapat
for /f "tokens=5" %%a in ('netstat -ano ^| findstr ":3000 " ^| findstr "LISTENING" 2^>nul') do (
    echo   Port 3000 mesgul, eski sunucu kapatiliyor...
    taskkill /PID %%a /F >nul 2>nul
)
timeout /t 1 /nobreak >nul

REM Sunucuyu arka planda başlat
echo   Sunucu baslatiliyor...
start "" /B node server.js
timeout /t 2 /nobreak >nul

REM Tarayıcıyı uygulama modunda aç (pencere çerçevesiz)
echo   Uygulama aciliyor...

set "CHROME_PATH=C:\Program Files\Google\Chrome\Application\chrome.exe"
set "CHROME86_PATH=C:\Program Files (x86)\Google\Chrome\Application\chrome.exe"
set "EDGE_PATH=C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe"
if not exist "%EDGE_PATH%" set "EDGE_PATH=C:\Program Files\Microsoft\Edge\Application\msedge.exe"

if exist "%CHROME_PATH%" (
    start "" "%CHROME_PATH%" --app=http://localhost:3000 --window-size=1280,800
) else if exist "%CHROME86_PATH%" (
    start "" "%CHROME86_PATH%" --app=http://localhost:3000 --window-size=1280,800
) else if exist "%EDGE_PATH%" (
    start "" "%EDGE_PATH%" --app=http://localhost:3000 --window-size=1280,800
) else (
    start http://localhost:3000
)

echo.
echo   SSN IDE çalışıyor!
echo   Bu pencereyi kapatmak sunucuyu durdurur.
echo.
echo   Kapatmak icin bir tusa basin...
pause >nul

REM Node sunucusunu kapat
for /f "tokens=5" %%a in ('netstat -ano ^| findstr ":3000 " ^| findstr "LISTENING" 2^>nul') do (
    taskkill /PID %%a /F >nul 2>nul
)
echo   Sunucu kapatildi.
