// ============================================================
// SSN Desktop IDE — Renderer (App.js)
// Monaco Editor + Derleyici Entegrasyonu + SSN Dil Tanımı
// Multi-Tab Desteği
// ============================================================

// ── Global Değişkenler ──
let editor = null;
let monacoInstance = null;
let isCompiling = false;

// ── Tab Sistemi ──
let tabs = [];          // { id, name, filePath, content, viewState, isModified }
let activeTabId = null;
let tabIdCounter = 0;
let suppressContentChange = false;  // setValue sırasında onChange'i bastır

// ============================================================
//  TAB YÖNETİMİ
// ============================================================

function generateTabId() {
    return ++tabIdCounter;
}

function createTab(name, filePath, content) {
    // Aynı dosya açıksa o tab'a geç (filePath veya isim ile kontrol)
    if (filePath) {
        const existing = tabs.find(t => t.filePath === filePath);
        if (existing) {
            switchTab(existing.id);
            return existing;
        }
    } else if (name && name !== 'Adsız') {
        // filePath yoksa isim ile kontrol (örnekler için)
        const existing = tabs.find(t => t.name === name && !t.filePath);
        if (existing) {
            switchTab(existing.id);
            return existing;
        }
    }

    const tab = {
        id: generateTabId(),
        name: name || 'Adsız',
        filePath: filePath || null,
        content: content || '',
        viewState: null,
        isModified: false
    };

    tabs.push(tab);
    switchTab(tab.id);
    return tab;
}

function switchTab(tabId) {
    if (activeTabId === tabId) {
        renderTabs();
        return;
    }

    // Mevcut tab'ın durumunu kaydet
    if (activeTabId !== null && editor) {
        const currentTab = tabs.find(t => t.id === activeTabId);
        if (currentTab) {
            currentTab.content = editor.getValue();
            currentTab.viewState = editor.saveViewState();
        }
    }

    activeTabId = tabId;
    const tab = tabs.find(t => t.id === tabId);
    if (!tab || !editor) return;

    // Yeni tab'ın içeriğini yükle (onChange'i bastır)
    suppressContentChange = true;
    editor.setValue(tab.content);
    suppressContentChange = false;

    if (tab.viewState) {
        editor.restoreViewState(tab.viewState);
    }

    renderTabs();

    // Render sonrası editöre odaklan
    setTimeout(() => {
        if (editor) editor.focus();
    }, 0);
}

function closeTab(tabId) {
    const tabIndex = tabs.findIndex(t => t.id === tabId);
    if (tabIndex === -1) return;

    const tab = tabs[tabIndex];

    // Değişiklik varsa uyar
    if (tab.isModified) {
        if (!confirm(`"${tab.name}" dosyasında kaydedilmemiş değişiklikler var.\nKapatmak istediğinize emin misiniz?`)) {
            return;
        }
    }

    tabs.splice(tabIndex, 1);

    // Hiç tab kalmadıysa yeni bir tane oluştur
    if (tabs.length === 0) {
        createTab('Adsız', null, '');
        return;
    }

    // Kapatılan aktif tab ise başka bir tab'a geç
    if (activeTabId === tabId) {
        const newIndex = Math.min(tabIndex, tabs.length - 1);
        activeTabId = null; // switchTab'ın çalışması için sıfırla
        switchTab(tabs[newIndex].id);
    } else {
        renderTabs();
    }
}

function renderTabs() {
    const tabBar = document.getElementById('tabBar');
    if (!tabBar) return;

    tabBar.innerHTML = '';

    tabs.forEach(tab => {
        const el = document.createElement('div');
        el.className = 'editor-tab';
        if (tab.id === activeTabId) el.classList.add('active');
        if (tab.isModified) el.classList.add('modified');

        el.innerHTML = `
            <span class="tab-icon"></span>
            <span class="tab-label">${escapeHtml(tab.name)}</span>
            <button class="tab-close" title="Sekmeyi Kapat"><span class="close-x">×</span></button>
        `;

        // Tab'a tıklayınca geç
        el.addEventListener('click', (e) => {
            // Close butonuna tıklandıysa tab'ı kapat
            if (e.target.closest('.tab-close')) return;
            switchTab(tab.id);
        });

        // Close butonuna tıklayınca kapat
        el.querySelector('.tab-close').addEventListener('click', (e) => {
            e.stopPropagation();
            closeTab(tab.id);
        });

        // Orta tuşla kapat
        el.addEventListener('mousedown', (e) => {
            if (e.button === 1) {
                e.preventDefault();
                closeTab(tab.id);
            }
        });

        tabBar.appendChild(el);
    });

    // Aktif tab'ı görünür yap
    const activeEl = tabBar.querySelector('.editor-tab.active');
    if (activeEl) {
        activeEl.scrollIntoView({ behavior: 'smooth', block: 'nearest', inline: 'nearest' });
    }
}

function getActiveTab() {
    return tabs.find(t => t.id === activeTabId) || null;
}

function markActiveTabModified() {
    const tab = getActiveTab();
    if (tab && !tab.isModified) {
        tab.isModified = true;
        renderTabs();
    }
}

function escapeHtml(str) {
    const div = document.createElement('div');
    div.textContent = str;
    return div.innerHTML;
}

// ============================================================
//  MONACO EDITOR BAŞLATMA
// ============================================================
document.addEventListener('DOMContentLoaded', () => {
    // Require configuration with relative paths
    require.config({ paths: { 'vs': '../node_modules/monaco-editor/min/vs' } });

    require(['vs/editor/editor.main'], function (monaco) {

        monacoInstance = monaco;

        // ── SSN Dil Tanımı ──
        registerSSN(monaco);

    // ── Editor Oluştur ──
    editor = monaco.editor.create(document.getElementById('monacoContainer'), {
        value: '',
        language: 'ssn',
        theme: 'ssn-dark',
        fontFamily: "'JetBrains Mono', 'Cascadia Code', 'Fira Code', 'Consolas', monospace",
        fontSize: 14,
        lineHeight: 24,
        tabSize: 4,
        insertSpaces: true,
        minimap: { enabled: true, maxColumn: 80 },
        scrollBeyondLastLine: false,
        roundedSelection: true,
        cursorBlinking: 'smooth',
        cursorSmoothCaretAnimation: 'on',
        smoothScrolling: true,
        renderLineHighlight: 'all',
        renderWhitespace: 'selection',
        bracketPairColorization: { enabled: true },
        guides: { bracketPairs: true, indentation: true },
        padding: { top: 12 },
        automaticLayout: true,
        wordWrap: 'off',
        suggest: {
            showKeywords: true,
            showSnippets: true
        }
    });

    // İlk tab'ı oluştur
    createTab('Adsız', null, '');

    // ── İmleç Konumu Takibi ──
    editor.onDidChangeCursorPosition((e) => {
        document.getElementById('cursorInfo').textContent =
            `Satır ${e.position.lineNumber}, Sütun ${e.position.column}`;
    });

    // ── Değişiklik Takibi ──
    editor.onDidChangeModelContent(() => {
        if (!suppressContentChange) {
            markActiveTabModified();
        }
    });

    // ── Ctrl+Enter → Derle ──
    editor.addAction({
        id: 'compile-run',
        label: 'Derle & Çalıştır',
        keybindings: [monaco.KeyMod.CtrlCmd | monaco.KeyCode.Enter],
        run: () => compileAndRun()
    });

    // ── Ctrl+S → Kaydet ──
    editor.addAction({
        id: 'save-file',
        label: 'Dosya Kaydet',
        keybindings: [monaco.KeyMod.CtrlCmd | monaco.KeyCode.KeyS],
        run: () => saveFile()
    });

    // ── Ctrl+N → Yeni Sekme ──
    editor.addAction({
        id: 'new-tab',
        label: 'Yeni Sekme',
        keybindings: [monaco.KeyMod.CtrlCmd | monaco.KeyCode.KeyN],
        run: () => createTab('Adsız', null, '')
    });

    // ── Ctrl+W → Sekme Kapat ──
    editor.addAction({
        id: 'close-tab',
        label: 'Sekmeyi Kapat',
        keybindings: [monaco.KeyMod.CtrlCmd | monaco.KeyCode.KeyW],
        run: () => { if (activeTabId !== null) closeTab(activeTabId); }
    });

    // Başlangıç mesajı
    console.log('SSN IDE: Monaco Editor hazır.');
    
    // Uygulamanın hazır olduğunu belirten eventi tetikle
    document.dispatchEvent(new Event('EditorReady'));
    });
});

// ============================================================
//  SSN DİL TANIMI ve TEMA
// ============================================================
function registerSSN(monaco) {

    // ── Dili Kaydet ──
    monaco.languages.register({
        id: 'ssn',
        extensions: ['.tc'],
        aliases: ['SSN', 'ssn']
    });

    // ── Token Kuralları (Monarch Sözdizimi) ──
    monaco.languages.setMonarchTokensProvider('ssn', {
        keywords: ['degisken', 'eger', 'ise', 'iken', 'yaz'],

        operators: ['+', '-', '*', '/', '=', '==', '!=', '<', '>'],

        symbols: /[=><~?:&|+\-*\/^%]+/,

        tokenizer: {
            root: [
                // Yorum satırları
                [/\/\/.*$/, 'comment'],

                // Boşluk
                [/\s+/, 'white'],

                // Anahtar kelimeler ve tanımlayıcılar
                [/[a-zA-Z_]\w*/, {
                    cases: {
                        '@keywords': 'keyword',
                        '@default': 'identifier'
                    }
                }],

                // Sayılar
                [/\d+\.?\d*/, 'number'],

                // String literalleri (interpolasyon desteği)
                [/"/, { token: 'string.quote', bracket: '@open', next: '@string' }],

                // Gruplama (parantez yerine ünlem)
                [/!/, 'delimiter.bracket'],

                // Parantezler (blok)
                [/[()]/, 'delimiter.parenthesis'],

                // Satır sonu (noktalı virgül)
                [/;/, 'delimiter'],

                // Operatörler
                [/==|!=/, 'operator'],
                [/[<>]=?/, 'operator'],
                [/[+\-*\/]/, 'operator'],
                [/=/, 'operator'],

                // é karakteri (satır sonu alternatifi)
                [/é/, 'delimiter'],
            ],

            string: [
                // String interpolasyon — &degisken
                [/&[a-zA-Z_]\w*/, 'string.interpolation'],
                // Escape sekansları
                [/\\[nrt"\\&]/, 'string.escape'],
                // String sonu
                [/"/, { token: 'string.quote', bracket: '@close', next: '@pop' }],
                // Normal karakter
                [/[^"\\&]+/, 'string'],
                // Diğer
                [/./, 'string'],
            ]
        }
    });

    // ── Otomatik Tamamlama (Autocomplete) ──
    monaco.languages.registerCompletionItemProvider('ssn', {
        provideCompletionItems: (model, position) => {
            const suggestions = [
                {
                    label: 'degisken',
                    kind: monaco.languages.CompletionItemKind.Keyword,
                    insertText: 'degisken ${1:isim} = ${2:deger} ;',
                    insertTextRules: monaco.languages.CompletionItemInsertTextRule.InsertAsSnippet,
                    documentation: 'Yeni bir değişken tanımla'
                },
                {
                    label: 'yaz',
                    kind: monaco.languages.CompletionItemKind.Keyword,
                    insertText: 'yaz ${1:ifade} ;',
                    insertTextRules: monaco.languages.CompletionItemInsertTextRule.InsertAsSnippet,
                    documentation: 'Değer yazdır'
                },
                {
                    label: 'eger',
                    kind: monaco.languages.CompletionItemKind.Keyword,
                    insertText: 'eger ${1:kosul} ise (\n\t${2}\n)',
                    insertTextRules: monaco.languages.CompletionItemInsertTextRule.InsertAsSnippet,
                    documentation: 'Koşullu ifade (if)'
                },
                {
                    label: 'iken',
                    kind: monaco.languages.CompletionItemKind.Keyword,
                    insertText: 'iken ${1:kosul} ise (\n\t${2}\n)',
                    insertTextRules: monaco.languages.CompletionItemInsertTextRule.InsertAsSnippet,
                    documentation: 'Döngü (while)'
                },
                {
                    label: 'ise',
                    kind: monaco.languages.CompletionItemKind.Keyword,
                    insertText: 'ise',
                    documentation: 'Koşul bloğu başlangıcı'
                }
            ];

            return { suggestions };
        }
    });

    // ── SSN Dark Tema ──
    monaco.editor.defineTheme('ssn-dark', {
        base: 'vs-dark',
        inherit: true,
        rules: [
            { token: 'keyword',              foreground: 'ff7b72', fontStyle: 'bold' },  // Kırmızımsı
            { token: 'identifier',           foreground: 'c9d1d9' },                     // Gri
            { token: 'number',               foreground: '79c0ff' },                     // Mavi
            { token: 'string',               foreground: 'a5d6ff' },                     // Açık mavi
            { token: 'string.quote',         foreground: 'a5d6ff' },
            { token: 'string.escape',        foreground: 'ffa657' },                     // Turuncu
            { token: 'string.interpolation', foreground: 'ffa657', fontStyle: 'bold' },  // Turuncu (kalın)
            { token: 'operator',             foreground: 'ff7b72' },                     // Kırmızımsı
            { token: 'delimiter',            foreground: '8b949e' },                     // Gri
            { token: 'delimiter.bracket',    foreground: 'd2a8ff' },                     // Mor (! gruplama)
            { token: 'delimiter.parenthesis',foreground: 'ffa657' },                     // Turuncu (blok)
            { token: 'comment',              foreground: '6e7681', fontStyle: 'italic' },// Yorum
        ],
        colors: {
            'editor.background':                '#0d1117',
            'editor.foreground':                '#c9d1d9',
            'editorCursor.foreground':          '#58a6ff',
            'editor.lineHighlightBackground':   '#161b2280',
            'editorLineNumber.foreground':      '#484f58',
            'editorLineNumber.activeForeground':'#e6edf3',
            'editor.selectionBackground':       '#264f78',
            'editor.inactiveSelectionBackground':'#264f7840',
            'editorIndentGuide.background':     '#21262d',
            'editorIndentGuide.activeBackground':'#30363d',
            'editorBracketMatch.background':    '#3fb95020',
            'editorBracketMatch.border':        '#3fb95060',
            'minimap.background':               '#0d1117',
        }
    });
}

// ============================================================
//  VARSAYILAN KOD
// ============================================================
function getDefaultCode() {
    return `// SSN — Merhaba Dünya!
// Kısayol: Ctrl+Enter → Derle & Çalıştır

degisken mesaj = "Merhaba Dunya" ;
yaz mesaj ;

degisken x = !5 + 3! * 2 ;
yaz "Sonuc: &x" ;

// Kontrol akışı örneği
eger x > 10 ise (
    yaz "x, 10'dan buyuk!" ;
)

// Döngü örneği
degisken i = 0 ;
iken i < 5 ise (
    yaz i ;
    i = i + 1 ;
)
`;
}

// ============================================================
//  ÖRNEK KODLAR
// ============================================================
const EXAMPLES = {
    aritmetik: `// Aritmetik İşlemler
degisken x = !5+3! * 2 ;
yaz x ;
degisken y = 10 + !2 * 3! ;
yaz y ;
degisken z = !x + y! * 2 ;
yaz z ;
`,
    kontrol: `// Kontrol Akışı: eger & iken
degisken x = 10 ;
eger x > 5 ise (
    yaz "Buyuk" ;
)
degisken i = 0 ;
iken i < 5 ise (
    yaz i ;
    i = i + 1 ;
)
`,
    string: `// String İşlemleri ve İnterpolasyon
degisken x = 42 ;
yaz "Sonuc = &x" ;
degisken isim = "Dunya" ;
yaz "Merhaba &isim!" ;
`,
    fibonacci: `// Fibonacci Dizisi (İlk 10 Terim)
degisken a = 0 ;
degisken b = 1 ;
degisken i = 0 ;
iken i < 10 ise (
    yaz a ;
    degisken temp = b ;
    b = a + b ;
    a = temp ;
    i = i + 1 ;
)
`,
    toplam: `// 1'den N'e Toplam
degisken n = 100 ;
degisken toplam = 0 ;
degisken i = 1 ;
iken i < n + 1 ise (
    toplam = toplam + i ;
    i = i + 1 ;
)
yaz "1'den 100'e toplam:" ;
yaz toplam ;
`
};

// ============================================================
//  DERLEME & ÇALIŞTIRMA
// ============================================================
async function compileAndRun() {
    if (isCompiling || !editor) return;

    const code = editor.getValue().trim();
    if (!code) {
        appendOutput('⚠ Editör boş! Lütfen kod yazın.', 'stderr');
        return;
    }

    isCompiling = true;
    setCompileState(true);

    // Çıktı paneline bilgi yaz
    appendSeparator();
    const tab = getActiveTab();
    const tabLabel = tab ? tab.name : 'Adsız';
    appendOutput(`▶ Derleniyor... [${tabLabel}]`, 'info');

    try {
        const result = await window.electronAPI.compileCode(code);

        if (result.success) {
            if (result.stdout) {
                appendOutput(result.stdout, 'stdout');
            }
            if (result.stderr) {
                appendOutput(result.stderr, 'stderr');
            }
            appendOutput(`✓ Başarıyla tamamlandı (${result.duration}ms)`, 'info');
            setStatus('ready', 'Hazır');
        } else {
            if (result.stdout) {
                appendOutput(result.stdout, 'stdout');
            }
            if (result.stderr) {
                appendOutput(result.stderr, 'stderr');
            }
            appendOutput(`✗ Hata ile sonlandı (${result.duration}ms)`, 'info');
            setStatus('error', 'Hata');
        }
    } catch (err) {
        appendOutput(`✗ Beklenmeyen hata: ${err.message}`, 'stderr');
        setStatus('error', 'Hata');
    } finally {
        isCompiling = false;
        setCompileState(false);
    }
}

// ============================================================
//  DOSYA İŞLEMLERİ
// ============================================================
async function openFile() {
    try {
        const result = await window.electronAPI.openFile();
        if (result.canceled) return;

        const fileName = result.filePath.split(/[/\\]/).pop();

        // Aynı dosya açıksa o tab'a geç
        const existing = tabs.find(t => t.filePath === result.filePath);
        if (existing) {
            switchTab(existing.id);
            appendOutput(`📂 Dosyaya geçildi: ${fileName}`, 'info');
            return;
        }

        // Yeni tab oluştur
        createTab(fileName, result.filePath, result.content);
        appendOutput(`📂 Dosya açıldı: ${fileName}`, 'info');
    } catch (err) {
        appendOutput(`✗ Dosya açma hatası: ${err.message}`, 'stderr');
    }
}

async function saveFile() {
    if (!editor) return;
    const tab = getActiveTab();
    if (!tab) return;

    // Aktif tab'ın en güncel içeriğini al
    tab.content = editor.getValue();

    try {
        const result = await window.electronAPI.saveFile({
            code: tab.content,
            filePath: tab.filePath
        });

        if (result.canceled) return;

        tab.filePath = result.filePath;
        tab.isModified = false;
        tab.name = result.filePath.split(/[/\\]/).pop();

        renderTabs();
        setStatus('ready', 'Kaydedildi');
        appendOutput(`💾 Kaydedildi: ${tab.name}`, 'info');
    } catch (err) {
        appendOutput(`✗ Kaydetme hatası: ${err.message}`, 'stderr');
    }
}

async function saveFileAs() {
    if (!editor) return;
    const tab = getActiveTab();
    if (!tab) return;

    tab.content = editor.getValue();

    try {
        const result = await window.electronAPI.saveFileAs({
            code: tab.content
        });

        if (result.canceled) return;

        tab.filePath = result.filePath;
        tab.isModified = false;
        tab.name = result.filePath.split(/[/\\]/).pop();

        renderTabs();
        setStatus('ready', 'Kaydedildi');
        appendOutput(`💾 Farklı kaydedildi: ${tab.name}`, 'info');
    } catch (err) {
        appendOutput(`✗ Kaydetme hatası: ${err.message}`, 'stderr');
    }
}

// ============================================================
//  ÇIKTI PANELİ FONKSİYONLARI
// ============================================================
function appendOutput(text, type = 'stdout') {
    const output = document.getElementById('outputContent');

    // İlk kullanımda hoş geldin mesajını temizle
    const welcome = output.querySelector('.welcome-message');
    if (welcome) welcome.remove();

    const line = document.createElement('div');

    switch (type) {
        case 'stdout':
            line.className = 'line-stdout';
            break;
        case 'stderr':
            line.className = 'line-stderr';
            break;
        case 'info':
            line.className = 'line-info';
            break;
        default:
            line.className = 'line-stdout';
    }

    line.textContent = text;
    output.appendChild(line);

    // Otomatik scroll
    output.scrollTop = output.scrollHeight;
}

function appendSeparator() {
    const output = document.getElementById('outputContent');
    const welcome = output.querySelector('.welcome-message');
    if (welcome) welcome.remove();

    const sep = document.createElement('span');
    sep.className = 'separator';
    output.appendChild(sep);

    // Zaman damgası
    const ts = document.createElement('div');
    ts.className = 'line-timestamp';
    ts.textContent = `[${new Date().toLocaleTimeString('tr-TR')}]`;
    output.appendChild(ts);
}

function clearOutput() {
    const output = document.getElementById('outputContent');
    output.innerHTML = '';
    document.getElementById('execTime').textContent = '';
}

// ============================================================
//  UI DURUM YÖNETİMİ
// ============================================================
function setCompileState(compiling) {
    const btn = document.getElementById('btnCompile');
    const dot = document.getElementById('statusDot');
    const text = document.getElementById('statusText');

    if (compiling) {
        btn.classList.add('running');
        btn.disabled = true;
        btn.querySelector('.btn-text').textContent = 'Çalışıyor...';
        dot.className = 'status-dot compiling';
        text.textContent = 'Derleniyor...';
    } else {
        btn.classList.remove('running');
        btn.disabled = false;
        btn.querySelector('.btn-text').textContent = 'Derle & Çalıştır';
    }
}

function setStatus(state, text) {
    const dot = document.getElementById('statusDot');
    const statusText = document.getElementById('statusText');

    dot.className = 'status-dot';
    if (state === 'error') dot.classList.add('error');

    statusText.textContent = text;

    // 4 saniye sonra "Hazır"'a dön
    if (state !== 'ready') {
        setTimeout(() => {
            dot.className = 'status-dot';
            statusText.textContent = 'Hazır';
        }, 4000);
    }
}

// ============================================================
//  KOMUT PANELİ
// ============================================================
function toggleCommands() {
    const overlay = document.getElementById('commandsOverlay');
    if (overlay.style.display === 'none') {
        overlay.style.display = 'flex';
    } else {
        overlay.style.display = 'none';
    }
}

// ============================================================
//  YENIDEN BOYUTLANDIRILABILIR PANEL (Resize Handle)
// ============================================================
function initResizeHandle() {
    const handle = document.getElementById('resizeHandle');
    const editorPanel = document.getElementById('editorPanel');
    const outputPanel = document.getElementById('outputPanel');
    const mainContent = document.querySelector('.main-content');

    let isResizing = false;

    handle.addEventListener('mousedown', (e) => {
        isResizing = true;
        handle.classList.add('active');
        document.body.style.cursor = 'ns-resize';
        document.body.style.userSelect = 'none';
        e.preventDefault();
    });

    document.addEventListener('mousemove', (e) => {
        if (!isResizing) return;

        const containerRect = mainContent.getBoundingClientRect();
        const mouseY = e.clientY - containerRect.top;
        const totalHeight = containerRect.height;

        const editorHeight = Math.max(150, Math.min(totalHeight - 120, mouseY));
        const outputHeight = totalHeight - editorHeight - 4; // 4px = handle

        editorPanel.style.flex = 'none';
        editorPanel.style.height = editorHeight + 'px';
        outputPanel.style.height = outputHeight + 'px';
    });

    document.addEventListener('mouseup', () => {
        if (isResizing) {
            isResizing = false;
            handle.classList.remove('active');
            document.body.style.cursor = '';
            document.body.style.userSelect = '';
        }
    });
}

// ============================================================
//  OLAY DİNLEYİCİLER
// ============================================================
document.addEventListener('DOMContentLoaded', () => {
    // Butonlar
    document.getElementById('btnCompile').addEventListener('click', compileAndRun);
    document.getElementById('btnOpen').addEventListener('click', openFile);
    document.getElementById('btnSave').addEventListener('click', saveFile);
    document.getElementById('btnClear').addEventListener('click', clearOutput);
    document.getElementById('btnCommands').addEventListener('click', toggleCommands);
    document.getElementById('btnCloseCommands').addEventListener('click', toggleCommands);
    document.getElementById('btnNewTab').addEventListener('click', () => {
        createTab('Adsız', null, '');
    });

    // Overlay'e tıklayınca kapat
    document.getElementById('commandsOverlay').addEventListener('click', (e) => {
        if (e.target === document.getElementById('commandsOverlay')) {
            toggleCommands();
        }
    });

    // Örnek butonları — her örnek yeni tab'da açılır
    document.querySelectorAll('.btn-example').forEach(btn => {
        btn.addEventListener('click', () => {
            const key = btn.dataset.example;
            if (EXAMPLES[key] && editor) {
                createTab(key + '.tc', null, EXAMPLES[key]);
            }
        });
    });

    // Resize handle
    initResizeHandle();

    // Sürükle-Bırak (Drag & Drop) Desteği
    document.addEventListener('dragover', (e) => {
        e.preventDefault();
        e.stopPropagation();
        e.dataTransfer.dropEffect = 'copy';
    });

    document.addEventListener('drop', async (e) => {
        e.preventDefault();
        e.stopPropagation();
        
        if (e.dataTransfer.files && e.dataTransfer.files.length > 0) {
            // Birden fazla dosya sürüklenebilir
            for (let i = 0; i < e.dataTransfer.files.length; i++) {
                const file = e.dataTransfer.files[i];
                const filePath = window.electronAPI?.getPathForFile ? window.electronAPI.getPathForFile(file) : file.path;

                if (filePath && window.electronAPI) {
                    try {
                        const result = await window.electronAPI.readFileFromPath(filePath);
                        if (!result.canceled) {
                            const fileName = result.filePath.split(/[/\\]/).pop();
                            createTab(fileName, result.filePath, result.content);
                            appendOutput(`📂 Dosya sürüklendi ve açıldı: ${fileName}`, 'info');
                        }
                    } catch (err) {
                        appendOutput(`✗ Dosya okuma hatası: ${err.message}`, 'stderr');
                    }
                } else {
                    appendOutput(`✗ Sürüklenen dosyanın yolu alınamadı. (Electron Sandbox)`, 'stderr');
                }
            }
        }
    });

    // Menü olayları (main process'ten gelen)
    if (window.electronAPI) {
        window.electronAPI.onMenuOpenFile(() => openFile());
        window.electronAPI.onMenuSaveFile(() => saveFile());
        window.electronAPI.onMenuSaveFileAs(() => saveFileAs());
        window.electronAPI.onMenuCompileRun(() => compileAndRun());
        window.electronAPI.onMenuNewTab(() => createTab('Adsız', null, ''));
        window.electronAPI.onMenuCloseTab(() => { if (activeTabId !== null) closeTab(activeTabId); });
    }
});
