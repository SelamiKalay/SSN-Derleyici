const pngToIco = require('png-to-ico');
const fs = require('fs');
const path = require('path');

const inputPng = path.join(__dirname, 'build', 'icon.png');
const outputIco = path.join(__dirname, 'build', 'icon.ico');

pngToIco(inputPng)
  .then(buf => {
    fs.writeFileSync(outputIco, buf);
    console.log('Successfully converted icon.png to icon.ico');
  })
  .catch(err => {
    console.error('Error converting to ico:', err);
    process.exit(1);
  });
