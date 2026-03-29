$ws = New-Object -ComObject WScript.Shell
$shortcut = $ws.CreateShortcut("$env:USERPROFILE\Desktop\SSN.lnk")
$shortcut.TargetPath = "$env:USERPROFILE\Desktop\SSN Derleyici\desktop-ide\dist\SSN-win32-x64\SSN.exe"
$shortcut.WorkingDirectory = "$env:USERPROFILE\Desktop\SSN Derleyici\desktop-ide\dist\SSN-win32-x64"
$shortcut.IconLocation = "$env:USERPROFILE\Desktop\SSN Derleyici\desktop-ide\dist\SSN-win32-x64\SSN.exe,0"
$shortcut.Description = "SSN Derleyici"
$shortcut.Save()
Write-Host "Shortcut created successfully!"
