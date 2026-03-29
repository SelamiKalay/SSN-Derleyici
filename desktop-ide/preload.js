// ============================================================
// SSN Desktop IDE — Preload Script (IPC Köprüsü)
// ============================================================
// contextBridge ile renderer process'e güvenli API sunuyoruz.
// renderer tarafı window.electronAPI üzerinden bu fonksiyonlara erişir.

const { contextBridge, ipcRenderer, webUtils } = require('electron');

contextBridge.exposeInMainWorld('electronAPI', {
    // Monaco path'i (renderer'da nodeIntegration kapalı olduğu için)
    monacoPath: require.resolve('monaco-editor/min/vs/loader.js')
        .replace(/\\/g, '/')
        .replace('/loader.js', ''),

    // Real file path helper for Drag and Drop Web Sandbox bypass
    getPathForFile: (file) => webUtils.getPathForFile(file),

    // ── Derle & Çalıştır ──
    compileCode: (code) => ipcRenderer.invoke('compile-code', code),

    // ── Dosya İşlemleri ──
    openFile: () => ipcRenderer.invoke('open-file'),
    readFileFromPath: (filePath) => ipcRenderer.invoke('read-file-from-path', filePath),
    saveFile: (data) => ipcRenderer.invoke('save-file', data),
    saveFileAs: (data) => ipcRenderer.invoke('save-file-as', data),

    // ── Menü Olaylarını Dinle ──
    onMenuOpenFile: (callback) => ipcRenderer.on('menu-open-file', callback),
    onMenuSaveFile: (callback) => ipcRenderer.on('menu-save-file', callback),
    onMenuSaveFileAs: (callback) => ipcRenderer.on('menu-save-file-as', callback),
    onMenuNewTab: (callback) => ipcRenderer.on('menu-new-tab', callback),
    onMenuCloseTab: (callback) => ipcRenderer.on('menu-close-tab', callback),
    onMenuCompileRun: (callback) => ipcRenderer.on('menu-compile-run', callback),
});
