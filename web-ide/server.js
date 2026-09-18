// ============================================================
// SSN Web IDE — Node.js Backend
// ============================================================

const express = require('express');
const { exec } = require('child_process');
const fs = require('fs');
const path = require('path');

const app = express();
const PORT = 3000;

// Middleware
app.use(express.json({ limit: '1mb' }));
app.use(express.static(path.join(__dirname, 'public')));

// Compiler yolu (aynı klasörde derlenmiş olmalı)
const COMPILER_PATH = path.join(__dirname, 'compiler.exe');
const TEMP_FILE = path.join(__dirname, 'temp_code.tc');

// ============================================================
// POST /run — Kod çalıştırma endpoint'i
// ============================================================
app.post('/run', (req, res) => {
    const { code } = req.body;

    if (!code || code.trim().length === 0) {
        return res.json({
            success: false,
            output: '',
            error: 'Kod boş olamaz!'
        });
    }

    // Kodu geçici dosyaya yaz
    const tempFileWithId = path.join(__dirname, `temp_${Date.now()}.tc`);

    try {
        fs.writeFileSync(tempFileWithId, code, { encoding: 'utf8' });
    } catch (writeErr) {
        console.error('Dosya yazma hatası:', writeErr);
        return res.status(500).json({ success: false, error: 'Sunucu hatası: Kod dosyası yazılamadı.' });
    }

    // Compiler'ı çalıştır - 5 saniye zaman aşımı
    // ÖNEMLİ: COMPILER_PATH boşluk içeriyorsa tırnak içine alınmalı
    exec(`"${COMPILER_PATH}" "${tempFileWithId}"`, { timeout: 5000 }, (error, stdout, stderr) => {
        // Geçici dosyayı temizle
        try {
            if (fs.existsSync(tempFileWithId)) fs.unlinkSync(tempFileWithId);
        } catch (e) { /* yoksay */ }

        if (error) {
            // Zaman aşımı
            if (error.signal === 'SIGTERM') {
                return res.json({
                    success: false,
                    output: 'HATA: İşlem zaman aşımına uğradı (5 saniye). Sonsuz döngü olabilir.\n',
                    error: 'Timeout'
                });
            }
            // Compiler hata kodu döndürdüyse (syntax hatası vs)
            // stdout/stderr yine de dolu olabilir
        }

        res.json({
            success: !error || error.code === 0,
            output: stdout || stderr || (error ? error.message : ''),
            error: error ? error.message : null
        });
    });
});

// Ana sayfa
app.get('/', (req, res) => {
    res.sendFile(path.join(__dirname, 'public', 'index.html'));
});

// Sunucuyu başlat
app.listen(PORT, () => {
    console.log(`\n  ╔══════════════════════════════════════════╗`);
    console.log(`  ║   SSN IDE                                ║`);
    console.log(`  ║   http://localhost:${PORT}                  ║`);
    console.log(`  ╚══════════════════════════════════════════╝\n`);

    // Compiler varlık kontrolü
    if (!fs.existsSync(COMPILER_PATH)) {
        console.warn('  ⚠ UYARI: compiler.exe bulunamadı!');
        console.warn('  Derlemek için:');
        console.warn('    cl /EHsc /utf-8 /O2 compiler.cpp /Fe:compiler.exe');
        console.warn('');
    } else {
        console.log('  ✓ Compiler hazır: compiler.exe');
    }
});

module.exports = app;
