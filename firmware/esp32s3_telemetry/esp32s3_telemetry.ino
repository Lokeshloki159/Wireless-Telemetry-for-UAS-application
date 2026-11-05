#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include "secrets.h"

// -------------------- User configuration --------------------
static constexpr uint8_t SDA_PIN = 8;
static constexpr uint8_t SCL_PIN = 9;
static constexpr uint8_t BUTTON_PIN = 4;
static constexpr uint32_t SAMPLE_INTERVAL_MS = 100;  // 10 Hz sensor sampling
static constexpr uint32_t WIFI_RETRY_INTERVAL_MS = 5000;
static constexpr char DEVICE_ID[] = "uas-telemetry-01";
// ------------------------------------------------------------

Adafruit_MPU6050 mpu;
WebServer server(80);

struct Telemetry {
  float ax = 0.0f;
  float ay = 0.0f;
  float az = 0.0f;
  float gx = 0.0f;
  float gy = 0.0f;
  float gz = 0.0f;
  float temperatureC = 0.0f;
  bool buttonPressed = false;
  bool sensorOk = false;
  uint32_t sequence = 0;
  uint32_t lastSampleMs = 0;
};

Telemetry telemetry;
bool serverStarted = false;
uint32_t lastSampleAttemptMs = 0;
uint32_t lastWiFiAttemptMs = 0;

const char DASHBOARD_HTML[] PROGMEM = R"HTML(
<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>UAS Telemetry Ground Monitor</title>
<style>
  body{font-family:Arial,sans-serif;margin:0;padding:20px;background:#101827;color:#eef4ff}
  main{max-width:900px;margin:auto}
  h1{font-size:1.6rem}
  .muted{color:#aab8cc}
  .grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(190px,1fr));gap:12px}
  .card{background:#1e293b;border:1px solid #34445c;border-radius:12px;padding:16px}
  .label{color:#aab8cc;font-size:.9rem}
  .value{font-size:1.7rem;font-weight:700;color:#56c8ff;margin-top:8px}
  #connection{font-weight:700;color:#62df9a}
  code{color:#9edfff}
</style>
</head>
<body>
<main>
  <h1>UAS Wireless Telemetry – ESP32-S3</h1>
  <p class="muted">Bench prototype · MPU6050 IMU · local Wi-Fi / HTTP JSON</p>
  <p>Link: <span id="connection">Connecting…</span></p>
  <section class="grid">
    <div class="card"><div class="label">Acceleration X (m/s²)</div><div class="value" id="ax">--</div></div>
    <div class="card"><div class="label">Acceleration Y (m/s²)</div><div class="value" id="ay">--</div></div>
    <div class="card"><div class="label">Acceleration Z (m/s²)</div><div class="value" id="az">--</div></div>
    <div class="card"><div class="label">Gyroscope X (rad/s)</div><div class="value" id="gx">--</div></div>
    <div class="card"><div class="label">Gyroscope Y (rad/s)</div><div class="value" id="gy">--</div></div>
    <div class="card"><div class="label">Gyroscope Z (rad/s)</div><div class="value" id="gz">--</div></div>
    <div class="card"><div class="label">MPU6050 temperature (°C)</div><div class="value" id="temp">--</div></div>
    <div class="card"><div class="label">Push button</div><div class="value" id="button">--</div></div>
    <div class="card"><div class="label">IMU status</div><div class="value" id="sensor">--</div></div>
    <div class="card"><div class="label">Sample sequence</div><div class="value" id="seq">--</div></div>
  </section>
  <p class="muted">Raw JSON endpoint: <code>/api/telemetry</code></p>
</main>
<script>
const fields = ['ax','ay','az','gx','gy','gz'];
async function refreshTelemetry(){
  try{
    const response = await fetch('/api/telemetry', {cache:'no-store'});
    if(!response.ok) throw new Error('HTTP ' + response.status);
    const d = await response.json();
    fields.forEach(k => {
      const group = k.startsWith('a') ? d.accel_mps2 : d.gyro_rads;
      document.getElementById(k).textContent = Number(group[k[1]]).toFixed(3);
    });
    document.getElementById('temp').textContent = Number(d.temperature_c).toFixed(1);
    document.getElementById('button').textContent = d.button_pressed ? 'PRESSED' : 'RELEASED';
    document.getElementById('sensor').textContent = d.sensor_ok ? 'CONNECTED' : 'NOT FOUND';
    document.getElementById('seq').textContent = d.sequence;
    document.getElementById('connection').textContent = 'Connected · ' + d.device_id;
  }catch(err){
    document.getElementById('connection').textContent = 'No response – check Wi-Fi / ESP32';
    console.error(err);
  }
}
refreshTelemetry();
setInterval(refreshTelemetry, 500);
</script>
</body>
</html>
)HTML";

void sampleSensors() {
  telemetry.buttonPressed = (digitalRead(BUTTON_PIN) == LOW);

  if (telemetry.sensorOk) {
    sensors_event_t accel;
    sensors_event_t gyro;
    sensors_event_t temp;
    mpu.getEvent(&accel, &gyro, &temp);

    telemetry.ax = accel.acceleration.x;
    telemetry.ay = accel.acceleration.y;
    telemetry.az = accel.acceleration.z;
    telemetry.gx = gyro.gyro.x;
    telemetry.gy = gyro.gyro.y;
    telemetry.gz = gyro.gyro.z;
    telemetry.temperatureC = temp.temperature;
  }

  telemetry.sequence++;
  telemetry.lastSampleMs = millis();
}

String makeTelemetryJson() {
  char json[512];
  const uint32_t uptimeMs = millis();
  snprintf(
      json, sizeof(json),
      "{\"device_id\":\"%s\",\"sequence\":%lu,\"uptime_ms\":%lu,"
      "\"sensor_ok\":%s,"
      "\"accel_mps2\":{\"x\":%.4f,\"y\":%.4f,\"z\":%.4f},"
      "\"gyro_rads\":{\"x\":%.4f,\"y\":%.4f,\"z\":%.4f},"
      "\"temperature_c\":%.2f,\"button_pressed\":%s}",
      DEVICE_ID,
      static_cast<unsigned long>(telemetry.sequence),
      static_cast<unsigned long>(uptimeMs),
      telemetry.sensorOk ? "true" : "false",
      telemetry.ax, telemetry.ay, telemetry.az,
      telemetry.gx, telemetry.gy, telemetry.gz,
      telemetry.temperatureC,
      telemetry.buttonPressed ? "true" : "false");
  return String(json);
}

void configureRoutes() {
  server.on("/", HTTP_GET, []() {
    server.send_P(200, "text/html; charset=utf-8", DASHBOARD_HTML);
  });

  server.on("/api/telemetry", HTTP_GET, []() {
    server.sendHeader("Cache-Control", "no-store");
    server.send(200, "application/json", makeTelemetryJson());
  });

  server.onNotFound([]() {
    server.send(404, "text/plain", "Not found. Open / or /api/telemetry");
  });
}

void maintainWiFi() {
  if (WiFi.status() == WL_CONNECTED) {
    if (!serverStarted) {
      server.begin();
      serverStarted = true;
      Serial.println("HTTP server started.");
      Serial.print("Dashboard: http://");
      Serial.println(WiFi.localIP());
    }
    return;
  }

  // Mark the server for re-initialization after the station reconnects.
  serverStarted = false;

  const uint32_t now = millis();
  if (now - lastWiFiAttemptMs >= WIFI_RETRY_INTERVAL_MS) {
    lastWiFiAttemptMs = now;
    Serial.println("Wi-Fi disconnected; retrying...");
    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  }
}

void setup() {
  Serial.begin(115200);
  delay(200);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(400000);

  Serial.println("\nUAS Wireless Telemetry – ESP32-S3");
  telemetry.sensorOk = mpu.begin(0x68, &Wire);
  if (!telemetry.sensorOk) {
    Serial.println("MPU6050 not found at 0x68; trying 0x69...");
    telemetry.sensorOk = mpu.begin(0x69, &Wire);
  }

  if (telemetry.sensorOk) {
    Serial.println("MPU6050 detected.");
    mpu.setAccelerometerRange(MPU6050_RANGE_4_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  } else {
    Serial.println("WARNING: MPU6050 not detected. GPIO telemetry remains available.");
  }

  configureRoutes();
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  lastWiFiAttemptMs = millis();
  Serial.println("Connecting to configured Wi-Fi...");
}

void loop() {
  const uint32_t now = millis();

  if (now - lastSampleAttemptMs >= SAMPLE_INTERVAL_MS) {
    lastSampleAttemptMs = now;
    sampleSensors();
  }

  maintainWiFi();
  if (WiFi.status() == WL_CONNECTED && serverStarted) {
    server.handleClient();
  }
}
