#define BLYNK_TEMPLATE_ID "TMPL27WhhDh2i"
#define BLYNK_TEMPLATE_NAME "Lab5"
#define BLYNK_AUTH_TOKEN "dgQL275oCq6ByvMgANG_rY63DKXHZBug"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <HTTPClient.h>
#include <time.h>

// 🔧 Datos WiFi
char ssid[] = "yesid";
char pass[] = "yesid1234";

// 🌐 URL de tu servidor (API o endpoint)
const char* serverName = "URL_DE_GOOGLE_SHEETS";  // 🔹 Cambia por tu dirección real

// ⚙️ Pinos
const int lz = 12;       // Zumbador o LED
const int pinPIR = 14;   // Sensor PIR

// 🔄 Variables de control
bool systemEnabled = false;
bool movimientoAnterior = false;  // Para evitar envíos repetidos

// 🕒 Zona horaria (-5 es para Colombia, Perú, Ecuador, México central)
const long gmtOffset_sec = -5 * 3600;
const int daylightOffset_sec = 0;

// --------------------------- FUNCIONES ---------------------------

// Función que se ejecuta cuando cambias el switch (V0) en Blynk
BLYNK_WRITE(V0) {
  systemEnabled = param.asInt();

  if (!systemEnabled) {
    digitalWrite(lz, LOW);
    Blynk.virtualWrite(V1, LOW);
    Serial.println("🚫 Sistema desactivado desde Blynk");
  } else {
    Serial.println("✅ Sistema activado desde Blynk");
  }
}

// Configuración inicial
void setup() {
  Serial.begin(115200);
  pinMode(lz, OUTPUT);
  pinMode(pinPIR, INPUT);

  // Conexión a WiFi y Blynk
  Serial.println("Conectando a WiFi...");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Configurar hora con NTP
  configTime(gmtOffset_sec, daylightOffset_sec, "pool.ntp.org", "time.nist.gov");
  Serial.println("⏱ Sincronizando hora...");

  Blynk.virtualWrite(V1, LOW);
}

// Bucle principal
void loop() {
  Blynk.run();

  int sensorValue = digitalRead(pinPIR);

  if (systemEnabled) {
    if (sensorValue == HIGH && !movimientoAnterior) {
      movimientoAnterior = true;
      digitalWrite(lz, HIGH);
      Blynk.virtualWrite(V1, HIGH);
      Serial.println("⚠️ Movimiento detectado");

      // Enviar al servidor
      SendData();

      delay(2000); // Pequeña pausa para evitar múltiples lecturas seguidas
    } else if (sensorValue == LOW) {
      movimientoAnterior = false;
      digitalWrite(lz, LOW);
      Blynk.virtualWrite(V1, LOW);
    }
  }
}

// --------------------------- FUNCIÓN DE ENVÍO ---------------------------

void SendData() {
  if (WiFi.status() == WL_CONNECTED) {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
      Serial.println("⚠️ No se pudo obtener la hora");
      return;
    }

    char fechaHora[25];
    strftime(fechaHora, sizeof(fechaHora), "%Y-%m-%d %H:%M:%S", &timeinfo);

    // Construir JSON
    String jsonData = "{\"message\":\"Estoy en movimiento\",\"DateTime\":\"" + String(fechaHora) + "\"}";

    HTTPClient http;
    http.begin(serverName);
    http.addHeader("Content-Type", "application/json");

    int httpResponseCode = http.POST(jsonData);

    Serial.println("📡 Enviando datos al servidor...");
    Serial.print("📤 JSON: ");
    Serial.println(jsonData);
    Serial.print("🔢 Código HTTP: ");
    Serial.println(httpResponseCode);

    http.end();
  } else {
    Serial.println("❌ No hay conexión WiFi");
  }
}
