# 🌱 Akıllı Sera Kontrol Sistemi

ESP8266 tabanlı IoT akıllı sera otomasyon sistemi. Web tabanlı kontrol arayüzü ile sera içi ortam parametrelerini izleyebilir ve ekipmanları uzaktan kontrol edebilirsiniz.

![Version](https://img.shields.io/badge/version-2.1-green.svg)
![Platform](https://img.shields.io/badge/platform-ESP8266-orange.svg)
![License](https://img.shields.io/badge/license-MIT-blue.svg)

---

## 📋 İçindekiler

- [Özellikler](#-özellikler)
- [Donanım Gereksinimleri](#-donanım-gereksinimleri)
- [Yazılım Gereksinimleri](#-yazılım-gereksinimleri)
- [Kurulum](#-kurulum)
- [Donanım Bağlantıları](#-donanım-bağlantıları)
- [Kullanım](#-kullanım)
- [Web Arayüzü](#-web-arayüzü)
- [API Endpoints](#-api-endpoints)
- [Proje Yapısı](#-proje-yapısı)
- [Sorun Giderme](#-sorun-giderme)
- [Geliştirici Notları](#-geliştirici-notları)

---

## ✨ Özellikler

### Sensör Özellikleri
- 🌡️ **Sıcaklık Ölçümü**: DHT11 sensörü ile ortam sıcaklığı takibi
- 💧 **Nem Ölçümü**: DHT11 sensörü ile hava nem oranı izleme
- 🌱 **Toprak Nem Ölçümü**: Analog toprak nem sensörü ile toprak nem seviyesi kontrolü
- ☀️ **Işık Seviyesi**: BH1750 I2C sensörü ile lux değeri ölçümü

### Kontrol Özellikleri
- 💡 **PWM Işık Kontrolü**: 0-100% arası ayarlanabilir LED/ışık kontrolü
- 💦 **PWM Pompa Kontrolü**: Su pompası hız kontrolü (0-100%)
- 🌬️ **PWM Fan Kontrolü**: Ventilasyon fanı hız kontrolü (0-100%)
- 🔥 **Isıtıcı Kontrolü**: Dijital ON/OFF ısıtıcı kontrolü
- 🚪 **Servo Kapı Kontrolü**: 0-180° arası kapı açma/kapama (non-blocking)

### Web Özellikleri
- 📱 **Responsive Tasarım**: Mobil ve masaüstü uyumlu modern arayüz
- 🔄 **WebSocket İletişimi**: Gerçek zamanlı veri güncellemesi
- 🎨 **Dinamik Renk Kodları**: Sensör değerlerine göre renkli gösterim
- ⚙️ **Kalibrasyon Arayüzü**: Toprak nem sensörü için web tabanlı kalibrasyon
- 🔧 **Servo Test Arayüzü**: Servo motor test ve ayar sayfası

---

## 🔧 Donanım Gereksinimleri

| Bileşen | Miktar | Açıklama |
|---------|--------|----------|
| ESP8266 (NodeMCU/Wemos D1 Mini) | 1 | Ana kontrol ünitesi |
| DHT11 Sensör | 1 | Sıcaklık ve nem ölçümü |
| BH1750 Sensör | 1 | Işık seviyesi ölçümü (I2C) |
| Toprak Nem Sensörü (Analog) | 1 | Toprak nem ölçümü |
| Servo Motor (SG90 veya benzeri) | 1 | Kapı kontrolü |
| LED/İşık Modülü | 1 | PWM kontrollü (isteğe bağlı) |
| Su Pompası | 1 | PWM kontrollü DC motor |
| Fan Modülü | 1 | PWM kontrollü DC fan |
| Isıtıcı Modülü | 1 | Dijital ON/OFF kontrollü (röle modülü) |
| Röle Modülü (5V) | 1 | Isıtıcı kontrolü için |
| Breadboard veya PCB | 1 | Bağlantılar için |
| Jumper Kablolar | - | Bağlantı kabloları |
| Dirençler | - | Gerekirse pull-up/pull-down |

### Güç Gereksinimleri
- **ESP8266**: 5V (USB veya harici güç kaynağı)
- **Servo Motor**: 5V ayrı güç kaynağı önerilir
- **Sensörler**: 3.3V veya 5V (ESP8266'den sağlanabilir)
- **Aktüatörler**: Ayrı güç kaynağı gerekebilir (fan, pompa, ısıtıcı)

---

## 💻 Yazılım Gereksinimleri

### Arduino IDE Kurulumu
1. [Arduino IDE](https://www.arduino.cc/en/software) (1.8.x veya 2.x) indirin ve kurun
2. **ESP8266 Board Manager** ekleyin:
   - Arduino IDE'de `Dosya > Tercihler`
   - "Ek Kart Yöneticisi URL'leri" bölümüne ekleyin:
     ```
     http://arduino.esp8266.com/stable/package_esp8266com_index.json
     ```
   - `Araçlar > Kart > Kart Yöneticisi` açın
   - "esp8266" araştırın ve **ESP8266** paketini kurun (versiyon 3.0.x)

### Gerekli Kütüphaneler

Arduino IDE'de **Kütüphane Yöneticisi**nden şu kütüphaneleri kurun:

| Kütüphane | Versiyon | Kurulum |
|-----------|----------|---------|
| **ESP8266WiFi** | - | ESP8266 paketi ile gelir |
| **ESP8266WebServer** | - | ESP8266 paketi ile gelir |
| **WebSocketsServer** | - | `Sketch > Include Library > Manage Libraries` → "WebSockets" araştırın |
| **ArduinoJson** | 6.x | "ArduinoJson" araştırın ve kurun |
| **DHT sensor library** | 1.4.x | "DHT sensor library" araştırın ve kurun |
| **Servo** | - | ESP8266 paketi ile gelir |
| **Wire** | - | ESP8266 paketi ile gelir |
| **BH1750** | - | "BH1750" araştırın ve kurun |

### Kütüphane Kurulum Komutları (Alternatif)
```
Sketch > Include Library > Manage Libraries
```

Sırasıyla şu kütüphaneleri arayın ve kurun:
- WebSockets (Markus Sattler)
- ArduinoJson (Benoit Blanchon)
- DHT sensor library (Adafruit)
- BH1750

---

## 🚀 Kurulum

### 1. Kod Hazırlığı

1. `GreenHouse.ino` dosyasını açın
2. **WiFi Bilgilerini Ayarlayın** (satır 23-24):
   ```cpp
   const char* ssid = "WIFI_SSID_BURAYA";
   const char* pass = "WIFI_SIFRE_BURAYA";
   ```

### 2. Arduino IDE Ayarları

1. Kart Seçimi: `Araçlar > Kart > ESP8266 Boards > NodeMCU 1.0 (ESP-12E Module)`
2. Upload Speed: `115200` (veya daha düşük bir değer)
3. CPU Frequency: `80 MHz`
4. Flash Frequency: `40 MHz`
5. Flash Size: `4MB (FS:2MB OTA:~1019KB)`
6. Port: USB bağlantı noktanızı seçin

### 3. Yükleme

1. ESP8266'yı bilgisayarınıza USB ile bağlayın
2. `Sketch > Yükle` veya `Ctrl+U` tuşlarına basın
3. Yükleme tamamlandıktan sonra `Seri Port İzleyici` açın (115200 baud)

### 4. İlk Başlatma

1. ESP8266'yı yeniden başlatın (reset butonu veya USB çıkar/tak)
2. Seri port izleyicide şu mesajları görmelisiniz:
   ```
   Sera Kontrol Sistemi Başlatılıyor...
   WiFi bağlanıyor...
   WiFi bağlandı!
   IP Adresi: 192.168.x.x
   HTTP sunucusu başlatıldı.
   WebSocket sunucusu başlatıldı.
   Kurulum tamamlandı!
   ```
3. IP adresini not edin

---

## 🔌 Donanım Bağlantıları

### Pin Bağlantı Şeması

```
ESP8266          Bileşen           Bağlantı
─────────────────────────────────────────────
D4 (GPIO2)    →  DHT11 DATA       →  DHT11 Data Pin
A0            →  Toprak Sensörü   →  Analog Çıkış
D2 (GPIO4)    →  BH1750 SDA       →  I2C SDA
D1 (GPIO5)    →  BH1750 SCL       →  I2C SCL
D3 (GPIO0)    →  LED/Işık Modülü →  PWM Kontrol
D6 (GPIO12)   →  Pompa (Röle)     →  PWM Kontrol
D7 (GPIO13)   →  Fan (Röle)       →  PWM Kontrol
D0 (GPIO16)   →  Isıtıcı (Röle)   →  Dijital ON/OFF
D5 (GPIO14)   →  Servo Signal     →  Servo Data Pin

GND           →  Tüm GND Uçlar    →  Ortak Toprak
3.3V/5V       →  Sensörler VCC    →  Güç Beslemesi
```

### Detaylı Bağlantı Diyagramı

```
┌─────────────────┐
│   ESP8266       │
│                 │
│  D4  ──────────→│ DHT11 DATA
│  A0  ──────────→│ Toprak Sensör Analog
│  D2  ──────────→│ BH1750 SDA (I2C)
│  D1  ──────────→│ BH1750 SCL (I2C)
│  D3  ──────────→│ LED/Işık (PWM)
│  D6  ──────────→│ Pompa Rölesi (PWM)
│  D7  ──────────→│ Fan Rölesi (PWM)
│  D0  ──────────→│ Isıtıcı Rölesi (DIGITAL)
│  D5  ──────────→│ Servo Signal
│  3.3V/5V ──────→│ Sensörler VCC
│  GND  ─────────→│ Tüm GND
└─────────────────┘
```

### Önemli Notlar

⚠️ **Güç Kaynağı**: Servo motor, fan, pompa ve ısıtıcı için ayrı güç kaynağı kullanın. ESP8266'nın 5V pininden yüksek akımlı cihazları beslemeyin!

⚠️ **Pull-up Dirençler**: DHT11 sensörü için 4.7kΩ - 10kΩ pull-up direnci gerekebilir.

⚠️ **I2C Pull-up**: BH1750 için I2C hatlarına pull-up dirençler eklenmelidir (genellikle modülde mevcuttur).

⚠️ **Röle Modülü**: Fan, pompa ve ısıtıcı için 5V röle modülü kullanın. Röle modülünün VCC'sini ESP8266'nın 5V pinine bağlamayın, harici güç kaynağı kullanın.

---

## 📱 Kullanım

### Web Arayüzüne Erişim

1. ESP8266 ile aynı WiFi ağına bağlı bir cihazdan tarayıcınızı açın
2. Adres çubuğuna ESP8266'nın IP adresini yazın (örnek: `192.168.1.100`)
3. Ana kontrol sayfası yüklenecektir

### Ana Sayfa Özellikleri

#### Sensör Verileri Bölümü
- **Sıcaklık**: Ortam sıcaklığı (°C) - Renk kodlu gösterim
  - 🔴 Kırmızı: >30°C (çok sıcak)
  - 🟡 Sarı: 25-30°C (sıcak)
  - 🟢 Yeşil: 15-25°C (ideal)
  - 🔵 Mavi: <15°C (soğuk)

- **Nem**: Hava nem oranı (%) - Renk kodlu gösterim
  - 🔵 Mavi: >80% (çok nemli)
  - 🟢 Yeşil: 40-80% (ideal)
  - 🟡 Sarı: <40% (kuru)

- **Toprak Nemi**: Toprak nem seviyesi (%) - Renk kodlu gösterim
  - 🔴 Kırmızı: <30% (çok kuru)
  - 🟢 Yeşil: 30-80% (ideal)
  - 🔵 Mavi: >80% (çok nemli)

- **Işık Seviyesi**: Lux değeri - Renk kodlu gösterim
  - 🟡 Sarı: >1000 lux (çok parlak)
  - 🟢 Yeşil: 200-1000 lux (ideal)
  - 🔵 Mavi: <200 lux (karanlık)

#### Kontrol Bölümü

- **💡 Işık Kontrolü**: Slider ile 0-100% arası PWM kontrolü
- **💦 Su Pompası**: Slider ile 0-100% arası pompa hız kontrolü
- **🌬️ Fan Kontrolü**: Slider ile 0-100% arası fan hız kontrolü
- **🔥 Isıtıcı**: Slider ile 0-100% arası kontrol (0% = OFF, >0% = ON)
- **🚪 Kapı Kontrolü**: Buton ile aç/kapa toggle işlemi

### Özel Sayfalar

#### Servo Test Sayfası
URL: `http://[ESP8266_IP]/servo`

- Servo motorun çalışmasını test edin
- 0°, 45°, 90°, 135°, 180° açılarına doğrudan geçiş
- Slider ile hassas açı kontrolü
- Otomatik test fonksiyonu

#### Kalibrasyon Sayfası
URL: `http://[ESP8266_IP]/calibrate`

- Toprak nem sensörünü kalibre edin
- Sensörün ham değerini görüntüleyin
- Kuru ve ıslak referans değerlerini kaydedin
- Kalibrasyon değerleri hafızada saklanır (yeniden başlatmada sıfırlanır)

---

## 🌐 Web Arayüzü

### Tasarım Özellikleri

- 🎨 **Modern Dark Theme**: Koyu tema ile göz yormayan arayüz
- 📱 **Tam Responsive**: Mobil, tablet ve masaüstü uyumlu
- 🔄 **Gerçek Zamanlı Güncelleme**: WebSocket ile anlık veri iletimi
- 🎯 **Renk Kodlu Göstergeler**: Sensör değerlerine göre dinamik renkler
- ⚡ **Hızlı ve Akıcı**: Optimize edilmiş JavaScript ve CSS

### Tarayıcı Desteği

- ✅ Chrome/Edge (önerilir)
- ✅ Firefox
- ✅ Safari
- ✅ Opera
- ❌ Internet Explorer (desteklenmez)

---

## 🔌 API Endpoints

### HTTP Endpoints

| Endpoint | Method | Açıklama | Parametreler |
|----------|--------|----------|--------------|
| `/` | GET | Ana kontrol sayfası | - |
| `/servo` | GET | Servo test sayfası | - |
| `/calibrate` | GET | Kalibrasyon sayfası | - |
| `/set-servo` | GET | Servo açısı ayarla | `angle` (0-180) |
| `/test-servo` | GET | Servo test fonksiyonu | - |
| `/raw` | GET | Toprak sensörü ham değer | - |
| `/set` | GET | Kalibrasyon değeri kaydet | `type` (dry/wet), `value` |

### WebSocket Protokolü

**Port**: 81

#### Gönderilen Veri (ESP8266 → İstemci)
```json
{
  "type": "sensor",
  "temp": 25,
  "hum": 60,
  "mois": 45,
  "lux": 500,
  "light": 0,
  "pump": 0,
  "fan": 0,
  "heat": 0,
  "door": false
}
```

#### Alınan Komutlar (İstemci → ESP8266)

**Işık Kontrolü:**
```json
{
  "type": "control",
  "target": "light",
  "value": 75
}
```

**Pompa Kontrolü:**
```json
{
  "type": "control",
  "target": "pump",
  "value": 50
}
```

**Fan Kontrolü:**
```json
{
  "type": "control",
  "target": "fan",
  "value": 80
}
```

**Isıtıcı Kontrolü:**
```json
{
  "type": "control",
  "target": "heat",
  "value": 100
}
```

**Kapı Kontrolü:**
```json
{
  "type": "control",
  "target": "door",
  "value": "toggle"
}
```

**Durum Sorgulama:**
```json
{
  "type": "getStatus"
}
```

**Kalibrasyon:**
```json
{
  "type": "calibrate",
  "min": 1023,
  "max": 300
}
```

---

## 📁 Proje Yapısı

```
GreenHouse.ino
├── Pin Tanımlamaları
│   ├── Sensör Pinleri (DHT11, Toprak, BH1750)
│   └── Aktüatör Pinleri (Işık, Pompa, Fan, Isıtıcı, Servo)
├── WiFi Konfigürasyonu
│   └── SSID ve Şifre Ayarları
├── Nesne Oluşturma
│   ├── Web Server (Port 80)
│   ├── WebSocket Server (Port 81)
│   ├── DHT Sensörü
│   ├── BH1750 Sensörü
│   └── Servo Motor
├── Fonksiyonlar
│   ├── readSensors() - Sensör okuma
│   ├── updateServo() - Servo kontrolü (non-blocking)
│   ├── sendSensorData() - WebSocket veri gönderimi
│   ├── webSocketEvent() - WebSocket olay yönetimi
│   └── setupServer() - HTTP endpoint tanımlamaları
├── HTML Arayüzü
│   ├── CSS Stilleri (Dark Theme)
│   └── JavaScript (WebSocket İletişimi)
└── Main Loop
    ├── Sensör Okuma (5 saniyede bir)
    ├── Servo Güncelleme
    └── Web Sunucu İşleme
```

---

## 🐛 Sorun Giderme

### WiFi Bağlantı Sorunları

**Problem**: WiFi bağlanmıyor
- ✅ SSID ve şifrenin doğru olduğundan emin olun
- ✅ ESP8266'nın WiFi ağınızın menzilinde olduğunu kontrol edin
- ✅ 2.4 GHz WiFi kullandığınızdan emin olun (ESP8266 5 GHz desteklemez)
- ✅ Seri port izleyicide hata mesajlarını kontrol edin

**Çözüm**: Seri port izleyicide `WiFi bağlantısı başarısız!` mesajı görürseniz, kodu yeniden yükleyin veya ESP8266'yı resetleyin.

### Sensör Okuma Sorunları

**Problem**: DHT11 okumaları NaN (Not a Number) gösteriyor
- ✅ DHT11'in bağlantılarını kontrol edin
- ✅ D4 pininin doğru bağlandığını kontrol edin
- ✅ 4.7kΩ pull-up direnci ekleyin
- ✅ Sensörün besleme voltajını kontrol edin (3.3V veya 5V)

**Problem**: Toprak nem sensörü yanlış değerler gösteriyor
- ✅ Sensörü kalibre edin (`/calibrate` sayfasından)
- ✅ Sensörün toprağa düzgün gömüldüğünden emin olun
- ✅ A0 pininin bağlantısını kontrol edin
- ✅ Sensörün ham değerlerini kontrol edin (`/raw` endpoint'i)

**Problem**: BH1750 okumaları çalışmıyor
- ✅ I2C bağlantılarını (SDA/SCL) kontrol edin
- ✅ Pull-up dirençlerini kontrol edin
- ✅ Sensör modülünün çalıştığından emin olun
- ✅ Seri port izleyicide I2C hatalarını kontrol edin

### Servo Motor Sorunları

**Problem**: Servo motor hareket etmiyor
- ✅ Servo'nun ayrı güç kaynağından beslendiğinden emin olun
- ✅ Signal kablosunun D5 pinine bağlı olduğunu kontrol edin
- ✅ Servo'nun GND'sinin ESP8266'nın GND'si ile ortak olduğunu kontrol edin
- ✅ `/servo` test sayfasını kullanarak servo'yu test edin

**Problem**: Servo titreşiyor veya kararsız
- ✅ Servo için yeterli güç kaynağı kullanın (en az 1A)
- ✅ Elektriksel gürültüyü azaltın (kondansatör ekleyin)
- ✅ Güç kablolarını kısa tutun

### Web Arayüzü Sorunları

**Problem**: Web sayfası yüklenmiyor
- ✅ ESP8266'nın IP adresini doğru girdiğinizden emin olun
- ✅ Tarayıcı önbelleğini temizleyin (Ctrl+F5)
- ✅ ESP8266 ile aynı WiFi ağında olduğunuzdan emin olun

**Problem**: WebSocket bağlantısı kurulamıyor
- ✅ Port 81'in açık olduğunu kontrol edin
- ✅ Tarayıcı konsolunda hata mesajlarını kontrol edin (F12)
- ✅ Firewall ayarlarını kontrol edin

### Aktüatör Kontrol Sorunları

**Problem**: Işık/Pompa/Fan çalışmıyor
- ✅ PWM pinlerinin doğru bağlandığını kontrol edin
- ✅ Röle modülünün çalıştığını test edin
- ✅ Aktüatörlerin güç kaynağını kontrol edin
- ✅ Seri port izleyicide kontrol komutlarının geldiğini kontrol edin

**Problem**: Isıtıcı çalışmıyor
- ✅ D0 pininin bağlantısını kontrol edin
- ✅ Röle modülünün çalıştığını test edin
- ✅ Isıtıcının güç kaynağını kontrol edin
- ✅ Slider değerinin 0'dan büyük olduğundan emin olun (>0% = ON)

---

## 👨‍💻 Geliştirici Notları

### Kod Yapısı

- **Non-blocking Servo Kontrolü**: `updateServo()` fonksiyonu, servo hareketini yumuşak bir şekilde gerçekleştirir ve ana loop'u bloklamaz
- **Sensör Okuma İntervali**: Sensörler 5 saniyede bir okunur (ayarlanabilir)
- **WebSocket Broadcast**: Tüm bağlı istemcilere otomatik veri gönderimi
- **WDT Besleme**: `yield()` çağrıları ile Watchdog Timer resetini önler

### Özelleştirme

**Sensör Okuma Sıklığını Değiştirme:**
```cpp
// loop() fonksiyonunda (satır 1069)
if (currentTime - lastSensorUpdate >= 5000) { // 5000 = 5 saniye
    // Değeri değiştirerek aralığı ayarlayın
}
```

**Servo Hareket Hızını Değiştirme:**
```cpp
// updateServo() fonksiyonunda (satır 93)
if (currentTime - lastServoUpdate >= 15) { // 15ms = hareket hızı
    // Daha küçük değer = daha hızlı
    // Daha büyük değer = daha yavaş
}
```

**Toprak Sensörü Varsayılan Değerleri:**
```cpp
// Global değişkenlerde (satır 41-42)
int soilMin = 1023; // Kuru değer (ayarlanabilir)
int soilMax = 300;  // Islak değer (ayarlanabilir)
```

### Bilinen Sınırlamalar

- ❌ Kalibrasyon değerleri EEPROM'da saklanmaz (yeniden başlatmada sıfırlanır)
- ❌ Otomatik kontrol algoritması yok (manuel kontrol gereklidir)
- ❌ Veri loglama özelliği yok
- ❌ Çoklu kullanıcı yönetimi yok
- ❌ SSL/TLS desteği yok (güvenlik için HTTPS önerilir)

### Gelecek Geliştirmeler

- ✅ EEPROM ile kalibrasyon değerlerini saklama
- ✅ Otomatik kontrol algoritmaları (örneğin: toprak nemi <30% ise pompa aç)
- ✅ Veri loglama ve grafik gösterimi
- ✅ Mobil uygulama desteği
- ✅ Email/SMS bildirimleri
- ✅ OTA (Over-The-Air) güncelleme desteği
- ✅ Çoklu cihaz yönetimi
- ✅ Veritabanı entegrasyonu

---

## 📝 Lisans

Bu proje eğitim amaçlı geliştirilmiştir. Kullanımınız kendi sorumluluğunuzdadır.

---

## 👥 Katkıda Bulunanlar

- Proje geliştiricisi: [Sizin Adınız]

---

## 📞 İletişim ve Destek

Sorularınız veya önerileriniz için:
- 📧 Email: [email adresiniz]
- 🐛 Issues: GitHub Issues kullanın

---

## 🙏 Teşekkürler

Bu projede kullanılan açık kaynak kütüphanelere teşekkürler:
- ESP8266 Community
- ArduinoJson - Benoit Blanchon
- WebSockets - Markus Sattler
- Adafruit DHT Library
- BH1750 Library

---

**🌱 Başarılı bir sera otomasyonu dileriz! 🌱**

---

*Son Güncelleme: 2024*
*Versiyon: 2.1*

