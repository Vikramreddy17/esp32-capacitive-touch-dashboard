#include <WiFi.h>
#include <WebServer.h>

// ============================================================
// ESP32 CAPACITIVE TOUCH + ACCESS POINT WEB DASHBOARD
// ============================================================

// ---------- Touch configuration ----------
#define TOUCH_PIN 4

const int NUM_SAMPLES = 10;
const int TOUCH_THRESHOLD = 15;
const int REQUIRED_TOUCH_COUNT = 3;

// ---------- Wi-Fi Access Point ----------
const char* AP_SSID = "ESP32-Touch";
const char* AP_PASSWORD = "128-12345678";

IPAddress local_IP(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

// ---------- Web server ----------
WebServer server(80);

// ---------- Touch state ----------
int touchCount = 0;
float currentAverage = 0.0;

bool touchDetected = false;
bool previousTouchState = false;

unsigned long lastTouchTime = 0;
unsigned long touchEventCount = 0;


// ============================================================
// READ TOUCH SENSOR
// ============================================================

float readTouchAverage() {

  uint32_t total = 0;

  for (int i = 0; i < NUM_SAMPLES; i++) {

    total += touchRead(TOUCH_PIN);

    delay(10);
  }

  return (float)total / NUM_SAMPLES;
}


// ============================================================
// UPDATE TOUCH STATE
// ============================================================

void updateTouchState() {

  currentAverage = readTouchAverage();

  // Touch candidate
  if (currentAverage < TOUCH_THRESHOLD) {

    touchCount++;

    if (touchCount >= REQUIRED_TOUCH_COUNT) {

      touchDetected = true;

      // Detect only the transition:
      // NO TOUCH -> TOUCH
      if (!previousTouchState) {

        lastTouchTime = millis();
        touchEventCount++;
      }
    }

  } else {

    // Reset consecutive touch counter
    touchCount = 0;

    touchDetected = false;
  }

  previousTouchState = touchDetected;
}


// ============================================================
// HTML PAGE
// ============================================================

const char MAIN_PAGE[] PROGMEM = R"rawliteral(

<!DOCTYPE html>

<html lang="en">

<head>

<meta charset="UTF-8">

<meta
  name="viewport"
  content="width=device-width, initial-scale=1.0"
>

<title>ESP32 Touch Panel</title>

<style>

* {
    box-sizing: border-box;
    margin: 0;
    padding: 0;
}

body {

    font-family:
        Inter,
        -apple-system,
        BlinkMacSystemFont,
        "Segoe UI",
        Roboto,
        Arial,
        sans-serif;

    min-height: 100vh;

    background:
        radial-gradient(
            circle at top left,
            #1e293b,
            #0f172a 45%,
            #020617
        );

    color: #f8fafc;

    display: flex;

    justify-content: center;

    align-items: center;

    padding: 24px;
}


.container {

    width: 100%;

    max-width: 920px;

    display: flex;

    flex-direction: column;

    gap: 20px;
}


/* =========================================================
   HEADER
   ========================================================= */

.header {

    display: flex;

    justify-content: space-between;

    align-items: center;

    background: rgba(15, 23, 42, 0.82);

    border: 1px solid rgba(255,255,255,0.08);

    backdrop-filter: blur(14px);

    border-radius: 22px;

    padding: 22px 26px;

    box-shadow:
        0 20px 60px rgba(0,0,0,0.30);
}


.brand {

    display: flex;

    align-items: center;

    gap: 14px;
}


.brand-icon {

    width: 48px;

    height: 48px;

    border-radius: 14px;

    display: flex;

    align-items: center;

    justify-content: center;

    background: linear-gradient(
        135deg,
        #2563eb,
        #7c3aed
    );

    font-size: 24px;

    box-shadow:
        0 8px 25px rgba(37,99,235,0.35);
}


.brand-text h1 {

    font-size: 21px;

    font-weight: 700;

    letter-spacing: -0.3px;
}


.brand-text p {

    margin-top: 4px;

    color: #94a3b8;

    font-size: 13px;
}


.connection {

    display: flex;

    align-items: center;

    gap: 8px;

    padding: 8px 12px;

    border-radius: 999px;

    background: rgba(34,197,94,0.10);

    color: #86efac;

    font-size: 12px;

    font-weight: 600;
}


.connection-dot {

    width: 8px;

    height: 8px;

    border-radius: 50%;

    background: #22c55e;

    box-shadow:
        0 0 12px rgba(34,197,94,0.9);

    animation: blink 2s infinite;
}


@keyframes blink {

    0%, 100% {
        opacity: 1;
    }

    50% {
        opacity: 0.4;
    }
}


/* =========================================================
   MAIN TOUCH CARD
   ========================================================= */

.touch-card {

    position: relative;

    overflow: hidden;

    background: rgba(15,23,42,0.82);

    border: 1px solid rgba(255,255,255,0.08);

    backdrop-filter: blur(14px);

    border-radius: 28px;

    padding: 42px 30px;

    text-align: center;

    box-shadow:
        0 25px 70px rgba(0,0,0,0.35);
}


.touch-card::before {

    content: "";

    position: absolute;

    width: 280px;

    height: 280px;

    border-radius: 50%;

    background: rgba(37,99,235,0.10);

    filter: blur(50px);

    top: -130px;

    left: -120px;
}


.touch-card::after {

    content: "";

    position: absolute;

    width: 280px;

    height: 280px;

    border-radius: 50%;

    background: rgba(124,58,237,0.10);

    filter: blur(50px);

    right: -120px;

    bottom: -130px;
}


.touch-area {

    position: relative;

    z-index: 1;

    display: flex;

    flex-direction: column;

    align-items: center;
}


.touch-circle {

    width: 170px;

    height: 170px;

    border-radius: 50%;

    display: flex;

    align-items: center;

    justify-content: center;

    margin-bottom: 26px;

    background:
        radial-gradient(
            circle,
            rgba(37,99,235,0.25),
            rgba(37,99,235,0.06)
        );

    border: 2px solid rgba(96,165,250,0.35);

    transition:
        transform 0.18s ease,
        background 0.18s ease,
        border-color 0.18s ease,
        box-shadow 0.18s ease;
}


.touch-inner {

    width: 110px;

    height: 110px;

    border-radius: 50%;

    display: flex;

    align-items: center;

    justify-content: center;

    background: rgba(15,23,42,0.92);

    border: 1px solid rgba(255,255,255,0.08);

    font-size: 46px;
}


.touch-circle.touched {

    background:
        radial-gradient(
            circle,
            rgba(34,197,94,0.38),
            rgba(34,197,94,0.08)
        );

    border-color: rgba(74,222,128,0.75);

    box-shadow:
        0 0 0 18px rgba(34,197,94,0.05),
        0 0 70px rgba(34,197,94,0.35);

    transform: scale(1.04);

    animation: touchPulse 1s infinite;
}


@keyframes touchPulse {

    0% {
        box-shadow:
            0 0 0 0 rgba(34,197,94,0.25),
            0 0 40px rgba(34,197,94,0.25);
    }

    70% {
        box-shadow:
            0 0 0 24px rgba(34,197,94,0),
            0 0 70px rgba(34,197,94,0.35);
    }

    100% {
        box-shadow:
            0 0 0 0 rgba(34,197,94,0),
            0 0 40px rgba(34,197,94,0.25);
    }
}


.status {

    font-size: 30px;

    font-weight: 750;

    letter-spacing: -0.8px;

    margin-bottom: 10px;
}


.status.no-touch {

    color: #e2e8f0;
}


.status.touch {

    color: #4ade80;
}


.description {

    color: #94a3b8;

    font-size: 14px;

    line-height: 1.6;

    max-width: 500px;
}


/* =========================================================
   LIVE VALUE
   ========================================================= */

.value-label {

    margin-top: 28px;

    color: #64748b;

    text-transform: uppercase;

    letter-spacing: 1.2px;

    font-size: 10px;

    font-weight: 700;
}


.value {

    margin-top: 6px;

    font-size: 38px;

    font-weight: 750;

    font-variant-numeric: tabular-nums;

    color: #f8fafc;
}


/* =========================================================
   STATS
   ========================================================= */

.stats {

    display: grid;

    grid-template-columns:
        repeat(3, 1fr);

    gap: 16px;
}


.stat-card {

    background: rgba(15,23,42,0.82);

    border: 1px solid rgba(255,255,255,0.08);

    border-radius: 20px;

    padding: 20px;

    box-shadow:
        0 15px 35px rgba(0,0,0,0.20);
}


.stat-title {

    color: #64748b;

    font-size: 11px;

    text-transform: uppercase;

    letter-spacing: 0.9px;

    font-weight: 700;

    margin-bottom: 10px;
}


.stat-value {

    font-size: 21px;

    font-weight: 700;

    font-variant-numeric: tabular-nums;
}


.stat-sub {

    margin-top: 5px;

    font-size: 12px;

    color: #94a3b8;
}


/* =========================================================
   FOOTER INFO
   ========================================================= */

.info {

    display: flex;

    justify-content: space-between;

    align-items: center;

    gap: 20px;

    background: rgba(15,23,42,0.65);

    border: 1px solid rgba(255,255,255,0.07);

    border-radius: 18px;

    padding: 15px 20px;

    color: #94a3b8;

    font-size: 12px;
}


.info strong {

    color: #e2e8f0;

    font-weight: 600;
}


@media (max-width: 700px) {

    body {
        padding: 14px;
    }

    .header {
        padding: 18px;
    }

    .brand-text h1 {
        font-size: 18px;
    }

    .connection {
        display: none;
    }

    .touch-card {
        padding: 34px 20px;
    }

    .touch-circle {
        width: 145px;
        height: 145px;
    }

    .touch-inner {
        width: 94px;
        height: 94px;
        font-size: 38px;
    }

    .status {
        font-size: 25px;
    }

    .stats {
        grid-template-columns: 1fr;
    }

    .info {
        flex-direction: column;
        align-items: flex-start;
    }
}

</style>

</head>


<body>

<div class="container">


    <!-- HEADER -->

    <header class="header">

        <div class="brand">

            <div class="brand-icon">
                ✋
            </div>

            <div class="brand-text">

                <h1>ESP32 Touch Panel</h1>

                <p>
                    Capacitive touch monitoring
                </p>

            </div>

        </div>


        <div class="connection">

            <span class="connection-dot"></span>

            ESP32 ONLINE

        </div>

    </header>



    <!-- TOUCH CARD -->

    <section class="touch-card">

        <div class="touch-area">

            <div id="touchCircle"
                 class="touch-circle">

                <div class="touch-inner">
                    ✋
                </div>

            </div>


            <div id="status"
                 class="status no-touch">

                NO TOUCH

            </div>


            <p id="description"
               class="description">

                Touch the connected copper plate
                to trigger the sensor.

            </p>


            <div class="value-label">
                Live sensor value
            </div>


            <div id="sensorValue"
                 class="value">

                --

            </div>

        </div>

    </section>



    <!-- LIVE STATS -->

    <section class="stats">


        <div class="stat-card">

            <div class="stat-title">
                Threshold
            </div>

            <div class="stat-value">
                15
            </div>

            <div class="stat-sub">
                Touch below this value
            </div>

        </div>


        <div class="stat-card">

            <div class="stat-title">
                Touch Events
            </div>

            <div id="touchEvents"
                 class="stat-value">

                0

            </div>

            <div class="stat-sub">
                Detected transitions
            </div>

        </div>


        <div class="stat-card">

            <div class="stat-title">
                Last Touch
            </div>

            <div id="lastTouch"
                 class="stat-value">

                Never

            </div>

            <div class="stat-sub">
                Latest trigger
            </div>

        </div>


    </section>



    <!-- DEVICE INFO -->

    <footer class="info">

        <div>
            Wi-Fi:
            <strong>ESP32-Touch</strong>
        </div>

        <div>
            IP:
            <strong>192.168.4.1</strong>
        </div>

        <div>
            GPIO:
            <strong>4</strong>
        </div>

    </footer>


</div>



<script>


const touchCircle =
    document.getElementById("touchCircle");

const statusElement =
    document.getElementById("status");

const description =
    document.getElementById("description");

const sensorValue =
    document.getElementById("sensorValue");

const touchEvents =
    document.getElementById("touchEvents");

const lastTouch =
    document.getElementById("lastTouch");


let lastEventCount = 0;


function updateUI(data) {

    const touched = data.touch;


    // Live sensor value

    sensorValue.textContent =
        Number(data.value).toFixed(1);


    // Event count

    touchEvents.textContent =
        data.events;


    // Touch state

    if (touched) {

        touchCircle.classList.add("touched");

        statusElement.className =
            "status touch";

        statusElement.textContent =
            "TOUCH DETECTED";

        description.textContent =
            "Copper plate touch detected.";

    } else {

        touchCircle.classList.remove("touched");

        statusElement.className =
            "status no-touch";

        statusElement.textContent =
            "NO TOUCH";

        description.textContent =
            "Touch the connected copper plate to trigger the sensor.";
    }


    // Update timestamp
    // only when a new touch event occurs

    if (data.events !== lastEventCount) {

        const now = new Date();

        lastTouch.textContent =
            now.toLocaleTimeString();

        lastEventCount = data.events;
    }
}



async function updateSensor() {

    try {

        const response =
            await fetch("/status", {
                cache: "no-store"
            });

        const data =
            await response.json();

        updateUI(data);

    }

    catch (error) {

        statusElement.textContent =
            "CONNECTION ERROR";

        statusElement.className =
            "status no-touch";

        touchCircle.classList.remove("touched");
    }
}


// Update approximately every 100 ms

setInterval(updateSensor, 100);


// Immediate first update

updateSensor();


</script>


</body>

</html>

)rawliteral";


// ============================================================
// ROOT PAGE
// ============================================================

void handleRoot() {

  server.send_P(
    200,
    "text/html",
    MAIN_PAGE
  );
}


// ============================================================
// SENSOR STATUS API
// ============================================================

void handleStatus() {

  String json = "{";

  json += "\"touch\":";
  json += touchDetected ? "true" : "false";

  json += ",\"value\":";
  json += String(currentAverage, 1);

  json += ",\"events\":";
  json += String(touchEventCount);

  json += ",\"threshold\":";
  json += String(TOUCH_THRESHOLD);

  json += "}";

  server.send(
    200,
    "application/json",
    json
  );
}


// ============================================================
// 404
// ============================================================

void handleNotFound() {

  server.send(
    404,
    "text/plain",
    "404 - Page not found"
  );
}


// ============================================================
// SETUP
// ============================================================

void setup() {

  Serial.begin(115200);

  delay(1000);


  Serial.println();
  Serial.println("=================================");
  Serial.println(" ESP32 CAPACITIVE TOUCH PANEL");
  Serial.println("=================================");


  // ----------------------------------------------------------
  // Start Access Point
  // ----------------------------------------------------------

  WiFi.mode(WIFI_AP);

  WiFi.softAPConfig(
    local_IP,
    gateway,
    subnet
  );

  bool apStarted =
    WiFi.softAP(
      AP_SSID,
      AP_PASSWORD
    );


  if (apStarted) {

    Serial.println("Access Point started");

  } else {

    Serial.println("Access Point FAILED");

  }


  Serial.println();

  Serial.print("SSID: ");
  Serial.println(AP_SSID);

  Serial.print("Password: ");
  Serial.println(AP_PASSWORD);

  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());

  Serial.println();


  // ----------------------------------------------------------
  // Web server routes
  // ----------------------------------------------------------

  server.on(
    "/",
    HTTP_GET,
    handleRoot
  );


  server.on(
    "/status",
    HTTP_GET,
    handleStatus
  );


  server.onNotFound(
    handleNotFound
  );


  server.begin();


  Serial.println("Web server started.");

  Serial.println();
  Serial.println("Open this in your browser:");
  Serial.println("http://192.168.4.1");

  Serial.println();
}


// ============================================================
// LOOP
// ============================================================

void loop() {

  // Update touch sensor

  updateTouchState();


  // Handle browser requests

  server.handleClient();
}
