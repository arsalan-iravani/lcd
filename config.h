#ifndef CONFIG_H
#define CONFIG_H

// ============================================================================
// DISPLAY & HARDWARE CONFIGURATION
// ============================================================================
#define TFT_WIDTH 240
#define TFT_HEIGHT 240
#define TFT_ROTATION 0

// TFT_eSPI Pin Configuration (keeping existing setup)
#define TFT_CS 15       // Chip Select
#define TFT_DC 4        // Data/Command
#define TFT_RST 2       // Reset
#define TFT_CLK 14      // Clock (SPI)
#define TFT_MOSI 13     // MOSI (SPI)
#define TFT_MISO 12     // MISO (SPI) - optional

// ============================================================================
// DISPLAY COLORS (16-bit RGB565)
// ============================================================================
#define COLOR_BLACK 0x0000
#define COLOR_WHITE 0xFFFF
#define COLOR_RED 0xF800
#define COLOR_GREEN 0x07E0
#define COLOR_BLUE 0x001F
#define COLOR_CYAN 0x07FF
#define COLOR_MAGENTA 0xF81F
#define COLOR_YELLOW 0xFFE0
#define COLOR_DARK_GRAY 0x2104
#define COLOR_LIGHT_GRAY 0xC618
#define COLOR_DARK_RED 0x8000
#define COLOR_SKIN 0xFBB4

// Eye Canvas
#define EYE_CENTER_X 120
#define EYE_CENTER_Y 120
#define SCLERA_RADIUS 95
#define IRIS_RADIUS 45
#define PUPIL_RADIUS 20
#define IRIS_MIN_RADIUS 35
#define IRIS_MAX_RADIUS 50

// ============================================================================
// ANIMATION TIMING (milliseconds)
// ============================================================================
#define BLINK_SPEED_NORMAL 120      // Close time
#define BLINK_SPEED_SLOW 200
#define BLINK_SPEED_FAST 80
#define BLINK_HOLD_TIME 50          // Eyes closed time
#define BLINK_DOUBLE_DELAY 150      // Time between double blink
#define BLINK_LONG_TIME 500         // Long blink duration

#define GAZE_HOLD_TIME 2000         // How long to look in one direction
#define GAZE_CHANGE_TIME 800        // Transition time between gazes
#define MICRO_MOVEMENT_INTERVAL 150 // Small random movements
#define MICRO_MOVEMENT_AMOUNT 3     // Pixels

#define PUPIL_DILATION_SPEED 50     // ms per size change
#define EYE_MOVEMENT_SPEED 100      // Base movement smoothing

// ============================================================================
// MOVEMENT CONSTRAINTS
// ============================================================================
#define MAX_GAZE_OFFSET_X 35        // Max horizontal offset from center
#define MAX_GAZE_OFFSET_Y 30        // Max vertical offset from center
#define MAX_PUPIL_SIZE 25
#define MIN_PUPIL_SIZE 15

// ============================================================================
// REALISTIC EYE PARAMETERS
// ============================================================================
#define SCLERA_VEIN_COUNT 8
#define SCLERA_VEIN_WIDTH 2
#define IRIS_FIBER_COUNT 12
#define CORNEA_HIGHLIGHT_SIZE 18
#define CORNEA_HIGHLIGHT_OFFSET_X -15
#define CORNEA_HIGHLIGHT_OFFSET_Y -15

// ============================================================================
// WIFI CONFIGURATION
// ============================================================================
#define WIFI_SSID "YOUR_SSID"           // Change this
#define WIFI_PASSWORD "YOUR_PASSWORD"   // Change this
#define WIFI_TIMEOUT 10000              // ms to wait for connection
#define HTTP_PORT 80
#define WEB_REFRESH_INTERVAL 100        // ms between web data updates

// ============================================================================
// STORAGE CONFIGURATION
// ============================================================================
#define EEPROM_SIZE 4096
#define SETTINGS_ADDR 0
#define SETTINGS_VERSION 1

// ============================================================================
// EMOTION TYPES
// ============================================================================
enum Emotion {
  EMO_NORMAL = 0,
  EMO_HAPPY = 1,
  EMO_SAD = 2,
  EMO_ANGRY = 3,
  EMO_SURPRISED = 4,
  EMO_SLEEPY = 5,
  EMO_CURIOUS = 6,
  EMO_SCARED = 7,
  EMO_LOVE = 8,
  EMO_ROBOT = 9
};

// ============================================================================
// IRIS COLOR PRESETS (RGB565)
// ============================================================================
enum IrisColor {
  IRIS_BROWN = 0x8440,      // Brown
  IRIS_HAZEL = 0xA640,      // Hazel
  IRIS_GREEN = 0x07C0,      // Green
  IRIS_BLUE = 0x019F,       // Blue
  IRIS_GRAY = 0x8410,       // Gray
  IRIS_RED = 0xF804,        // Red (cyborg/robot)
  IRIS_CUSTOM = 0xFFFF      // Custom RGB
};

// ============================================================================
// BLINK STYLES
// ============================================================================
enum BlinkStyle {
  BLINK_NORMAL = 0,
  BLINK_SLOW = 1,
  BLINK_FAST = 2,
  BLINK_DOUBLE = 3,
  BLINK_LONG = 4,
  BLINK_RANDOM = 5
};

// ============================================================================
// SETTINGS STRUCTURE (for EEPROM)
// ============================================================================
struct EyeSettings {
  uint8_t version;
  uint16_t irisColor;
  uint8_t emotion;
  uint8_t blinkStyle;
  uint8_t movementSpeed;      // 0-100
  uint8_t microMovementLevel; // 0-100
  bool randomGaze;
  bool realisticMode;
  uint8_t pupilSize;          // 0-100 (relative)
};

// ============================================================================
// PERFORMANCE TUNING
// ============================================================================
#define TARGET_FPS 25                   // Target frames per second
#define FRAME_TIME (1000 / TARGET_FPS)  // ~40ms per frame
#define MAX_DRAW_CALLS_PER_FRAME 50     // Limit updates per frame

// ============================================================================
// DEBUG OPTIONS
// ============================================================================
#define DEBUG_ENABLED 0
#if DEBUG_ENABLED
  #define DEBUG_PRINT(x) Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
  #define DEBUG_PRINTF(fmt, ...) Serial.printf(fmt, __VA_ARGS__)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINTF(fmt, ...)
#endif

#endif // CONFIG_H
