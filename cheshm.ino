#include <TFT_eSPI.h>
#include <SPI.h>
#include <math.h>

TFT_eSPI tft = TFT_eSPI();

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

// =====================================================
// موقعیت مردمک
// =====================================================

int pupilX = CX;
int pupilY = CY;

int oldPupilX = CX;
int oldPupilY = CY;


// =====================================================
// رسم پایه چشم
// =====================================================

void drawBaseEye()
{
    tft.fillScreen(BG);

    // دایره بیرونی
    tft.fillCircle(
        CX,
        CY,
        EYE_RADIUS,
        RED_1
    );

    // عنبیه بزرگ
    tft.fillCircle(
        CX,
        CY,
        IRIS_RADIUS,
        RED_2
    );

    // لایه دوم
    tft.fillCircle(
        CX,
        CY,
        96,
        RED_3
    );

    // لایه سوم
    tft.fillCircle(
        CX,
        CY,
        84,
        RED_4
    );

    // مرکز روشن
    tft.fillCircle(
        CX,
        CY,
        70,
        RED_5
    );

    // =================================================
    // فقط رگه‌های اطراف عنبیه
    // =================================================

    tft.drawCircle(
        CX,
        CY,
        103,
        DARK_RED
    );

    tft.drawCircle(
        CX,
        CY,
        106,
        RED_3
    );

    tft.drawCircle(
        CX,
        CY,
        108,
        DARK_RED
    );
}


// =====================================================
// بازسازی دقیق زیر مردمک (جلوگیری از اشکال در پاک‌سازی)
// این تابع فقط ناحیه‌ای هم‌اندازه مردمک و حاشیه را دوباره رسم می‌کند
// =====================================================

void redrawIrisUnderPupil(int x, int y)
{
    // شعاع بازسازی: کمی بزرگتر از مردمک
    int r = PUPIL_RADIUS + 10;

    // برای جلوگیری از مرزهای سخت، از چندین لایه حلقه استفاده می‌کنیم
    // هر لایه را با رنگ‌های عنبیه مطابق با drawBaseEye بازمی‌سازیم

    // پاک کردن ناحیه با رنگ عنبیه پایه
    tft.fillCircle(CX, CY, IRIS_RADIUS, RED_2);

    // بازسازی لایه‌های میانی که ممکن است زیر مردمک قرار گیرند
    tft.fillCircle(CX, CY, 96, RED_3);
    tft.fillCircle(CX, CY, 84, RED_4);
    tft.fillCircle(CX, CY, 70, RED_5);

    // نوارهای دور عنبیه
    tft.drawCircle(CX, CY, 103, DARK_RED);
    tft.drawCircle(CX, CY, 106, RED_3);
    tft.drawCircle(CX, CY, 108, DARK_RED);

    // اگر مردمک در لبه‌ی عنبیه باشد، ممکن است بخواهیم بخش‌هایی از حلقه بیرونی را هم بکشیم
    // برای اطمینان، یک ماسک گرد شکل در محل مردمک می‌کشیم تا جایگزینی کامل انجام شود
    tft.fillCircle(x, y, r, RED_4);

    // در صورت نیاز، برق‌های کوچک را دوباره رسم می‌کنیم (تا catch highlight حفظ شود)
    tft.fillCircle(
        CX - 10,
        CY - 10,
        6,
        WHITE
    );
    tft.fillCircle(
        CX + 9,
        CY + 10,
        2,
        WHITE
    );
}


// =====================================================
// پاک کردن محل قبلی مردمک
// =====================================================

void clearPupilArea(
    int x,
    int y
)
{
    // بازسازی زیر مردمک به جای پرکردن با رنگ ثابت
    redrawIrisUnderPupil(x, y);
}


// =====================================================
// رسم مردمک
// =====================================================

void drawPupil(
    int x,
    int y
)
{
    // مردمک
    tft.fillCircle(
        x,
        y,
        PUPIL_RADIUS,
        BLACK
    );

    // حلقه دور مردمک
    tft.drawCircle(
        x,
        y,
        PUPIL_RADIUS + 2,
        DARK_RED
    );

    // برق اصلی
    tft.fillCircle(
        x - 10,
        y - 10,
        6,
        WHITE
    );

    // برق کوچک
    tft.fillCircle(
        x + 9,
        y + 10,
        2,
        WHITE
    );
}


// =====================================================
// حرکت نرم مردمک
// =====================================================

void movePupil(
    int targetX,
    int targetY
)
{
    // محدود کردن هدف داخل عنبیه
    float dx = targetX - CX;
    float dy = targetY - CY;
    float dist = sqrt(dx*dx + dy*dy);
    float maxDist = IRIS_RADIUS - PUPIL_RADIUS - 4; // حاشیه کوچک

    if (dist > maxDist) {
        float s = maxDist / dist;
        targetX = CX + (int)(dx * s);
        targetY = CY + (int)(dy * s);
    }

    oldPupilX = pupilX;
    oldPupilY = pupilY;

    const int steps = 25;

    for (int i = 1; i <= steps; i++)
    {
        float p =
            (float)i / steps;

        float smooth =
            p * p * (3.0 - 2.0 * p);

        int newX =
            oldPupilX +
            (targetX - oldPupilX) * smooth;

        int newY =
            oldPupilY +
            (targetY - oldPupilY) * smooth;

        // پاک کردن موقعیت قبلی به صورت بازسازی دقیق زیر مردمک
        clearPupilArea(
            pupilX,
            pupilY
        );

        pupilX = newX;
        pupilY = newY;

        // رسم موقعیت جدید
        drawPupil(
            pupilX,
            pupilY
        );

        delay(18);
    }
}


// =====================================================
// Setup
// =====================================================

void setup()
{
    Serial.begin(115200);

    tft.init();

    tft.setRotation(0);

    drawBaseEye();

    drawPupil(
        CX,
        CY
    );
}


// =====================================================
// Loop
// =====================================================

void loop()
{
    // چپ
    movePupil(88, 120);
    delay(700);

    // راست
    movePupil(152, 120);
    delay(700);

    // بالا
    movePupil(120, 88);
    delay(700);

    // پایین
    movePupil(120, 152);
    delay(700);

    // مرکز
    movePupil(120, 120);
    delay(1200);
}
