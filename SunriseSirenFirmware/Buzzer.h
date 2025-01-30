#define BUZZER_PIN 18

class SunriseSiren3000Buzzer {
  private:
    bool lastEnabled;

  public:
    bool enabled;

    void init() {
      pinMode(BUZZER_PIN, OUTPUT);
    }

    void update() {
      if (lastEnabled != enabled) digitalWrite(BUZZER_PIN, enabled);

      lastEnabled = enabled;
    }
};
