#include <TFT_eSPI.h>
#include <SPI.h>
#include <WiFi.h>
#include <WebServer.h>
#include "config.h"

TFT_eSPI tft = TFT_eSPI();
WebServer server(HTTP_PORT);

#define CX 120
#define CY 120

// =====================================================
// اندازه‌ها
// =====================================================

#define EYE_RADIUS    119

// عنبیه / قرنیه بزرگ‌تر
#define IRIS_RADIUS   108

// مردمک بزرگ‌تر
#define PUPIL_RADIUS  29

// =====================================================
// رنگ‌ها
// =====================================================

#define BG          TFT_BLACK

#define RED_1       0x3000
#define RED_2       0x5800
#define RED_3       0x9000
#define RED_4       0xF800
#define RED_5       0xF920

#define DARK_RED    0x2800

#define BLACK       TFT_BLACK
#define WHITE       TFT_WHITE

// Runtime state
int pupilX = CX;
int pupilY = CY;
int pupilRadius = PUPIL_RADIUS;
uint16_t irisColor = IRIS_BROWN;
int emotion = EMO_NORMAL;
int blinkStyle = BLINK_NORMAL;

// Timing
unsigned long lastBlink = 0;

// =====================================================
// Drawing helpers (same logic as previous fixed example)
// =====================================================

void drawBaseEye()
{
    tft.fillScreen(BG);

    // دایره بیرونی
    tft.fillCircle(CX, CY, EYE_RADIUS, RED_1);

    // عنبیه بزرگ
    tft.fillCircle(CX, CY, IRIS_RADIUS, RED_2);

    // لایه دوم
    tft.fillCircle(CX, CY, 96, RED_3);

    // لایه سوم
    tft.fillCircle(CX, CY, 84, RED_4);

    // مرکز روشن
    tft.fillCircle(CX, CY, 70, RED_5);

    // حلقه‌ها
    tft.drawCircle(CX, CY, 103, DARK_RED);
    tft.drawCircle(CX, CY, 106, RED_3);
    tft.drawCircle(CX, CY, 108, DARK_RED);
}

void redrawIrisUnderPupil(int x, int y)
{
    int r = pupilRadius + 10;
    tft.fillCircle(CX, CY, IRIS_RADIUS, RED_2);
    tft.fillCircle(CX, CY, 96, RED_3);
    tft.fillCircle(CX, CY, 84, RED_4);
    tft.fillCircle(CX, CY, 70, RED_5);
    tft.drawCircle(CX, CY, 103, DARK_RED);
    tft.drawCircle(CX, CY, 106, RED_3);
    tft.drawCircle(CX, CY, 108, DARK_RED);
    tft.fillCircle(x, y, r, RED_4);
    // catchlights relative to pupil
    tft.fillCircle(x - 10, y - 10, 6, WHITE);
    tft.fillCircle(x + 9, y + 10, 2, WHITE);
}

void clearPupilArea(int x, int y)
{
    redrawIrisUnderPupil(x, y);
}

void drawPupil(int x, int y)
{
    tft.fillCircle(x, y, pupilRadius, BLACK);
    tft.drawCircle(x, y, pupilRadius + 2, DARK_RED);
    tft.fillCircle(x - 10, y - 10, max(2, pupilRadius / 4), WHITE);
}

// Constrain a gaze point inside the iris
void constrainInsideIris(int &tx, int &ty)
{
    float dx = tx - CX;
    float dy = ty - CY;
    float dist = sqrt(dx*dx + dy*dy);
    float maxDist = IRIS_RADIUS - pupilRadius - 4;
    if (dist > maxDist && dist > 0.001f) {
        float s = maxDist / dist;
        tx = CX + (int)(dx * s);
        ty = CY + (int)(dy * s);
    }
}

void movePupil(int targetX, int targetY, int duration_ms = GAZE_CHANGE_TIME)
{
    constrainInsideIris(targetX, targetY);
    int ox = pupilX;
    int oy = pupilY;
    const int steps = max(4, duration_ms / 18);
    for (int i = 1; i <= steps; i++) {
        float p = (float)i / steps;
        float smooth = p * p * (3.0f - 2.0f * p);
        int nx = ox + (int)((targetX - ox) * smooth);
        int ny = oy + (int)((targetY - oy) * smooth);
        clearPupilArea(pupilX, pupilY);
        pupilX = nx; pupilY = ny;
        drawPupil(pupilX, pupilY);
        delay(18);
    }
}

void blinkOnce(int speed_ms = BLINK_SPEED_NORMAL)
{
    int steps = max(3, speed_ms / 25);
    for (int i = 0; i <= steps; i++) {
        int h = (EYE_RADIUS * i) / steps;
        // draw covers
        drawBaseEye();
        tft.fillRect(0, 0, TFT_WIDTH, CY - (EYE_RADIUS - h), COLOR_BLACK);
        tft.fillRect(0, CY + (EYE_RADIUS - h), TFT_WIDTH, TFT_HEIGHT - (CY + (EYE_RADIUS - h)), COLOR_BLACK);
        delay(20);
    }
    delay(BLINK_HOLD_TIME);
    for (int i = steps; i >= 0; i--) {
        int h = (EYE_RADIUS * i) / steps;
        drawBaseEye();
        tft.fillRect(0, 0, TFT_WIDTH, CY - (EYE_RADIUS - h), COLOR_BLACK);
        tft.fillRect(0, CY + (EYE_RADIUS - h), TFT_WIDTH, TFT_HEIGHT - (CY + (EYE_RADIUS - h)), COLOR_BLACK);
        drawPupil(pupilX, pupilY);
        delay(20);
    }
}

// =====================================================
// Web / Serial control
// =====================================================

void sendCORS() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
}

String jsonStatus() {
    String s = "{";
    s += "\"emotion\":" + String(emotion) + ",";
    s += "\"irisColor\":" + String(irisColor) + ",";
    s += "\"blinkStyle\":" + String(blinkStyle) + ",";
    s += "\"pupilRadius\":" + String(pupilRadius) + ",";
    s += "\"pupilX\":" + String(pupilX) + ",";
    s += "\"pupilY\":" + String(pupilY);
    s += "}";
    return s;
}

void handleStatus() {
    sendCORS();
    server.send(200, "application/json", jsonStatus());
}

void handleSetEmotion() {
    sendCORS();
    if (server.hasArg("v")) {
        int v = server.arg("v").toInt();
        if (v >= 0 && v <= 9) { emotion = v; server.send(200, "application/json", "{\"ok\":1}\" "); return; }
    }
    server.send(400, "application/json", "{\"ok\":0,\"err\":\"invalid\"}");
}

void handleSetIris() {
    sendCORS();
    if (server.hasArg("color")) {
        String c = server.arg("color");
        c.toLowerCase();
        if (c == "brown") irisColor = IRIS_BROWN;
        else if (c == "hazel") irisColor = IRIS_HAZEL;
        else if (c == "green") irisColor = IRIS_GREEN;
        else if (c == "blue") irisColor = IRIS_BLUE;
        else if (c == "gray") irisColor = IRIS_GRAY;
        else if (c == "red") irisColor = IRIS_RED;
        else { server.send(400, "application/json", "{\"ok\":0,\"err\":\"unknown color\"}"); return; }
        // redraw base with new iris color
        drawBaseEye();
        drawPupil(pupilX, pupilY);
        server.send(200, "application/json", "{\"ok\":1}");
        return;
    }
    server.send(400, "application/json", "{\"ok\":0,\"err\":\"no color\"}");
}

void handleGaze() {
    sendCORS();
    if (server.hasArg("x") && server.hasArg("y")) {
        int x = server.arg("x").toInt();
        int y = server.arg("y").toInt();
        movePupil(x, y);
        server.send(200, "application/json", "{\"ok\":1}\" ");
        return;
    }
    server.send(400, "application/json", "{\"ok\":0,\"err\":\"missing\"}");
}

void handleBlink() {
    sendCORS();
    blinkOnce(BLINK_SPEED_NORMAL);
    server.send(200, "application/json", "{\"ok\":1}");
}

void handleControlPage() {
    sendCORS();
    String html = "<html><head><meta name=viewport content=\"width=device-width,initial-scale=1\"></head><body>";
    html += "<h3>Eye Control</h3>";
    html += "<button onclick=\"fetch('/api/blink')\">Blink</button> ";
    html += "<button onclick=\"fetch('/api/setIris?color=blue')\">Iris Blue</button> ";
    html += "<button onclick=\"fetch('/api/setIris?color=brown')\">Iris Brown</button> ";
    html += "<br><br>Gaze: <button onclick=\"fetch('/api/gaze?x=60&y=120')\">Left</button> ";
    html += "<button onclick=\"fetch('/api/gaze?x=180&y=120')\">Right</button>";
    html += "<button onclick=\"fetch('/api/gaze?x=120&y=60')\">Up</button>";
    html += "<button onclick=\"fetch('/api/gaze?x=120&y=180')\">Down</button>";
    html += "<button onclick=\"fetch('/api/gaze?x=120&y=120')\">Center</button>";
    html += "</body></html>";
    server.send(200, "text/html", html);
}

void handleNotFound(){ sendCORS(); server.send(404, "text/plain", "Not found"); }

// Serial parsing: simple commands
void handleSerialLine(String line) {
    line.trim();
    if (line.length() == 0) return;
    // split
    int sp = line.indexOf(' ');
    String cmd = (sp < 0) ? line : line.substring(0, sp);
    String arg = (sp < 0) ? "" : line.substring(sp + 1);
    cmd.toLowerCase();
    if (cmd == "blink") {
        blinkOnce(BLINK_SPEED_NORMAL);
    } else if (cmd == "gaze") {
        // arg: x y
        int sp2 = arg.indexOf(' ');
        if (sp2 > 0) {
            int x = arg.substring(0, sp2).toInt();
            int y = arg.substring(sp2 + 1).toInt();
            movePupil(x, y);
        }
    } else if (cmd == "iris") {
        String c = arg;
        c.toLowerCase();
        if (c == "blue") irisColor = IRIS_BLUE;
        else if (c == "green") irisColor = IRIS_GREEN;
        else if (c == "brown") irisColor = IRIS_BROWN;
        drawBaseEye(); drawPupil(pupilX, pupilY);
    } else if (cmd == "pupil") {
        int s = arg.toInt();
        if (s >= MIN_PUPIL_SIZE && s <= MAX_PUPIL_SIZE) { pupilRadius = s; drawBaseEye(); drawPupil(pupilX, pupilY); }
    } else if (cmd == "status") {
        Serial.println(jsonStatus());
    }
}

String jsonStatus() {
    String s = "{";
    s += "\"emotion\":" + String(emotion) + ",";
    s += "\"irisColor\":" + String(irisColor) + ",";
    s += "\"blinkStyle\":" + String(blinkStyle) + ",";
    s += "\"pupilRadius\":" + String(pupilRadius) + ",";
    s += "\"pupilX\":" + String(pupilX) + ",";
    s += "\"pupilY\":" + String(pupilY);
    s += "}";
    return s;
}

void startWiFi()
{
    Serial.printf("Connecting to WiFi '%s'\n", WIFI_SSID);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - start) < WIFI_TIMEOUT) {
        delay(200);
        Serial.print('.');
    }
    Serial.println();
    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("WiFi connected, IP: %s\n", WiFi.localIP().toString().c_str());
    } else {
        Serial.println("WiFi failed to connect");
    }
}

void setup()
{
    Serial.begin(115200);
    tft.init();
    tft.setRotation(TFT_ROTATION);

    drawBaseEye();
    drawPupil(CX, CY);

    startWiFi();

    // routes
    server.on("/", HTTP_GET, handleControlPage);
    server.on("/api/status", HTTP_GET, handleStatus);
    server.on("/api/setEmotion", HTTP_GET, handleSetEmotion);
    server.on("/api/setIris", HTTP_GET, handleSetIris);
    server.on("/api/gaze", HTTP_GET, handleGaze);
    server.on("/api/blink", HTTP_GET, handleBlink);
    server.onNotFound(handleNotFound);

    server.begin();
    Serial.printf("HTTP server started on port %d\n", HTTP_PORT);
}

String serialBuf = "";

void loop()
{
    // handle web
    server.handleClient();

    // serial line reading
    while (Serial.available()) {
        char c = (char)Serial.read();
        if (c == '\n' || c == '\r') {
            if (serialBuf.length() > 0) {
                handleSerialLine(serialBuf);
                serialBuf = "";
            }
        } else serialBuf += c;
    }

    // occasional automatic blink if idle
    if (millis() - lastBlink > 5000 + (rand() % 6000)) {
        lastBlink = millis();
        blinkOnce(BLINK_SPEED_NORMAL);
    }

    delay(10);
}
