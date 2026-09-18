// ============================================================
// SSN Desktop IDE — Electron Ana Süreç (Main Process)
// ============================================================

const { app, BrowserWindow, ipcMain, dialog, Menu } = require('electron');
const { execFile } = require('child_process');
const path = require('path');
const fs = require('fs');

// Compiler yolu: Kendi içindeki compiler.exe
const COMPILER_PATH = path.join(__dirname, 'compiler.exe');

let mainWindow;

// ── Pencere Oluşturma ──
function createWindow() {
    mainWindow = new BrowserWindow({
        width: 1400,
        height: 900,
        minWidth: 900,
        minHeight: 600,
        title: 'SSN IDE',
        backgroundColor: '#0d1117',
        webPreferences: {
            preload: path.join(__dirname, 'preload.js'),
            contextIsolation: true,
            nodeIntegration: false,
            sandbox: false
        }
    });
    mainWindow.loadFile(path.join(__dirname, 'renderer', 'index.html'));
}

// ── Menü ──
function createMenu() {
    const template = [
        {
            label: 'Dosya',
            submenu: [
                { label: 'Yeni Sekme', accelerator: 'CmdOrCtrl+N', click: () => mainWindow.webContents.send('menu-new-tab') },
                { type: 'separator' },
                { label: 'Aç', accelerator: 'CmdOrCtrl+O', click: () => mainWindow.webContents.send('menu-open-file') },
                { label: 'Kaydet', accelerator: 'CmdOrCtrl+S', click: () => mainWindow.webContents.send('menu-save-file') },
                { label: 'Farklı Kaydet', accelerator: 'CmdOrCtrl+Shift+S', click: () => mainWindow.webContents.send('menu-save-file-as') },
                { type: 'separator' },
                { label: 'Sekmeyi Kapat', accelerator: 'CmdOrCtrl+W', click: () => mainWindow.webContents.send('menu-close-tab') },
                { type: 'separator' },
                { label: 'Çıkış', role: 'quit' }
            ]
        },
        {
            label: 'Düzenle',
            submenu: [
                { label: 'Geri Al', role: 'undo' },
                { label: 'Yinele', role: 'redo' },
                { type: 'separator' },
                { label: 'Kes', role: 'cut' },
                { label: 'Kopyala', role: 'copy' },
                { label: 'Yapıştır', role: 'paste' },
                { label: 'Tümünü Seç', role: 'selectAll' }
            ]
        },
        {
            label: 'Çalıştır',
            submenu: [
                { label: 'Derle & Çalıştır', accelerator: 'CmdOrCtrl+Enter', click: () => mainWindow.webContents.send('menu-compile-run') }
            ]
        },
        {
            label: 'Yardım',
            submenu: [
                { label: 'Geliştirici Araçları', accelerator: 'F12', click: () => mainWindow.webContents.toggleDevTools() },
                {
                    label: 'Hakkında',
                    click: () => {
                        dialog.showMessageBox(mainWindow, {
                            type: 'info',
                            title: 'SSN IDE Hakkında',
                            message: 'SSN Desktop IDE v1.0',
                            detail: 'Türkçe Sözdizimli Programlama Dili\nBytecode Compiler & Stack-Based VM\n\nElectron + Monaco Editor'
                        });
                    }
                }
            ]
        }
    ];
    Menu.setApplicationMenu(Menu.buildFromTemplate(template));
}

// ── IPC Handlers ──
function registerIPCHandlers() {
    // Derle & Çalıştır
    ipcMain.handle('compile-code', async (event, code) => {
        return new Promise((resolve) => {
            const tempFile = path.join(app.getPath('temp'), `ssn_${Date.now()}.tc`);

            // Yorum satırlarını (//) string ("") içerisine denk gelmediği sürece temizle
            let cleanCode = '';
            let inString = false;
            for (let i = 0; i < code.length; i++) {
                if (code[i] === '"') inString = !inString;
                if (!inString && code[i] === '/' && code[i+1] === '/') {
                    while (i < code.length && code[i] !== '\n') i++;
                    cleanCode += '\n'; // Satır sayısını bozmamak için newline ekle
                    continue;
                }
                cleanCode += code[i];
            }

            try {
                fs.writeFileSync(tempFile, cleanCode, { encoding: 'utf8' });
            } catch (err) {
                resolve({ success: false, stdout: '', stderr: `Dosya yazma hatası: ${err.message}`, duration: 0 });
                return;
            }

            if (!fs.existsSync(COMPILER_PATH)) {
                resolve({
                    success: false, stdout: '',
                    stderr: `HATA: Derleyici bulunamadı!\nAranan yol: ${COMPILER_PATH}\n\nÇözüm: compiler.exe dosyasını web-ide klasörüne yerleştirin.`,
                    duration: 0
                });
                return;
            }

            const startTime = Date.now();

            execFile(COMPILER_PATH, [tempFile], { timeout: 10000 }, (error, stdout, stderr) => {
                const duration = Date.now() - startTime;
                try { if (fs.existsSync(tempFile)) fs.unlinkSync(tempFile); } catch (e) { }

                if (error) {
                    if (error.signal === 'SIGTERM' || error.killed) {
                        resolve({ success: false, stdout: stdout || '', stderr: 'HATA: İşlem zaman aşımına uğradı (10 saniye).\nSonsuz döngü olabilir.', duration });
                        return;
                    }
                    resolve({ success: false, stdout: stdout || '', stderr: stderr || error.message || 'Bilinmeyen hata', duration });
                    return;
                }

                resolve({ success: true, stdout: stdout || '', stderr: stderr || '', duration });
            });
        });
    });

    // Dosya Aç
    ipcMain.handle('open-file', async () => {
        const result = await dialog.showOpenDialog(mainWindow, {
            title: 'SSN Dosyası Aç',
            filters: [
                { name: 'SSN Dosyaları', extensions: ['tc'] },
                { name: 'Tüm Dosyalar', extensions: ['*'] }
            ],
            properties: ['openFile']
        });
        if (result.canceled || result.filePaths.length === 0) return { canceled: true };
        const filePath = result.filePaths[0];
        const content = fs.readFileSync(filePath, 'utf8');
        return { canceled: false, filePath, content };
    });

    // Belirli bir yoldan dosya okuma (Drag & Drop için)
    ipcMain.handle('read-file-from-path', async (event, filePath) => {
        try {
            const content = fs.readFileSync(filePath, 'utf8');
            return { canceled: false, filePath, content };
        } catch (error) {
            throw error;
        }
    });

    // Dosya Kaydet
    ipcMain.handle('save-file', async (event, { code, filePath }) => {
        let savePath = filePath;
        if (!savePath) {
            const result = await dialog.showSaveDialog(mainWindow, {
                title: 'SSN Dosyası Kaydet',
                defaultPath: 'SSN.tc',
                filters: [
                    { name: 'SSN Dosyaları', extensions: ['tc'] },
                    { name: 'Tüm Dosyalar', extensions: ['*'] }
                ]
            });
            if (result.canceled) return { canceled: true };
            savePath = result.filePath;
        }
        fs.writeFileSync(savePath, code, 'utf8');
        return { canceled: false, filePath: savePath };
    });

    // Farklı Kaydet
    ipcMain.handle('save-file-as', async (event, { code }) => {
        const result = await dialog.showSaveDialog(mainWindow, {
            title: 'Farklı Kaydet',
            defaultPath: 'program.tc',
            filters: [
                { name: 'SSN Dosyaları', extensions: ['tc'] },
                { name: 'Tüm Dosyalar', extensions: ['*'] }
            ]
        });
        if (result.canceled) return { canceled: true };
        fs.writeFileSync(result.filePath, code, 'utf8');
        return { canceled: false, filePath: result.filePath };
    });
}

// ── Uygulama Yaşam Döngüsü ──
app.whenReady().then(() => {
    console.log('[SSN IDE] Başlatılıyor...');

    // Gizli ikon üretme modu
    if (process.argv.includes('--make-icon')) {
        const win = new BrowserWindow({
            width: 256,
            height: 256,
            show: false,
            frame: false,
            useContentSize: true,
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
        win.loadURL(`data:text/html;charset=utf-8,${encodeURIComponent(html)}`).then(() => {
            setTimeout(async () => {
                const image = await win.webContents.capturePage();
                fs.writeFileSync(path.join(__dirname, 'build', 'icon.png'), image.toPNG());
                console.log("Icon generated: build/icon.png");
                app.quit();
            }, 500);
        });
        return;
    }

    registerIPCHandlers();
    createMenu();
    createWindow();

    app.on('activate', () => {
        if (BrowserWindow.getAllWindows().length === 0) createWindow();
    });
});

app.on('window-all-closed', () => {
    if (process.platform !== 'darwin') app.quit();
});
