#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <Servo.h>
#include <Wire.h>
#include <BH1750.h>

// Pin Tanımlamaları
#define DHT_PIN   D4
#define DHT_TYPE  DHT11
#define MOIS      A0
#define SDA_PIN   D2
#define SCL_PIN   D1
#define LIGHT_PIN D3
#define PUMP_PIN  D6
#define HEAT_PIN  D0
#define FAN_PIN   D7
#define DOOR_PIN  D5

// WiFi Bilgileri
const char* ssid = "WIFI_SSID_BURAYA";
const char* pass = "WIFI_SIFRE_BURAYA";

// Nesneleri oluştur
ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);
DHT dht(DHT_PIN, DHT_TYPE);
BH1750 lightSensor;
Servo servo;

// Global değişkenler
int temp = 25, hum = 50, mois = 50;
int luxValue = 100; // Işık sensörü değeri
int currentAngle = 0, targetAngle = 0;
int lightValue = 0, pumpValue = 0, fanValue = 0, heatValue = 0;
bool doorOpen = false;
unsigned long lastSensorUpdate = 0;
unsigned long lastServoUpdate = 0;
int soilMin = 1023; // Toprak sensörü kuru değeri
int soilMax = 300;  // Toprak sensörü ıslak değeri

// Sensör okuma fonksiyonu
void readSensors() {
  // DHT okuma
  float tempReading = dht.readTemperature();
  float humReading = dht.readHumidity();
  
  if (!isnan(tempReading)) {
    temp = (int)tempReading;
  }
  
  if (!isnan(humReading)) {
    hum = (int)humReading;
  }
  
  // Toprak nemi okuma
  int reading = analogRead(MOIS);
  if (reading > 0) { // Geçerli okuma kontrolü
    // Aralığı genişlet, daha geniş bir değer aralığı kullan
    // Not: Toprak sensörü ıslakken düşük, kuru iken yüksek değer verir
    mois = map(reading, soilMin, soilMax, 0, 100);
    mois = constrain(mois, 0, 100);
    
    // DEBUG: Sensör okumalarını göster
    Serial.println("Toprak Sensörü Ham Değer: " + String(reading) + 
                   ", Dönüştürülmüş Değer: " + String(mois) + "%");
  }
  
  // Işık sensörü okuma
  if (lightSensor.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    float lux = lightSensor.readLightLevel();
    if (lux >= 0 && lux < 65535) {
      luxValue = (int)lux;
    }
  }
  
  Serial.println("Sensör Değerleri - Sıcaklık: " + String(temp) + 
                 "°C, Nem: " + String(hum) + 
                 "%, Toprak Nemi: " + String(mois) + 
                 "%, Işık: " + String(luxValue) + " lux");
                 
  // Sensör verilerini tüm WebSocket istemcilerine gönder
  sendSensorData();
}

// Servo kontrol (non-blocking) - daha güvenilir hale getirildi
void updateServo() {
  // Eğer kapı kontrol ediliyorsa
  if (currentAngle != targetAngle) {
    unsigned long currentTime = millis();
    if (currentTime - lastServoUpdate >= 15) { // Servo hareket hızı
      lastServoUpdate = currentTime;
      
      if (currentAngle < targetAngle) {
        currentAngle++;
      } else if (currentAngle > targetAngle) {
        currentAngle--;
      }
      
      servo.write(currentAngle);
      
      // DEBUG: Servo pozisyonunu göster
      Serial.printf("Servo Pozisyonu güncellendi: %d, Hedef: %d\n", currentAngle, targetAngle);
      
      yield(); // WDT'yi besle
    }
  }
}

// WebSocket üzerinden sensör verilerini gönder
void sendSensorData() {
  DynamicJsonDocument doc(256);
  doc["type"] = "sensor";
  doc["temp"] = temp;
  doc["hum"] = hum;
  doc["mois"] = mois;
  doc["lux"] = luxValue;
  doc["light"] = lightValue;
  doc["pump"] = pumpValue;
  doc["fan"] = fanValue;
  doc["heat"] = heatValue;
  doc["door"] = doorOpen;
  
  String jsonString;
  serializeJson(doc, jsonString);
  webSocket.broadcastTXT(jsonString);
}

// HTML ana sayfası
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="tr">
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <meta charset="UTF-8">
    <title>Akıllı Sera Kontrolü</title>
    <style>
        :root {
            --bg-dark: #121212;
            --bg-card: #1e1e1e;
            --text-primary: #ffffff;
            --text-secondary: #b0b0b0;
            --primary: #4ade80;
            --primary-dark: #22c55e;
            --primary-light: #86efac;
            --accent: #60a5fa;
            --danger: #f87171;
            --warning: #fbbf24;
            --success: #34d399;
            --border-radius: 12px;
            --card-shadow: 0 4px 6px rgba(0, 0, 0, 0.5);
            --card-shadow-hover: 0 10px 15px rgba(0, 0, 0, 0.5);
        }
        
        * { 
            box-sizing: border-box; 
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            transition: all 0.3s ease;
        }
        
        body { 
            margin: 0; 
            padding: 20px; 
            background-color: var(--bg-dark);
            color: var(--text-primary);
            min-height: 100vh;
        }
        
        .container { 
            max-width: 1000px;
            margin: 0 auto;
            padding: 20px 0;
        }
        
        header {
            display: flex;
            align-items: center;
            justify-content: center;
            margin-bottom: 30px;
        }
        
        .icon {
            font-size: 36px;
            margin-right: 10px;
            color: var(--primary);
        }
        
        h1 { 
            color: var(--primary);
            text-align: center;
            font-size: 2.5rem;
            margin: 0;
            text-shadow: 0 0 10px rgba(74, 222, 128, 0.3);
        }
        
        h2 {
            color: var(--primary);
            font-size: 1.5rem;
            border-bottom: 2px solid var(--primary);
            padding-bottom: 8px;
            margin-top: 0;
        }
        
        .card { 
            background: var(--bg-card);
            border-radius: var(--border-radius);
            padding: 25px;
            margin-bottom: 25px;
            box-shadow: var(--card-shadow);
            border: 1px solid rgba(255, 255, 255, 0.1);
            position: relative;
            overflow: hidden;
        }
        
        .card:hover {
            box-shadow: var(--card-shadow-hover);
            transform: translateY(-5px);
        }
        
        .card:after {
            content: '';
            position: absolute;
            top: 0;
            left: 0;
            right: 0;
            height: 3px;
            background: linear-gradient(90deg, var(--primary), var(--accent));
            border-radius: var(--border-radius) var(--border-radius) 0 0;
        }
        
        .grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 20px;
        }
        
        .sensor-card {
            background: rgba(255, 255, 255, 0.05);
            border-radius: var(--border-radius);
            padding: 20px;
            text-align: center;
            border: 1px solid rgba(255, 255, 255, 0.1);
            position: relative;
            overflow: hidden;
        }
        
        .sensor-icon {
            font-size: 2rem;
            margin-bottom: 10px;
        }
        
        .sensor-title {
            color: var(--text-secondary);
            font-size: 0.9rem;
            margin-bottom: 10px;
        }
        
        .sensor-value {
            font-size: 2rem;
            font-weight: bold;
            margin-bottom: 5px;
        }
        
        .temp-icon { color: var(--warning); }
        .humidity-icon { color: var(--accent); }
        .soil-icon { color: var(--primary); }
        .light-icon { color: var(--warning); }
        
        .sensor-unit {
            font-size: 0.9rem;
            color: var(--text-secondary);
        }
        
        .control { 
            margin: 25px 0; 
            position: relative;
        }
        
        .slider-container {
            position: relative;
            padding-bottom: 25px;
        }
        
        label { 
            display: block; 
            margin-bottom: 10px; 
            font-weight: 500; 
            color: var(--text-primary);
            font-size: 1.1rem;
        }
        
        .value-display {
            position: absolute;
            bottom: 0;
            right: 0;
            background-color: var(--primary);
            color: black;
            font-weight: bold;
            padding: 3px 8px;
            border-radius: 10px;
            font-size: 0.9rem;
        }
        
        input[type="range"] { 
            width: 100%;
            height: 8px;
            -webkit-appearance: none;
            background: rgba(255, 255, 255, 0.1);
            border-radius: 5px;
            outline: none;
            opacity: 0.7;
        }
        
        input[type="range"]:hover {
            opacity: 1;
        }
        
        input[type="range"]::-webkit-slider-thumb {
            -webkit-appearance: none;
            width: 20px;
            height: 20px;
            background: var(--primary);
            border-radius: 50%;
            cursor: pointer;
            box-shadow: 0 0 5px rgba(0, 0, 0, 0.5);
        }
        
        button { 
            background: var(--primary);
            color: black;
            border: none;
            padding: 15px;
            border-radius: var(--border-radius);
            cursor: pointer;
            font-weight: bold;
            font-size: 1.1rem;
            width: 100%;
            text-transform: uppercase;
            letter-spacing: 1px;
            position: relative;
            overflow: hidden;
            box-shadow: 0 4px 15px rgba(74, 222, 128, 0.4);
        }
        
        button:hover { 
            background: var(--primary-dark);
            box-shadow: 0 6px 20px rgba(74, 222, 128, 0.6);
            transform: translateY(-2px);
        }

        button:active {
            transform: translateY(1px);
        }
        
        .button-icon {
            margin-right: 8px;
        }
        
        .status { 
            padding: 15px;
            text-align: center;
            font-weight: bold;
            position: fixed;
            top: 20px;
            left: 50%;
            transform: translateX(-50%);
            border-radius: var(--border-radius);
            z-index: 100;
            box-shadow: var(--card-shadow);
            transition: all 0.5s ease;
        }
        
        .online { 
            background-color: rgba(52, 211, 153, 0.2);
            color: var(--success);
            border: 1px solid var(--success);
        }
        
        .offline { 
            background-color: rgba(248, 113, 113, 0.2);
            color: var(--danger);
            border: 1px solid var(--danger);
        }
        
        .connecting {
            background-color: rgba(251, 191, 36, 0.2);
            color: var(--warning);
            border: 1px solid var(--warning);
        }
        
        footer { 
            text-align: center;
            margin-top: 40px;
            padding-top: 20px;
            color: var(--text-secondary);
            font-size: 0.9rem;
            border-top: 1px solid rgba(255, 255, 255, 0.1);
        }
        
        @media (max-width: 768px) {
            .grid {
                grid-template-columns: repeat(2, 1fr);
            }
            
            h1 {
                font-size: 1.8rem;
            }
            
            .sensor-value {
                font-size: 1.5rem;
            }
            
            .status {
                width: 90%;
                font-size: 0.9rem;
            }
        }
        
        @media (max-width: 480px) {
            .grid {
                grid-template-columns: 1fr;
            }
            
            .card {
                padding: 15px;
            }
        }
        
        /* Yüklenme animasyonu */
        .loading {
            display: inline-block;
            position: relative;
            width: 15px;
            height: 15px;
            margin-left: 5px;
        }
        
        .loading div {
            position: absolute;
            width: 12px;
            height: 12px;
            background: var(--warning);
            border-radius: 50%;
            animation: loading 1.2s linear infinite;
        }
        
        @keyframes loading {
            0% { transform: scale(0); opacity: 1; }
            100% { transform: scale(1); opacity: 0; }
        }
        
        /* Gece modu / Gündüz modu için koşullu renk */
        .day-time {
            background-color: rgba(251, 191, 36, 0.1);
        }
        
        .night-time {
            background-color: rgba(96, 165, 250, 0.1);
        }
    </style>
</head>
<body>
    <div class="container">
        <header>
            <div class="icon">🌱</div>
            <h1>Akıllı Sera Kontrolü</h1>
        </header>
        
        <div id="status" class="status connecting">Bağlanıyor... <span class="loading"><div></div></span></div>
        
        <div class="card">
            <h2>Sensör Verileri</h2>
            <div class="grid">
                <div class="sensor-card">
                    <div class="sensor-icon temp-icon">🌡️</div>
                    <div class="sensor-title">SICAKLIK</div>
                    <div class="sensor-value" id="tempValue">--</div>
                    <div class="sensor-unit">°C</div>
                </div>
                
                <div class="sensor-card">
                    <div class="sensor-icon humidity-icon">💧</div>
                    <div class="sensor-title">NEM</div>
                    <div class="sensor-value" id="humValue">--</div>
                    <div class="sensor-unit">%</div>
                </div>
                
                <div class="sensor-card">
                    <div class="sensor-icon soil-icon">🌱</div>
                    <div class="sensor-title">TOPRAK NEMİ</div>
                    <div class="sensor-value" id="moisValue">--</div>
                    <div class="sensor-unit">%</div>
                </div>
                
                <div class="sensor-card">
                    <div class="sensor-icon light-icon">☀️</div>
                    <div class="sensor-title">IŞIK SEVİYESİ</div>
                    <div class="sensor-value" id="luxValue">--</div>
                    <div class="sensor-unit">lux</div>
                </div>
            </div>
        </div>
        
        <div class="card">
            <h2>Sistem Kontrolleri</h2>
            
            <div class="control">
                <label for="light">💡 Işık Kontrolü</label>
                <div class="slider-container">
                    <input type="range" id="light" min="0" max="100" value="0" oninput="updateLight(this.value)">
                    <div class="value-display" id="lightVal">0%</div>
                </div>
            </div>
            
            <div class="control">
                <label for="pump">💦 Su Pompası</label>
                <div class="slider-container">
                    <input type="range" id="pump" min="0" max="100" value="0" oninput="updatePump(this.value)">
                    <div class="value-display" id="pumpVal">0%</div>
                </div>
            </div>
            
            <div class="control">
                <label for="fan">🌬️ Fan Kontrolü</label>
                <div class="slider-container">
                    <input type="range" id="fan" min="0" max="100" value="0" oninput="updateFan(this.value)">
                    <div class="value-display" id="fanVal">0%</div>
                </div>
            </div>
            
            <div class="control">
                <label for="heat">🔥 Isıtıcı</label>
                <div class="slider-container">
                    <input type="range" id="heat" min="0" max="100" value="0" oninput="updateHeat(this.value)">
                    <div class="value-display" id="heatVal">0%</div>
                </div>
            </div>
            
            <button id="doorBtn" onclick="toggleDoor()"><span class="button-icon">🚪</span> <span id="doorText">Kapı: Kapalı</span></button>
        </div>
        
        <footer>
            <p>Akıllı Sera Otomasyonu &copy; 2023 | Versiyon 2.1</p>
            <p>Bağlantı durumu: <span id="connectionStatus">Bağlanıyor...</span></p>
        </footer>
    </div>

    <script>
        var gateway = `ws://${window.location.hostname}:81/`;
        var websocket;
        
        window.addEventListener('load', onLoad);
        
        function onLoad(event) {
            initWebSocket();
            updateThemeBasedOnTime();
        }
        
        function updateThemeBasedOnTime() {
            const hour = new Date().getHours();
            const isDay = hour >= 6 && hour < 18;
            
            document.querySelectorAll('.sensor-card').forEach(card => {
                if (isDay) {
                    card.classList.add('day-time');
                    card.classList.remove('night-time');
                } else {
                    card.classList.add('night-time');
                    card.classList.remove('day-time');
                }
            });
        }
        
        function initWebSocket() {
            console.log('Websocket bağlantısı kuruluyor...');
            websocket = new WebSocket(gateway);
            websocket.onopen = onOpen;
            websocket.onclose = onClose;
            websocket.onmessage = onMessage;
        }
        
        function onOpen(event) {
            console.log('Bağlantı kuruldu');
            document.getElementById('status').innerHTML = 'Bağlantı kuruldu';
            document.getElementById('status').className = 'status online';
            document.getElementById('connectionStatus').textContent = 'Bağlı';
            document.getElementById('connectionStatus').style.color = 'var(--success)';
            
            setTimeout(() => {
                document.getElementById('status').style.opacity = '0';
                setTimeout(() => {
                    document.getElementById('status').style.display = 'none';
                }, 500);
            }, 3000);
            
            // Başlangıç durumunu al
            sendJson({type: 'getStatus'});
        }
        
        function onClose(event) {
            console.log('Bağlantı kesildi, yeniden deneniyor...');
            document.getElementById('status').style.display = 'block';
            document.getElementById('status').style.opacity = '1';
            document.getElementById('status').innerHTML = 'Bağlantı kesildi! Yeniden bağlanıyor...';
            document.getElementById('status').className = 'status offline';
            document.getElementById('connectionStatus').textContent = 'Bağlantı kesildi';
            document.getElementById('connectionStatus').style.color = 'var(--danger)';
            
            setTimeout(initWebSocket, 2000);
        }
        
        function onMessage(event) {
            try {
                var data = JSON.parse(event.data);
                console.log('Veri alındı:', data);
                
                if (data.type === 'sensor') {
                    // Sensör değerlerini güncelle - sayıya çevirerek göster
                    document.getElementById('tempValue').textContent = Number(data.temp) || 0;
                    document.getElementById('humValue').textContent = Number(data.hum) || 0;
                    document.getElementById('moisValue').textContent = Number(data.mois) || 0;
                    document.getElementById('luxValue').textContent = Number(data.lux) || 0;
                    
                    // Kontrol değerlerini güncelle - sayıya çevirerek göster
                    const lightVal = Number(data.light) || 0;
                    const pumpVal = Number(data.pump) || 0;
                    const fanVal = Number(data.fan) || 0;
                    const heatVal = Number(data.heat) || 0;
                    
                    document.getElementById('light').value = lightVal;
                    document.getElementById('lightVal').textContent = lightVal + '%';
                    
                    document.getElementById('pump').value = pumpVal;
                    document.getElementById('pumpVal').textContent = pumpVal + '%';
                    
                    document.getElementById('fan').value = fanVal;
                    document.getElementById('fanVal').textContent = fanVal + '%';
                    
                    document.getElementById('heat').value = heatVal;
                    document.getElementById('heatVal').textContent = heatVal + '%';
                    
                    const doorStatus = data.door === true ? 'Açık' : 'Kapalı';
                    document.getElementById('doorText').textContent = `Kapı: ${doorStatus}`;
                    
                    // Değerlere göre renkleri güncelle
                    updateColors(data);
                }
            } catch (error) {
                console.error('JSON ayrıştırma hatası:', error, event.data);
            }
        }
        
        function updateColors(data) {
            // Sıcaklık rengini güncelle
            const tempElement = document.getElementById('tempValue');
            if (data.temp > 30) {
                tempElement.style.color = 'var(--danger)';
            } else if (data.temp > 25) {
                tempElement.style.color = 'var(--warning)';
            } else if (data.temp < 15) {
                tempElement.style.color = 'var(--accent)';
            } else {
                tempElement.style.color = 'var(--success)';
            }
            
            // Nem rengini güncelle
            const humElement = document.getElementById('humValue');
            if (data.hum > 80) {
                humElement.style.color = 'var(--accent)';
            } else if (data.hum < 40) {
                humElement.style.color = 'var(--warning)';
            } else {
                humElement.style.color = 'var(--success)';
            }
            
            // Toprak nemi rengini güncelle
            const moisElement = document.getElementById('moisValue');
            if (data.mois < 30) {
                moisElement.style.color = 'var(--danger)';
            } else if (data.mois > 80) {
                moisElement.style.color = 'var(--accent)';
            } else {
                moisElement.style.color = 'var(--success)';
            }
            
            // Işık seviyesi rengini güncelle
            const luxElement = document.getElementById('luxValue');
            if (data.lux > 1000) {
                luxElement.style.color = 'var(--warning)';
            } else if (data.lux < 200) {
                luxElement.style.color = 'var(--accent)';
            } else {
                luxElement.style.color = 'var(--success)';
            }
        }
        
        function sendJson(data) {
            if (websocket.readyState === WebSocket.OPEN) {
                websocket.send(JSON.stringify(data));
            }
        }
        
        function updateLight(value) {
            document.getElementById('lightVal').textContent = value + '%';
            sendJson({type: 'control', target: 'light', value: parseInt(value)});
        }
        
        function updatePump(value) {
            document.getElementById('pumpVal').textContent = value + '%';
            sendJson({type: 'control', target: 'pump', value: parseInt(value)});
        }
        
        function updateFan(value) {
            document.getElementById('fanVal').textContent = value + '%';
            sendJson({type: 'control', target: 'fan', value: parseInt(value)});
        }
        
        function updateHeat(value) {
            document.getElementById('heatVal').textContent = value + '%';
            sendJson({type: 'control', target: 'heat', value: parseInt(value)});
        }
        
        function toggleDoor() {
            sendJson({type: 'control', target: 'door', value: 'toggle'});
        }
    </script>
</body>
</html>
)rawliteral";

// WebSocket olaylarını yönet
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Bağlantı kesildi!\n", num);
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] %d.%d.%d.%d bağlandı\n", num, ip[0], ip[1], ip[2], ip[3]);
        sendSensorData();
      }
      break;
    case WStype_TEXT:
      {
        Serial.printf("[%u] Alınan metin: %s\n", num, payload);
        
        DynamicJsonDocument doc(256);
        DeserializationError error = deserializeJson(doc, payload);
        
        if (error) {
          Serial.print("deserializeJson() başarısız: ");
          Serial.println(error.f_str());
          return;
        }
        
        String type = doc["type"];
        
        if (type == "control") {
          String target = doc["target"];
          
          if (target == "light") {
            lightValue = doc["value"];
            analogWrite(LIGHT_PIN, map(lightValue, 0, 100, 0, 255));
            Serial.println("Işık: " + String(lightValue));
          }
          else if (target == "pump") {
            pumpValue = doc["value"];
            analogWrite(PUMP_PIN, map(pumpValue, 0, 100, 0, 255));
            Serial.println("Pompa: " + String(pumpValue));
          }
          else if (target == "fan") {
            fanValue = doc["value"];
            analogWrite(FAN_PIN, map(fanValue, 0, 100, 0, 255));
            Serial.println("Fan: " + String(fanValue));
          }
          else if (target == "heat") {
            heatValue = doc["value"];
            digitalWrite(HEAT_PIN, heatValue > 0 ? HIGH : LOW);
            Serial.println("Isıtıcı: " + String(heatValue));
          }
          else if (target == "door") {
            if (doc["value"] == "toggle") {
              // Kapı durumunu değiştir
              doorOpen = !doorOpen;
              targetAngle = doorOpen ? 180 : 0;
              
              // Servo kontrolünü güçlendir
              Serial.println("Kapı durumu değişti: " + String(doorOpen ? "Açılıyor" : "Kapanıyor"));
              Serial.println("Servo hedef açısı: " + String(targetAngle));
            }
          }
          
          sendSensorData();
        }
        else if (type == "getStatus") {
          sendSensorData();
        }
        else if (type == "calibrate") {
          // Toprak nem sensörünü kalibre et
          if (doc.containsKey("min") && doc.containsKey("max")) {
            soilMin = doc["min"];
            soilMax = doc["max"];
            Serial.println("Toprak nem sensörü kalibre edildi: Min=" + String(soilMin) + ", Max=" + String(soilMax));
          }
        }
      }
      break;
  }
}

// Toprak sensörü kalibrasyonu için endpoint ekle
void setupServer() {
  // Ana sayfa
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", index_html);
  });
  
  // Servo test sayfası ekle
  server.on("/servo", HTTP_GET, []() {
    String html = "<html><head>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    html += "<style>body{font-family:Arial;margin:20px;background:#222;color:#fff}";
    html += "h1{color:#4ade80}";
    html += ".btn{background:#4ade80;border:none;padding:10px 15px;color:#000;";
    html += "font-weight:bold;border-radius:5px;cursor:pointer;margin:5px}";
    html += "input{padding:8px;margin:5px;width:100px}";
    html += ".slider{width:100%;margin:20px 0;}</style>";
    html += "</head><body>";
    html += "<h1>Servo Motor Test</h1>";
    html += "<p>Servo kontrol için değer ayarlayın:</p>";
    html += "<input type='range' min='0' max='180' value='90' class='slider' id='servoSlider'>";
    html += "<p>Açı: <span id='servoValue'>90</span>°</p>";
    html += "<button class='btn' onclick='setServo(0)'>0°</button>";
    html += "<button class='btn' onclick='setServo(45)'>45°</button>";
    html += "<button class='btn' onclick='setServo(90)'>90°</button>";
    html += "<button class='btn' onclick='setServo(135)'>135°</button>";
    html += "<button class='btn' onclick='setServo(180)'>180°</button>";
    html += "<p><button class='btn' onclick='testServo()'>Test Et</button></p>";
    html += "<p>Servo Durumu: <span id='status'>-</span></p>";
    html += "<p><a href='/' style='color:#4ade80'>Ana Sayfaya Dön</a></p>";
    html += "<script>";
    html += "const slider = document.getElementById('servoSlider');";
    html += "const output = document.getElementById('servoValue');";
    html += "output.innerHTML = slider.value;";
    html += "slider.oninput = function() {";
    html += "  output.innerHTML = this.value;";
    html += "  setServo(this.value);";
    html += "}";
    html += "function setServo(angle) {";
    html += "  document.getElementById('status').textContent = 'Ayarlanıyor...';";
    html += "  slider.value = angle;";
    html += "  output.innerHTML = angle;";
    html += "  fetch('/set-servo?angle='+angle)";
    html += "    .then(response => response.text())";
    html += "    .then(data => { document.getElementById('status').textContent = data; });";
    html += "}";
    html += "function testServo() {";
    html += "  document.getElementById('status').textContent = 'Test ediliyor...';";
    html += "  fetch('/test-servo')";
    html += "    .then(response => response.text())";
    html += "    .then(data => { document.getElementById('status').textContent = data; });";
    html += "}";
    html += "</script>";
    html += "</body></html>";
    server.send(200, "text/html", html);
  });
  
  // Servo ayarlama endpoint'i
  server.on("/set-servo", HTTP_GET, []() {
    if (server.hasArg("angle")) {
      int angle = server.arg("angle").toInt();
      angle = constrain(angle, 0, 180);
      
      // Bu noktada kapı kontrolünden bağımsız olarak direk servo kontrolü
      servo.write(angle);
      currentAngle = angle;
      targetAngle = angle;
      
      server.send(200, "text/plain", "Servo " + String(angle) + "° açısına ayarlandı");
      Serial.println("Servo test: " + String(angle) + "° açısına ayarlandı");
    } else {
      server.send(400, "text/plain", "Hata: 'angle' parametresi gerekli");
    }
  });
  
  // Servo test endpoint'i
  server.on("/test-servo", HTTP_GET, []() {
    server.send(200, "text/plain", "Servo test ediliyor...");
    
    // Async test, hemen cevap veriyoruz ama ESPnin geri kalanına etki etmesin
    targetAngle = 0;
    delay(500);
    targetAngle = 90;
    delay(500);
    targetAngle = 180;
    delay(500);
    targetAngle = 90;
    delay(500);
    targetAngle = 0;
    
    Serial.println("Servo test tamamlandı");
  });
  
  // Kalibrasyon sayfası
  server.on("/calibrate", HTTP_GET, []() {
    String html = "<html><head>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    html += "<style>body{font-family:Arial;margin:20px;background:#222;color:#fff}";
    html += "h1{color:#4ade80}";
    html += ".btn{background:#4ade80;border:none;padding:10px 15px;color:#000;";
    html += "font-weight:bold;border-radius:5px;cursor:pointer;margin:5px}";
    html += "input{padding:8px;margin:5px;width:100px}</style>";
    html += "</head><body>";
    html += "<h1>Toprak Nem Sensörü Kalibrasyonu</h1>";
    html += "<p>Şu anki değerler: Min=" + String(soilMin) + ", Max=" + String(soilMax) + "</p>";
    html += "<p>Şu anki ham değer: <span id='rawValue'>--</span></p>";
    html += "<div><button class='btn' onclick='readRaw()'>Ham Değeri Oku</button></div>";
    html += "<div style='margin-top:20px'>";
    html += "<p>Sensör <b>KURU</b> iken değeri okuyup 'Kuru Değeri Kaydet' butonuna basın:</p>";
    html += "<input type='number' id='dryValue' placeholder='Kuru değer'>";
    html += "<button class='btn' onclick='saveDry()'>Kuru Değeri Kaydet</button>";
    html += "</div>";
    html += "<div style='margin-top:20px'>";
    html += "<p>Sensör <b>ISLAK</b> iken değeri okuyup 'Islak Değeri Kaydet' butonuna basın:</p>";
    html += "<input type='number' id='wetValue' placeholder='Islak değer'>";
    html += "<button class='btn' onclick='saveWet()'>Islak Değeri Kaydet</button>";
    html += "</div>";
    html += "<p style='margin-top:20px'><a href='/' style='color:#4ade80'>Ana Sayfaya Dön</a></p>";
    html += "<script>";
    html += "function readRaw() {";
    html += "  fetch('/raw').then(r=>r.text()).then(v=>{";
    html += "    document.getElementById('rawValue').textContent = v;";
    html += "  });";
    html += "}";
    html += "function saveDry() {";
    html += "  const v = document.getElementById('dryValue').value;";
    html += "  if(v) {";
    html += "    fetch('/set?type=dry&value='+v).then(()=>alert('Kuru değer kaydedildi!'));";
    html += "  }";
    html += "}";
    html += "function saveWet() {";
    html += "  const v = document.getElementById('wetValue').value;";
    html += "  if(v) {";
    html += "    fetch('/set?type=wet&value='+v).then(()=>alert('Islak değer kaydedildi!'));";
    html += "  }";
    html += "}";
    html += "setInterval(readRaw, 2000);"; // Her 2 saniyede bir ham değeri oku
    html += "</script>";
    html += "</body></html>";
    server.send(200, "text/html", html);
  });
  
  // Ham değer okuma endpoint'i
  server.on("/raw", HTTP_GET, []() {
    int raw = analogRead(MOIS);
    server.send(200, "text/plain", String(raw));
  });
  
  // Değer ayarlama endpoint'i
  server.on("/set", HTTP_GET, []() {
    String type = server.arg("type");
    int value = server.arg("value").toInt();
    
    if (type == "dry") {
      soilMin = value;
      Serial.println("Kuru değer ayarlandı: " + String(soilMin));
    } else if (type == "wet") {
      soilMax = value;
      Serial.println("Islak değer ayarlandı: " + String(soilMax));
    }
    
    server.send(200, "text/plain", "OK");
  });
  
  server.begin();
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n\nSera Kontrol Sistemi Başlatılıyor...");
  
  // Çıkış pinlerini ayarla
  pinMode(LIGHT_PIN, OUTPUT);
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(HEAT_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  
  // I2C başlat
  Wire.begin(SDA_PIN, SCL_PIN);
  
  // Işık sensörünü başlat
  lightSensor.begin(BH1750::CONTINUOUS_HIGH_RES_MODE);
  
  // DHT sensörünü başlat
  dht.begin();
  
  // Servonun GND ve VCC bağlantılarını kontrol edin!
  // Servo başlat
  delay(1000); // Başlamadan önce biraz bekle
  servo.attach(DOOR_PIN);
  delay(500);  // Servonun bağlanması için bekle
  
  // Servo pozisyonlarını test etmek için
  Serial.println("Servo test ediliyor...");
  
  servo.write(0);
  Serial.println("Servo 0° pozisyona getirildi");
  delay(1000);
  
  servo.write(90);
  Serial.println("Servo 90° pozisyona getirildi");
  delay(1000);
  
  servo.write(180);
  Serial.println("Servo 180° pozisyona getirildi");
  delay(1000);
  
  servo.write(0);
  Serial.println("Servo 0° pozisyona getirildi (final)");
  delay(500);
  
  // Servo pozisyonu ayarla
  currentAngle = 0;
  targetAngle = 0;
  
  // WiFi'a bağlan
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  
  Serial.print("WiFi bağlanıyor");
  for (int i = 0; i < 20 && WiFi.status() != WL_CONNECTED; i++) {
    delay(500);
    Serial.print(".");
    ESP.wdtFeed();
  }
  Serial.println();
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi bağlandı!");
    Serial.print("IP Adresi: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("WiFi bağlantısı başarısız! Lütfen yeniden başlatın.");
  }
  
  // Web sunucusunu ayarla
  setupServer();
  Serial.println("HTTP sunucusu başlatıldı.");
  
  // WebSocket sunucusunu başlat
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  Serial.println("WebSocket sunucusu başlatıldı.");
  
  Serial.println("Kurulum tamamlandı!");
}

void loop() {
  unsigned long currentTime = millis();
  
  // Web sunucusunu ve WebSocket istemcilerini işle
  server.handleClient();
  webSocket.loop();
  
  // Sensörleri düzenli olarak oku (5 saniyede bir)
  if (currentTime - lastSensorUpdate >= 5000) {
    lastSensorUpdate = currentTime;
    readSensors();
  }
  
  // Servo pozisyonunu güncelle
  updateServo();
  
  // WDT resetini önlemek için
  yield();
}


