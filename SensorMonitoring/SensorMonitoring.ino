#define BLYNK_TEMPLATE_ID "xxxxxxxxxxx"
#define BLYNK_TEMPLATE_NAME "xxxxxxxxxxxx"
#define BLYNK_AUTH_TOKEN "xxxxxxxxxxxx"

#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_BMP280.h>

char ssid[] = "Z Fold";
char pass[] = "12345678";

// ----------------- KONFIGURASI PIN -----------------
const int mq2Pin = 36;
const int ldrPin = 39;
const int trigPin = 5;
const int echoPin = 18;

// ----------------- OBJEK PERANGKAT -----------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
// Inisialisasi OLED pada I2C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
// Inisialisasi bmp280
Adafruit_BMP280 bmp; 

BlynkTimer timer;

// Fungsi membaca sensor, tampil ke OLED & Kirim ke Blynk
void readAndSendSensors() {
  // 1. Baca bmp280 (Suhu dan Tekanan Udara)
  float temp = bmp.readTemperature();
  float pressure = bmp.readPressure() / 100.0F; // Dibagi 100 untuk konversi Pa ke hPa

  // 2. Baca MQ-2 (Gas) & Konversi ke PPM
  int gasValue = analogRead(mq2Pin);
  int gasPPM = map(gasValue, 0, 4095, 0, 10000);

  // 3. Baca LDR (Cahaya) & Konversi ke Persen (%)
  int ldrValue = analogRead(ldrPin);
  int ldrPercent = map(ldrValue, 0, 4095, 100, 0);

  // 4. Baca HC-SR04 (Ultrasonic)
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH);
  int distance = duration * 0.034 / 2; // Konversi waktu ke cm

  // --- KIRIM DATA KE BLYNK ---
  Blynk.virtualWrite(V6, temp);
  Blynk.virtualWrite(V7, pressure);    // Mengirim data tekanan ke V11
  Blynk.virtualWrite(V8, gasPPM);      
  Blynk.virtualWrite(V9, ldrPercent);  
  Blynk.virtualWrite(V10, distance);

  // --- TAMPILKAN KE LAYAR OLED ---
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  
  display.setCursor(0, 0);
  display.printf("Temp: %.1f C", temp);
  
  display.setCursor(0, 12);
  display.printf("Pres: %.1f hPa", pressure); // Menampilkan data tekanan
  
  display.setCursor(0, 24);
  display.printf("Gas : %d ppm", gasPPM);      
  
  display.setCursor(0, 36);
  display.printf("LDR : %d %%", ldrPercent);   
  
  display.setCursor(0, 48);
  display.printf("Dist: %d cm", distance);
  
  display.display();
  
  // Debugging Serial
  Serial.printf("Temp: %.1f C, Pres: %.1f hPa, Gas: %d ppm, LDR: %d %%, Dist: %d cm\n", temp, pressure, gasPPM, ldrPercent, distance);
}

void setup() {
  Serial.begin(115200);

  // Setup Pin HC-SR04
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Mulai bus I2C (Pin 21 SDA, Pin 22 SCL)
  Wire.begin(21, 22);

  // Inisialisasi OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Gagal menemukan layar OLED"));
  } else {
    display.clearDisplay();
    display.display();
  }

  // Inisialisasi bmp280
  if (!bmp.begin(0x76)) {
    Serial.println("Gagal menemukan bmp280, cek kabel SDA/SCL!");
  }

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  timer.setInterval(2000L, readAndSendSensors);
}

void loop() {
  Blynk.run();
  timer.run();
}