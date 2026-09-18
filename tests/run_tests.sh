#!/usr/bin/env bash
# SSN test çalıştırıcısı
# Kullanım: bash tests/run_tests.sh <derleyici-yolu>
#   tests/ornekler/*.tc  -> çıktı, yanındaki .out dosyasıyla birebir aynı olmalı
#   tests/hatali/*.tc    -> derleyici hata vermeli (çıkış kodu != 0, "HATA" mesajı)
set -u
BIN="${1:?Kullanım: bash tests/run_tests.sh <derleyici-yolu>}"
DIR="$(cd "$(dirname "$0")" && pwd)"
gecti=0; kaldi=0

# Derleyiciyi çalıştırır; Windows satır sonlarını (CRLF) LF'e çevirir.
# Çıktıyı $cikti, çıkış kodunu $kod değişkenine yazar.
calistir() {
    local ham
    ham="$("$BIN" "$1" 2>&1)"; kod=$?
    cikti="$(printf '%s' "$ham" | tr -d '\r')"
}

for tc in "$DIR"/ornekler/*.tc; do
    ad="$(basename "$tc")"
    beklenen="$(tr -d '\r' < "${tc%.tc}.out")"
    calistir "$tc"
    if [ "$kod" -eq 0 ] && [ "$cikti" = "$beklenen" ]; then
        echo "  GEÇTİ    $ad"; gecti=$((gecti + 1))
    else
        echo "  KALDI    $ad (çıkış kodu: $kod)"
        diff <(printf '%s\n' "$beklenen") <(printf '%s\n' "$cikti") | sed 's/^/           /'
        kaldi=$((kaldi + 1))
    fi
done

for tc in "$DIR"/hatali/*.tc; do
    ad="$(basename "$tc")"
    calistir "$tc"
    if [ "$kod" -ne 0 ] && printf '%s' "$cikti" | grep -q "HATA"; then
        echo "  GEÇTİ    hatali/$ad"; gecti=$((gecti + 1))
    else
        echo "  KALDI    hatali/$ad (hata bekleniyordu, çıkış kodu: $kod)"
        kaldi=$((kaldi + 1))
    fi
done

echo
echo "Sonuç: $gecti geçti, $kaldi kaldı"
[ "$kaldi" -eq 0 ]
