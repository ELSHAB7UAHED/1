// AhmedAP/ AhmedAP.ino
#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "Ahmed";
const char* password = "1234567890";

const int LED_PIN = 2; // غيره لو لمبتك على دبوس آخر (مثلاً 5 أو 4...)
WebServer server(80);

String htmlPage() {
  String html = R"rawliteral(
<!doctype html>
<html>
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <title>Ahmed ESP32 - تحكم باللمبة</title>
  <style>
    body { font-family: Arial, Helvetica, sans-serif; text-align:center; padding:20px; }
    .led {
      margin: 20px auto;
      width: 80px; height: 80px;
      border-radius: 50%;
      background: #222; /* off */
      box-shadow: 0 0 10px rgba(0,0,0,0.3);
    }
    .led.on { background: #00f; box-shadow: 0 0 30px rgba(0,0,255,0.7); } /* أزرق لما شغّال */
    button { padding: 12px 20px; font-size:16px; cursor:pointer; border-radius:6px; }
  </style>
</head>
<body>
  <h2>شبكة: Ahmed — صفحة التحكم</h2>
  <div id="led" class="led"></div>
  <div>
    <button id="toggleBtn">تحميل...</button>
  </div>

<script>
async function setLed(state) {
  // state: "on" أو "off"
  try {
    await fetch('/set?state=' + state);
    updateStatus();
  } catch(e) {
    console.error(e);
  }
}

async function getStatus() {
  try {
    const res = await fetch('/status');
    if (!res.ok) return {on:false};
    return res.json();
  } catch(e) {
    return {on:false};
  }
}

async function updateStatus() {
  const s = await getStatus();
  const led = document.getElementById('led');
  const btn = document.getElementById('toggleBtn');
  if (s.on) {
    led.classList.add('on');
    btn.textContent = 'إطفاء اللمبة';
    btn.onclick = () => setLed('off');
  } else {
    led.classList.remove('on');
    btn.textContent = 'تشغيل اللمبة';
    btn.onclick = () => setLed('on');
  }
}

window.addEventListener('load', () => {
  updateStatus();
  // تحديث الحالة كل 3 ثواني
  setInterval(updateStatus, 3000);
});
</script>
</body>
</html>
)rawliteral";
  return html;
}

void handleRoot() {
  server.send(200, "text/html; charset=utf-8", htmlPage());
}

// endpoint: /set?state=on أو off
void handleSet() {
  if (server.hasArg("state")) {
    String s = server.arg("state");
    if (s == "on") {
      digitalWrite(LED_PIN, HIGH);
      server.send(200, "application/json", "{\"result\":\"ok\",\"on\":true}");
      return;
    } else if (s == "off") {
      digitalWrite(LED_PIN, LOW);
      server.send(200, "application/json", "{\"result\":\"ok\",\"on\":false}");
      return;
    }
  }
  server.send(400, "application/json", "{\"result\":\"error\",\"msg\":\"state missing or invalid\"}");
}

void handleStatus() {
  bool on = digitalRead(LED_PIN) == HIGH;
  String payload = String("{\"on\":") + (on ? "true" : "false") + "}";
  server.send(200, "application/json", payload);
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW); // ابدأ مطفي

  Serial.begin(115200);
  delay(100);

  WiFi.mode(WIFI_AP);
  bool ok = WiFi.softAP(ssid, password);
  if (ok) {
    Serial.println();
    Serial.print("AP started: ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.softAPIP());
  } else {
    Serial.println("Failed to start AP");
  }

  server.on("/", handleRoot);
  server.on("/set", handleSet);
  server.on("/status", handleStatus);
  server.onNotFound([](){
    server.send(404, "text/plain", "Not found");
  });

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}
