# LCD Eye (headlight display)

This repository contains the Arduino/ESP32 sketch and supporting files for a 240×240 eye display intended to be installed inside a car headlight. The project provides a realistic/premium iris rendering, pupil animation, blink, and a web/serial control API with a Wi‑Fi captive portal for initial configuration.

Files of interest
- cheshm.ino — main sketch (ESP32, TFT_eSPI) with drawing, web API, captive portal, and serial control.
- config.h — configuration constants (display sizes, colors, timings, pins, Wi‑Fi defaults).
- platformio.ini — PlatformIO project file (if using PlatformIO).
- test_api.sh — helper script to exercise HTTP API endpoints.
- test_serial.py — helper script to send serial test commands to the board.
- TEST_PLAN.md — step-by-step test cases and acceptance criteria.

Required libraries
- TFT_eSPI (Bodmer) — for TFT display
- EEPROM (built‑in) — for storing Wi‑Fi credentials
- WebServer (built‑in for ESP32 Arduino core)

Build & upload
Option A — Arduino IDE
1. Install TFT_eSPI library (use Library Manager). Configure User_Setup.h in the TFT_eSPI library or edit the project to match your display pins (config.h pins must match the library setup).
2. Open cheshm.ino in Arduino IDE.
3. Select your ESP32 board and the proper COM port.
4. Compile and upload.

Option B — PlatformIO
1. Install PlatformIO in VSCode.
2. Place this repository in a PlatformIO project workspace (platformio.ini present).
3. Build and Upload using the PlatformIO toolbar.

Hardware wiring (typical for SPI TFT)
- TFT_CS  -> pin defined in config.h (default 15)
- TFT_DC  -> pin defined in config.h (default 4)
- TFT_RST -> pin defined in config.h (default 2)
- CLK (SCK) -> pin defined (default 14)
- MOSI -> pin defined (default 13)
- MISO -> optional (default 12)
- 3.3V and GND to display power. Use a stable supply; automotive environments may require voltage regulation and filtering.

How the Wi‑Fi config flow works
- The sketch tries saved credentials in EEPROM first, then compile‑time credentials from config.h.
- If both fail, it starts a SoftAP and serves a simple configuration page at 192.168.4.1 (AP mode). Enter your network SSID and password and the device saves them to EEPROM and restarts.

HTTP API (for testing)
- GET /                 — control page (buttons)
- GET /api/status       — JSON status
- GET /api/setIris?color=<brown|hazel|green|blue|gray|red>
- GET /api/gaze?x=<0-239>&y=<0-239>
- GET /api/blink
- GET /api/setEmotion?v=<0..9>

Testing quickly
1. Upload sketch and open serial monitor at 115200.
2. If device connects to Wi‑Fi, note IP printed in serial. If not, connect to the SoftAP and configure network.
3. Run the API tests (replace <IP>):
   - curl http://<IP>/api/status
   - curl "http://<IP>/api/setIris?color=blue"
   - curl "http://<IP>/api/gaze?x=60&y=120"
   - curl "http://<IP>/api/blink"

Acceptance criteria
- Eye is rendered on TFT at boot.
- Pupil moves smoothly and stops inside iris boundary.
- Catchlights move with the pupil.
- Blink covers and re-opens eye smoothly.
- Web API responds and commands (gaze, iris change, blink) take effect.

If you want, I can:
- Integrate sprite-based iris rendering for better perf and photoreal assets
- Add OTA or SPIFFS image loading for high-quality iris PNGs
- Harden the captive portal with a DNS redirect / captive-portal UX

