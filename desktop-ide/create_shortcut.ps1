$wshell = New-Object -ComObject WScript.Shell
$desktop = [Environment]::GetFolderPath('Desktop')
$shortcut = $wshell.CreateShortcut("$desktop\SSN IDE.lnk")
$shortcut.TargetPath = "c:\Users\selam\Desktop\yeni proje\desktop-ide\dist\SSN-win32-x64\SSN.exe"
$shortcut.WorkingDirectory = "c:\Users\selam\Desktop\yeni proje\desktop-ide\dist\SSN-win32-x64"
$shortcut.Description = "SSN Desktop IDE"
$shortcut.Save()
Write-Host "Success"
