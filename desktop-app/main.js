// Debug: check what require('electron') returns
const e = require('electron');
console.log('typeof e:', typeof e);
console.log('e value:', e);
console.log('has app?', !!e.app);
console.log('process.type:', process.type);
console.log('electron version:', process.versions.electron);
process.exit(0);
