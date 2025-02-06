#include <FastLED.h>

#define NEOPIXEL_PIN 32

#define MAX_BRIGHTNESS 70
#define ALARM_BRIGHTNESS_INCREMENT 30

#define DIGIT_DEGREE 10
#define DIGIT_C 11
#define DIGIT_H 12
#define DIGIT_u 13
#define DIGIT_HYPHEN 14
#define DIGIT_OFF 15

class SunriseSiren3000Lighting {
  private:
    static constexpr byte numbers[16][7] = {
      {7, 7, 7, 0, 7, 7, 7}, // 0
      {0, 0, 7, 0, 0, 0, 7}, // 1
      {7, 7, 0, 7, 0, 7, 7}, // 2
      {0, 7, 7, 7, 0, 7, 7}, // 3
      {0, 0, 7, 7, 7, 0, 7}, // 4
      {0, 7, 7, 7, 7, 7, 0}, // 5
      {7, 7, 7, 7, 7, 7, 0}, // 6
      {0, 0, 7, 0, 0, 7, 7}, // 7
      {7, 7, 7, 7, 7, 7, 7}, // 8
      {0, 7, 7, 7, 7, 7, 7}, // 9
      {0, 0, 0, 7, 7, 7, 7}, // ° (10)
      {7, 7, 0, 0, 7, 7, 0}, // C (11)
      {7, 0, 7, 7, 7, 0, 7}, // H (12)
      {7, 7, 7, 0, 0, 0 ,0}, // u (13)
      {0, 0, 0, 7, 0, 0, 0}, // - (14)
      {0, 0, 0, 0, 0, 0, 0} // off (15)
    };
    static constexpr int startPositions[] = {0, 21, 44, 65};
    static constexpr int colonPoints[] = {42, 43};

    CRGB leds[86];

  public:
    CRGB defaultColor;
    CRGB alarmColor;

    void init() {
      FastLED.addLeds<WS2811, NEOPIXEL_PIN, GRB>(this->leds, 86).setCorrection(TypicalLEDStrip);
      FastLED.show();
    }

    void showTime(String time, CRGB color, bool leadingZero = true) {
      for (int i=0; i<4; i++) {
        int output = (time.equals("9999")) ? DIGIT_HYPHEN : String(time[i]).toInt(); // replace disabled alarms in preview with --:--
        if (i == 0 && output == 0 && !leadingZero) output = DIGIT_OFF; // remove leading zero if configured so

        this->showSingleDigit(i, output, color);
      }
    }

    void showNumber(int number, CRGB color) {
      int actualNumber = constrain(number, 0, 9999);
      int digits[] = {
        actualNumber / 1000 % 10,
        actualNumber / 100 % 10,
        actualNumber / 10 % 10,
        actualNumber % 10
      };
      for (int i=0; i<4; i++) this->showSingleDigit(i, digits[i], color);
    }

    void showSingleDigit(int position, int number, CRGB color) {
      byte segmentList[7];
      for (int i=0; i<7; i++) segmentList[i] = this->numbers[number][i];

      for (int i=0; i<7; i++) { // i = one out of seven segments per digit
        int start = startPositions[position] + i * 3;

        for (int j=0; j<3; j++) { // j = one out of three pixels per segment
          byte pixelBinary = pow(2, j);
          byte arrayBinary = segmentList[i];
          this->leds[start + j] = ((pixelBinary & arrayBinary) > 0) ? color : CRGB::Black;
        }
      }
    }

    void showCustomDigit(int position, int customValue, CRGB color) {
      for (int i=0; i<21; i++) {
        bool pixelEnabled = (customValue & (int) pow(2, i)) > 0;
        this->leds[startPositions[position] + i] = pixelEnabled ? color : CRGB::Black;
      }
    }

    void setColonPoint(CRGB color) {
      for (int i=0; i<2; i++) this->leds[colonPoints[i]] = color;
    }

    void update(uint8_t brightness) {
      FastLED.setBrightness(brightness);
      FastLED.show();
    }

    void error() {
      FastLED.showColor(CRGB::Red);
    }

};