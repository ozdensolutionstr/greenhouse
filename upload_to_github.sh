#!/bin/bash

cd "/Volumes/M3 Portable/Kocaeli_üni_sera"

echo "📁 Proje klasörüne geçildi: $(pwd)"

echo "📦 Git durumu kontrol ediliyor..."
git status

echo "📝 Değişiklikler commit ediliyor..."
git add .

# Commit var mı kontrol et
if git diff --cached --quiet; then
    echo "ℹ️  Commit edilecek değişiklik yok."
else
    git commit -m "Update: Akıllı Sera Kontrol Sistemi - ESP8266 IoT sera otomasyonu"
fi

echo "🔗 Eski remote kaldırılıyor (varsa)..."
git remote remove origin 2>/dev/null || true

echo "🚀 GitHub'da 'greenhouse' repository oluşturuluyor..."
gh repo create greenhouse --public --source=. --remote=origin --description "ESP8266 tabanlı IoT akıllı sera otomasyon sistemi - Web tabanlı kontrol, sensör izleme, otomatik ekipman kontrolü ve WebSocket gerçek zamanlı güncelleme" --push || {
    echo "⚠️  GitHub CLI ile repo oluşturulamadı. Manuel olarak oluşturun ve şu komutu çalıştırın:"
    echo "   git remote add origin https://github.com/KULLANICI_ADINIZ/greenhouse.git"
    echo "   git push -u origin main"
    exit 1
}

echo "✅ Proje başarıyla GitHub'a yüklendi!"
echo "🔗 Repository URL: https://github.com/ozdensolutionstr/greenhouse"

