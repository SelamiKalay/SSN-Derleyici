// ============================================================
// SSN Web IDE — Node.js Backend
// ============================================================

const express = require('express');
const { execFile } = require('child_process');
const fs = require('fs');
const path = require('path');

const app = express();
const PORT = 3000;

// Middleware
app.use(express.json({ limit: '1mb' }));
app.use(express.static(path.join(__dirname, 'public')));

// Derleyici: src/ altindan CMake ile derlenen `ssn`. Aranan yerler, sirayla:
//   1. SSN_COMPILER ortam degiskeni
//   2. ../build/Release/ssn.exe, ../build/ssn.exe, ../build/ssn  (cmake -B build)
//   3. bu klasordeki compiler.exe (eski kurulumlarla uyumluluk)
function findCompiler() {
    const candidates = [
        process.env.SSN_COMPILER,
        path.join(__dirname, '..', 'build', 'Release', 'ssn.exe'),
        path.join(__dirname, '..', 'build', 'ssn.exe'),
        path.join(__dirname, '..', 'build', 'ssn'),
        path.join(__dirname, 'compiler.exe'),
    ];
    return candidates.find(p => p && fs.existsSync(p)) || null;
}
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
    const compilerPath = findCompiler();
    if (!compilerPath) {
        try { fs.unlinkSync(tempFileWithId); } catch (_) {}
        return res.json({ success: false, output: '',
            error: 'HATA: Derleyici (ssn) bulunamadı. Proje kökünde derleyin: cmake -B build && cmake --build build' });
    }
    execFile(compilerPath, [tempFileWithId], { timeout: 5000 }, (error, stdout, stderr) => {
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
app.listen(PORT, '127.0.0.1', () => {
    console.log(`\n  ╔══════════════════════════════════════════╗`);
    console.log(`  ║   SSN IDE                                ║`);
    console.log(`  ║   http://localhost:${PORT}                  ║`);
    console.log(`  ╚══════════════════════════════════════════╝\n`);

    // Compiler varlık kontrolü
    const compilerPath = findCompiler();
    if (!compilerPath) {
        console.warn('  ⚠ UYARI: derleyici (ssn) bulunamadı!');
        console.warn('  Proje kökünde derlemek için:');
        console.warn('    cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build --config Release');
        console.warn('');
    } else {
        console.log(`  ✓ Derleyici hazır: ${compilerPath}`);
    }
});

module.exports = app;
