SSN Web IDE Kurulum ve Çalıştırma

1. Bu klasördeki "SSN_IDE.bat" dosyasına çift tıklayın.
2. Uygulama otomatik olarak açılacaktır.

Eğer masaüstünde kısayol istiyorsanız:
1. "SSN_IDE.bat" dosyasına sağ tıklayın.
2. "Gönder" -> "Masaüstü (kısayol oluştur)" seçeneğini tıklayın.

Not: Node.js (https://nodejs.org) kurulu olmalıdır. Derleyici proje kökünde
CMake ile derlenmiş olmalıdır (build\Release\ssn.exe):
  cmake -B build -DCMAKE_BUILD_TYPE=Release
  cmake --build build --config Release
Farklı bir yer için SSN_COMPILER ortam değişkenini kullanın.
