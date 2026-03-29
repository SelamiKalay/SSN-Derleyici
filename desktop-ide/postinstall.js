// postinstall.js — npm install sonrasi calisir
// Electron modül cakismasini cozer:
// node_modules/electron/index.js dosyasini yeniden adlandirir
// boylece require('electron') Electron'un dahili modulunu yukler

const fs = require('fs');
const path = require('path');

const electronIndex = path.join(__dirname, 'node_modules', 'electron', 'index.js');
const electronIndexBak = electronIndex + '.original';

if (fs.existsSync(electronIndex) && !fs.existsSync(electronIndexBak)) {
    fs.renameSync(electronIndex, electronIndexBak);
    console.log('[postinstall] Electron modul cakismasi giderildi.');
    console.log('  Yeniden adlandirilan: node_modules/electron/index.js -> index.js.original');
}
