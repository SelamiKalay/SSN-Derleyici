const electron = require('electron');
console.log('typeof electron:', typeof electron);
console.log('electron app:', electron.app ? 'exists' : 'undefined');
console.log('process.versions.electron:', process.versions.electron);
console.log('process.type:', process.type);
console.log('__dirname:', __dirname);

if (electron.app) {
    electron.app.whenReady().then(() => {
        console.log('Electron app is ready!');
        electron.app.quit();
    });
} else {
    console.error('electron.app is undefined! Exiting.');
    process.exit(1);
}
