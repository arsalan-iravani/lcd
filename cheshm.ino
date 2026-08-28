#include <TFT_eSPI.h>
#include <SPI.h>
#include "config.h"

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite irisSprite = TFT_eSprite(&tft);

// Center
#define CX EYE_CENTER_X
#define CY EYE_CENTER_Y

// Runtime state
uint16_t irisColor = IRIS_BROWN;
int pupilX = CX;
int pupilY = CY;
int pupilRadius = PUPIL_RADIUS;

// Helpers
static inline int clampInt(int v, int a, int b) { return v < a ? a : (v > b ? b : v); }

// Draw a radial gradient into a sprite
void renderIrisToSprite(TFT_eSprite &s, int radius, uint16_t baseColor) {
  int w = radius * 2;
  int h = radius * 2;
  s.fillSprite(COLOR_BLACK); // clear

  // precompute base RGB components (5/6/5)
  int br = (baseColor & 0xF800) >> 11; // 0..31
  int bg = (baseColor & 0x07E0) >> 5;  // 0..63
  int bb = (baseColor & 0x001F);       // 0..31

  int steps = 18;
  for (int i = 0; i < steps; i++) {
    float p = (float)i / (steps - 1); // 0..1
    float brightness = 0.45f + 0.55f * (1.0f - p);
    uint8_t rr = clampInt((int)(br * brightness), 0, 31);
    uint8_t gg = clampInt((int)(bg * brightness), 0, 63);
    uint8_t bbv = clampInt((int)(bb * brightness), 0, 31);
    uint16_t col = (rr << 11) | (gg << 5) | bbv;
    int rstep = radius - (radius * i / steps);
    s.fillCircle(radius, radius, rstep, col);
  }

  // draw fibers
  for (int i = 0; i < IRIS_FIBER_COUNT * 2; i++) {
    float ang = ((float)i / (IRIS_FIBER_COUNT * 2)) * 2.0f * PI;
    float ja = ((rand() % 200) - 100) / 2000.0f; // small jitter
    float a = ang + ja;
    int l1 = radius * 0.18;
    int l2 = radius - (rand() % (radius / 6));
    int x1 = radius + cos(a) * l1;
    int y1 = radius + sin(a) * l1;
    int x2 = radius + cos(a) * l2;
    int y2 = radius + sin(a) * l2;
    int dv = (rand() % 10) - 5;
    uint8_t rr = clampInt(br + dv, 0, 31);
    uint8_t gg = clampInt(bg + dv * 2, 0, 63);
    uint8_t bbv = clampInt(bb + dv, 0, 31);
    uint16_t col = (rr << 11) | (gg << 5) | bbv;
    int thick = (rand() % 2) + 1;
    for (int w = 0; w < thick; w++) {
      s.drawLine(x1 + w, y1 + w, x2 + w, y2 + w, col);
    }
  }

  // small inner glow
  s.fillCircle(radius, radius, radius / 4, RED_5);

  // corneal highlight (white ellipse-like)
  int hx = radius + CORNEA_HIGHLIGHT_OFFSET_X / 2;
  int hy = radius + CORNEA_HIGHLIGHT_OFFSET_Y / 2;
  s.fillCircle(hx, hy, CORNEA_HIGHLIGHT_SIZE, COLOR_WHITE);
  s.fillCircle(hx + 4, hy + 4, max(1, CORNEA_HIGHLIGHT_SIZE - 6), COLOR_BLACK);
}

// Build iris sprite at startup
void buildIrisSprite() {
  int size = IRIS_RADIUS * 2 + 4; // padding
  irisSprite.createSprite(size, size);
  renderIrisToSprite(irisSprite, IRIS_RADIUS + 2, irisColor);
}

void drawFrame() {
  // blit iris sprite centered
  int sW = irisSprite.width();
  int sH = irisSprite.height();
  irisSprite.pushSprite(CX - sW / 2, CY - sH / 2);

  // draw pupil on top
  tft.fillCircle(pupilX, pupilY, pupilRadius, COLOR_BLACK);
  tft.drawCircle(pupilX, pupilY, pupilRadius + 1, COLOR_DARK_GRAY);
  // catchlights
  tft.fillCircle(pupilX - (pupilRadius / 2), pupilY - (pupilRadius / 2), max(2, pupilRadius / 4), COLOR_WHITE);
}

// Smooth move pupil
void movePupilTo(int tx, int ty, int duration_ms) {
  int steps = max(4, duration_ms / 20);
  int ox = pupilX, oy = pupilY;
  for (int i = 1; i <= steps; i++) {
    float p = (float)i / steps;
    float smooth = p * p * (3.0f - 2.0f * p);
    int nx = ox + (int)((tx - ox) * smooth);
    int ny = oy + (int)((ty - oy) * smooth);
    pupilX = nx; pupilY = ny;
    drawFrame();
    delay(20);
  }
}

// Blink using simple covers; when closed, draw a thin specular line to simulate lash seam
void blinkOnce(int speed_ms) {
  int steps = max(3, speed_ms / 25);
  for (int i = 0; i <= steps; i++) {
    int h = (EYE_RADIUS * i) / steps;
    // redraw base and top/bottom covers
    irisSprite.pushSprite(CX - irisSprite.width() / 2, CY - irisSprite.height() / 2);
    tft.fillRect(0, 0, TFT_WIDTH, CY - (EYE_RADIUS - h), COLOR_BLACK);
    tft.fillRect(0, CY + (EYE_RADIUS - h), TFT_WIDTH, TFT_HEIGHT - (CY + (EYE_RADIUS - h)), COLOR_BLACK);
    delay(20);
  }
  delay(BLINK_HOLD_TIME);
  // open
  for (int i = steps; i >= 0; i--) {
    int h = (EYE_RADIUS * i) / steps;
    irisSprite.pushSprite(CX - irisSprite.width() / 2, CY - irisSprite.height() / 2);
    tft.fillRect(0, 0, TFT_WIDTH, CY - (EYE_RADIUS - h), COLOR_BLACK);
    tft.fillRect(0, CY + (EYE_RADIUS - h), TFT_WIDTH, TFT_HEIGHT - (CY + (EYE_RADIUS - h)), COLOR_BLACK);
    drawFrame();
    delay(20);
  }
}

unsigned long lastMicro = 0;
unsigned long lastGaze = 0;
unsigned long lastBlink = 0;

void setup() {
  Serial.begin(115200);
  tft.init();
  tft.setRotation(TFT_ROTATION);
  randomSeed(analogRead(0));

  // create iris sprite once
  buildIrisSprite();

  // initial frame
  pupilX = CX;
  pupilY = CY;
  drawFrame();
}

void loop() {
  unsigned long now = millis();

  // periodic gaze change
  if (now - lastGaze > GAZE_HOLD_TIME) {
    lastGaze = now;
    int gx = CX + (rand() % (MAX_GAZE_OFFSET_X * 2)) - MAX_GAZE_OFFSET_X;
    int gy = CY + (rand() % (MAX_GAZE_OFFSET_Y * 2)) - MAX_GAZE_OFFSET_Y;
    movePupilTo(gx, gy, GAZE_CHANGE_TIME);
  }

  // micro movements
  if (now - lastMicro > MICRO_MOVEMENT_INTERVAL) {
    lastMicro = now;
    int mx = pupilX + (rand() % (MICRO_MOVEMENT_AMOUNT * 2)) - MICRO_MOVEMENT_AMOUNT;
    int my = pupilY + (rand() % (MICRO_MOVEMENT_AMOUNT * 2)) - MICRO_MOVEMENT_AMOUNT;
    mx = clampInt(mx, CX - MAX_GAZE_OFFSET_X, CX + MAX_GAZE_OFFSET_X);
    my = clampInt(my, CY - MAX_GAZE_OFFSET_Y, CY + MAX_GAZE_OFFSET_Y);
    movePupilTo(mx, my, 100);
  }

  // occasional blink
  if (now - lastBlink > 3000 + (rand() % 4000)) {
    lastBlink = now;
    blinkOnce(BLINK_SPEED_NORMAL);
  }

  // small idle repaint to ensure catchlights remain crisp
  drawFrame();
  delay(20);
}
