
#include <Wire.h>
#include "DHT.h"
#include <BH1750.h>
#include "esp_sleep.h"

// Definiciones para DHT22
#define DHTPIN 2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Definiciones para el sensor de humedad del suelo
const int AirValue = 4095;
const int WaterValue = 950;
const int SoilMoisturePin = 35; // GPIO 35
int soilMoistureValue = 0;
int soilmoisturepercent = 0;

// Definiciones para el sensor UV
const int uvSensorPin = 34; // GPIO 34
const float Vref = 3.3;
const int adcResolution = 4095;
const float calibrationFactor = 15.0 / Vref;

// Definiciones para el sensor de luz BH1750
BH1750 lightMeter;

// Definiciones para el módulo LoRa E32 900T20D
#define LORA_TX_PIN 15
#define LORA_RX_PIN 14
HardwareSerial LoRaSerial(2); // Usando UART2

// Variables para el control de tiempo
const unsigned long updateInterval = 180000; // 3 minutos en milisegundos

// Identificador del transmisor (CAMBIAR a "2" para el Transmisor 2)
const String transmitterID = "1";

void setup() {
  // Inicializar Serial
  Serial.begin(9600);
  while (!Serial);

  // Inicializar sensores
  dht.begin();
  Wire.begin();
  if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    Serial.println("BH1750 inicializado");
  } else {
    Serial.println("Error al inicializar BH1750");
  }
  pinMode(SoilMoisturePin, INPUT);
  pinMode(uvSensorPin, INPUT);

  // Inicializar comunicación LoRa
  LoRaSerial.begin(9600, SERIAL_8N1, LORA_RX_PIN, LORA_TX_PIN);
  Serial.println("LoRa Serial inicializado");

  // Esperar un breve momento para asegurar que todo está listo
  delay(2000);
}

void loop() {
  // Leer sensor DHT22
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (isnan(h) || isnan(t)) {
    Serial.println("Error leyendo DHT22");
  } else {
    Serial.print("Humedad: ");
    Serial.print(h);
    Serial.print("%  Temperatura: ");
    Serial.print(t);
    Serial.println("°C");
  }

  // Leer sensor BH1750
  float lux = lightMeter.readLightLevel();
  if (lux < 0) {
    Serial.println("Error leyendo BH1750");
  } else {
    Serial.print("Luz: ");
    Serial.print(lux);
    Serial.println(" lx");
  }

  // Leer sensor de humedad del suelo
  soilMoistureValue = analogRead(SoilMoisturePin);
  soilmoisturepercent = map(soilMoistureValue, AirValue, WaterValue, 0, 100);
  soilmoisturepercent = constrain(soilmoisturepercent, 0, 100);
  Serial.print("Humedad del suelo: ");
  Serial.print(soilmoisturepercent);
  Serial.println("%");

  // Leer sensor UV
  int analogValue = analogRead(uvSensorPin);
  float voltage = (analogValue / (float)adcResolution) * Vref;
  float uvIndex = voltage * calibrationFactor;
  Serial.print("Índice UV: ");
  Serial.println(uvIndex, 2);

  // Crear cadena de datos con identificador
  String dataString = transmitterID + "," + String(h) + "," + String(t) + "," + String(lux) + "," +
                      String(soilmoisturepercent) + "," + String(uvIndex);

  // Enviar datos vía LoRa
  LoRaSerial.println(dataString);
  Serial.println("Datos enviados vía LoRa:");
  Serial.println(dataString);

  // Esperar un momento para asegurar que los datos se envíen
  delay(500);

  // Entrar en modo Deep Sleep
  Serial.println("Entrando en modo Deep Sleep");
  esp_sleep_enable_timer_wakeup(updateInterval * 1000ULL); // Convertir a microsegundos
  esp_deep_sleep_start();
}
