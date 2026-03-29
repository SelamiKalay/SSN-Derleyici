@echo off
chcp 65001 >nul
title MuhendisC IDE

REM Node.js PATH ayari
set "NODEPATH=%~dp0..\web-ide\node"
set "PATH=%NODEPATH%;%PATH%"

REM BU COK ONEMLI: Sistem genelinde set edilen bu degisken
REM Electron'un GUI yerine arkaplan Node.js islemi olarak calismasina
REM neden oluyordu (app.whenReady() undefined hatasi).
set "ELECTRON_RUN_AS_NODE="

REM Uygulamayi npm start ile baslat
"%NODEPATH%\npm.cmd" start
