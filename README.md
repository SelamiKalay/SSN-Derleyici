# SSN Derleyici

Türkçe söz dizimli bir programlama dili ve onun derleyicisi. Kaynak kod önce
bytecode'a derlenir, ardından yığın (stack) tabanlı bir sanal makinede çalıştırılır.
Derleyici hiçbir dış kütüphane kullanmadan, sıfırdan C++17 ile yazılmıştır.

```
Kaynak Kod → Lexer → Parser → AST → Compiler → Bytecode → VM
```

## Örnek

```
// Fibonacci dizisi (ilk 10 terim)
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

degisken x = !5 + 3! * 2 ;
yaz "Sonuc: &x" ;
```

## Dil Özellikleri

| Yapı | Söz dizimi |
|---|---|
| Değişken tanımlama | `degisken x = 5 ;` |
| Atama | `x = x + 1 ;` |
| Ekrana yazdırma | `yaz x ;` |
| Koşul | `eger x > 10 ise ( ... )` |
| Döngü | `iken i < 5 ise ( ... )` |
| Gruplama (parantez yerine) | `!5 + 3! * 2` |
| String interpolasyonu | `"Merhaba &isim"` |
| Satır sonu | `;` veya `é` |

- Aritmetik: `+ - * /` (operatör önceliğiyle)
- Karşılaştırma: `< > == !=`
- Veri tipleri: sayı (ondalıklı) ve metin

## Derleyici Aşamaları

- **Lexer** (`src/lexer.cpp`) — anahtar kelimeler, sayılar, metinler, `&değişken`
  interpolasyonu ve UTF-8 `é` karakteri dahil tokenizasyon
- **Parser** (`src/parser.cpp`) — recursive descent ile AST üretimi, satır numaralı
  hata mesajları
- **Compiler** (`src/compiler.cpp`) — AST'den bytecode üretimi (koşullu/koşulsuz
  sıçramalar, string birleştirme)
- **VM** (`src/vm.cpp`) — 18 opcode'lu yığın tabanlı sanal makine
- `--debug` modu: token listesi ve bytecode disassembly çıktısı

## IDE'ler

- **desktop-ide/** — Electron + Monaco Editor tabanlı masaüstü IDE (söz dizimi
  renklendirme, otomatik tamamlama, hazır örnekler, `Ctrl+Enter` ile derle & çalıştır)
- **web-ide/** — Node.js/Express sunuculu tarayıcı tabanlı IDE (`SSN_IDE.bat` ile başlatılır; derleyicinin tek dosyalık sürümü `web-ide/compiler.cpp`)

## Derleme ve Çalıştırma

Derleyici (CMake + C++17 derleyici gerekir):

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

```bash
ssn program.tc                  # programı çalıştırır
ssn --debug program.tc          # token ve bytecode çıktısını da gösterir
```

Masaüstü IDE (derlenmiş `compiler.exe` dosyası `desktop-ide/` klasörüne konulmalıdır):

```bash
cd desktop-ide
npm install
npm start
```

## Kullanılan Teknolojiler

C++17 · CMake · Electron · Monaco Editor · Node.js / Express
