
#include <WiFi.h>
#include <ESP32Servo.h>
#include "DHT.h"

// ---------------- WIFI ----------------
const char* ssid = "Capstrack";
const char* password = "987654321";
WiFiServer server(80);

// ---------------- PINS ----------------
#define IR1 33
#define IR2 32
#define DOOR_SERVO 18
#define WINDOW_SERVO 19
#define DHTPIN 4
#define DHTTYPE DHT22
#define FLAME_SENSOR 27
#define BUZZER 25
#define FAN_PWM 26
#define RELAY_PIN 23

// ---------------- OBJECTS ----------------
Servo doorServo;
Servo windowServo;
DHT dht(DHTPIN, DHTTYPE);

// ---------------- VARIABLES ----------------
int peopleCount = 0;
int state = 0;
unsigned long triggerTime = 0;
unsigned long sensorTimeout = 2000;

bool flameDetected = false;
float temperature = 0;
bool windowStatus = false;
bool doorStatus = false;

const int pwmChannel = 2;
const int pwmFreq = 5000;
const int pwmResolution = 8;

// ---------------- SETUP ----------------
void setup() {
  Serial.begin(115200);

  pinMode(IR1, INPUT);
  pinMode(IR2, INPUT);
  pinMode(FLAME_SENSOR, INPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);

  digitalWrite(RELAY_PIN, HIGH);

  doorServo.attach(DOOR_SERVO, 500, 2400);
  windowServo.attach(WINDOW_SERVO, 500, 2400);

  doorServo.write(0);
  windowServo.write(0);

  dht.begin();

  ledcSetup(pwmChannel, pwmFreq, pwmResolution);
  ledcAttachPin(FAN_PWM, pwmChannel);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);

  Serial.println("Connected to WiFi");
  Serial.println(WiFi.localIP());

  server.begin();
}

// ---------------- LOOP ----------------
void loop() {
  temperature = dht.readTemperature();
  int fanSpeed = 0;

  if (!isnan(temperature)) {
    if (temperature < 25) fanSpeed = 0;
    else if (temperature < 30) fanSpeed = 120;
    else if (temperature < 35) fanSpeed = 200;
    else fanSpeed = 255;
  }

  flameDetected = (digitalRead(FLAME_SENSOR) == LOW);

  if (flameDetected || temperature > 34) {
    if (!windowStatus) {
      windowServo.write(90);
      windowStatus = true;
    }
    digitalWrite(BUZZER, HIGH);
  } else {
    if (windowStatus) {
      windowServo.write(0);
      windowStatus = false;
    }
    digitalWrite(BUZZER, LOW);
  }

  ledcWrite(pwmChannel, fanSpeed);

  int ir1 = digitalRead(IR1);
  int ir2 = digitalRead(IR2);

  if (state == 0) {
    if (ir1 == LOW) { state = 1; triggerTime = millis(); }
    else if (ir2 == LOW) { state = 2; triggerTime = millis(); }
  }
  else if (state == 1) {
    if (ir2 == LOW) { peopleCount++; openDoor(); state = 0; }
    else if (millis() - triggerTime > sensorTimeout) state = 0;
  }
  else if (state == 2) {
    if (ir1 == LOW) { if(peopleCount>0) peopleCount--; openDoor(); state = 0; }
    else if (millis() - triggerTime > sensorTimeout) state = 0;
  }

  digitalWrite(RELAY_PIN, peopleCount > 0 ? LOW : HIGH);

  WiFiClient client = server.available();
  if (client) {
    client.readStringUntil('\r');
    client.flush();

    int tempDeg = map((int)temperature, 0, 50, 0, 270);
    int fanDeg = map(fanSpeed, 0, 255, 0, 270);

    client.println("HTTP/1.1 200 OK");
    client.println("Content-type:text/html");
    client.println();

    client.println("<!DOCTYPE html><html><head>");
    client.println("<meta name='viewport' content='width=device-width, initial-scale=1'>");
    client.println("<meta http-equiv='refresh' content='2'>");

    client.println("<style>");
    client.println("body{margin:0;font-family:Segoe UI;background:linear-gradient(135deg,#0f2027,#203a43,#2c5364);color:white;text-align:center;}");
    client.println(".grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(220px,1fr));gap:20px;padding:20px;}");
    client.println(".card{background:rgba(255,255,255,0.08);padding:20px;border-radius:20px;box-shadow:0 0 25px rgba(0,255,200,0.2);}");
    client.println(".value{font-size:26px;color:#36f5b0;margin:15px 0;}");
    client.println(".circle{width:160px;height:160px;border-radius:50%;margin:auto;display:flex;align-items:center;justify-content:center;}");
    client.println(".inner{width:120px;height:120px;background:#0f2027;border-radius:50%;display:flex;flex-direction:column;align-items:center;justify-content:center;color:#36f5b0;font-size:22px;}");
    client.println(".watermark{position:fixed;top:50%;left:50%;transform:translate(-50%,-50%);font-size:140px;font-weight:900;letter-spacing:20px;color:rgba(0,255,200,0.05);z-index:0;pointer-events:none;}");
    client.println("</style></head><body>");

    client.println("<div class='watermark'>NEXORA</div>");
    client.println("<h1>NEXORA Smart Ventilation System</h1>");
    client.println("<div class='grid'>");

    client.println("<div class='card'><h3>Temperature</h3>");
    client.println("<div class='circle' style='background:conic-gradient(#36f5b0 "+String(tempDeg)+"deg,#111 "+String(tempDeg)+"deg);'><div class='inner'>"+String(temperature)+"°C</div></div></div>");

    client.println("<div class='card'><h3>Fan Speed</h3>");
    client.println("<div class='circle' style='background:conic-gradient(#36f5b0 "+String(fanDeg)+"deg,#111 "+String(fanDeg)+"deg);'><div class='inner'>"+String(fanSpeed)+" PWM</div></div></div>");

    client.println("<div class='card'><h3>People Count</h3><div class='value'>"+String(peopleCount)+"</div></div>");
    client.println("<div class='card'><h3>Door</h3><div class='value'>"+String(doorStatus?"OPEN":"CLOSED")+"</div></div>");
    client.println("<div class='card'><h3>Window</h3><div class='value'>"+String(windowStatus?"OPEN":"CLOSED")+"</div></div>");
    client.println("<div class='card'><h3>Flame</h3><div class='value'>"+String(flameDetected?"🔥 DETECTED":"SAFE")+"</div></div>");

    client.println("</div></body></html>");
    client.stop();
  }

  delay(200);
}

void openDoor() {
  doorServo.write(90);
  doorStatus = true;
  delay(2000);
  doorServo.write(0);
  doorStatus = false;
}
