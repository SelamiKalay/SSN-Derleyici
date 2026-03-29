const { app, BrowserWindow } = require('electron');
const fs = require('fs');
const path = require('path');

app.whenReady().then(async () => {
    const win = new BrowserWindow({
        width: 256,
        height: 256,
        show: false,
        webPreferences: { nodeIntegration: true }
    });

    const svgData = `
        <svg width="256" height="256" viewBox="0 0 100 100" fill="white" xmlns="http://www.w3.org/2000/svg" style="background-color: black;">
            <path d="M15,80 L25,80 L25,45 L40,45 L40,65 C40,75 35,85 20,85 Z" />
            <path d="M30,50 L45,50 L45,30 C45,20 35,15 25,15 L25,25 L40,25 Z" />
            <path d="M45,80 L55,80 L55,45 L70,45 L70,65 C70,75 60,85 50,85 Z" />
            <path d="M60,50 L75,50 L75,30 C75,20 65,15 55,15 L55,25 L70,25 Z" />
            <path d="M75,80 L85,80 L85,20 L95,60 L95,80 L85,40 Z" />
        </svg>
    `;

    const html = `<html><body style="margin:0;padding:0;overflow:hidden">${svgData}</body></html>`;
    await win.loadURL(`data:text/html;charset=utf-8,${encodeURIComponent(html)}`);

    // Wait for render
    await new Promise(resolve => setTimeout(resolve, 500));

    const image = await win.webContents.capturePage();
    const buffer = image.toPNG();
    fs.writeFileSync(path.join(__dirname, 'build', 'icon.png'), buffer);
    
    console.log("Icon generate edildi: build/icon.png");
    app.quit();
});
