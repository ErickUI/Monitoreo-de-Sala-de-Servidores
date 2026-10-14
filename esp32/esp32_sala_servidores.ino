#include <WiFi.h>
#include <PubSubClient.h>
#include <sys/time.h>
#include <time.h>
#include "DHT.h"

#define DHT_PIN   4
#define DHT_TYPE  DHT22
#define LDR_PIN   34

DHT dht(DHT_PIN, DHT_TYPE);

const bool LDR_INVERTIDO = true;

const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASS = "";

const char* MQTT_SERVER = "broker.hivemq.com";
const int   MQTT_PORT   = 1883;

const char* DEVICE_ID      = "esp32-sala01";
const char* TOPIC_LECTURAS = "datacenter_andinoSAC/sala_servidores/lecturas";
const char* TOPIC_ESTADO   = "datacenter_andinoSAC/sala_servidores/estado";
const char* TOPIC_ERROR    = "datacenter_andinoSAC/sala_servidores/error";

const char* NTP_1 = "pool.ntp.org";
const char* NTP_2 = "time.google.com";
const unsigned long RESYNC_NTP_MS = 15000;
unsigned long ultimoSyncNtp = 0;

WiFiClient espClient;
PubSubClient client(espClient);

const unsigned long INTERVALO_MS = 2000;
unsigned long ultimoEnvio = 0;
unsigned long ultimoIntentoMqtt = 0;
uint32_t secuencia = 0;

void setup_wifi() {
  Serial.print("Conectando a WiFi: ");
  Serial.println(WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado. IP: " + WiFi.localIP().toString());
}

void sincronizarHora() {
  configTime(0, 0, NTP_1, NTP_2);
  ultimoSyncNtp = millis();
}

int64_t ahoraMs() {
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  if (tv.tv_sec < 1700000000L) return 0;
  return (int64_t)tv.tv_sec * 1000LL + (int64_t)(tv.tv_usec / 1000);
}

void conectarMQTT() {
  if (client.connected()) return;
  if (millis() - ultimoIntentoMqtt < 5000) return;
  ultimoIntentoMqtt = millis();

  Serial.print("Conectando a MQTT... ");
  String clientId = String(DEVICE_ID) + "-" + String((uint32_t)ESP.getEfuseMac(), HEX);

  if (client.connect(clientId.c_str(), nullptr, nullptr,
                     TOPIC_ESTADO, 1, true, "offline")) {
    Serial.println("conectado");
    client.publish(TOPIC_ESTADO, "online", true);
  } else {
    Serial.print("fallo, rc=");
    Serial.println(client.state());
  }
}

bool leerDHT(float &t, float &h) {
  for (int i = 0; i < 3; i++) {
    h = dht.readHumidity();
    t = dht.readTemperature();
    if (!isnan(t) && !isnan(h) && t > -40 && t < 80 && h >= 0 && h <= 100) {
      return true;
    }
    delay(500);
  }
  return false;
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  dht.begin();
  setup_wifi();

  sincronizarHora();
  unsigned long t0 = millis();
  while (ahoraMs() == 0 && millis() - t0 < 10000) {
    delay(200);
  }
  if (ahoraMs() > 0) {
    Serial.printf("Hora NTP sincronizada: %lld ms\n", (long long)ahoraMs());
  } else {
    Serial.println("Aviso: NTP no sincronizo; ts_ms se enviara como 0");
  }

  client.setServer(MQTT_SERVER, MQTT_PORT);
  client.setBufferSize(512);
  client.setSocketTimeout(5);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi perdido, reconectando...");
    WiFi.reconnect();
    delay(1000);
    return;
  }

  if (millis() - ultimoSyncNtp >= RESYNC_NTP_MS) sincronizarHora();

  conectarMQTT();
  client.loop();

  if (millis() - ultimoEnvio < INTERVALO_MS) return;
  ultimoEnvio = millis();

  float temperatura, humedad;
  if (!leerDHT(temperatura, humedad)) {
    Serial.println("Error: lectura invalida del DHT22");
    if (client.connected()) {
      char err[120];
      snprintf(err, sizeof(err),
               "{\"dispositivo\":\"%s\",\"error\":\"dht22_invalido\"}", DEVICE_ID);
      client.publish(TOPIC_ERROR, err);
    }
    return;
  }

  long long tsMs = (long long)ahoraMs();

  int ldrRaw = analogRead(LDR_PIN);
  int luzPct = map(ldrRaw, 0, 4095, 0, 100);
  luzPct = constrain(luzPct, 0, 100);
  if (LDR_INVERTIDO) luzPct = 100 - luzPct;

  char mensaje[320];
  snprintf(mensaje, sizeof(mensaje),
           "{\"dispositivo\":\"%s\",\"seq\":%lu,\"ts_ms\":%lld,"
           "\"temperatura\":%.2f,\"humedad\":%.1f,"
           "\"ldr_raw\":%d,\"luz_porcentaje\":%d}",
           DEVICE_ID, (unsigned long)secuencia, tsMs,
           temperatura, humedad, ldrRaw, luzPct);

  Serial.printf("[seq %lu] ts=%lld | T=%.2f C | H=%.1f %% | LDR=%d | Luz=%d %%\n",
                (unsigned long)secuencia, tsMs, temperatura, humedad, ldrRaw, luzPct);

  if (client.connected()) {
    bool ok = client.publish(TOPIC_LECTURAS, mensaje);
    Serial.println(ok ? "Publicado OK" : "Error al publicar");
  } else {
    Serial.println("Sin MQTT: mensaje no enviado");
  }
  secuencia++;
}
