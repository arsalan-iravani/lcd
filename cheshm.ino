#include <TFT_eSPI.h>
#include <SPI.h>
#include "config.h"

TFT_eSPI tft = TFT_eSPI();

// Center
#define CX EYE_CENTER_X
#define CY EYE_CENTER_Y

// Runtime state
uint16_t irisColor = IRIS_BROWN;
int pupilX = CX;
int pupilY = CY;
int targetPupilX = CX;
int targetPupilY = CY;
int pupilRadius = PUPIL_RADIUS;

// Helpers
static inline int clamp(int v, int a, int b) { return v < a ? a : (v > b ? b : v); }

// Draw a radial gradient circle (approx) by drawing concentric circles
void drawRadialIris(int cx, int cy, int radius, uint16_t baseColor) {
  // base: darker outer ring
  int steps = 12;
  for (int i = 0; i < steps; i++) {
    float p = (float)i / (steps - 1); // 0..1
    // interpolate between a darker and lighter color (simple approach)
    uint8_t r = ((baseColor & 0xF800) >> 11);
    uint8_t g = ((baseColor & 0x07E0) >> 5);
    uint8_t b = (baseColor & 0x001F);

    // vary brightness
    float brightness = 0.6 + 0.4 * (1.0f - p);
    uint8_t rr = clamp((int)(r * brightness), 0, 31);
    uint8_t gg = clamp((int)(g * brightness), 0, 63);
    uint8_t bb = clamp((int)(b * brightness), 0, 31);

    uint16_t col = (rr << 11) | (gg << 5) | bb;
    int rstep = radius - (radius * i / steps);
    tft.fillCircle(cx, cy, rstep, col);
  }
}

// Draw iris fibers — radial lines from center to near edge
void drawIrisFibers(int cx, int cy, int radius, uint16_t baseColor, int count) {
  // Slightly darker/lighter variations
  for (int i = 0; i < count; i++) {
    float a = (float)i / count * 2.0 * PI;
    // jitter angle a tiny bit
    float ja = ((rand() % 100) - 50) / 1000.0f;
    float ang = a + ja;
    // fiber length
    int l1 = radius * 0.28; // start a bit away from center
    int l2 = radius - (rand() % (radius / 8));
    int x1 = cx + cos(ang) * l1;
    int y1 = cy + sin(ang) * l1;
    int x2 = cx + cos(ang) * l2;
    int y2 = cy + sin(ang) * l2;

    // color variation
    uint8_t r = ((baseColor & 0xF800) >> 11);
    uint8_t g = ((baseColor & 0x07E0) >> 5);
    uint8_t b = (baseColor & 0x001F);
    int dv = (rand() % 8) - 4; // -4..3
    uint8_t rr = clamp(r + dv, 0, 31);
    uint8_t gg = clamp(g + dv, 0, 63);
    uint8_t bb = clamp(b + dv, 0, 31);
    uint16_t col = (rr << 11) | (gg << 5) | bb;

    // thickness 1-2
    int thick = (rand() % 2) + 1;
    for (int w = 0; w < thick; w++) {
      tft.drawLine(x1 + w, y1 + w, x2 + w, y2 + w, col);
    }
  }
}

// Draw corneal highlight (simple glossy overlay)
void drawCorneaHighlight(int cx, int cy) {
  int hx = cx + CORNEA_HIGHLIGHT_OFFSET_X;
  int hy = cy + CORNEA_HIGHLIGHT_OFFSET_Y;
  tft.fillCircle(hx, hy, CORNEA_HIGHLIGHT_SIZE, COLOR_WHITE);
  // small mask to make it look elliptical
  tft.fillCircle(hx + 4, hy + 4, CORNEA_HIGHLIGHT_SIZE - 6, COLOR_BLACK);
}

// Draw subtle veins / metallic ridges around iris (since headlight eye likely stylized)
void drawScleraVeins(int cx, int cy, int radius) {
  for (int i = 0; i < SCLERA_VEIN_COUNT; i++) {
    float ang = ((float)rand() / RAND_MAX) * 2.0 * PI;
    int sx = cx + cos(ang) * (radius + 6 + rand() % 10);
    int sy = cy + sin(ang) * (radius + 6 + rand() % 10);
    int tx = cx + cos(ang) * (radius + 24 + rand() % 10);
    int ty = cy + sin(ang) * (radius + 24 + rand() % 10);
    // draw curvy small lines by segments
    int segments = 4;
    int px = sx, py = sy;
    for (int s = 0; s < segments; s++) {
      int nx = px + (tx - sx) * (s + 1) / segments + (rand() % 6) - 3;
      int ny = py + (ty - sy) * (s + 1) / segments + (rand() % 6) - 3;
      tft.drawLine(px, py, nx, ny, COLOR_DARK_RED);
      px = nx; py = ny;
    }
  }
}

// Draw the entire base eye (sclera + iris layers)
void drawBaseEye() {
  tft.fillScreen(COLOR_BLACK);

  // Outer ring / housings
  tft.fillCircle(CX, CY, EYE_RADIUS, RED_1);

  // Iris large layered
  drawRadialIris(CX, CY, IRIS_RADIUS, irisColor);

  // Inner decorative rings
  tft.drawCircle(CX, CY, IRIS_RADIUS + 6, COLOR_DARK_GRAY);
  tft.drawCircle(CX, CY, IRIS_RADIUS + 12, RED_3);

  // Iris fibers
  drawIrisFibers(CX, CY, IRIS_RADIUS - 6, irisColor, IRIS_FIBER_COUNT);

  // subtle veins/metallic details outside iris
  drawScleraVeins(CX, CY, IRIS_RADIUS + 6);

  // corneal highlight
  drawCorneaHighlight(CX, CY);

  // slight inner glow
  tft.fillCircle(CX, CY, IRIS_RADIUS / 3, RED_5);
}

void clearPupilArea(int x, int y) {
  // restore iris region under pupil by redrawing a small section of iris
  int r = pupilRadius + 6;
  // redraw using small clip: simply fill circle with mid-iris color then redraw fibers over it
  tft.fillCircle(x, y, r, RED_4);
  // re-draw fibers under area (cheap approach: draw a few short radial lines)
  for (int i = 0; i < 6; i++) {
    float a = ((float)rand() / RAND_MAX) * 2.0 * PI;
    int x1 = CX + cos(a) * (IRIS_RADIUS * 0.2);
    int y1 = CY + sin(a) * (IRIS_RADIUS * 0.2);
    int x2 = CX + cos(a) * (IRIS_RADIUS * 0.6);
    int y2 = CY + sin(a) * (IRIS_RADIUS * 0.6);
    tft.drawLine(x1, y1, x2, y2, RED_3);
  }
}

void drawPupil(int x, int y, int r) {
  // Pupil
  tft.fillCircle(x, y, r, COLOR_BLACK);
  // ring
  tft.drawCircle(x, y, r + 1, COLOR_DARK_GRAY);
  // bright catch
  tft.fillCircle(x - (r/2), y - (r/2), max(2, r/4), COLOR_WHITE);
}

// Smooth move pupil
void movePupilTo(int tx, int ty, int duration_ms) {
  int steps = max(6, duration_ms / 20);
  int ox = pupilX, oy = pupilY;
  for (int i = 1; i <= steps; i++) {
    float p = (float)i / steps;
    float smooth = p * p * (3.0 - 2.0 * p);
    int nx = ox + (tx - ox) * smooth;
    int ny = oy + (ty - oy) * smooth;
    clearPupilArea(pupilX, pupilY);
    pupilX = nx; pupilY = ny;
    drawPupil(pupilX, pupilY, pupilRadius);
    delay(20);
  }
}

// Blink animation: simple covering top and bottom eyelids toward center
void blinkOnce(int speed_ms) {
  int steps = max(4, speed_ms / 30);
  // top
  for (int i = 0; i <= steps; i++) {
    int h = (EYE_RADIUS * i) / steps;
    // top cover
    tft.fillRect(0, 0, TFT_WIDTH, CY - h, COLOR_BLACK);
    // bottom cover
    tft.fillRect(0, CY + h, TFT_WIDTH, TFT_HEIGHT - (CY + h), COLOR_BLACK);
    delay(20);
  }
  // hold closed
  delay(BLINK_HOLD_TIME);
  // open
  for (int i = steps; i >= 0; i--) {
    int h = (EYE_RADIUS * i) / steps;
    tft.fillRect(0, 0, TFT_WIDTH, CY - h, COLOR_BLACK);
    tft.fillRect(0, CY + h, TFT_WIDTH, TFT_HEIGHT - (CY + h), COLOR_BLACK);
    // redraw center region as it becomes visible
    drawBaseEye();
    drawPupil(pupilX, pupilY, pupilRadius);
    delay(20);
  }
}

void randomGazeRoutine() {
  // pick random target within constraints
  int gx = CX + (rand() % (MAX_GAZE_OFFSET_X * 2)) - MAX_GAZE_OFFSET_X;
  int gy = CY + (rand() % (MAX_GAZE_OFFSET_Y * 2)) - MAX_GAZE_OFFSET_Y;
  movePupilTo(gx, gy, GAZE_CHANGE_TIME);
}

void setup() {
  Serial.begin(115200);
  tft.init();
  tft.setRotation(TFT_ROTATION);
  randomSeed(analogRead(0));

  // initial draw
  drawBaseEye();
  drawPupil(CX, CY, pupilRadius);
}

unsigned long lastMicro = 0;
unsigned long lastGaze = 0;
unsigned long lastBlink = 0;

void loop() {
  unsigned long now = millis();

  // Every few seconds change gaze
  if (now - lastGaze > GAZE_HOLD_TIME) {
    lastGaze = now;
    // random move
    int gx = CX + (rand() % (MAX_GAZE_OFFSET_X * 2)) - MAX_GAZE_OFFSET_X;
    int gy = CY + (rand() % (MAX_GAZE_OFFSET_Y * 2)) - MAX_GAZE_OFFSET_Y;
    movePupilTo(gx, gy, GAZE_CHANGE_TIME);
  }

  // micro-movements
  if (now - lastMicro > MICRO_MOVEMENT_INTERVAL) {
    lastMicro = now;
    int mx = pupilX + (rand() % (MICRO_MOVEMENT_AMOUNT * 2)) - MICRO_MOVEMENT_AMOUNT;
    int my = pupilY + (rand() % (MICRO_MOVEMENT_AMOUNT * 2)) - MICRO_MOVEMENT_AMOUNT;
    mx = clamp(mx, CX - MAX_GAZE_OFFSET_X, CX + MAX_GAZE_OFFSET_X);
    my = clamp(my, CY - MAX_GAZE_OFFSET_Y, CY + MAX_GAZE_OFFSET_Y);
    movePupilTo(mx, my, 120);
  }

  // occasional blink
  if (now - lastBlink > 4000 + (rand() % 4000)) {
    lastBlink = now;
    blinkOnce(BLINK_SPEED_NORMAL);
  }

  // small idle delay to yield
  delay(10);
}
