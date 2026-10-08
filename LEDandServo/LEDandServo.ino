#define BLYNK_TEMPLATE_ID "xxxxxxxxxxx"
#define BLYNK_TEMPLATE_NAME "xxxxxxxxxxxx"
#define BLYNK_AUTH_TOKEN "xxxxxxxxxxxx"



#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <ESP32Servo.h>

char ssid[] = "Z Fold";
char pass[] = "12345678";

// Konfigurasi Pin I/O terstruktur menggunakan Array
const int pinBtns[4] = {33, 25, 14, 15};
const int pinLeds[4] = {4, 2, 26, 27};
const int vPins[4] = {V0, V1, V2, V3};
const int pinServo = 23;
const int pinPotensio = 35;

Servo myServo;
BlynkTimer timer;

// State Variabel untuk Debouncing & Sinkronisasi
bool ledStates[4] = {false, false, false, false};
int lastBtnStates[4] = {HIGH, HIGH, HIGH, HIGH};
int lastPotValue = -1;

// Fungsi ini dijalankan berkala (Non-Blocking)
void checkPhysicalInputs() {
  // 1. Cek Data ADC Potensiometer
  int potValue = analogRead(pinPotensio);
  if (abs(potValue - lastPotValue) > 50) { // Filter noise & jitter ADC
    lastPotValue = potValue;
    int servoPos = map(potValue, 0, 4095, 0, 180);
    myServo.write(servoPos);          // Gerakkan mekanik fisik servo
    Blynk.virtualWrite(V4, servoPos); // Update posisi Slider di Blynk
    Blynk.virtualWrite(V5, potValue); // Update Gauge angka potensio di Blynk
  }

  // 2. Cek ke-4 Tombol Fisik
  for (int i = 0; i < 4; i++) {
    int currentBtnState = digitalRead(pinBtns[i]);
    // Deteksi event tekanan (transisi dari HIGH ke LOW karena PULLUP)
    if (lastBtnStates[i] == HIGH && currentBtnState == LOW) { 
      ledStates[i] = !ledStates[i];               // Balikkan state (Toggle)
      digitalWrite(pinLeds[i], ledStates[i]);     // Eksekusi aktuator LED
      Blynk.virtualWrite(vPins[i], ledStates[i]); // Sinkronkan status ke widget Blynk
    }
    lastBtnStates[i] = currentBtnState;
  }
}

// Sinkronisasi instruksi dari Web Dashboard Blynk -> Hardware
BLYNK_WRITE(V0) { ledStates[0] = param.asInt(); digitalWrite(pinLeds[0], ledStates[0]); }
BLYNK_WRITE(V1) { ledStates[1] = param.asInt(); digitalWrite(pinLeds[1], ledStates[1]); }
BLYNK_WRITE(V2) { ledStates[2] = param.asInt(); digitalWrite(pinLeds[2], ledStates[2]); }
BLYNK_WRITE(V3) { ledStates[3] = param.asInt(); digitalWrite(pinLeds[3], ledStates[3]); }

// Kontrol Servo manual melalui Slider di Web Dashboard
BLYNK_WRITE(V4) { 
  int pos = param.asInt();
  myServo.write(pos); 
}

void setup() {
  Serial.begin(115200);
  
  // Inisialisasi Register I/O
  for (int i = 0; i < 4; i++) {
    pinMode(pinBtns[i], INPUT_PULLUP);
    pinMode(pinLeds[i], OUTPUT);
    digitalWrite(pinLeds[i], LOW);
  }
  myServo.attach(pinServo);
  
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  
  // Jadwalkan pembacaan sensor fisik setiap 100ms untuk menekan beban loop CPU
  timer.setInterval(100L, checkPhysicalInputs);
}

void loop() {
  Blynk.run();
  timer.run();
}