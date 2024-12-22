#include <WiFi.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include "DHT.h"
#include <BH1750.h>
#include "ThingSpeak.h"
#include <time.h>
#include "esp_sleep.h"

// Definiciones de WiFi y ThingSpeak
#define SECRET_SSID "Tu_SSID"
#define SECRET_PASS "Tu_Contraseña"
#define SECRET_CH_ID 1234567
#define SECRET_WRITE_APIKEY "Tu_API_Key"
#define SECRET_CH_ID2 1234568
#define SECRET_WRITE_APIKEY2 "Tu_API_Key2"
#define SECRET_CH_ID3 1234569
#define SECRET_WRITE_APIKEY3 "Tu_API_Key3"

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
WiFiClient client;

unsigned long myChannelNumber = SECRET_CH_ID;
const char* myWriteAPIKey = SECRET_WRITE_APIKEY;
unsigned long myChannelNumber2 = SECRET_CH_ID2;
const char* myWriteAPIKey2 = SECRET_WRITE_APIKEY2;
unsigned long myChannelNumber3 = SECRET_CH_ID3;
const char* myWriteAPIKey3 = SECRET_WRITE_APIKEY3;

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

// Variables para el control de tiempo
const unsigned long sleepInterval = 150000; // 2.5 minutos en milisegundos
const unsigned long awakeDuration = 60000; // Permanecer despierto durante 60 segundos

// Pines SPI para la tarjeta SD
#define SD_CS 5
#define SD_MOSI 23
#define SD_SCK 18
#define SD_MISO 19
SPIClass spiSD(VSPI);

// Configuración para sincronización de tiempo
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -5 * 3600; // UTC-5 para Perú
const int daylightOffset_sec = 0;

// Definiciones para el módulo LoRa E32 900T30D
#define LORA_TX_PIN 15
#define LORA_RX_PIN 14
HardwareSerial LoRaSerial(2); // Usando UART2

void setup() {
  Serial.begin(9600);
  while (!Serial);

  // Inicializar WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  Serial.print("Conectando a WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado a WiFi");

  // Inicializar ThingSpeak
  ThingSpeak.begin(client);

  // Sincronización de tiempo
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Error al obtener la hora");
  } else {
    Serial.println("Tiempo sincronizado:");
    Serial.println(&timeinfo, "%Y-%m-%d %H:%M:%S");
  }

  // Inicializar tarjeta SD
  Serial.println("Inicializando tarjeta SD...");
  spiSD.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  if (!SD.begin(SD_CS, spiSD)) {
    Serial.println("Error al montar la tarjeta SD");
  } else {
    Serial.println("Tarjeta SD inicializada.");
    // Verificar y crear data.csv
    if (!SD.exists("/data.csv")) {
      File dataFile = SD.open("/data.csv", FILE_WRITE);
      if (dataFile) {
        dataFile.println("Fecha y Hora,Humedad (%),Temperatura (C),Luz (lx),Humedad Suelo (%),Indice UV");
        dataFile.close();
        Serial.println("Creado data.csv con cabecera.");
      } else {
        Serial.println("Error al crear data.csv");
      }
    }
    // Verificar y crear data2.csv
    if (!SD.exists("/data2.csv")) {
      File dataFile = SD.open("/data2.csv", FILE_WRITE);
      if (dataFile) {
        dataFile.println("Fecha y Hora,Humedad (%),Temperatura (C),Luz (lx),Humedad Suelo (%),Indice UV");
        dataFile.close();
        Serial.println("Creado data2.csv con cabecera.");
      } else {
        Serial.println("Error al crear data2.csv");
      }
    }
    // Verificar y crear data3.csv
    if (!SD.exists("/data3.csv")) {
      File dataFile = SD.open("/data3.csv", FILE_WRITE);
      if (dataFile) {
        dataFile.println("Fecha y Hora,Humedad (%),Temperatura (C),Luz (lx),Humedad Suelo (%),Indice UV");
        dataFile.close();
        Serial.println("Creado data3.csv con cabecera.");
      } else {
        Serial.println("Error al crear data3.csv");
      }
    }
  }

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

  // Esperar un breve momento
  delay(2000);
}

void loop() {
  unsigned long startTime = millis();

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

  // Enviar datos a ThingSpeak Canal 1
  ThingSpeak.setField(1, h);
  ThingSpeak.setField(2, t);
  ThingSpeak.setField(3, lux);
  ThingSpeak.setField(4, soilmoisturepercent);
  ThingSpeak.setField(5, uvIndex);

  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if (x == 200) {
    Serial.println("Actualización del Canal 1 exitosa.");
  } else {
    Serial.println("Error al actualizar Canal 1. Código de error HTTP " + String(x));
  }

  // Obtener hora actual
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Error al obtener la hora para registro.");
  } else {
    char timeString[25];
    strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", &timeinfo);

    // Guardar datos en data.csv
    File dataFile = SD.open("/data.csv", FILE_APPEND);
    if (dataFile) {
      dataFile.print(timeString);
      dataFile.print(",");
      dataFile.print(h);
      dataFile.print(",");
      dataFile.print(t);
      dataFile.print(",");
      dataFile.print(lux);
      dataFile.print(",");
      dataFile.print(soilmoisturepercent);
      dataFile.print(",");
      dataFile.println(uvIndex);
      dataFile.close();
      Serial.println("Datos guardados en data.csv.");
    } else {
      Serial.println("Error al abrir data.csv.");
    }
  }

  // Mantenerse despierto durante 'awakeDuration' o hasta que se reciba datos de los transmisores
  while (millis() - startTime < awakeDuration) {
    // Recibir datos de transmisores vía LoRa
    if (LoRaSerial.available()) {
      String receivedData = LoRaSerial.readStringUntil('\n');
      receivedData.trim();
      Serial.println("Datos recibidos vía LoRa:");
      Serial.println(receivedData);

      // Extraer identificador
      int identifier = receivedData.substring(0, receivedData.indexOf(',')).toInt();
      String dataString = receivedData.substring(receivedData.indexOf(',') + 1);

      // Variables para almacenar datos parseados
      float h2, t2, lux2, soilmoisturepercent2, uvIndex2;

      // Parsear datos recibidos
      sscanf(dataString.c_str(), "%f,%f,%f,%f,%f", &h2, &t2, &lux2, &soilmoisturepercent2, &uvIndex2);

      // Obtener hora actual
      struct tm timeinfo2;
      if (!getLocalTime(&timeinfo2)) {
        Serial.println("Error al obtener la hora para registro de datos recibidos.");
      } else {
        char timeString2[25];
        strftime(timeString2, sizeof(timeString2), "%Y-%m-%d %H:%M:%S", &timeinfo2);

        if (identifier == 1) {
          // Enviar datos a ThingSpeak Canal 2
          ThingSpeak.setField(1, h2);
          ThingSpeak.setField(2, t2);
          ThingSpeak.setField(3, lux2);
          ThingSpeak.setField(4, soilmoisturepercent2);
          ThingSpeak.setField(5, uvIndex2);

          int y = ThingSpeak.writeFields(myChannelNumber2, myWriteAPIKey2);
          if (y == 200) {
            Serial.println("Actualización del Canal 2 exitosa.");
          } else {
            Serial.println("Error al actualizar Canal 2. Código de error HTTP " + String(y));
          }

          // Guardar datos en data2.csv
          File dataFile2 = SD.open("/data2.csv", FILE_APPEND);
          if (dataFile2) {
            dataFile2.print(timeString2);
            dataFile2.print(",");
            dataFile2.print(h2);
            dataFile2.print(",");
            dataFile2.print(t2);
            dataFile2.print(",");
            dataFile2.print(lux2);
            dataFile2.print(",");
            dataFile2.print(soilmoisturepercent2);
            dataFile2.print(",");
            dataFile2.println(uvIndex2);
            dataFile2.close();
            Serial.println("Datos guardados en data2.csv.");
          } else {
            Serial.println("Error al abrir data2.csv.");
          }

        } else if (identifier == 2) {
          // Enviar datos a ThingSpeak Canal 3
          ThingSpeak.setField(1, h2);
          ThingSpeak.setField(2, t2);
          ThingSpeak.setField(3, lux2);
          ThingSpeak.setField(4, soilmoisturepercent2);
          ThingSpeak.setField(5, uvIndex2);

          int y = ThingSpeak.writeFields(myChannelNumber3, myWriteAPIKey3);
          if (y == 200) {
            Serial.println("Actualización del Canal 3 exitosa.");
          } else {
            Serial.println("Error al actualizar Canal 3. Código de error HTTP " + String(y));
          }

          // Guardar datos en data3.csv
          File dataFile3 = SD.open("/data3.csv", FILE_APPEND);
          if (dataFile3) {
            dataFile3.print(timeString2);
            dataFile3.print(",");
            dataFile3.print(h2);
            dataFile3.print(",");
            dataFile3.print(t2);
            dataFile3.print(",");
            dataFile3.print(lux2);
            dataFile3.print(",");
            dataFile3.print(soilmoisturepercent2);
            dataFile3.print(",");
            dataFile3.println(uvIndex2);
            dataFile3.close();
            Serial.println("Datos guardados en data3.csv.");
          } else {
            Serial.println("Error al abrir data3.csv.");
          }

        } else {
          Serial.println("Identificador de transmisor desconocido: " + String(identifier));
        }
      }
    }
  }

  // Entrar en modo Deep Sleep
  Serial.println("Entrando en modo Deep Sleep");
  esp_sleep_enable_timer_wakeup(sleepInterval * 1000ULL); // Convertir a microsegundos
  esp_deep_sleep_start();
}
